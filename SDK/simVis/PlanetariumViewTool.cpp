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
#include "osg/Geode"
#include "osg/CullFace"
#include "osg/LineWidth"
#include "osgEarthAnnotation/AnnotationUtils"
#include "simCore/Calc/Angle.h"
#include "simCore/Time/TimeClass.h"
#include "simNotify/Notify.h"
#include "simVis/Beam.h"
#include "simVis/Gate.h"
#include "simVis/Locator.h"
#include "simVis/Platform.h"
#include "simVis/Scenario.h"
#include "simVis/Utils.h"
#include "simVis/PlanetariumViewTool.h"

#define OVERRIDE_TAG "PlanetariumViewTool"

#define LC "[PlanetariumView] "

//-------------------------------------------------------------------

namespace
{
  /**
   * Adapter that routes geometry update calls back to our object.
   */
  struct UpdateGeometryAdapter : public simVis::TargetDelegation::UpdateGeometryCallback
  {
    simVis::PlanetariumViewTool* tool_;
    explicit UpdateGeometryAdapter(simVis::PlanetariumViewTool* tool) : tool_(tool) { }
    void operator()(osg::MatrixTransform* xform, const osg::Vec3d& ecef)
    {
      tool_->updateTargetGeometry(xform, ecef);
    }
  };
}

//-------------------------------------------------------------------
namespace simVis
{
PlanetariumViewTool::PlanetariumViewTool(PlatformNode* host) :
  host_(host),
  range_(1000.0),
  domeColor_(0.8f, 1.0f, 0.8f, 0.5f), // RGBA
  displayTargetVectors_(true)
{
  family_.reset();

  // the geofence will filter out visible objects
  fence_ = new HorizonGeoFence();

  // build the geometry for a target node
  targetGeode_ = new osg::Geode();
  scaleTargetGeode_(targetGeode_.get(), range_);
}

osg::Node* PlanetariumViewTool::getNode() const
{
  return root_.get();
}

void PlanetariumViewTool::setRange(double range)
{
  if (range != range_)
  {
    range_ = range;

    // clear all target delegates
    if (targets_.valid())
      targets_->removeAll();

    updateDome_();

    // rescale the one geode that is reused for all target delegates
    scaleTargetGeode_(targetGeode_.get(), range_);

    // recreate our target delegates
    if (scenario_.valid() && targets_.valid())
    {
      EntityVector entities;
      scenario_->getAllEntities(entities);
      for (EntityVector::const_iterator i = entities.begin(); i != entities.end(); ++i)
      {
        PlatformNode* platform = dynamic_cast<PlatformNode*>(i->get());
        if (platform != NULL && platform != host_.get() && platform->isActive())
          targets_->addOrUpdate(platform);
      }
    }

    applyOverrides_(true);
  }
}

void PlanetariumViewTool::setColor(const osg::Vec4f& color)
{
  if (color != domeColor_)
  {
    domeColor_ = color;
    updateDome_();
  }
}

void PlanetariumViewTool::setBeamPrefs(const simData::BeamPrefs& prefs)
{
  beamPrefs_ = prefs;
  applyOverrides_(true);
}

void PlanetariumViewTool::setGatePrefs(const simData::GatePrefs& prefs)
{
  gatePrefs_ = prefs;
  applyOverrides_(true);
}

void PlanetariumViewTool::setDisplayTargetVectors(bool value)
{
  displayTargetVectors_ = value;
}

void PlanetariumViewTool::onInstall(const ScenarioManager& scenario)
{
  // create a node to track the position of the host:
  root_ = new LocatorNode(new Locator(host_->getLocator(), Locator::COMP_POSITION));
  root_->setName("Planetarium Tool Root Node");

  // build the dome
  updateDome_();

  // reset the delegate graph.
  targets_ = new TargetDelegation();
  targets_->setGeoFence(fence_.get());
  targets_->addUpdateGeometryCallback(new UpdateGeometryAdapter(this));
  root_->addChild(targets_.get());

  // sets horizon geofence to host position, which does not work correctly
  simCore::Vec3 ecef;
  root_->getPosition(&ecef);
  fence_->setLocation(osg::Vec3d(ecef.x(), ecef.y(), ecef.z()));

  // initial pull of active target platforms
  EntityVector entities;
  scenario.getAllEntities(entities);
  onUpdate(scenario, simCore::MIN_TIME_STAMP, entities);

  // collect the entity list from the scenario
  family_.reset();
  family_.add(scenario, host_->getId());

  // install all overrides
  applyOverrides_(true);

  // cache the scenario pointer
  scenario_ = &scenario;
}

void PlanetariumViewTool::onUninstall(const ScenarioManager& scenario)
{
  // disable all overrides
  applyOverrides_(false);
  family_.reset();

  if (dome_.valid())
    dome_->removeDrawables(0, dome_->getNumDrawables());
  if (targets_.valid())
    targets_->removeChildren(0, targets_->getNumChildren());

  // scenario has already removed us from the scenegraph
  root_ = NULL;
  targets_ = NULL;
  dome_ = NULL;
}

void PlanetariumViewTool::onEntityAdd(const ScenarioManager& scenario, EntityNode* entity)
{
  if (family_.invite(entity))
  {
    applyOverrides_(entity, true);
  }
}

void PlanetariumViewTool::onEntityRemove(const ScenarioManager& scenario, EntityNode* entity)
{
  if (family_.dismiss(entity))
  {
    applyOverrides_(entity, false);
  }
  else if (dynamic_cast<PlatformNode*>(entity))
  {
    targets_->remove(static_cast<PlatformNode*>(entity));
  }
}

void PlanetariumViewTool::onUpdate(const ScenarioManager& scenario, const simCore::TimeStamp& timeStamp, const EntityVector& updates)
{
  // update the fence
  fence_->setLocation(osg::Vec3d(0, 0, 0) * root_->getMatrix());

  // check any entity updates for positional changes
  for (EntityVector::const_iterator i = updates.begin(); i != updates.end(); ++i)
  {
    PlatformNode* platform = dynamic_cast<PlatformNode*>(i->get());
    if (!platform || platform == host_.get())
      continue;
    if (platform->isActive())
      targets_->addOrUpdate(platform);
    else
      targets_->remove(platform);
  }
}

void PlanetariumViewTool::updateTargetGeometry(osg::MatrixTransform* mt,
                                          const osg::Vec3d&     ecef)
{
  static osg::Vec3d s_up(0.0, 0.0, 1.0);

  // if the transform has no children, create the initial subgraph.
  if (mt->getNumChildren() == 0)
  {
    mt->addChild(targetGeode_.get());
    mt->addChild(buildVectorGeode_());
  }

  // transform the target position into planetarium-local space:
  osg::Vec3d local = ecef * root_->getInverseMatrix();
  double     local_len = local.length();
  osg::Vec3d local_n   = local / local_len;

  // update the vector.
  osg::Geode* vectorNode = static_cast<osg::Geode*>(mt->getChild(1));
  vectorNode->setNodeMask(displayTargetVectors_ ? ~0 : 0);
  osg::Geometry* g = vectorNode->getDrawable(0)->asGeometry();
  osg::Vec3Array* v = (osg::Vec3Array*)g->getVertexArray();
  (*v)[1] = s_up * (local_len - range_);
  v->dirty();

  // create the target vector and scale it to the dome's surface.
  mt->setMatrix(
    osg::Matrix::rotate(s_up, local_n) *
    osg::Matrix::translate(local_n * range_));
}

void PlanetariumViewTool::updateDome_()
{
  if (root_.valid())
  {
    if (!dome_.valid())
    {
      dome_ = new osg::Geode();
      root_->addChild(dome_.get());
    }
    else
    {
      dome_->removeDrawables(0, dome_->getNumDrawables());
    }

    // build a sphere
    osg::Geometry* drawable = osgEarth::Annotation::AnnotationUtils::createEllipsoidGeometry(range_, range_, range_, domeColor_);
    osg::StateSet* stateSet = drawable->getOrCreateStateSet();
    stateSet->setMode(GL_BLEND, 1);
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0, 1, false));
    dome_->addDrawable(drawable);
  }
}

void PlanetariumViewTool::applyOverrides_(bool enable)
{
  for (EntityFamily::EntityObserverSet::iterator i = family_.members().begin();
      i != family_.members().end();
      ++i)
  {
    if (i->valid())
      applyOverrides_(i->get(), enable);
  }
}

void PlanetariumViewTool::applyOverrides_(EntityNode* entity, bool enable)
{
  simVis::BeamNode* beam = dynamic_cast<BeamNode*>(entity);
  if (beam)
  {
    if (enable)
    {
      // draw the beam clamped to the dome's surface, unless the
      // beam's range is less than the dome range, in which case
      // don't draw the beam at all.
      const simData::BeamUpdate* lastUpdate = beam->getLastUpdateFromDS();
      if (lastUpdate && lastUpdate->range() >= range_)
      {
        simData::BeamPrefs prefs(beamPrefs_);
        prefs.set_drawtype(simData::BeamPrefs_DrawType_COVERAGE);
        beam->setPrefsOverride(OVERRIDE_TAG, prefs);

        simData::BeamUpdate update;
        update.set_range(range_);
        beam->setUpdateOverride(OVERRIDE_TAG, update);
      }
      else
      {
        simData::BeamPrefs prefs(beamPrefs_);
        prefs.mutable_commonprefs()->set_draw(false);
        beam->setPrefsOverride(OVERRIDE_TAG, prefs);
      }
    }
    else
    {
      beam->removePrefsOverride(OVERRIDE_TAG);
      beam->removeUpdateOverride(OVERRIDE_TAG);
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
      // since this does not override centroid, gate's localgrid will display at actual gate centroid location (not at edge of planetarium)
      gate->setUpdateOverride(OVERRIDE_TAG, update);

      // prefs override forces gate rebuild, so do it after after update override (which gate handles in-place)
      simData::GatePrefs prefs(gatePrefs_);
      prefs.set_drawcentroid(false);
      gate->setPrefsOverride(OVERRIDE_TAG, prefs);
    }
    else
    {
      gate->removePrefsOverride(OVERRIDE_TAG);
      gate->removeUpdateOverride(OVERRIDE_TAG);
    }
    return;
  }
}

void PlanetariumViewTool::scaleTargetGeode_(osg::Geode* geode, double range) const
{
  if (geode->getNumDrawables() > 0)
    geode->removeDrawables(0);

  // the graphic used for target delegates is scaled based on range (planetarium radius), this might be a dimension in meters
  // this formula for calculating s is purely trial-and-error, intended to maintain a minimum size at low range, but scale slowly with increasing range.
  const float s = static_cast<float>(20.0 + range / 60.0);
  osg::Vec3Array* v = new osg::Vec3Array(4);
  (*v)[0].set(-s, -s, 0.0f);
  (*v)[1].set(s, s, 0.0f);
  (*v)[2].set(-s, s, 0.0f);
  (*v)[3].set(s, -s, 0.0f);

  osg::Vec4Array* c = new osg::Vec4Array(1);
  (*c)[0].set(1.0f, 1.0f, 1.0f, 1.0f);

  osg::PrimitiveSet* prim = new osg::DrawArrays(GL_LINES, 0, 4);

  osg::Geometry* geom = new osg::Geometry();
  geom->setUseVertexBufferObjects(true);
  geom->setUseDisplayList(false);
  geom->setVertexArray(v);
  geom->setColorArray(c);
  geom->setColorBinding(osg::Geometry::BIND_OVERALL);
  geom->addPrimitiveSet(prim);

  osg::StateSet* stateset = geom->getOrCreateStateSet();
  simVis::setLighting(stateset, 0);
  stateset->setAttributeAndModes(new osg::LineWidth(2.0f), 1);

  geode->addDrawable(geom);
}

osg::Geode* PlanetariumViewTool::buildVectorGeode_()
{
  osg::Vec3Array* v = new osg::Vec3Array(4);
  (*v)[0].set(0.0f, 0.0f, 0.0f);
  (*v)[1].set(0.0f, 0.0f, 1.0f);

  osg::Vec4Array* c = new osg::Vec4Array(1);
  (*c)[0].set(0.0f, 1.0f, 0.0f, 1.0f);

  osg::PrimitiveSet* prim = new osg::DrawArrays(GL_LINES, 0, 2);

  osg::Geometry* geom = new osg::Geometry();
  geom->setUseVertexBufferObjects(true);
  geom->setUseDisplayList(false);
  geom->setVertexArray(v);
  geom->setColorArray(c);
  geom->setColorBinding(osg::Geometry::BIND_OVERALL);
  geom->addPrimitiveSet(prim);

  osg::StateSet* stateset = geom->getOrCreateStateSet();
  simVis::setLighting(stateset, 0);

  osg::Geode* geode = new osg::Geode();
  geode->addDrawable(geom);

  return geode;
}
}

