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
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osg/Depth"
#include "osg/Geode"
#include "osg/CullFace"
#include "osgEarth/LineDrawable"
#include "osgEarth/AnnotationUtils"
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

#undef LC
#define LC "[PlanetariumView] "

//-------------------------------------------------------------------

namespace
{
  /** Number of segments in the vector line */
  static const unsigned int NUM_VECTOR_SEGS = 25;

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

PlanetariumViewTool::BeamHistory::BeamHistory(simVis::BeamNode* beam)
  : osg::Group(),
  beam_(beam)
{}

PlanetariumViewTool::BeamHistory::~BeamHistory()
{
}

void PlanetariumViewTool::BeamHistory::updateBeamHistory(double time, double range)
{
  if (!beam_.valid())
    return;

  const simData::BeamUpdate* lastUpdate = beam_->getLastUpdateFromDS();
  if (!lastUpdate)
    return;

  simData::BeamPrefs prefs(beam_->getPrefs());
  // Add a locator node for the most recent update if not already done
  if (historyNodes_.find(lastUpdate->time()) == historyNodes_.end())
  {
    simData::BeamUpdate update(*lastUpdate);
    update.set_range(range);

    prefs.set_blended(true);
    prefs.set_drawtype(simData::BeamPrefs_DrawType_COVERAGE);

    osg::ref_ptr<BeamVolume> volume = new BeamVolume(prefs, update);

    Locator* beamLocator = beam_->getLocator();

    // Get the origin locator, which is the parent
    Locator* parentLocator = beamLocator->getParentLocator();

    simCore::Vec3 pos, ori;
    beamLocator->getLocalOffsets(pos, ori);

    Locator* newBeamLocator = nullptr;

    ResolvedPositionLocator* positionLocator = dynamic_cast<ResolvedPositionLocator*>(beamLocator);
    if (positionLocator)
      newBeamLocator = new ResolvedPositionLocator(parentLocator, Locator::COMP_ALL);
    else
      newBeamLocator = new ResolvedPositionOrientationLocator(parentLocator, Locator::COMP_ALL);

    newBeamLocator->setLocalOffsets(pos, ori, update.time(), false);

    LocatorNode* locatorNode = new LocatorNode(newBeamLocator);
    locatorNode->addChild(volume.get());

    historyNodes_[update.time()] = locatorNode;
  }

  static const double historyInSeconds = 10.; // TODO: Make user configurable
  // Update which history nodes are displayed based on the current time
  removeChildren(0, getNumChildren());

  // If all points wouldn't be displayed due to being out of the time window,
  // show the newest one with the full color. TODO: need to improve this bit,
  // as what we want to show will depend on the beam type (and may possibly
  // be independent of the draw and data draw states
  if (!historyNodes_.empty() && historyNodes_.rbegin()->first < (time - historyInSeconds))
  {
    const auto& iter = historyNodes_.rbegin();
    addChild(iter->second);
    osg::ref_ptr<BeamVolume> bv = dynamic_cast<BeamVolume*>(iter->second->asGroup()->getChild(0));
    if (bv)
    {
      // TODO: preserve color at each history point rather than overwriting with current color SIM-13559
      simData::BeamPrefs newPrefs(prefs);
      Color color = Color(prefs.commonprefs().color());
      newPrefs.mutable_commonprefs()->set_color(color.asABGR());
      bv->performInPlacePrefChanges(&prefs, &newPrefs);
    }
    return;
  }

  for (const auto& iter : historyNodes_)
  {
    if (iter.first > time)
      continue; // In the future
    else if (iter.first < (time - historyInSeconds))
      continue; // Too old

    addChild(iter.second);
    osg::ref_ptr<BeamVolume> bv = dynamic_cast<BeamVolume*>(iter.second->asGroup()->getChild(0));
    if (bv)
    {
      // TODO: preserve color at each history point rather than overwriting with current color SIM-13559
      simData::BeamPrefs newPrefs(prefs);
      Color color = Color(prefs.commonprefs().color());
      color.a() = 1. - ((time - iter.first) / historyInSeconds);
      newPrefs.mutable_commonprefs()->set_color(color.asABGR());
      bv->performInPlacePrefChanges(&prefs, &newPrefs);
    }
  }
}

//-------------------------------------------------------------------

PlanetariumViewTool::PlanetariumViewTool(PlatformNode* host) :
  host_(host),
  range_(1000.0),
  domeColor_(0.8f, 1.0f, 0.8f, 0.5f), // RGBA
  displayTargetVectors_(true),
  displayBeamHistory_(false),
  displayGates_(false)
{
  family_.reset();

  // the geofence will filter out visible objects
  fence_ = new HorizonGeoFence();

  // build the geometry for a target node
  osgEarth::LineDrawable* geom = new osgEarth::LineDrawable(GL_LINES);
  geom->allocate(4);
  geom->setColor(simVis::Color::White);
  geom->setLineWidth(2.0f);
  geom->setDataVariance(osg::Object::DYNAMIC);
  targetGeom_ = geom;

  scaleTargetGeometry_(range_);
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

    // clear all beam history
    for (const auto& hist : history_)
      root_->removeChild(hist.second.get());

    updateDome_();

    // rescale the one geode that is reused for all target delegates
    scaleTargetGeometry_(range_);

    // recreate our target delegates
    if (scenario_.valid() && targets_.valid())
    {
      EntityVector entities;
      scenario_->getAllEntities(entities);
      for (EntityVector::const_iterator i = entities.begin(); i != entities.end(); ++i)
      {
        PlatformNode* platform = dynamic_cast<PlatformNode*>(i->get());
        if (platform != nullptr && platform != host_.get() && platform->isActive())
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

void PlanetariumViewTool::setDisplayBeamHistory(bool display)
{
  displayBeamHistory_ = display;
  if (!displayBeamHistory_)
  {
    for (const auto& hist : history_)
      root_->removeChild(hist.second.get());
    // Don't clear the history, can be recalled later
  }
}

bool PlanetariumViewTool::getDisplayBeamHistory() const
{
  return displayBeamHistory_;
}

void PlanetariumViewTool::setDisplayGates(bool display)
{
  displayGates_ = display;
  applyOverrides_(true);
}

bool PlanetariumViewTool::getDisplayGates() const
{
  return displayGates_;
}

void PlanetariumViewTool::onInstall(const ScenarioManager& scenario)
{
  root_ = new osg::Group;

  // create a node to track the position of the host:
  locatorRoot_ = new LocatorNode(new Locator(host_->getLocator(), Locator::COMP_POSITION));
  locatorRoot_->setName("Planetarium Tool Root Node");

  root_->addChild(locatorRoot_.get());

  // build the dome
  updateDome_();

  // reset the delegate graph.
  targets_ = new TargetDelegation();
  targets_->setGeoFence(fence_.get());
  targets_->addUpdateGeometryCallback(new UpdateGeometryAdapter(this));
  locatorRoot_->addChild(targets_.get());

  // state for the delegation group:
  simVis::setLighting(targets_->getOrCreateStateSet(), 0);

  // sets horizon geofence to host position, which does not work correctly
  simCore::Vec3 ecef;
  locatorRoot_->getPosition(&ecef);
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

  if (targets_.valid())
    targets_->removeChildren(0, targets_->getNumChildren());

  // scenario has already removed us from the scenegraph
  locatorRoot_ = nullptr;
  targets_ = nullptr;
  dome_ = nullptr;
  root_ = nullptr;
}

void PlanetariumViewTool::onEntityAdd(const ScenarioManager& scenario, EntityNode* entity)
{
  if (family_.invite(entity))
  {
    applyOverrides_(entity, true);

    osg::ref_ptr<simVis::BeamNode> beam = dynamic_cast<simVis::BeamNode*>(entity);
    if (beam.get())
    {
      osg::ref_ptr<BeamHistory> history = new BeamHistory(beam);
      history_[beam->getId()] = history;
      root_->addChild(history.get());
    }
  }
}

void PlanetariumViewTool::onEntityRemove(const ScenarioManager& scenario, EntityNode* entity)
{
  if (family_.dismiss(entity))
  {
    applyOverrides_(entity, false);
    osg::ref_ptr<simVis::BeamNode> beam = dynamic_cast<simVis::BeamNode*>(entity);
    if (beam.get() && history_.find(beam->getId()) != history_.end())
    {
      auto history = history_.find(beam->getId())->second;
      root_->removeChild(history);
      history_.erase(beam->getId());
    }
  }
  else if (dynamic_cast<PlatformNode*>(entity))
  {
    targets_->remove(static_cast<PlatformNode*>(entity));
  }
}

void PlanetariumViewTool::onUpdate(const ScenarioManager& scenario, const simCore::TimeStamp& timeStamp, const EntityVector& updates)
{
  // update the fence
  fence_->setLocation(osg::Vec3d(0, 0, 0) * locatorRoot_->getMatrix());

  for (EntityVector::const_iterator i = updates.begin(); i != updates.end(); ++i)
  {
    // Update beam node history
    BeamNode* beam = dynamic_cast<BeamNode*>(i->get());
    if (beam)
    {

      if (family_.isMember(beam->getId()) && displayBeamHistory_)
      {
        auto historyIter = history_.find(beam->getId());
        osg::ref_ptr<BeamHistory> history;
        if (historyIter == history_.end())
        {
          history = new BeamHistory(beam);
          history_[beam->getId()] = history;
          root_->addChild(history.get());
        }
      }
      continue;
    }

    // check any entity updates for positional changes
    PlatformNode* platform = dynamic_cast<PlatformNode*>(i->get());
    if (!platform || platform == host_.get())
      continue;
    if (platform->isActive())
      targets_->addOrUpdate(platform);
    else
      targets_->remove(platform);
  }

  for (const auto& iter : history_)
    iter.second->updateBeamHistory(timeStamp.secondsSinceRefYear(), range_);

  setDirty(); // Force an update each time scenario manager updates
}

void PlanetariumViewTool::updateTargetGeometry(osg::MatrixTransform* mt,
                                               const osg::Vec3d&     ecef)
{
  static osg::Vec3d s_up(0.0, 0.0, 1.0);

  // if the transform has no children, create the initial subgraph.
  if (mt->getNumChildren() == 0)
  {
    mt->addChild(targetGeom_.get());
    mt->addChild(buildVectorGeometry_());
  }

  // transform the target position into planetarium-local space:
  osg::Vec3d local = ecef * locatorRoot_->getInverseMatrix();
  double     local_len = local.length();
  osg::Vec3d local_n   = local / local_len;

  // update the vector.
  osgEarth::LineDrawable* vector = static_cast<osgEarth::LineDrawable*>(mt->getChild(1));
  vector->setNodeMask(displayTargetVectors_ ? ~0 : 0);
  vector->setVertex(1, s_up * (local_len - range_));

  osg::Vec3d V(s_up * (local_len - range_));
  for (unsigned int i = 1; i < NUM_VECTOR_SEGS; ++i)
  {
    const double t = static_cast<double>(i) / static_cast<double>(NUM_VECTOR_SEGS - 1);
    vector->setVertex(i, V * t);
  }

  // create the target vector and scale it to the dome's surface.
  mt->setMatrix(
    osg::Matrix::rotate(s_up, local_n) *
    osg::Matrix::translate(local_n * range_));
}

void PlanetariumViewTool::updateDome_()
{
  if (locatorRoot_.valid())
  {
    if (dome_.valid())
      locatorRoot_->removeChild(dome_.get());

    // build a sphere
    osg::Geometry* drawable = osgEarth::AnnotationUtils::createEllipsoidGeometry(range_, range_, range_, domeColor_);
    osg::StateSet* stateSet = drawable->getOrCreateStateSet();
    stateSet->setMode(GL_BLEND, 1);
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0, 1, false));
    locatorRoot_->addChild(drawable);
    dome_ = drawable;
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
    if (enable && displayGates_)
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
  }
}

void PlanetariumViewTool::scaleTargetGeometry_(double range) const
{
  // the graphic used for target delegates is scaled based on range (planetarium radius), this might be a dimension in meters
  // this formula for calculating s is purely trial-and-error, intended to maintain a minimum size at low range, but scale slowly with increasing range.
  const float s = static_cast<float>(20.0 + range / 60.0);

  osgEarth::LineDrawable* geom = static_cast<osgEarth::LineDrawable*>(targetGeom_.get());
  geom->setVertex(0, osg::Vec3(-s, -s, 0.0f));
  geom->setVertex(1, osg::Vec3( s,  s, 0.0f));
  geom->setVertex(2, osg::Vec3(-s,  s, 0.0f));
  geom->setVertex(3, osg::Vec3( s, -s, 0.0f));
}

osg::Node* PlanetariumViewTool::buildVectorGeometry_()
{
  osgEarth::LineDrawable* geom = new osgEarth::LineDrawable(GL_LINE_STRIP);
  geom->allocate(NUM_VECTOR_SEGS);
  geom->setColor(simVis::Color::White);
  geom->setDataVariance(osg::Object::DYNAMIC);
  return geom;
}

}

