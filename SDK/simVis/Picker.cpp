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
#include "simVis/Shaders.h"
#include "simVis/Picker.h"

namespace simVis {

/** RTTPicker callback that will transmit the picked ID to the parent Picker instance */
class PickerCallback : public osgEarth::Util::RTTPicker::Callback
{
public:
  PickerCallback(Picker& picker)
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
    // Always pick, on every event
    return true;
  }

private:
  Picker& picker_;
};

/////////////////////////////////////////////////////////////////

class ViewsWatcher : public simVis::ViewManager::Callback
{
public:
  ViewsWatcher(osgEarth::Util::RTTPicker* picker)
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

Picker::Picker(simVis::ViewManager* viewManager, simVis::ScenarioManager* scenarioManager, int cameraSize)
  : pickedId_(0),
    highlightIdUniform_(new osg::Uniform("sdk_pick_highlight_objectid", 0u)),
    highlightEnabledUniform_(new osg::Uniform("sdk_pick_highlight_enabled", true)),
    rttPicker_(new osgEarth::Util::RTTPicker(cameraSize)),
    viewManager_(viewManager),
    scenarioManager_(scenarioManager)
{
  rttPicker_->addChild(scenarioManager_.get());
  if (viewManager)
  {
    ViewsWatcher* viewManagerCallback = new ViewsWatcher(rttPicker_);
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
  // TODO: Pending osgEarth pull request integration
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,7,0) && 0
  unsigned int ignoreMask = simVis::DISPLAY_MASK_LABEL | simVis::DISPLAY_MASK_TRACK_HISTORY | simVis::DISPLAY_MASK_LOCAL_GRID;
  rttPicker_->setCullMask(~ignoreMask);
#endif
}

Picker::~Picker()
{
  osg::ref_ptr<simVis::ViewManager> viewManager;
  if (viewManager_.lock(viewManager))
    viewManager->removeCallback(viewManagerCallback_);
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
  return dynamic_cast<osg::Node*>(picked_.get());
}

simVis::EntityNode* Picker::pickedEntity() const
{
  return osgEarth::findFirstParentOfType<simVis::EntityNode>(pickedNode());
}

simVis::PlatformNode* Picker::pickedPlatform() const
{
  // TODO: Test this against beams and gates and attached GOGs.  Might be better to return pickedEntity()
  return osgEarth::findFirstParentOfType<simVis::PlatformNode>(pickedNode());
}

void Picker::setPickedId(unsigned int id)
{
  if (pickedId_ == id)
    return;
  // Update internal state
  pickedId_ = id;
  if (pickedId_ == 0)
    picked_ = NULL;
  else
    picked_ = osgEarth::Registry::objectIndex()->get<osg::Referenced>(id);
  highlightIdUniform_->set(pickedId_);

  // Tell listeners
  firePickChanged_(pickedId_, picked_.get());
}

void Picker::installHighlightShader()
{
  if (!scenarioManager_.valid())
    return;
  osg::StateSet* stateSet = scenarioManager_->getOrCreateStateSet();
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateSet);

  // Load the vertex and fragment shaders
  Shaders package;
  package.load(vp, package.pickerVertex());
  package.load(vp, package.pickerFragment());

  // Since we're accessing object IDs, we need to load the indexing shader as well
  osgEarth::Registry::objectIndex()->loadShaders(vp);

  // A uniform that will tell the shader which object to highlight
  stateSet->addUniform(highlightIdUniform_);
  stateSet->addUniform(highlightEnabledUniform_);
}

void Picker::setHighlightEnabled(bool enabled)
{
  highlightEnabledUniform_->set(enabled);
}

bool Picker::isHighlightEnabled() const
{
  bool isEnabled = false;
  // Ensure get() succeeds and returns that the uniform is enabled.
  return highlightEnabledUniform_->get(isEnabled) && isEnabled;
}

osg::Texture2D* Picker::getOrCreateTexture(simVis::View* fromView)
{
  return rttPicker_->getOrCreateTexture(fromView);
}

void Picker::setUpViewWithDebugTexture(osgViewer::View* intoView, simVis::View* fromView)
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

osgEarth::Util::RTTPicker* Picker::rttPicker() const
{
  return rttPicker_;
}

void Picker::addCallback(Callback* callback)
{
  callbacks_.push_back(callback);
}

void Picker::removeCallback(Callback* callback)
{
  callbacks_.erase(std::remove(callbacks_.begin(), callbacks_.end(), callback), callbacks_.end());
}

void Picker::firePickChanged_(unsigned int pickedId, osg::Referenced* picked)
{
  for (auto i = callbacks_.begin(); i != callbacks_.end(); ++i)
    (*i)->pickChanged(pickedId, picked);
}

}
