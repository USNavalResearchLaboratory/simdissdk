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
#include <algorithm>
#include "osg/Uniform"
#include "osg/BlendFunc"
#include "osg/Switch"
#include "osgEarth/VirtualProgram"
#include "osgEarth/Registry"
#include "osgEarth/NodeUtils"
#include "osgEarth/ObjectIndex"
#include "osgEarthUtil/RTTPicker"
#include "simVis/Scenario.h"
#include "simVis/ViewManager.h"
#include "simVis/View.h"
#include "simVis/osgEarthVersion.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Shaders.h"
#include "simVis/Picker.h"

namespace simVis {

static const std::string SDK_PICK_HIGHLIGHT_OBJECTID = "sdk_pick_highlight_objectid";
static const std::string SDK_PICK_HIGHLIGHT_ENABLED = "sdk_pick_highlight_enabled";
static const unsigned int DEFAULT_PICK_MASK = simVis::DISPLAY_MASK_PLATFORM | simVis::DISPLAY_MASK_PLATFORM_MODEL;

/////////////////////////////////////////////////////////////////

PickerHighlightShader::PickerHighlightShader(osg::StateSet* stateset)
  : stateset_(stateset)
{
}

PickerHighlightShader::~PickerHighlightShader()
{
}

void PickerHighlightShader::installShaderProgram(osg::StateSet* intoStateSet, bool defaultEnabled)
{
  if (!intoStateSet)
    return;
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(intoStateSet);

  // Load the vertex and fragment shaders
  Shaders package;
  package.load(vp, package.pickerVertex());
  package.load(vp, package.pickerFragment());

  // Since we're accessing object IDs, we need to load the indexing shader as well
  osgEarth::Registry::objectIndex()->loadShaders(vp);

  // A uniform that will tell the shader which object to highlight
  intoStateSet->getOrCreateUniform(SDK_PICK_HIGHLIGHT_OBJECTID, osg::Uniform::UNSIGNED_INT)->set(0u);
  intoStateSet->getOrCreateUniform(SDK_PICK_HIGHLIGHT_ENABLED, osg::Uniform::BOOL)->set(defaultEnabled);
}

void PickerHighlightShader::installShaderProgram(bool defaultEnabled)
{
  osg::ref_ptr<osg::StateSet> stateset;
  if (stateset_.lock(stateset))
    PickerHighlightShader::installShaderProgram(stateset.get(), defaultEnabled);
}

bool PickerHighlightShader::isEnabled() const
{
  osg::ref_ptr<osg::StateSet> stateset;
  if (stateset_.lock(stateset))
  {
    bool isEnabled = false;
    osg::Uniform* enabledUniform = stateset->getUniform(SDK_PICK_HIGHLIGHT_ENABLED);
    // Note that get() returns true if it succeeds
    return enabledUniform && enabledUniform->get(isEnabled) && isEnabled;
  }
  return false;
}

void PickerHighlightShader::setEnabled(bool enabled)
{
  osg::ref_ptr<osg::StateSet> stateset;
  if (stateset_.lock(stateset))
    stateset->getOrCreateUniform(SDK_PICK_HIGHLIGHT_ENABLED, osg::Uniform::BOOL)->set(enabled);
}

void PickerHighlightShader::setId(unsigned int tagId)
{
  osg::ref_ptr<osg::StateSet> stateset;
  if (stateset_.lock(stateset))
    stateset->getOrCreateUniform(SDK_PICK_HIGHLIGHT_OBJECTID, osg::Uniform::UNSIGNED_INT)->set(tagId);
}

/////////////////////////////////////////////////////////////////

Picker::Picker(osg::StateSet* stateSet)
  : pickedId_(0),
    shaderValues_(new PickerHighlightShader(stateSet))
{
}

Picker::~Picker()
{
}

void Picker::addCallback(Callback* callback)
{
  callbacks_.push_back(callback);
}

void Picker::removeCallback(Callback* callback)
{
  callbacks_.erase(std::remove(callbacks_.begin(), callbacks_.end(), callback), callbacks_.end());
}

void Picker::setPicked_(unsigned int pickedId, osg::Referenced* picked)
{
  if (pickedId == pickedId_ && picked_ == picked)
    return;
  shaderValues_->setId(pickedId);
  pickedId_ = pickedId;
  picked_ = picked;
  for (auto i = callbacks_.begin(); i != callbacks_.end(); ++i)
    (*i)->pickChanged(pickedId, picked);
}

unsigned int Picker::pickedId() const
{
  return pickedId_;
}

osg::Referenced* Picker::picked() const
{
  return picked_.get();
}

osg::Node* Picker::pickedNode() const
{
  return dynamic_cast<osg::Node*>(picked());
}

simVis::EntityNode* Picker::pickedEntity() const
{
  return osgEarth::findFirstParentOfType<simVis::EntityNode>(pickedNode());
}

simVis::PlatformNode* Picker::pickedPlatform() const
{
  return osgEarth::findFirstParentOfType<simVis::PlatformNode>(pickedNode());
}

/////////////////////////////////////////////////////////////////

class IntersectPicker::IntersectEventHandler : public osgGA::GUIEventHandler
{
public:
  IntersectEventHandler(IntersectPicker& picker)
    : picker_(picker),
      repickNeeded_(false)
  {
  }

  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::MOVE:
    case osgGA::GUIEventAdapter::DRAG:
      picker_.lastMouseView_ = dynamic_cast<simVis::View*>(aa.asView());
      picker_.mx_ = ea.getX();
      picker_.my_ = ea.getY();
      repickNeeded_ = true;
      break;

    case osgGA::GUIEventAdapter::FRAME:
      picker_.pickedThisFrame_ = false;
      // If the mouse moved, we need to re-pick to capture movement
      if (repickNeeded_)
      {
        repickNeeded_ = false;
        picker_.pickThisFrame_();
      }
      break;

    default:
      // Most events: do nothing
      break;
    }
    // Never intercept an event
    return false;
  }

private:
  IntersectPicker& picker_;
  bool repickNeeded_;
};

/////////////////////////////////////////////////////////////////

IntersectPicker::IntersectPicker(simVis::ViewManager* viewManager, simVis::ScenarioManager* scenarioManager)
  : Picker(scenarioManager->getOrCreateStateSet()),
    mx_(0.0),
    my_(0.0),
    pickedThisFrame_(false),
    viewManager_(viewManager),
    scenario_(scenarioManager)
{
  guiEventHandler_ = new IntersectEventHandler(*this);
  addHandlerToViews_ = new AddEventHandlerToViews(guiEventHandler_.get());
  if (viewManager_.valid())
  {
    addHandlerToViews_->addToViews(*viewManager_);
    viewManager_->addCallback(addHandlerToViews_.get());
  }
}

IntersectPicker::~IntersectPicker()
{
  if (viewManager_.valid())
  {
    addHandlerToViews_->removeFromViews(*viewManager_);
    viewManager_->removeCallback(addHandlerToViews_.get());
  }
}

void IntersectPicker::pickThisFrame_()
{
  pickedThisFrame_ = true;
  // Intersect picker should only pick on Platforms and Platform Models
  unsigned int acceptMask = DEFAULT_PICK_MASK;
  simVis::EntityNode* pickedEntity = NULL;
  if (lastMouseView_.valid())
    pickedEntity = scenario_->find(lastMouseView_.get(), mx_, my_, acceptMask);
  if (pickedEntity == NULL)
  {
    setPicked_(0, NULL);
    return;
  }

  // Find a child of the node that has a tag
  simVis::PlatformNode* platform = dynamic_cast<simVis::PlatformNode*>(pickedEntity);
  if (platform)
    setPicked_(platform->getModel()->objectIndexTag(), pickedEntity);
  else
    setPicked_(0, pickedEntity);
}

/////////////////////////////////////////////////////////////////

class ViewsWatcher : public simVis::ViewManager::Callback
{
public:
  explicit ViewsWatcher(osgEarth::Util::RTTPicker* picker)
    : picker_(picker)
  {
  }

  void addToView(simVis::View* view)
  {
    if (view->type() == simVis::View::VIEW_SUPERHUD || !picker_.valid())
      return;
    view->addEventHandler(picker_.get());

#if 0
    // Presumes that pick camera is NESTED_RENDER.  If not, then we need to
    // copy over the pre-draw callbacks, else LDB will cause issues with
    // multi-inset use cases.
    osg::Camera* pickCamera = picker_->getOrCreatePickCamera(view);
    pickCamera->setPreDrawCallback(view->getCamera()->getPreDrawCallback());
#endif
  }

  void removeFromView(simVis::View* view)
  {
    if (view->type() == simVis::View::VIEW_SUPERHUD || !picker_.valid())
      return;
    view->removeEventHandler(picker_.get());
  }

  virtual void operator()(simVis::View* inset, const EventType& e)
  {
    switch (e)
    {
    case VIEW_ADDED:
      addToView(inset);
      break;
    case VIEW_REMOVED:
      removeFromView(inset);
      break;
    }
  }

private:
  osg::observer_ptr<osgEarth::Util::RTTPicker> picker_;
};

/////////////////////////////////////////////////////////////////

/** RTTPicker callback that will transmit the picked ID to the parent Picker instance */
class PickerCallback : public osgEarth::Util::RTTPicker::Callback
{
public:
  explicit PickerCallback(RTTPicker& picker)
    : picker_(picker)
  {
  }

  void onHit(osgEarth::ObjectID id)
  {
    picker_.setPickedId(id);
  }

  void onMiss()
  {
    picker_.setPickedId(0);
  }

  bool accept(const osgGA::GUIEventAdapter& ea, const osgGA::GUIActionAdapter& aa)
  {
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::FRAME:
      if (underCursor_.valid() && underCursor_.get() == dynamic_cast<const simVis::View*>(&aa))
        return true;
      break;
    case osgGA::GUIEventAdapter::MOVE:
    case osgGA::GUIEventAdapter::DRAG:
    case osgGA::GUIEventAdapter::PUSH:
      underCursor_ = dynamic_cast<const simVis::View*>(&aa);
      break;

    default:
      // Do nothing for most events
      break;
    }
    return false;
  }

private:
  RTTPicker& picker_;
  osg::observer_ptr<const simVis::View> underCursor_;
};

/////////////////////////////////////////////////////////////////

RTTPicker::RTTPicker(simVis::ViewManager* viewManager, simVis::ScenarioManager* scenarioManager, int cameraSize)
  : Picker(scenarioManager->getOrCreateStateSet()),
    rttPicker_(new osgEarth::Util::RTTPicker(cameraSize)),
    viewManager_(viewManager)
{
  rttPicker_->addChild(scenarioManager);
  if (viewManager)
  {
    ViewsWatcher* viewManagerCallback = new ViewsWatcher(rttPicker_.get());
    viewManager->addCallback(viewManagerCallback);

    std::vector<simVis::View*> views;
    viewManager->getViews(views);
    for (auto i = views.begin(); i != views.end(); ++i)
      viewManagerCallback->addToView(*i);
    viewManagerCallback_ = viewManagerCallback;
  }
  // Install a callback that controls the picker and listens for hits.
  rttPicker_->setDefaultCallback(new PickerCallback(*this));

  // Set up the picker to ignore various features of SIMDIS that aren't pickable
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,7,0)
  rttPicker_->setCullMask(DEFAULT_PICK_MASK);
#endif
}

RTTPicker::~RTTPicker()
{
  // Reset RTT Picker's callback to avoid possible invalid-memory situation if RTT Picker outlives us
  rttPicker_->setDefaultCallback(NULL);
  osg::ref_ptr<simVis::ViewManager> viewManager;
  if (viewManager_.lock(viewManager))
    viewManager->removeCallback(viewManagerCallback_.get());
}

void RTTPicker::setPickedId(unsigned int id)
{
  // Tell listeners
  osg::Referenced* ref = osgEarth::Registry::objectIndex()->get<osg::Referenced>(id).get();
  setPicked_(id, ref);
}

osg::Texture2D* RTTPicker::getOrCreateTexture(simVis::View* fromView)
{
  return rttPicker_->getOrCreateTexture(fromView);
}

void RTTPicker::setUpViewWithDebugTexture(osgViewer::View* intoView, simVis::View* fromView)
{
  if (!intoView || !fromView)
    return;
  osg::Texture* rttTex = getOrCreateTexture(fromView);
  if (!rttTex)
    return;

  intoView->setCameraManipulator(0L);
  intoView->getCamera()->setName("RTT view");
  intoView->getCamera()->setViewport(0, 0, 256, 256);
  intoView->getCamera()->setClearColor(osg::Vec4(1, 1, 1, 1));
  intoView->getCamera()->setProjectionMatrixAsOrtho2D(-.5, .5, -.5, .5);
  intoView->getCamera()->setViewMatrixAsLookAt(osg::Vec3d(0, -1, 0), osg::Vec3d(0, 0, 0), osg::Vec3d(0, 0, 1));
  intoView->getCamera()->setProjectionResizePolicy(osg::Camera::FIXED);

  osg::Vec3Array* v = new osg::Vec3Array(6);
  (*v)[0].set(-.5, 0, -.5); (*v)[1].set(.5, 0, -.5); (*v)[2].set(.5, 0, .5); (*v)[3].set((*v)[2]); (*v)[4].set(-.5, 0, .5); (*v)[5].set((*v)[0]);

  osg::Vec2Array* t = new osg::Vec2Array(6);
  (*t)[0].set(0, 0); (*t)[1].set(1, 0); (*t)[2].set(1, 1); (*t)[3].set((*t)[2]); (*t)[4].set(0, 1); (*t)[5].set((*t)[0]);

  osg::Geometry* g = new osg::Geometry();
  g->setUseVertexBufferObjects(true);
  g->setUseDisplayList(false);
  g->setVertexArray(v);
  g->setTexCoordArray(0, t);
  g->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, 6));

  osg::Geode* geode = new osg::Geode();
  geode->addDrawable(g);

  osg::StateSet* stateSet = geode->getOrCreateStateSet();
  stateSet->setDataVariance(osg::Object::DYNAMIC);

  stateSet->setTextureAttributeAndModes(0, rttTex, 1);
  rttTex->setUnRefImageDataAfterApply(false);
  rttTex->setResizeNonPowerOfTwoHint(false);

  stateSet->setMode(GL_LIGHTING, 0);
  stateSet->setMode(GL_CULL_FACE, 0);
  stateSet->setAttributeAndModes(new osg::BlendFunc(GL_ONE, GL_ZERO), 1);

  const char* fs =
    "#version " GLSL_VERSION_STR "\n"
    "void swap(inout vec4 c) { c.rgba = c==vec4(0)? vec4(1) : vec4(vec3((c.r+c.g+c.b+c.a)/4.0),1); }\n";
  osgEarth::Registry::shaderGenerator().run(geode);
  osgEarth::VirtualProgram::getOrCreate(geode->getOrCreateStateSet())->setFunction("swap", fs, osgEarth::ShaderComp::LOCATION_FRAGMENT_COLORING);

  intoView->setSceneData(geode);
}

osgEarth::Util::RTTPicker* RTTPicker::rttPicker() const
{
  return rttPicker_.get();
}

}
