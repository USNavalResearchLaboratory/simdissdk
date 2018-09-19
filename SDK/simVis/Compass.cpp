/* -*- mode: c++ -*- */
/****************************************************************************
 *****                                                                  *****
 *****                   Classification: UNCLASSIFIED                   *****
 *****                    Classified By:                                *****
 *****                    Declassify On:                                *****
 *****                                                                  *****
 ****************************************************************************
 *
 *
 * Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div.
 *               EW Modeling & Simulation, Code 5773
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <osgEarthUtil/Controls>
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simVis/osgEarthVersion.h"
#include "simVis/Registry.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/View.h"
#include "simVis/Compass.h"

namespace simVis
{

/**
 * Callback functor for view focus changes
 */
class CompassFocusManagerAdapter::FocusCallback : public simVis::FocusManager::Callback
{
public:
  /** Constructor */
  explicit FocusCallback(CompassFocusManagerAdapter* adapter) : adapter_(adapter)
  {}
  /** Changes the compass to start monitoring a new view when view focus changes */
  virtual void operator()(simVis::View *view, const simVis::FocusManager::Callback::EventType &e)
  {
    // if a focus event, update our state
    if (e == simVis::FocusManager::Callback::VIEW_FOCUSED)
    {
      adapter_->focusView(view);
    }
  }
protected:
  virtual ~FocusCallback(){}
private:
  CompassFocusManagerAdapter* adapter_;
};

/**
 * Adapter class that allows compass to switch its display to reflect newly focused view
 */
CompassFocusManagerAdapter::CompassFocusManagerAdapter(simVis::FocusManager* focusManager, simVis::Compass* compass) :
focusManager_(focusManager),
compass_(compass),
callback_(NULL)
{
  callback_ = new FocusCallback(this);
  focusManager_->addCallback(callback_.get());
}

CompassFocusManagerAdapter::~CompassFocusManagerAdapter()
{
  if (focusManager_.valid())
  {
    focusManager_.get()->removeCallback(callback_.get());
  }
}

void CompassFocusManagerAdapter::focusView(simVis::View* focusedView)
{
  if (compass_.valid())
  {
    compass_.get()->setActiveView(focusedView);
  }
}

/**
 * Callback handler for frame updates, for viewpoint/heading changes
 */
class Compass::FrameEventHandler : public osgGA::GUIEventHandler
{
public:
  /** Constructs a new FrameEventHandler */
  explicit FrameEventHandler(simVis::Compass* compass) : compass_(compass)
  {}
  /** Handles frame updates and returns false so other handlers can process as well */
  bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
  {
    if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
      compass_->update_();

    return false;
  }
  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }
  /** Return the class name */
  virtual const char* className() const { return "Compass::FrameEventHandler"; }

protected:
  virtual ~FrameEventHandler(){}
private:
  simVis::Compass* compass_;
};

Compass::Compass(const std::string& compassFilename) :
height_(0),
drawView_(NULL),
activeView_(NULL),
compass_(NULL),
readout_(NULL),
pointer_(NULL),
compassUpdateEventHandler_(NULL)
{
  osg::ref_ptr<osg::Image> image;
  if (!compassFilename.empty())
    image = osgDB::readImageFile(compassFilename);

  if (image.valid())
  {
    height_ = image->t();
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,9,0) && defined(OSG_GL3_AVAILABLE)
    // Use the swizzle ImageControl(Texture) constructor, if available, to avoid GL3
    // errors with the texture being black due to not supporting GL_LUMINANCE*

    // Create a texture so we can set the swizzle
    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(image.get());
    texture->setResizeNonPowerOfTwoHint(false);
    texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    simVis::fixTextureForGlCoreProfile(texture.get());
    compass_ = new osgEarth::Util::Controls::ImageControl(texture.get());
#else
    // Fall back to ImageControl(Image) constructor, which fails in GL3 on some textures
    compass_ = new osgEarth::Util::Controls::ImageControl(image.get());
#endif

    compass_->setAbsorbEvents(false);
    compass_->setHorizAlign(osgEarth::Util::Controls::Control::ALIGN_RIGHT);
    compass_->setVertAlign(osgEarth::Util::Controls::Control::ALIGN_BOTTOM);
    compass_->setFixSizeForRotation(true);
    compass_->setName("Compass Image");

    // get the compass size to place text properly
    const float compassSize = static_cast<float>(image->t());
    // font size will be 12% size of the total image
    const int fontSize = static_cast<int>(image->t() * 0.12);

    // using default font and color
    readout_ = new osgEarth::Util::Controls::LabelControl("0.0", static_cast<float>(fontSize));
    readout_->setFont(simVis::Registry::instance()->getOrCreateFont("arial.ttf"));
    readout_->setAbsorbEvents(false);
    readout_->setHorizAlign(osgEarth::Util::Controls::Control::ALIGN_RIGHT);
    readout_->setVertAlign(osgEarth::Util::Controls::Control::ALIGN_BOTTOM);
    // set the text to appear in the upper middle of the compass image, 79% up, 53% across
    readout_->setPadding(osgEarth::Util::Controls::Control::SIDE_BOTTOM, compassSize * 0.79);
    readout_->setPadding(osgEarth::Util::Controls::Control::SIDE_RIGHT, compassSize * 0.53);
    readout_->setHaloColor(osgEarth::Symbology::Color::Black);
    readout_->setName("Compass Readout");

    // pointer is a text character, using default font
    pointer_ = new osgEarth::Util::Controls::LabelControl("|", simVis::Color::Red, static_cast<float>(fontSize));
    pointer_->setFont(simVis::Registry::instance()->getOrCreateFont("arial.ttf"));
    pointer_->setAbsorbEvents(false);
    pointer_->setHorizAlign(osgEarth::Util::Controls::Control::ALIGN_RIGHT);
    pointer_->setVertAlign(osgEarth::Util::Controls::Control::ALIGN_BOTTOM);
    // set the pointer to appear near the top middle of the compass image, 99% up, 69% across
    pointer_->setPadding(osgEarth::Util::Controls::Control::SIDE_BOTTOM, compassSize * 0.99);
    pointer_->setPadding(osgEarth::Util::Controls::Control::SIDE_RIGHT, compassSize * 0.685);
    pointer_->setName("Compass Pointer");

    compassUpdateEventHandler_ = new FrameEventHandler(this);
  }
}

Compass::~Compass()
{
  if (drawView_.valid())
  {
    drawView_.get()->removeEventHandler(compassUpdateEventHandler_);
  }
}

void Compass::setListener(simVis::CompassUpdateListenerPtr listener)
{
  compassUpdateListener_ = listener;
}

void Compass::removeListener(const simVis::CompassUpdateListenerPtr& listener)
{
  if (compassUpdateListener_ == listener)
    compassUpdateListener_.reset();
}

void Compass::setDrawView(simVis::View* drawView)
{
  if (drawView == NULL)
  {
    removeFromView();
    return;
  }
  assert(!drawView_.valid());
  if (compass_ && drawView && !drawView_.valid())
  {
    drawView_ = drawView;
    drawView->addOverlayControl(compass_.get());
    drawView->addOverlayControl(readout_.get());
    drawView->addOverlayControl(pointer_.get());

    // set up the callback for frame updates
    drawView->addEventHandler(compassUpdateEventHandler_);
  }
}

simVis::View* Compass::drawView() const
{
  return drawView_.get();
}

void Compass::removeFromView()
{
  if (compass_ && drawView_.valid())
  {
    drawView_.get()->removeOverlayControl(compass_.get());
    drawView_.get()->removeOverlayControl(readout_.get());
    drawView_.get()->removeOverlayControl(pointer_.get());

    // stop callbacks for frame updates
    drawView_.get()->removeEventHandler(compassUpdateEventHandler_.get());
    drawView_ = NULL;
  }
}

void Compass::setActiveView(simVis::View* activeView)
{
  activeView_ = activeView;
}

int Compass::size() const
{
  return height_;
}

void Compass::update_()
{
  const double TWO_DECIMAL_PLACES = 1e-02;
  if (!drawView_.valid())
  {
    return;
  }
  // if activeView not already set, or if it went away, set the active view to the draw view
  if (!activeView_.valid())
  {
    activeView_ = drawView_.get();
  }

  double heading = 0.0; // degrees
  if (!activeView_->isOverheadEnabled())
  {
    // Figure out the camera heading; use EarthManipulator to account for tether mode rotations
    const osgEarth::Util::EarthManipulator* manip = dynamic_cast<const osgEarth::Util::EarthManipulator*>(activeView_->getCameraManipulator());
    if (manip != NULL)
    {
      manip->getCompositeEulerAngles(&heading);
      // Convert to degrees
      heading = simCore::angFix360(heading * simCore::RAD2DEG);
    }
    else
    {
      // Fall back to the viewpoint's heading
      heading = simCore::angFix360(activeView_->getViewpoint().heading()->as(Units::DEGREES));
    }

    // make sure that anything equivalent to 0.00 is displayed as 0.00
    if (simCore::areEqual(heading, 0.0, TWO_DECIMAL_PLACES) || simCore::areEqual(heading, 360.0, TWO_DECIMAL_PLACES))
      heading = 0.0;
  }

  // check to see if this is a change of heading; note that compass rotation is -heading
  if (compass_ && !simCore::areEqual(compass_->getRotation().as(osgEarth::Units::DEGREES), -heading, TWO_DECIMAL_PLACES))
  {
    compass_->setRotation(-heading);
    // test to make sure -that a negative rotation is not converted to a positive 360+rotation
    assert(simCore::areEqual(compass_->getRotation().as(osgEarth::Units::DEGREES), -heading));

    if (readout_ && readout_.valid())
    {
      std::stringstream str;
      str << std::fixed << std::setprecision(2) << heading;
      std::string dirStr = str.str();
      if (dirStr.empty())
        return;
      if (readout_->text() != dirStr)
        readout_->setText(dirStr);
    }

    // if we have a listener, notify that we have updated
    if (compassUpdateListener_)
      compassUpdateListener_->onUpdate(heading);
  }
}

} // namespace simVis
