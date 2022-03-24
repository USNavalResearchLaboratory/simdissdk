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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <algorithm>
#include "osg/Depth"
#include "osg/CullFace"
#include "osg/TransferFunction"
#include "osgEarth/LineDrawable"
#include "osgEarth/AnnotationUtils"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Time/TimeClass.h"
#include "simData/MemoryDataStore.h"
#include "simData/LimitData.h"
#include "simVis/Beam.h"
#include "simVis/DisableDepthOnAlpha.h"
#include "simVis/Gate.h"
#include "simVis/GeoFence.h"
#include "simVis/LocatorNode.h"
#include "simVis/Platform.h"
#include "simVis/Projector.h"
#include "simVis/Scenario.h"
#include "simVis/Shaders.h"
#include "simVis/SphericalVolume.h"
#include "simVis/TargetDelegation.h"
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
// the planetarium moves with host position only.
// all beams and gates must use locators to dynamically track their host position,
// in order for them to move with their host (and with the planetarium.)
//
// in this implementation,
// a beam history point is intended to fix that spot on the planetarium
// that the beam painted at a specific time; that spot does not move relative to the host,
// regardless of host orientation changes.

PlanetariumViewTool::BeamHistory::BeamHistory(simVis::BeamNode* beam, simData::DataStore& ds, double range)
  : osg::Group(),
  beam_(beam),
  historyLength_(10.0),
  useGradient_(false),
  limitingData_(ds.dataLimiting()),
  firstTime_(std::numeric_limits<double>::max()),
  range_(range),
  lastUpdateTime_(-std::numeric_limits<double>::max())
{
  beamUpdateSlice_ = ds.beamUpdateSlice(beam_->getId());
  if (!beamUpdateSlice_)
  {
    // beam must have an update to be here
    assert(0);
    return;
  }
  firstTime_ = beamUpdateSlice_->firstTime();
  beamCommandSlice_ = ds.beamCommandSlice(beam_->getId());
  if (!beamCommandSlice_)
  {
    // beam must have a cmd slice to be here
    assert(0);
    return;
  }
}

PlanetariumViewTool::BeamHistory::~BeamHistory()
{
}

void PlanetariumViewTool::BeamHistory::updateBeamHistory(double time)
{
  // remove all this beam's history from scenegraph
  removeChildren(0, getNumChildren());
  if (!beam_.valid())
  {
    // probably this can't happen; but maybe when a beam is about to be deleted.
    return;
  }
  const simData::BeamPrefs& prefs = beam_->getPrefs();
  if (!prefs.commonprefs().draw())
  {
    // ensure that history is correctly limited relative to current prefs, then exit
    applyDataLimiting_(prefs);
    return;
  }

  // assumes that time is moving forward, need to think through what happens if time moves backward

  if (time > lastUpdateTime_)
  {
    // add all points in (lastUpdateTime_, time]
    backfill_(lastUpdateTime_, time);
    lastUpdateTime_ = time; // remember time after updating, to be used next time on backfill
  }
  applyDataLimiting_(prefs);

  if (historyPoints_.empty())
    return;

  simVis::Color color;
  // initialize color to reasonable values
  if (prefs.commonprefs().useoverridecolor())
  {
    // if beam override color is active, it overrides all history points (when not using gradient)
    color = Color(prefs.commonprefs().overridecolor(), osgEarth::Color::RGBA);
  }
  else
  {
    color = historyPoints_.crbegin()->second->color;
    if (color == NO_COMMANDED_COLOR)
      color = Color(prefs.commonprefs().color(), osgEarth::Color::RGBA);
  }
  // use initial color to initialize alpha for fading/gradient alpha
  const float origAlpha = color.a();

  for (const auto& iter : historyPoints_)
  {
    if (iter.first > time)
      continue; // In the future
    // historyLength_ == 0 means no limiting by history
    else if (historyLength_ != 0 && iter.first < (time - historyLength_))
      continue; // Too old

    osg::ref_ptr<BeamVolume> bv = dynamic_cast<BeamVolume*>(iter.second->node->asGroup()->getChild(0));
    if (!bv)
    {
      // can't be a history point without a beam volume
      assert(0);
      return;
    }
    // addPointFromUpdate_ guarantees that nodemask is set correctly
    assert(iter.second->node->getNodeMask() == simVis::DISPLAY_MASK_BEAM);

    // add to the scenegraph
    addChild(iter.second->node);

    float divisor = historyLength_;
    if (historyLength_ == 0)
    {
      if (firstTime_ != std::numeric_limits<double>::max())
        divisor = time - firstTime_;
      else
        divisor = time; // fall back to time as divisor, prevents errors/crashes
    }
    if (divisor == 0)
      divisor = 1.0; // ensure divide by zero doesn't happen
    const float zeroToOne = (1. - ((time - iter.first) / divisor));
    // Use color from history point to ensure color history is preserved
    if (useGradient_)
    {
      // if useGradient_ is set, ignore beam override color
      if (gradientFunction_ == nullptr)
        initGradient_();
      color = gradientFunction_->getColor(zeroToOne);
      color.a() = origAlpha;
    }
    else
    {
      if (!prefs.commonprefs().useoverridecolor() &&
        iter.second->color != NO_COMMANDED_COLOR)
      {
        // use commanded color when it is set and override is not active
        color = iter.second->color;
      }
      // else, color has already been set (once) before loop

      // this code must guarantee this; NO_COMMANDED_COLOR must always be replaced with a valid color
      assert(color != NO_COMMANDED_COLOR);

      // Fade the alpha based on the point's age and based on the current color's alpha
      color.a() = zeroToOne * origAlpha;
    }

    SVFactory::updateColor(bv, color);
    if (!iter.second->hasCommandedHbw)
    {
      int hbwStatus = SVFactory::updateHorizAngle(bv, prefs.horizontalwidth());
      // TODO: what to do if this fails? recreate beam history with new hbw?
    }
    if (!iter.second->hasCommandedVbw)
      SVFactory::updateVertAngle(bv, prefs.verticalwidth());
  }
}

void PlanetariumViewTool::BeamHistory::backfill_(double lastTime, double currentTime)
{
  if (!beamUpdateSlice_)
  {
    // slice_ must be valid
    assert(0);
    return;
  }

  // starting point is: each point sets hbw, vbw & color to sentinel value;
  // only set to non-sentinel value if a command is found below
  // updateBeamHistory replaces sentinel with current pref if no command found
  double hbw = NO_COMMANDED_BEAMWIDTH;
  double vbw = NO_COMMANDED_BEAMWIDTH;
  simVis::Color color = NO_COMMANDED_COLOR;

  // prepare the prefs for all points being added
  const simData::BeamPrefs& prefs = beam_->getPrefs();
  simData::BeamPrefs pointPrefs(prefs);
  pointPrefs.mutable_commonprefs()->set_useoverridecolor(false);
  pointPrefs.set_blended(true);
  pointPrefs.set_drawtype(simData::BeamPrefs_DrawType_COVERAGE);

  // Declared outside for loop so we can continue iteration after finding nearly-recent command
  auto commandIter = beamCommandSlice_->lower_bound(-1.0);
  // iterate once to find hbw, vbw & color commands up to lastTime
  for (; commandIter.peekNext() != nullptr; commandIter.next())
  {
    auto next = commandIter.peekNext();
    if (next->time() > lastTime)
      break;
    if (!next->has_updateprefs())
      continue;
    if (next->updateprefs().has_horizontalwidth())
      hbw = next->updateprefs().horizontalwidth();
    if (next->updateprefs().has_verticalwidth())
      vbw = next->updateprefs().verticalwidth();
    if (next->updateprefs().commonprefs().has_color())
      color = simVis::Color(next->updateprefs().commonprefs().color(), osgEarth::Color::RGBA);
  }

  // add all data points from after lastTime to/including currentTime, if the range
  auto updateIter = beamUpdateSlice_->upper_bound(lastTime);
  while (updateIter.hasNext())
  {
    const simData::BeamUpdate* update = updateIter.next();
    if (update->time() > currentTime)
      break;

    if (update->range() < range_)
      continue;

    // determine if there is a new command for this update's time
    for (; commandIter.peekNext() != nullptr; commandIter.next())
    {
      auto next = commandIter.peekNext();
      if (next->time() > update->time())
        break;
      if (!next->has_updateprefs())
        continue;
      if (next->updateprefs().has_horizontalwidth())
        hbw = next->updateprefs().horizontalwidth();
      if (next->updateprefs().has_verticalwidth())
        vbw = next->updateprefs().verticalwidth();
      if (next->updateprefs().commonprefs().has_color())
        color = simVis::Color(next->updateprefs().commonprefs().color(), osgEarth::Color::RGBA);
    }

    const bool hasCommandedHbw = (hbw != NO_COMMANDED_BEAMWIDTH);
    if (hasCommandedHbw)
      pointPrefs.set_horizontalwidth(hbw);
    else
      pointPrefs.set_horizontalwidth(prefs.horizontalwidth());
    const bool hasCommandedVbw = (vbw != NO_COMMANDED_BEAMWIDTH);
    if (hasCommandedVbw)
      pointPrefs.set_verticalwidth(vbw);
    else
      pointPrefs.set_verticalwidth(prefs.verticalwidth());
    addPointFromUpdate_(pointPrefs, hasCommandedHbw, hasCommandedVbw, color, update, update->time());
  }
}

void PlanetariumViewTool::BeamHistory::addPointFromUpdate_(const simData::BeamPrefs& prefs,
  bool hasCommandedHbw, bool hasCommandedVbw, const simVis::Color& color,
  const simData::BeamUpdate* update, double updateTime)
{
  if (historyPoints_[updateTime].get())
  {
    // already have this point; but this should not happen
    assert(0);
    return;
  }
  Locator* beamOrientationLocator = beam_->getLocator();

  // inherit only the dynamic resolved position of the beam origin
  // this includes beam-position-offsets; but see note below.
  // dynamic because planetarium is always relative to current host position.
  // this locator establishes that beam origin position and adds historical beam az/el as an offset.
  // (BeamVolume adds range)
  Locator* beamHistoryPointLocator = new Locator(beamOrientationLocator, Locator::COMP_POSITION);

  // offset prefs (position and orientation) are not implemented as commands, and do not have history:
  // points will retain the offsets as set when point is created, but
  // if offsets are changed, the new value and the old value may be used in unexpected ways
  // depending on how points are added - if backfilling a large interval of points,
  // the current values will be applied to the entire interval of points.

  // ori offset beam implementation: ori offset should only be applied if useoffsetbeam is set
  // beam orientation offsets are simply added to beam az/el data; they are not processed as a separate modeling transformation
  const simCore::Vec3& beamOrientation = (prefs.useoffsetbeam()) ?
    simCore::Vec3(update->azimuth() + prefs.azimuthoffset(), update->elevation() + prefs.elevationoffset(), prefs.rolloffset())
    : simCore::Vec3(update->azimuth(), update->elevation(), 0.0);

  beamHistoryPointLocator->setLocalOffsets(simCore::Vec3(), beamOrientation, updateTime, true);

  simData::BeamUpdate newUpdate(*update);
  newUpdate.set_range(range_);

  osg::ref_ptr<BeamVolume> volume = new BeamVolume(prefs, newUpdate);
  LocatorNode* beamHistoryPointLocatorNode = new LocatorNode(beamHistoryPointLocator, volume.get());

  std::unique_ptr<HistoryPoint> newPoint(new HistoryPoint);
  newPoint->node = beamHistoryPointLocatorNode;
  newPoint->node->setNodeMask(simVis::DISPLAY_MASK_BEAM);
  newPoint->color = color;
  newPoint->hasCommandedHbw = hasCommandedHbw;
  newPoint->hasCommandedVbw = hasCommandedVbw;
  historyPoints_[updateTime] = std::move(newPoint);
}

void PlanetariumViewTool::BeamHistory::setHistoryLength(double historyLength)
{
  // No need to trigger update, caller will do so
  historyLength_ = historyLength;
}

void PlanetariumViewTool::BeamHistory::setUseGradient(bool useGradient)
{
  // No need to trigger update, caller will do so
  useGradient_ = useGradient;
}

void PlanetariumViewTool::BeamHistory::setRange(double range)
{
  if (range_ == range)
    return;
  range_ = range;

  // wipe history, reset times, rebuild
  historyPoints_.clear();
  firstTime_ = beamUpdateSlice_->firstTime();
  const double time = lastUpdateTime_;
  lastUpdateTime_ = -std::numeric_limits<double>::max();
  updateBeamHistory(time);
}

void PlanetariumViewTool::BeamHistory::applyDataLimiting_(const simData::BeamPrefs& prefs)
{
  if (!limitingData_ || historyPoints_.empty())
    return;
  simData::limitData<std::unique_ptr<HistoryPoint> >(historyPoints_, prefs.commonprefs().datalimittime(), prefs.commonprefs().datalimitpoints());
  // data limiting always leaves at least one point in a non-empty container
  assert(historyPoints_.size() >= 1);
  firstTime_ = historyPoints_.begin()->first;
}

void PlanetariumViewTool::BeamHistory::initGradient_()
{
  if (gradientFunction_ != nullptr)
    return;
  gradientFunction_ = new osg::TransferFunction1D;
  auto& map = gradientFunction_->getColorMap();
  map[0.00f] = osg::Vec4(0.f, 0.f, 1.f, 1.f); // blue
  map[0.25f] = osg::Vec4(0.f, 1.f, 1.f, 1.f); // cyan
  map[0.50f] = osg::Vec4(0.f, 1.f, 0.f, 1.f); // green
  map[0.75f] = osg::Vec4(1.f, 1.f, 0.f, 1.f); // yellow
  map[1.00f] = osg::Vec4(1.f, 0.f, 0.f, 1.f); // red
}

//-------------------------------------------------------------------

/** Calls a lambda function when preferences change. */
class PrefsChangeLambda : public simData::DataStore::DefaultListener
{
public:
  PrefsChangeLambda(const std::function<void()>& func, simData::ObjectId id)
    : lambda_(func),
    idOfInterest_(id)
  {
  }

  virtual void onPrefsChange(simData::DataStore* source, simData::ObjectId id)
  {
    if (id == idOfInterest_)
      lambda_();
  }

private:
  std::function<void()> lambda_;
  simData::ObjectId idOfInterest_;
};

/**
 * Encapsulates the update, remove, and prefs detection logic for draping a projector on the
 * Planetarium dome, watching for commonPrefs.acceptProjectorId changes on the host. Implemented
 * as a standalone struct to increase cohesion and reduce scattered code in Planetarium.
 */
class PlanetariumViewTool::ProjectorMonitor
{
public:
  explicit ProjectorMonitor(PlanetariumViewTool& parent)
    : parent_(parent)
  {
    if (parent_.host_.valid())
      hostId_ = parent_.host_->getId();

    prefsChange_.reset(new PrefsChangeLambda([this]() { checkForPrefsUpdate(); }, hostId_));
    parent_.ds_.addListener(prefsChange_);
    // Do an initial check
    checkForPrefsUpdate();
  }

  /** Uninstall the projection code on destruction */
  virtual ~ProjectorMonitor()
  {
    parent_.ds_.removeListener(prefsChange_);
    for (const auto& projectorNode : projectorNodes_)
    {
      if (projectorNode.valid())
        projectorNode->removeProjectionFromNode(parent_.root_.get());
    }
  }

  /**
   * Call this once per update, to monitor for changes in the prefs. This could be optimized
   * by only being called when prefs from the planetarium platform host change, or if we
   * knew when the acceptprojectorid() field changes. Automatically called by data store listener.
   */
  void checkForPrefsUpdate()
  {
    // Need a scenario, or all work below is useless (and can be delayed until there is a scenario)
    if (!parent_.scenario_.valid() || !parent_.root_.valid())
      return;

    std::vector<simData::ObjectId> newProjIds;
    { // Transaction minimal scope
      simData::DataStore::Transaction txn;
      const auto* prefs = parent_.ds_.platformPrefs(hostId_, &txn);
      if (prefs)
      {
        newProjIds = simData::DataStoreHelpers::vecFromRepeated(prefs->commonprefs().acceptprojectorids());
        // Remove "0" entries, which might be present for Commands
        newProjIds.erase(std::remove(newProjIds.begin(), newProjIds.end(), 0), newProjIds.end());
      }
    }

    // Did the Accepts Projector pref change on the host?
    if (newProjIds == projectorIds_)
      return;

    projectorIds_ = newProjIds;
    // Remove old projections
    for (const auto& projectorNode : projectorNodes_)
    {
      if (projectorNode.valid())
        projectorNode->removeProjectionFromNode(parent_.root_.get());
    }
    projectorNodes_.clear();

    // Try to re-add projection from nodes
    for (const auto& projectorId : projectorIds_)
    {
      auto* projectorNode = parent_.scenario_->find<simVis::ProjectorNode>(projectorId);
      if (projectorNode)
      {
        projectorNode->addProjectionToNode(parent_.root_.get(), parent_.root_.get());
        projectorNodes_.emplace_back(projectorNode);
      }
    }
  }

  /** Forward calls from PlanetariumViewTool::onEntityAdd() here. */
  void notifyNewEntity(EntityNode* entity)
  {
    if (!entity || entity->type() != simData::PROJECTOR)
      return;
    if (std::find(projectorIds_.begin(), projectorIds_.end(), entity->getId()) == projectorIds_.end())
      return;
    auto* projectorNode = dynamic_cast<ProjectorNode*>(entity);
    if (projectorNode)
    {
      projectorNode->addProjectionToNode(parent_.root_.get(), parent_.root_.get());
      projectorNodes_.emplace_back(projectorNode);
    }
  }

  /** Forward calls from PlanetariumViewTool::onEntityRemove() here. */
  void notifyRemoveEntity(EntityNode* entity)
  {
    if (!entity || entity->type() != simData::PROJECTOR)
      return;
    // Remove from projectorNodes_ if it was in there, pruning nulls as needed
    auto iter = projectorNodes_.begin();
    while (iter != projectorNodes_.end())
    {
      if (!(*iter).valid())
      {
        // pruned null
        iter = projectorNodes_.erase(iter);
      }
      else if ((*iter).get() == entity)
      {
        // Remove entry, but first remove the projection from our node
        auto* projectorNode = dynamic_cast<ProjectorNode*>(entity);
        if (projectorNode)
          projectorNode->removeProjectionFromNode(parent_.root_.get());

        iter = projectorNodes_.erase(iter);
      }
      else
        ++iter;
    }
  }

private:
  PlanetariumViewTool& parent_;
  simData::ObjectId hostId_ = 0;
  std::vector<simData::ObjectId> projectorIds_;
  std::vector<osg::observer_ptr<simVis::ProjectorNode> > projectorNodes_;
  std::shared_ptr<PrefsChangeLambda> prefsChange_;
};

//-------------------------------------------------------------------

PlanetariumViewTool::PlanetariumViewTool(PlatformNode* host, simData::DataStore& ds)
  : host_(host),
  ds_(ds),
  range_(1000.0),
  domeColor_(0.8f, 1.0f, 0.8f, 0.5f), // RGBA
  displayTargetVectors_(true),
  displayBeamHistory_(false),
  displayGates_(false),
  historyLength_(10.),
  lastUpdateTime_(-1.),
  useGradient_(false),
  useSector_(false),
  sectorAzDeg_(0.),
  sectorElDeg_(0.),
  sectorWidthDeg_(90.),
  sectorHeightDeg_(60.)
{
  family_.reset();
  // Add all initial textures
  for (int k = 0; k <= static_cast<int>(TextureUnit::UNIT3); ++k)
    textures_.push_back(TextureData());

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

PlanetariumViewTool::~PlanetariumViewTool()
{
}

osg::Node* PlanetariumViewTool::getNode() const
{
  return root_.get();
}

void PlanetariumViewTool::setRange(double range)
{
  if (range_ == range)
    return;

  // Remember the new range even if root is not valid
  range_ = range;

  if (!root_.valid())
    return;

  // clear all target delegates
  if (targets_.valid())
    targets_->removeAll();

  if (displayBeamHistory_)
  {
    // update all beam history
    for (const auto& hist : history_)
      hist.second->setRange(range_);
  }

  updateDome_();

  // rescale the one target geometry that is reused for all target delegates
  scaleTargetGeometry_(range_);

  // recreate our target delegates
  if (scenario_.valid() && targets_.valid())
  {
    EntityVector entities;
    scenario_->getAllEntities(entities);
    for (EntityVector::const_iterator i = entities.begin(); i != entities.end(); ++i)
    {
      const PlatformNode* platform = dynamic_cast<PlatformNode*>(i->get());
      if (platform != nullptr && platform != host_.get() && platform->isActive())
        targets_->addOrUpdate(platform);
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
  if (displayBeamHistory_ == display)
    return;

  displayBeamHistory_ = display;
  for (const auto& hist : history_)
  {
    if (displayBeamHistory_)
    {
      root_->addChild(hist.second.get());
      // ensure that beam history has current params; it is not current when not displayed
      hist.second->setHistoryLength(historyLength_);
      hist.second->setUseGradient(useGradient_);
      hist.second->setRange(range_);
      hist.second->updateBeamHistory(lastUpdateTime_);
    }
    else
      root_->removeChild(hist.second.get());
  }
  // Don't clear the history, can be recalled later
}

bool PlanetariumViewTool::getDisplayBeamHistory() const
{
  return displayBeamHistory_;
}

void PlanetariumViewTool::setBeamHistoryLength(double history)
{
  if (historyLength_ == history)
    return;
  historyLength_ = history;
  if (displayBeamHistory_)
  {
    for (const auto& hist : history_)
    {
      hist.second->setHistoryLength(historyLength_);
      // Trigger an update to the last update time to fix the history to the new length
      hist.second->updateBeamHistory(lastUpdateTime_);
    }
  }
}

double PlanetariumViewTool::getBeamHistoryLength() const
{
  return historyLength_;
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

void PlanetariumViewTool::setUseGradient(bool useGradient)
{
  if (useGradient_ == useGradient)
    return;
  useGradient_ = useGradient;
  if (displayBeamHistory_)
  {
    for (const auto& hist : history_)
    {
      hist.second->setUseGradient(useGradient_);
      // Trigger an update to the last update time to fix the history to the new colors
      hist.second->updateBeamHistory(lastUpdateTime_);
    }
  }
}

bool PlanetariumViewTool::useGradient() const
{
  return useGradient_;
}

void PlanetariumViewTool::setUseSector(bool useSector)
{
  if (useSector_ == useSector)
    return;
  useSector_ = useSector;
  updateDome_();
}

bool PlanetariumViewTool::getUseSector() const
{
  return useSector_;
}

void PlanetariumViewTool::setSectorAzimuth(double azDeg)
{
  if (sectorAzDeg_ == azDeg)
    return;
  sectorAzDeg_ = simCore::angFix360(azDeg);
  updateDome_();
}

double PlanetariumViewTool::getSectorAzimuth() const
{
  return sectorAzDeg_;
}

void PlanetariumViewTool::setSectorElevation(double elDeg)
{
  if (sectorElDeg_ == elDeg)
    return;
  sectorElDeg_ = simCore::clamp(elDeg, 0.01, 90.);
  updateDome_();
}

double PlanetariumViewTool::getSectorElevation() const
{
  return sectorElDeg_;
}

void PlanetariumViewTool::setSectorWidth(double widthDeg)
{
  if (sectorWidthDeg_ == widthDeg)
    return;
  sectorWidthDeg_ = simCore::clamp(widthDeg, 0.01, 360.0);
  updateDome_();
}

double PlanetariumViewTool::getSectorWidth() const
{
  return sectorWidthDeg_;
}

void PlanetariumViewTool::setSectorHeight(double heightDeg)
{
  if (sectorHeightDeg_ == heightDeg)
    return;
  sectorHeightDeg_ = simCore::clamp(heightDeg, 0.01, 180.);
  updateDome_();
}

double PlanetariumViewTool::getSectorHeight() const
{
  return sectorHeightDeg_;
}

void PlanetariumViewTool::onInstall(const ScenarioManager& scenario)
{
  root_ = new osg::Group;
  root_->setName("Planetarium Tool Root Node");
  root_->getOrCreateStateSet()->setRenderBinDetails(BIN_AZIM_ELEV_TOOL, BIN_GLOBAL_SIMSDK);
  simVis::DisableDepthOnAlpha::setValues(root_->getOrCreateStateSet(), osg::StateAttribute::OFF);

  // create a node to track the position of the host:
  locatorRoot_ = new LocatorNode(new Locator(host_->getLocator(), Locator::COMP_POSITION));
  locatorRoot_->setName("Planetarium Dome Root Node");
  // Turn off cull face, so that both sides of the planetarium get drawn, in order for projectors
  // to render properly on both sides.
  locatorRoot_->getOrCreateStateSet()->setMode(GL_CULL_FACE, 0);

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
  fence_->setLocation(locatorRoot_->getMatrix().getTrans());

  // initial pull of active target platforms
  EntityVector entities;
  scenario.getAllEntities(entities);

  // collect the entity list from the scenario
  family_.reset();
  family_.add(scenario, host_->getId());

  const simCore::TimeStamp dsTimeStamp(ds_.referenceYear(), ds_.updateTime());
  onUpdate(scenario, dsTimeStamp, entities);

  // install all overrides
  applyOverrides_(true);

  // cache the scenario pointer
  scenario_ = &scenario;

  // Configure projection
  projectorMonitor_.reset(new ProjectorMonitor(*this));
}

void PlanetariumViewTool::onUninstall(const ScenarioManager& scenario)
{
  projectorMonitor_.reset();
  // disable all overrides
  applyOverrides_(false);
  family_.reset();
  history_.clear();
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
      addBeamToBeamHistory_(beam);
  }

  if (projectorMonitor_)
    projectorMonitor_->notifyNewEntity(entity);
}

void PlanetariumViewTool::onEntityRemove(const ScenarioManager& scenario, EntityNode* entity)
{
  if (family_.dismiss(entity))
  {
    applyOverrides_(entity, false);
    flushFamilyEntity_(entity);
  }
  else if (dynamic_cast<PlatformNode*>(entity))
  {
    targets_->remove(static_cast<PlatformNode*>(entity));
  }

  if (projectorMonitor_)
    projectorMonitor_->notifyRemoveEntity(entity);
}

void PlanetariumViewTool::onUpdate(const ScenarioManager& scenario, const simCore::TimeStamp& timeStamp, const EntityVector& updates)
{
  // update the fence
  fence_->setLocation(locatorRoot_->getMatrix().getTrans());

  lastUpdateTime_ = timeStamp.secondsSinceRefYear();

  for (auto entityNode : updates)
  {
    // Update beam node history
    BeamNode* beam = dynamic_cast<BeamNode*>(entityNode.get());
    if (beam)
    {
      if (family_.isMember(beam->getId()))
      {
        // revisit current beams: enable ones that now qualify, disable ones that don't have range, etc.
        applyOverrides_(beam, true);
        addBeamToBeamHistory_(beam);
      }
      continue;
    }

    // check any entity updates for positional changes
    PlatformNode* platform = dynamic_cast<PlatformNode*>(entityNode.get());
    if (!platform || platform == host_.get())
      continue;
    if (platform->isActive())
      targets_->addOrUpdate(platform);
    else
      targets_->remove(platform);
  }
  if (displayBeamHistory_)
  {
    for (const auto& iter : history_)
      iter.second->updateBeamHistory(lastUpdateTime_);
  }

  // Force a call to this method next time scenario manager updates, even if there are no EntityVector updates
  setDirty();
}

void PlanetariumViewTool::onFlush(const ScenarioManager& scenario, simData::ObjectId flushedId)
{
  if (flushedId == 0)
  {
    // scenario flush: clear all beam history
    for (const auto& entityObsPtr : family_.members())
      flushFamilyEntity_(entityObsPtr.get());

    // clear all target delegates
    targets_->removeAll();
    return;
  }

  const EntityNode* entity = scenario_->find(flushedId);
  if (dynamic_cast<const PlatformNode*>(entity))
    targets_->remove(static_cast<const PlatformNode*>(entity));
  else if (family_.isMember(flushedId))
    flushFamilyEntity_(entity);
}

// common code for flush and remove
void PlanetariumViewTool::flushFamilyEntity_(const EntityNode* entity)
{
  const simVis::BeamNode* beam = dynamic_cast<const simVis::BeamNode*>(entity);
  if (beam)
  {
    const auto historyIter = history_.find(beam->getId());
    if (historyIter != history_.end())
    {
      // remove history from scenegraph, will re-add if entity gets a new update
      root_->removeChild(historyIter->second.get());
      history_.erase(beam->getId());
    }
  }
}

void PlanetariumViewTool::updateTargetGeometry(osg::MatrixTransform* mt,
                                               const osg::Vec3d& ecef) const
{
  const static osg::Vec3d s_up(0.0, 0.0, 1.0); // z_axis

  // if the transform has no children, create the initial subgraph.
  if (mt->getNumChildren() == 0)
  {
    mt->addChild(targetGeom_.get());      // simple cross is the target delegate (on planetarium surface)
    mt->addChild(buildVectorGeometry_()); // line from target delegate to delegate's actual platform
  }

  // transform the target position into planetarium-local space:
  const osg::Vec3d local = ecef * locatorRoot_->getInverseMatrix();
  const double     local_len = local.length();
  const osg::Vec3d local_n   = local / local_len;  // unit vector from host to target

  // update the line drawable vertices
  osgEarth::LineDrawable* targetVector = static_cast<osgEarth::LineDrawable*>(mt->getChild(1));
  if (!targetVector)
  {
    // child 1 is set by mt->addChild(buildVectorGeometry_()); above
    assert(0);
    return;
  }
  targetVector->setNodeMask(displayTargetVectors_ ? ~0 : 0);
  // create simple vector of desired length (dome to target)
  const osg::Vec3d V(s_up * (local_len - range_));
  for (unsigned int i = 0; i < NUM_VECTOR_SEGS; ++i)
  {
    const double t = static_cast<double>(i) / (NUM_VECTOR_SEGS - 1);
    targetVector->setVertex(i, V * t);
  }
  // orient & position the delegate and vector: rotate to point at target, translate to the dome's surface.
  mt->setMatrix(
    osg::Matrix::rotate(s_up, local_n) *
    osg::Matrix::translate(local_n * range_));
}

void PlanetariumViewTool::updateDome_()
{
  if (!locatorRoot_.valid())
    return;

  if (dome_.valid())
  {
    locatorRoot_->removeChild(dome_.get());
    dome_ = nullptr;
  }
  if (sector_.valid())
  {
    locatorRoot_->removeChild(sector_.get());
    sector_ = nullptr;
  }

  if (useSector_)
    createSector_();
  else
  {
    // build a sphere
    dome_ = simVis::createEllipsoidGeometry(range_, range_, range_,
      domeColor_,
      10.f, -90.f, 90.f, -180.f, 180.f, true);
    dome_->setName("Planetarium Sphere Geometry");
    osg::StateSet* stateSet = dome_->getOrCreateStateSet();
    stateSet->setMode(GL_BLEND, 1);
    stateSet->setMode(GL_CULL_FACE, 0 | osg::StateAttribute::PROTECTED);

    // Maximum number of textures supported
    stateSet->setDefine("SIMVIS_PLANETARIUM_NUM_TEXTURES", std::to_string(1 + static_cast<int>(TextureUnit::UNIT3)));
    // Dome just got recreated, reapply all textures
    applyAllTextures_();

    osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateSet);
    simVis::Shaders package;
    package.load(vp, package.planetariumTexture());

    // Turn off the depth writes to help with transparency
    stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0, 1, false));
    locatorRoot_->addChild(dome_.get());
  }
}

void PlanetariumViewTool::createSector_()
{
  simVis::SVData sv;

  // Set up defaults
  sv.shape_ = simVis::SVData::SHAPE_PYRAMID;
  sv.drawMode_ = (simVis::SVData::DRAW_MODE_SOLID | simVis::SVData::DRAW_MODE_OUTLINE);
  sv.color_ = domeColor_;
  sv.blendingEnabled_ = true;

  sv.azimOffset_deg_ = sectorAzDeg_;
  sv.elevOffset_deg_ = sectorElDeg_;
  sv.hfov_deg_ = sectorWidthDeg_;
  sv.vfov_deg_ = sectorHeightDeg_;

  // Below implementation matches resolution/tessellation implementation for GateVolume
  const float maxFov = simCore::sdkMax(sv.hfov_deg_, sv.vfov_deg_);
  const float capRes = osg::clampBetween((maxFov / 5.f), 5.f, 24.f);
  sv.capRes_ = static_cast<unsigned int>(0.5f + capRes);
  sv.wallRes_ = 3;

  // No need to set up sv.nearRange_, as it is ignored when sv.drawCone_ is false
  sv.farRange_ = range_;
  sv.drawCone_ = false; // Draw flat sector only (no side/top/bottom walls)
  sv.drawAsSphereSegment_ = true;

  sector_ = simVis::SVFactory::createNode(sv, osg::Y_AXIS);
  // Turn off the depth writes to help with transparency
  sector_->getOrCreateStateSet()->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false));
  locatorRoot_->addChild(sector_.get());
}

void PlanetariumViewTool::applyOverrides_(bool enable)
{
  for (auto entityObsPtr : family_.members())
  {
    if (entityObsPtr.valid())
      applyOverrides_(entityObsPtr.get(), enable);
  }
}

void PlanetariumViewTool::applyOverrides_(EntityNode* entity, bool enable) const
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

osg::Node* PlanetariumViewTool::buildVectorGeometry_() const
{
  osgEarth::LineDrawable* geom = new osgEarth::LineDrawable(GL_LINE_STRIP);
  geom->allocate(NUM_VECTOR_SEGS);
  geom->setColor(simVis::Color::White);
  geom->setDataVariance(osg::Object::DYNAMIC);
  return geom;
}

void PlanetariumViewTool::addBeamToBeamHistory_(simVis::BeamNode* beam)
{
  // SIM-13705 - only supporting beam history for absolute/linear beams;
  //   body beam implementation is difficult and not relevant for customer
  const simData::BeamProperties& props = beam->getProperties();
  const bool isLinearBeam = (props.has_type() && props.type() == simData::BeamProperties_BeamType_ABSOLUTE_POSITION);
  if (isLinearBeam && history_.find(beam->getId()) == history_.end())
  {
    osg::ref_ptr<BeamHistory> history = new BeamHistory(beam, ds_, range_);
    history->setUseGradient(useGradient_);
    history->setHistoryLength(historyLength_);
    history_[beam->getId()] = history;
    if (displayBeamHistory_)
      root_->addChild(history.get());
  }
}

PlanetariumViewTool::TextureData& PlanetariumViewTool::getTexture_(TextureUnit texUnit)
{
  return textures_[static_cast<int>(texUnit)];
}

const PlanetariumViewTool::TextureData& PlanetariumViewTool::getTexture_(TextureUnit texUnit) const
{
  return textures_[static_cast<int>(texUnit)];
}

void PlanetariumViewTool::setTextureImage(TextureUnit texUnit, osg::Image* image)
{
  if (getTexture_(texUnit).image == image)
    return;
  getTexture_(texUnit).image = image;
  applyTexture_(texUnit);
}

osg::Image* PlanetariumViewTool::getTextureImage(TextureUnit texUnit) const
{
  return getTexture_(texUnit).image.get();
}

void PlanetariumViewTool::setTextureCoords(TextureUnit texUnit, double minLat, double maxLat, double minLon, double maxLon)
{
  auto& td = getTexture_(texUnit);
  if (minLat == td.latitudeSpan.x() && maxLat == td.latitudeSpan.y() &&
    minLon == td.longitudeSpan.x() && maxLon == td.longitudeSpan.y())
    return;
  td.latitudeSpan.x() = minLat;
  td.latitudeSpan.y() = maxLat;
  td.longitudeSpan.x() = minLon;
  td.longitudeSpan.y() = maxLon;
  applyTexture_(texUnit);
}

void PlanetariumViewTool::getTextureCoords(TextureUnit texUnit, double& minLat, double& maxLat, double& minLon, double& maxLon) const
{
  auto& td = getTexture_(texUnit);
  minLat = td.latitudeSpan.x();
  maxLat = td.latitudeSpan.y();
  minLon = td.longitudeSpan.x();
  maxLon = td.longitudeSpan.y();
}

void PlanetariumViewTool::setTextureAlpha(TextureUnit texUnit, float alpha)
{
  if (getTexture_(texUnit).alpha == alpha)
    return;
  getTexture_(texUnit).alpha = alpha;
  applyTexture_(texUnit);
}

float PlanetariumViewTool::getTextureAlpha(TextureUnit texUnit) const
{
  return getTexture_(texUnit).alpha;
}

void PlanetariumViewTool::setTextureEnabled(TextureUnit texUnit, bool active)
{
  if (getTexture_(texUnit).enabled == active)
    return;
  getTexture_(texUnit).enabled = active;
  applyTexture_(texUnit);
}

bool PlanetariumViewTool::getTextureEnabled(TextureUnit texUnit) const
{
  return getTexture_(texUnit).enabled;
}

void PlanetariumViewTool::applyAllTextures_()
{
  for (int k = 0; k <= static_cast<int>(TextureUnit::UNIT3); ++k)
    applyTexture_(static_cast<TextureUnit>(k));
}

/** Helper function for applyTexture_() to set a uniform value in an array (because osg interface requires const char*) */
template<typename T>
void setUniformArrayValue(osg::StateSet& ss, PlanetariumViewTool::TextureUnit arrayIndex, const std::string& param, const T& value)
{
  std::stringstream stream;
  stream << "sv_planet_tex[" << static_cast<int>(arrayIndex) << "]." << param;
  ss.addUniform(new osg::Uniform(stream.str().c_str(), value));
}

void PlanetariumViewTool::applyTexture_(TextureUnit texUnit)
{
  // Need a valid dome to apply texture content
  if (!dome_.valid())
    return;

  // Extract the texture data, state set
  auto& td = getTexture_(texUnit);
  auto* ss = dome_->getOrCreateStateSet();

  // Configure all shader uniform values
  setUniformArrayValue(*ss, texUnit, "alpha", td.alpha);
  setUniformArrayValue(*ss, texUnit, "coords", osg::Vec4f(
    td.longitudeSpan.x(), td.longitudeSpan.y(),
    td.latitudeSpan.x(), td.latitudeSpan.y()));
  const bool enabled = td.image.valid() && td.enabled;
  setUniformArrayValue(*ss, texUnit, "enabled", enabled);
  const int glTextureUnit = static_cast<int>(texUnit);
  setUniformArrayValue(*ss, texUnit, "sampler", glTextureUnit);

  // Create a texture if needed
  if (!td.texture)
    td.texture = new osg::Texture2D(td.image.get());
  td.texture->setImage(td.image.get());
  ss->setTextureAttribute(glTextureUnit, td.texture.get());
}

}
