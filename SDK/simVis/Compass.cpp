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
#include "osgEarth/NodeUtils"
#include "osgEarthAnnotation/AnnotationUtils"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simVis/Registry.h"
#include "simVis/Utils.h"
#include "simVis/View.h"
#include "simVis/Compass.h"

namespace simVis
{

/** Expected size of the compass in pixels. */
static const double COMPASS_SIZE = 128.0;

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
CompassFocusManagerAdapter::CompassFocusManagerAdapter(simVis::FocusManager* focusManager, simVis::CompassNode* compass)
 : focusManager_(focusManager),
   compass_(compass),
   callback_(NULL)
{
  callback_ = new FocusCallback(this);
  focusManager_->addCallback(callback_.get());
}

CompassFocusManagerAdapter::~CompassFocusManagerAdapter()
{
  if (focusManager_.valid())
    focusManager_->removeCallback(callback_.get());
}

void CompassFocusManagerAdapter::focusView(simVis::View* focusedView)
{
  if (compass_.valid())
    compass_->setActiveView(focusedView);
}

///////////////////////////////////////////

CompassNode::CompassNode(const std::string& compassFilename)
  : valueText_(new osgText::Text),
    lastHeadingDeg_(0.0)
{
  osg::ref_ptr<osg::Image> image;
  if (!compassFilename.empty())
    image = osgDB::readImageFile(compassFilename);

  valueText_->setName("Compass Value Readout");
  valueText_->setText("0.00");
  valueText_->setCharacterSize(simVis::osgFontSize(11.f));
  valueText_->setFont(simVis::Registry::instance()->getOrCreateFont("arial.ttf"));
  valueText_->setAlignment(osgText::TextBase::RIGHT_BASE_LINE);
  valueText_->setAxisAlignment(osgText::TextBase::SCREEN);
  valueText_->setBackdropColor(osgEarth::Symbology::Color::Black);
  valueText_->setDataVariance(osg::Object::DYNAMIC);
  valueText_->setPosition(osg::Vec3f(19.f, 13.f, 0.f));
  addChild(valueText_);

  compassImageXform_ = new osg::MatrixTransform;
  addChild(compassImageXform_);
  if (image.valid())
  {
    osg::Geometry* compass = osgEarth::Annotation::AnnotationUtils::createImageGeometry(
      image.get(),
      osg::Vec2s(0, 0),            // pixel offsets from center
      0,                           // texture image unit
      0.0,                         // heading
      image->s() / COMPASS_SIZE);  // scale, down to 128x128
                            
    // Texture is likely GL_LUMINANCE or GL_LUMINANCE_ALPHA; fix it if so
    if (compass && compass->getStateSet())
    {
      osg::Texture* texture = dynamic_cast<osg::Texture*>(compass->getStateSet()->getTextureAttribute(0, osg::StateAttribute::TEXTURE));
      if (texture)
        simVis::fixTextureForGlCoreProfile(texture);
    }
    compassImageXform_->addChild(compass);
  }

  // Move the compass image back slightly so it doesn't occlude text
  compassImageXform_->setMatrix(osg::Matrix::translate(osg::Vec3f(0.f, 0.f, -0.01f)));

  {
    // Add a red line (tristrip) to indicate the pointing angle
    osg::Geometry* pointer = new osg::Geometry;
    osg::Vec3Array* points = new osg::Vec3Array;
    points->push_back(osg::Vec3(-0.5f, 39.f, 0.f));
    points->push_back(osg::Vec3(0.5f, 39.f, 0.f));
    points->push_back(osg::Vec3(-0.5f, 52.f, 0.f));
    points->push_back(osg::Vec3(0.5f, 52.f, 0.f));
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osgEarth::Symbology::Color::Red);
    pointer->setVertexArray(points);
    pointer->setColorArray(colors, osg::Array::BIND_OVERALL);
    pointer->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    addChild(pointer);
  }

  getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
  osgEarth::Registry::shaderGenerator().run(this);
  // We want to use our traverse() method to update the compass direction
  setNumChildrenRequiringUpdateTraversal(1);
}

CompassNode::~CompassNode()
{
}

void CompassNode::setActiveView(simVis::View* activeView)
{
  if (activeView_.get() == activeView)
    return;
  activeView_ = activeView;
}

simVis::View* CompassNode::activeView() const
{
  return activeView_.get();
}

void CompassNode::traverse(osg::NodeVisitor& nv)
{
  if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
    updateCompass_();
  osg::MatrixTransform::traverse(nv);
}

void CompassNode::updateCompass_()
{
  // Need a view to be able to update
  osg::ref_ptr<simVis::View> view;
  if (!activeView_.lock(view))
    return;

  const double TWO_DECIMAL_PLACES = 1e-02;
  double headingDeg = 0.0; // degrees

  // Overhead mode is always at true north, 0.0 degrees
  if (!view->isOverheadEnabled())
  {
    // Figure out the camera heading; use EarthManipulator to account for tether mode rotations
    const osgEarth::Util::EarthManipulator* manip = dynamic_cast<const osgEarth::Util::EarthManipulator*>(view->getCameraManipulator());
    if (manip != NULL)
    {
      manip->getCompositeEulerAngles(&headingDeg);
      // Convert to degrees
      headingDeg = simCore::angFix360(headingDeg * simCore::RAD2DEG);
    }
    else
    {
      // Fall back to the viewpoint's heading
      headingDeg = simCore::angFix360(view->getViewpoint().heading()->as(Units::DEGREES));
    }

    // make sure that anything equivalent to 0.00 is displayed as 0.00
    if (simCore::areEqual(headingDeg, 0.0, TWO_DECIMAL_PLACES) || simCore::areEqual(headingDeg, 360.0, TWO_DECIMAL_PLACES))
      headingDeg = 0.0;
  }

  // If we match the old heading value, return now
  if (simCore::areEqual(lastHeadingDeg_, headingDeg, TWO_DECIMAL_PLACES))
    return;
  lastHeadingDeg_ = headingDeg;

  // Rotate the compass
  osg::Matrix m;
  m.postMultRotate(osg::Quat(osg::inDegrees(headingDeg), osg::Vec3d(0.0, 0.0, 1.0)));
  m.postMultTranslate(osg::Vec3f(0.f, 0.f, -0.01f));
  compassImageXform_->setMatrix(m);

  // Update the read-out
  std::stringstream str;
  str << std::fixed << std::setprecision(2) << headingDeg;
  valueText_->setText(str.str());

  // if we have a listener, notify that we have updated
  if (compassUpdateListener_)
    compassUpdateListener_->onUpdate(headingDeg);
}

void CompassNode::setListener(simVis::CompassUpdateListenerPtr listener)
{
  compassUpdateListener_ = listener;
}

void CompassNode::removeListener(const simVis::CompassUpdateListenerPtr& listener)
{
  if (compassUpdateListener_ == listener)
    compassUpdateListener_.reset();
}

int CompassNode::size() const
{
  // Image is fixed in size at 128x128
  return static_cast<int>(COMPASS_SIZE);
}

///////////////////////////////////////////

/** Responsible for detecting screen size changes and repositioning the widget */
class Compass::RepositionEventHandler : public osgGA::GUIEventHandler
{
public:
  explicit RepositionEventHandler(Compass* parent)
    : compass_(parent)
  {
  }

  /** Handle frame updates, searching for viewport size changes for repositioning */
  virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
  {
    if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
    {
      const osg::View* view = aa.asView();
      const osg::Camera* camera = (view ? view->getCamera() : NULL);
      const osg::Viewport* viewport = (camera ? camera->getViewport() : NULL);
      if (viewport)
      {
        osg::Vec2d newWh(viewport->width(), viewport->height());
        if (newWh != oldWh_)
        {
          oldWh_ = newWh;
          // New position has bottom-right being 25,25 away from lower-right corner.
          // Since the anchor is at center, offset by 64 pixels (half size of the compass)
          const osg::Vec3f newPos(oldWh_.x() - 89.f, 89.f, 0.f);
          compass_->setMatrix(osg::Matrix::translate(newPos));
        }
      }
    }
    return false;
  }

private:
  Compass* compass_;
  osg::Vec2d oldWh_;
};

///////////////////////////////////////////

Compass::Compass(const std::string& compassFilename)
  : CompassNode(compassFilename)
{
  repositionEventHandler_ = new RepositionEventHandler(this);
}

Compass::~Compass()
{
}

void Compass::setDrawView(simVis::View* drawView)
{
  if (drawView == NULL)
  {
    removeFromView();
    return;
  }
  assert(!drawView_.valid());
  if (drawView && !drawView_.valid())
  {
    drawView_ = drawView;
    drawView_->getOrCreateHUD()->addChild(this);
    // set up the callback for frame updates
    drawView_->addEventHandler(repositionEventHandler_.get());
  }
}

simVis::View* Compass::drawView() const
{
  return drawView_.get();
}

void Compass::removeFromView()
{
  if (drawView_.valid())
  {
    // stop callbacks for frame updates
    drawView_->removeEventHandler(repositionEventHandler_.get());
    drawView_->getOrCreateHUD()->removeChild(this);
    drawView_ = NULL;
  }
}

void Compass::traverse(osg::NodeVisitor& nv)
{
  if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
  {
    // if activeView not already set, or if it went away, set the active view to the draw view
    if (!activeView() && drawView_.valid())
      setActiveView(drawView_.get());
  }
  CompassNode::traverse(nv);
}

} // namespace simVis
