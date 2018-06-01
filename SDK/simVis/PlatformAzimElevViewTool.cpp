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
#include "osg/Depth"
#include "osgText/Text"
#include "osgEarth/VirtualProgram"
#include "osgEarth/ShaderGenerator"
#include "osgEarth/Registry"
#include "simVis/LineDrawable.h"

#include "simCore/Calc/Angle.h"
#include "simVis/Beam.h"
#include "simVis/Gate.h"
#include "simVis/Locator.h"
#include "simVis/Platform.h"
#include "simVis/Shaders.h"
#include "simVis/Scenario.h"
#include "simVis/Utils.h"
#include "simVis/PlatformAzimElevViewTool.h"
#define OVERRIDE_TAG "PlatformAzimElevViewTool"

//#define DEBUG_LABELS

namespace
{
  // Builds a program component that warps the geometry in the
  // view space's XY plane.
  osg::StateAttribute* createWarpingProgram()
  {
    osgEarth::VirtualProgram* vp = new osgEarth::VirtualProgram();
    simVis::Shaders package;
    package.load(vp, package.platformAzimElevWarpVertex());
    return vp;
  }

  /**
   * Adapter that routes geometry update calls back to our object.
   */
  struct UpdateGeometryAdapter : public simVis::TargetDelegation::UpdateGeometryCallback
  {
    simVis::PlatformAzimElevViewTool* tool_;
    explicit UpdateGeometryAdapter(simVis::PlatformAzimElevViewTool* tool) : tool_(tool) { }
    void operator()(osg::MatrixTransform* xform, const osg::Vec3d& ecef)
    {
      tool_->updateTargetGeometry(xform, ecef);
    }
  };
}

//-------------------------------------------------------------------

namespace simVis
{
PlatformAzimElevViewTool::PlatformAzimElevViewTool(EntityNode* host) :
  host_(host),
  range_(20000.0),
  elevLabelAngle_((float)osg::PI_2)
{
  warpingProgram_ = createWarpingProgram();
  grid_ = createAzElGrid_();
  grid_->setMatrix(osg::Matrix::scale(range_, range_, range_));

  targetGeom_ = buildTargetGeometry_();

  // the geofence will filter out visible objects
  fence_ = new HorizonGeoFence();
}

osg::Node* PlatformAzimElevViewTool::getNode() const
{
  return root_.get();
}

void PlatformAzimElevViewTool::setRange(double range)
{
  if (range != range_)
  {
    range_ = range;
    rebuild_();
    applyOverrides_();
  }
}

void PlatformAzimElevViewTool::setElevLabelAngle(float angle)
{
  if (angle != elevLabelAngle_)
  {
    elevLabelAngle_ = angle;
    if (root_.valid() && grid_.valid())
      root_->removeChild(grid_.get());
    grid_ = createAzElGrid_();
    if (root_.valid())
      root_->addChild(grid_.get());
    rebuild_();
    applyOverrides_();
  }
}

void PlatformAzimElevViewTool::setBeamPrefs(const simData::BeamPrefs& prefs)
{
  beamPrefs_ = prefs;
  applyOverrides_();
}

void PlatformAzimElevViewTool::setGatePrefs(const simData::GatePrefs& prefs)
{
  gatePrefs_ = prefs;
  applyOverrides_();
}

void PlatformAzimElevViewTool::onInstall(const ScenarioManager& scenario)
{
  // create a node to track the position of the host:
  root_ = new LocatorNode(new Locator(host_->getLocator(), Locator::COMP_POSITION));
  root_->addChild(grid_.get());
  root_->setName("Platform Az/El Tool Root Node");

  // delegate target geometry:
  targets_ = new TargetDelegation();
  targets_->setGeoFence(fence_.get());
  targets_->addUpdateGeometryCallback(new UpdateGeometryAdapter(this));
  root_->addChild(targets_.get());

  // set up state for the delegation:
  simVis::setLighting(targets_->getOrCreateStateSet(), 0);

  // build the scene elements:
  rebuild_();

  // collect the entity list from the scenario:
  family_.reset();
  family_.add(scenario, host_->getId());

  root_->addCullCallback(new osgEarth::InstallViewportSizeUniform());

  // install all overrides
  applyOverrides_(true);
}

void PlatformAzimElevViewTool::onUninstall(const ScenarioManager& scenario)
{
  // disable all overrides
  applyOverrides_(false);
  family_.reset();

  if (targets_.valid())
    targets_->removeChildren(0, targets_->getNumChildren());

  // scenario has already removed us from the scenegraph
  root_ = NULL;
  targets_ = NULL;
}

bool PlatformAzimElevViewTool::isInstalled_() const
{
  return root_.valid();
}

void PlatformAzimElevViewTool::onEntityAdd(const ScenarioManager& scenario, EntityNode* entity)
{
  if (family_.invite(entity))
  {
    applyOverrides_(entity);
  }
}

void PlatformAzimElevViewTool::onEntityRemove(const ScenarioManager& scenario, EntityNode* entity)
{
  simData::ObjectId id = entity->getId();
  if (family_.dismiss(entity))
  {
    applyOverrides_(entity, false);
  }
}

void PlatformAzimElevViewTool::onUpdate(const ScenarioManager& scenario, const simCore::TimeStamp& timeStamp, const EntityVector& updates)
{
  // update the fence
  fence_->setLocation(osg::Vec3d(0, 0, 0) * root_->getMatrix());

  // check any entity updates for positional changes
  for (EntityVector::const_iterator i = updates.begin(); i != updates.end(); ++i)
  {
    const PlatformNode* platform = dynamic_cast<PlatformNode*>(i->get());
    if (!platform || platform == host_.get())
      continue;
    if (platform->isActive())
      targets_->addOrUpdate(platform);
    else
      targets_->remove(platform);
  }
}

void PlatformAzimElevViewTool::rebuild_()
{
  grid_->setMatrix(osg::Matrix::scale(range_, range_, range_));
  this->setDirty();
}

void PlatformAzimElevViewTool::applyOverrides_()
{
  applyOverrides_(isInstalled_());
}

void PlatformAzimElevViewTool::applyOverrides_(bool enable)
{
  for (EntityFamily::EntityObserverSet::iterator i = family_.members().begin(); i != family_.members().end(); ++i)
  {
    if (i->valid())
    {
      applyOverrides_(i->get(), enable);
    }
  }
}

void PlatformAzimElevViewTool::applyOverrides_(EntityNode* entity)
{
  applyOverrides_(entity, isInstalled_());
}

void PlatformAzimElevViewTool::applyOverrides_(EntityNode* entity, bool enable)
{
  simVis::BeamNode* beam = dynamic_cast<BeamNode*>(entity);
  if (beam)
  {
    if (enable)
    {
      simData::BeamPrefs prefs(beamPrefs_);
      prefs.set_drawtype(simData::BeamPrefs_DrawType_COVERAGE);
      beam->setPrefsOverride(OVERRIDE_TAG, prefs);

      simData::BeamUpdate update;
      update.set_range(range_);
      beam->setUpdateOverride(OVERRIDE_TAG, update);

      osg::StateSet* sset = beam->getOrCreateStateSet();
      sset->setAttributeAndModes(warpingProgram_.get(), 1);
    }
    else
    {
      beam->removePrefsOverride(OVERRIDE_TAG);
      beam->removeUpdateOverride(OVERRIDE_TAG);

      osg::StateSet* sset = beam->getOrCreateStateSet();
      sset->removeAttribute(warpingProgram_.get());
    }

    return;
  }

  GateNode* gate = dynamic_cast<GateNode*>(entity);
  if (gate)
  {
    if (enable)
    {
      simData::GateUpdate update;
      // overriding minrange and maxrange to same value to draw only the far face of the gate
      update.set_minrange(range_);
      update.set_maxrange(range_);
      gate->setUpdateOverride(OVERRIDE_TAG, update);

      // prefs override forces gate rebuild, so do it after update override (which gate handles in-place)
      simData::GatePrefs prefs(gatePrefs_);
      prefs.set_drawcentroid(false);
      gate->setPrefsOverride(OVERRIDE_TAG, prefs);

      osg::StateSet* sset = gate->getOrCreateStateSet();
      sset->setAttributeAndModes(warpingProgram_.get(), 1);
    }
    else
    {
      gate->removePrefsOverride(OVERRIDE_TAG);
      gate->removeUpdateOverride(OVERRIDE_TAG);

      osg::StateSet* sset = gate->getOrCreateStateSet();
      sset->removeAttribute(warpingProgram_.get());
    }
    return;
  }
}

// Builds the geometry for the elevation ring grid.
osg::MatrixTransform* PlatformAzimElevViewTool::createAzElGrid_()
{
  osgEarth::LineGroup* geomGroup = new osgEarth::LineGroup();
  osg::Group* textGroup = new osg::Group();

  osg::Depth* noDepthTest = new osg::Depth(osg::Depth::ALWAYS, 0, 1, false);

  // all rings are drawn on the unit circle, and scaled elsewhere
  const unsigned int numAzPts = 72;
  const unsigned int numElevRings = 9;
  const unsigned int numTicks = 36; // 2 verts per tick

  const double azMax = 2.0 * M_PI;
  const double azStep = azMax / numAzPts;
  const double elevMax = M_PI_2;
  const double elevStep = elevMax / numElevRings;

  // 9 concentric elevation circles:
  for (unsigned int elevIndex = 0; elevIndex < numElevRings; elevIndex++)
  {
    const double e = elevIndex * elevStep;
    const double cose = cos(e);
    const float z = static_cast<float>(sin(e));

    osgEarth::LineDrawable* ring = new osgEarth::LineDrawable(GL_LINE_LOOP);
    ring->reserve(numAzPts);

    for (unsigned int azIndex = 0; azIndex < numAzPts; azIndex++)
    {
      const double a = azIndex * azStep;
      const float x = static_cast<float>(cos(a) * cose);
      const float y = static_cast<float>(sin(a) * cose);
      ring->pushVertex(osg::Vec3f(x, y, z));
    }
    ring->dirty();
    geomGroup->addChild(ring);
  }

  // 36 azimuth ticks (one every 10 degrees):
  osgEarth::LineDrawable* ticks = new osgEarth::LineDrawable(GL_LINES);
  ticks->reserve(numTicks*2);
  const double tickstep = azMax / numTicks;
  const float z = static_cast<float>(sin(tickstep * 0.25));
  for (unsigned int azIndex = 0; azIndex < numTicks; azIndex++)
  {
    const double a = azIndex * tickstep;
    const float x = static_cast<float>(cos(a));
    const float y = static_cast<float>(sin(a));
    ticks->pushVertex(osg::Vec3f(x, y, 0.f));
    ticks->pushVertex(osg::Vec3f(x, y, z));
  }
  ticks->dirty();
  geomGroup->addChild(ticks);

  // N indicator
  osgText::Text* text = new osgText::Text();
  text->setPosition(osg::Vec3(0.0f, 1.05f, 0.0f));
  text->setText("N");
  text->setFont(osgEarth::Registry::instance()->getDefaultFont());
  text->setAutoRotateToScreen(false);
  text->setCharacterSizeMode(osgText::TextBase::OBJECT_COORDS);
  text->setAlignment(osgText::TextBase::CENTER_BOTTOM);
  text->setBackdropType(osgText::Text::DROP_SHADOW_BOTTOM_RIGHT);
  text->setCharacterSize(elevStep * 0.75f);
  text->getOrCreateStateSet()->setRenderBinToInherit();
  text->getOrCreateStateSet()->setAttributeAndModes(noDepthTest, 1);
  textGroup->addChild(text);

  // Elev indicators
  const double cosElevLabelAngle = cos(elevLabelAngle_);
  const double sinElevLabelAngle = sin(elevLabelAngle_);
  for (double e = elevStep; e < elevMax - elevStep; e += elevStep)
  {
    const double cose = cos(e);
    const float x = static_cast<float>(cosElevLabelAngle * cose);
    const float y = static_cast<float>(sinElevLabelAngle * cose);
    const float z = static_cast<float>(sin(e));
    const int label = static_cast<int>(floor(0.5 + simCore::RAD2DEG * e)); // round

    osgText::Text* text = new osgText::Text();
    text->setPosition(osg::Vec3(x, y, z));
    text->setText(osgEarth::Stringify() << label);
    text->setFont(osgEarth::Registry::instance()->getDefaultFont());
    text->setAutoRotateToScreen(false);
    text->setCharacterSizeMode(osgText::TextBase::OBJECT_COORDS);
    text->setAlignment(osgText::TextBase::CENTER_BOTTOM);
    text->setBackdropType(osgText::Text::DROP_SHADOW_BOTTOM_RIGHT);
    text->setCharacterSize(static_cast<float>(elevStep * 0.35));
    text->getOrCreateStateSet()->setRenderBinToInherit();
    text->getOrCreateStateSet()->setAttributeAndModes(noDepthTest, 1);
    textGroup->addChild(text);
  }

  // install default shader programs
  osgEarth::ShaderGenerator sg;
  textGroup->accept(sg);

  osg::MatrixTransform* scaler = new osg::MatrixTransform();
  scaler->addChild(geomGroup);
  scaler->addChild(textGroup);

  // warp the geometry so the elevation rings are equidistant
  scaler->getOrCreateStateSet()->setAttributeAndModes(warpingProgram_.get(), 1);

  // draw grid+labels first, so items are on top
  scaler->getOrCreateStateSet()->setRenderBinDetails(BIN_AZIM_ELEV_TOOL, BIN_GLOBAL_SIMSDK);

  return scaler;
}

osg::Node* PlatformAzimElevViewTool::buildTargetGeometry_()
{
  const float s = 3000.0f;
  osgEarth::LineDrawable* geom = new osgEarth::LineDrawable(GL_LINES);

  geom->allocate(4);
  geom->setVertex(0, osg::Vec3(-s, -s, 0.0f));
  geom->setVertex(1, osg::Vec3( s,  s, 0.0f));
  geom->setVertex(2, osg::Vec3(-s,  s, 0.0f));
  geom->setVertex(3, osg::Vec3( s, -s, 0.0f));

  geom->setColor(simVis::Color::White);
  geom->setLineWidth(2.0f);
  return geom;
}

#ifdef DEBUG_LABELS
namespace
{
  osg::Geode* createText()
  {
    osgText::Text* t = new osgText::Text();
    t->setCharacterSizeMode(osgText::Text::SCREEN_COORDS);
    t->setCharacterSize(24);
    t->setText("S");
    t->setDataVariance(osg::Object::DYNAMIC);
    t->setAutoRotateToScreen(true);
    osg::Geode* geode = new osg::Geode();
    geode->addDrawable(t);
    return geode;
  }
}
#endif

void PlatformAzimElevViewTool::updateTargetGeometry(osg::MatrixTransform* mt, const osg::Vec3d& ecef)
{
  static osg::Vec3d s_up(0, 0, 1);

  // if the transform has no children, create the initial subgraph.
  if (mt->getNumChildren() == 0)
  {
    mt->addChild(targetGeom_.get());

#ifdef DEBUG_LABELS
    mt->addChild(createText());
#endif
  }

  // transform the target position into planetarium-local space:
  osg::Vec3d local = ecef * root_->getInverseMatrix();
  double     local_len = local.length();
  osg::Vec3d local_n   = local / local_len;

  // warp the target location for the azim el plot (instead of using
  // the shader, which only works on localized verts)
  osg::Vec3d v = local_n * range_;
  double radius = range_;
  v.z() = 0.0;
  double a  = asin(v.length() / radius);
  double arclen = radius * a;
  v.normalize();
  v *= arclen;
  v.z() = arclen;
  mt->setMatrix(osg::Matrix::translate(v));

#ifdef DEBUG_LABELS
  double angle = 90.0 - osg::RadiansToDegrees(acos(local_n * s_up));
  ((osgText::Text*)((osg::Geode*)mt->getChild(1))->getDrawable(0))->setText(
    osgEarth::Stringify() << std::setprecision(2) << angle);
#endif
}
}

