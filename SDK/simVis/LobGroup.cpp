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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osgEarth/GeoData"
#include "osgEarth/Horizon"
#include "osgEarth/ObjectIndex"
#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/MultiFrameCoordinate.h"
#include "simData/DataTable.h"
#include "simData/LinearInterpolator.h"
#include "simVis/AnimatedLine.h"
#include "simVis/EntityLabel.h"
#include "simVis/LabelContentManager.h"
#include "simVis/LocalGrid.h"
#include "simVis/Locator.h"
#include "simVis/OverheadMode.h"
#include "simVis/Shaders.h"
#include "simVis/Utils.h"
#include "simVis/LobGroup.h"

#undef LC
#define LC "[simVis::LobGroup] "

namespace
{

/// Uniform shader variable for flashing the LOB
static const std::string SIMVIS_FLASHING_ENABLE = "simvis_flashing_enable";

/** Determines whether the new prefs will require new geometry */
bool prefsRequiresRebuild(const simData::LobGroupPrefs* a, const simData::LobGroupPrefs* b)
{
  if (a == NULL || b == NULL)
    return false; // simple case

  if (PB_SUBFIELD_CHANGED(a, b, commonprefs, useoverridecolor))
  {
    // force rebuild if useOverrideColor pref changed
    return true;
  }
  else if (b->commonprefs().useoverridecolor() && PB_SUBFIELD_CHANGED(a, b, commonprefs, overridecolor))
  {
    // force rebuild if override color changed and it is being used
    return true;
  }

  return false; // TODO, further optimization
}
} // anonymous namespace

namespace simVis
{

/// maps time to one or more animated lines
class LobGroupNode::Cache
{
public:
  /** Constructor */
  Cache()
  {
  }

  ~Cache()
  {
  }

  /** Retrieves the number of animated lines in the cache */
  int numLines() const
  {
    return static_cast<int>(entries_.size());
  }

  /** Removes all animated lines from the cache under the parent */
  void clearCache(osg::Group* parent)
  {
    // removes all LOB draw nodes
    for (LineCache::const_iterator iter = entries_.begin(); iter != entries_.end(); ++iter)
    {
      parent->removeChild(iter->second);
    }
    // clear out cache
    entries_.clear();
  }

  /** Removes items from the cache that are outside [firstTime,lastTime] */
  void pruneCache(osg::Group* parent, double firstTime, double lastTime)
  {
    // since the ordered multimap keys are ordered by time, remove all entries < firstTime time
    const LineCache::iterator newStart = entries_.find(firstTime);
    for (LineCache::const_iterator iter = entries_.begin(); iter != newStart; ++iter)
    {
      parent->removeChild(iter->second);
    }
    if (newStart != entries_.begin())
      entries_.erase(entries_.begin(), newStart);

    // since the ordered multimap keys are ordered by time, remove all entries > lastTime time
    const LineCache::iterator newEnd = entries_.upper_bound(lastTime);
    for (LineCache::const_iterator iter = newEnd; iter != entries_.end(); ++iter)
    {
      parent->removeChild(iter->second);
    }
    if (newEnd != entries_.end())
      entries_.erase(newEnd, entries_.end());
  }

  /// update all lines to have the prefs in 'p'
  void setAllLineProperties(const simData::LobGroupPrefs &p)
  {
    // TODO body offset
    for (LineCache::const_iterator i = entries_.begin(); i != entries_.end(); ++i)
    {
      // only changeable pref is color override (maxdatapoints and maxdataseconds are handled in refresh())
      if (p.commonprefs().useoverridecolor())
        i->second->setColorOverride(simVis::ColorUtils::RgbaToVec4(p.commonprefs().overridecolor()));
      else
        i->second->clearColorOverride();
    }
  }

  /// return true if there are any lines for time 't'
  bool hasTime(double t) const
  {
    return (entries_.find(t) != entries_.end());
  }

  /// add animated line 'a' at time 't'
  void addLineAtTime(double t, AnimatedLineNode *a)
  {
    entries_.insert(std::make_pair(t, osg::ref_ptr<AnimatedLineNode>(a)));
  }

  /// Gets the endpoints of all lines in the cache
  void getVisibleEndpoints(std::vector<osg::Vec3d>& ecefVec) const
  {
    simCore::MultiFrameCoordinate first;
    simCore::MultiFrameCoordinate second;
    for (LineCache::const_iterator iter = entries_.begin(); iter != entries_.end(); ++iter)
    {
      // Only save points of lines that are visible
      if (iter->second->getNodeMask() != 0 && iter->second->getEndPoints(first, second) == 0)
      {
        const simCore::Vec3 firstPos = first.ecefCoordinate().position();
        const simCore::Vec3 secondPos = second.ecefCoordinate().position();
        ecefVec.push_back(osg::Vec3d(firstPos.x(), firstPos.y(), firstPos.z()));
        ecefVec.push_back(osg::Vec3d(secondPos.x(), secondPos.y(), secondPos.z()));
      }
    }
  }

protected:
  typedef std::multimap<double, osg::ref_ptr<AnimatedLineNode> > LineCache;
  /** Multimap of scenario time to the animated lines at that time */
  LineCache entries_;
};

LobGroupNode::LobGroupNode(const simData::LobGroupProperties &props, EntityNode* host, CoordSurfaceClamping* surfaceClamping, simData::DataStore &ds)
  : EntityNode(simData::LOB_GROUP, new CachingLocator()), // lobgroup locator is independent of host locator
  lastProps_(props),
  hasLastUpdate_(false),
  lastPrefsValid_(false),
  surfaceClamping_(surfaceClamping),
  coordConverter_(new simCore::CoordinateConverter()),
  ds_(ds),
  hostId_(host->getId()),
  lineCache_(new Cache),
  label_(NULL),
  lastFlashingState_(false),
  objectIndexTag_(0)
{
  setName("LobGroup");
  localGrid_ = new LocalGridNode(getLocator(), host, ds.referenceYear());
  addChild(localGrid_);

  label_ = new EntityLabelNode(getLocator());
  this->addChild(label_);

  // horizon culling: entity culling based on bounding sphere
  addCullCallback( new osgEarth::HorizonCullCallback() );
  // labels are culled based on entity center point
  osgEarth::HorizonCullCallback* callback = new osgEarth::HorizonCullCallback();
  callback->setCullByCenterPointOnly(true);
  // SIM-11395 - set default ellipsoid, when osgEarth supports it
  //  callback->setHorizon(new osgEarth::Horizon(*getLocator()->getSRS()->getEllipsoid()));
  callback->setProxyNode(this);
  label_->addCullCallback(callback);

  // flatten in overhead mode.
  simVis::OverheadMode::enableGeometryFlattening(true, this);

  // Add a tag for picking
  objectIndexTag_ = osgEarth::Registry::objectIndex()->tagNode(this, this);
}

LobGroupNode::~LobGroupNode()
{
  osgEarth::Registry::objectIndex()->remove(objectIndexTag_);

  delete coordConverter_;
  coordConverter_ = NULL;
  lineCache_->clearCache(this);
  delete lineCache_;
  lineCache_ = NULL;
}

void LobGroupNode::installShaderProgram(osg::StateSet* intoStateSet)
{
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(intoStateSet);
  simVis::Shaders package;
  package.load(vp, package.flashingFragment());
  intoStateSet->getOrCreateUniform(SIMVIS_FLASHING_ENABLE, osg::Uniform::BOOL)->set(false);
}

void LobGroupNode::updateLabel_(const simData::LobGroupPrefs& prefs)
{
  if (hasLastUpdate_)
  {
    std::string label = getEntityName_(prefs.commonprefs(), EntityNode::DISPLAY_NAME, false);
    if (prefs.commonprefs().labelprefs().namelength() > 0)
      label = label.substr(0, prefs.commonprefs().labelprefs().namelength());

    std::string text;
    if (prefs.commonprefs().labelprefs().draw())
      text = labelContentCallback().createString(prefs, lastUpdate_, prefs.commonprefs().labelprefs().displayfields());

    if (!text.empty())
    {
      label += "\n";
      label += text;
    }

    const float zOffset = 0.0f;
    label_->update(prefs.commonprefs(), label, zOffset);
  }
}

std::string LobGroupNode::popupText() const
{
  if (hasLastUpdate_ && lastPrefsValid_)
  {
    std::string prefix;
    // if alias is defined show both in the popup to match SIMDIS 9's behavior.  SIMDIS-2241
    if (!lastPrefs_.commonprefs().alias().empty())
    {
      if (lastPrefs_.commonprefs().usealias())
        prefix = getEntityName(EntityNode::REAL_NAME);
      else
        prefix = getEntityName(EntityNode::ALIAS_NAME);
      prefix += "\n";
    }
    return prefix + labelContentCallback().createString(lastPrefs_, lastUpdate_, lastPrefs_.commonprefs().labelprefs().hoverdisplayfields());
  }

  return "";
}

std::string LobGroupNode::hookText() const
{
  if (hasLastUpdate_ && lastPrefsValid_)
    return labelContentCallback().createString(lastPrefs_, lastUpdate_, lastPrefs_.commonprefs().labelprefs().hookdisplayfields());
  return "";
}

std::string LobGroupNode::legendText() const
{
  if (hasLastUpdate_ && lastPrefsValid_)
    return labelContentCallback().createString(lastPrefs_, lastUpdate_, lastPrefs_.commonprefs().labelprefs().legenddisplayfields());
  return "";
}

void LobGroupNode::setPrefs(const simData::LobGroupPrefs &prefs)
{
  // update the visibility
  const bool drawnLOBs = hasLastUpdate_ && (lastUpdate_.datapoints_size() > 0);
  const bool drawn = prefs.commonprefs().datadraw() && prefs.commonprefs().draw();
  setNodeMask((drawnLOBs && drawn) ? DISPLAY_MASK_LOB_GROUP : DISPLAY_MASK_NONE);

  // validate local grid prefs changes that might provide user notifications
  localGrid_->validatePrefs(prefs.commonprefs().localgrid());
  localGrid_->setPrefs(prefs.commonprefs().localgrid());

  // process pref change - only override color
  if (!lastPrefsValid_ || prefsRequiresRebuild(&lastPrefs_, &prefs))
    lineCache_->setAllLineProperties(prefs);

  // check for override range change or clamping change
  // either: use override change or
  // we are using override, and the override value changed
  if (!lastPrefsValid_ ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, userangeoverride) ||
      (lastPrefs_.userangeoverride() && PB_FIELD_CHANGED(&lastPrefs_, &prefs, rangeoverridevalue)) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, lobuseclampalt))
  {
    // rebuild all lines
    lineCache_->clearCache(this);
    const simData::LobGroupUpdateSlice *updateSlice = ds_.lobGroupUpdateSlice(lastProps_.id());
    if (updateSlice)
    {
      const simData::LobGroupUpdate *currentUpdate = ds_.lobGroupUpdateSlice(lastProps_.id())->current();
      if (currentUpdate)
        updateCache_(*currentUpdate, prefs);
    }
  }

  lastPrefs_ = prefs;
  lastPrefsValid_ = true;

  // label does not perform any PB_FIELD_CHANGED tests on prefs, requires that lastPrefs_ be the up-to-date prefs.
  updateLabel_(prefs);
}

void LobGroupNode::setLineDrawStyle_(double time, simVis::AnimatedLineNode& line, const simData::LobGroupPrefs& defaultValues)
{
  simData::DataTable* table = ds_.dataTableManager().findTable(getId(), simData::INTERNAL_LOB_DRAWSTYLE_TABLE);
  if (table == NULL)
  {
    setLineValueFromPrefs_(line, defaultValues);
    return;
  }

  simData::LobGroupPrefs prefs;
  // initialize to the current pref values
  prefs.CopyFrom(defaultValues);
  uint32_t color1;
  uint32_t color2;
  uint16_t stipple1;
  uint16_t stipple2;
  uint8_t lineWidth;
  // update the draw style values from internal data table, if the values exist
  if (getColumnValue_(simData::INTERNAL_LOB_COLOR1_COLUMN, *table, time, color1) == 0)
    prefs.set_color1(color1);
  if (getColumnValue_(simData::INTERNAL_LOB_COLOR2_COLUMN, *table, time, color2) == 0)
    prefs.set_color2(color2);
  if (getColumnValue_(simData::INTERNAL_LOB_STIPPLE1_COLUMN, *table, time, stipple1) == 0)
    prefs.set_stipple1(stipple1);
  if (getColumnValue_(simData::INTERNAL_LOB_STIPPLE2_COLUMN, *table, time, stipple2) == 0)
    prefs.set_stipple2(stipple2);
  if (getColumnValue_(simData::INTERNAL_LOB_LINEWIDTH_COLUMN, *table, time, lineWidth) == 0)
    prefs.set_lobwidth(lineWidth);

  setLineValueFromPrefs_(line, prefs);
}

void LobGroupNode::setLineValueFromPrefs_(AnimatedLineNode& line, const simData::LobGroupPrefs& prefs) const
{
  line.setStipple1(prefs.stipple1());
  line.setStipple2(prefs.stipple2());
  line.setLineWidth(prefs.lobwidth());
  line.setColor1(simVis::ColorUtils::RgbaToVec4(prefs.color1()));
  line.setColor2(simVis::ColorUtils::RgbaToVec4(prefs.color2()));

  if (prefs.commonprefs().useoverridecolor())
    line.setColorOverride(simVis::ColorUtils::RgbaToVec4(prefs.commonprefs().overridecolor()));
}

template <class T>
int LobGroupNode::getColumnValue_(const std::string& columnName, const simData::DataTable& table, double time, T& value) const
{
  simData::TableColumn* column = table.column(columnName);
  if (column == NULL)
    return 1;

  simData::TableColumn::Iterator iter = column->findAtOrBeforeTime(time);
  if (!iter.hasNext())
  {
    return 1;
  }
  if (iter.next()->getValue(value).isError())
  {
    assert(0); // getValue failed, but hasNext claims it exists. Check DataSlice Iterator logic for data tables
    return 1;
  }
  return 0;
}

void LobGroupNode::updateCache_(const simData::LobGroupUpdate &update, const simData::LobGroupPrefs& prefs)
{
  const int numLines = update.datapoints_size();
  const simData::PlatformUpdateSlice *platformData = ds_.platformUpdateSlice(hostId_);
  if ((numLines <= 0) || (platformData == NULL))
  {
    // no lines, clear out cache and remove all draw nodes
    lineCache_->clearCache(this);
    return;
  }

  // prune the cache, since the data max values may adjust how much data is shown
  const double firstTime = update.datapoints(0).time();
  const double lastTime = update.datapoints(numLines-1).time();
  lineCache_->pruneCache(this, firstTime, lastTime);

  simData::Interpolator* li = ds_.interpolator();
  for (int index = 0; index < numLines;) // Incremented in the for loop below
  {
    // handle all lines with this time (if time is not already in the cache)
    const double time = update.datapoints(index).time();
    if (lineCache_->hasTime(time))
    {
      ++index;
      continue;
    }

    // prepare to add this line to the cache - process the host platform position once for all endpoints
    const simData::PlatformUpdateSlice::Iterator platformIter = platformData->upper_bound(time);
    if (!platformIter.hasPrevious())
    {
      // cannot process this LOB since there is no platform position at or before lob time; possibly the platform point was removed by data limiting.
      ++index;
      continue;
      // note that this will create the condition that numLines != lineCache_->numLines()
    }
    // last update at or before t:
    const simData::PlatformUpdate* platformUpdate = platformIter.peekPrevious();

    // interpolation may be required for LOBs on a moving platform
    simData::PlatformUpdate interpolatedPlatformUpdate;
    if (platformUpdate->time() != time && li != NULL && platformIter.hasNext())
    {
      // defn of upper_bound previous()
      assert(platformUpdate->time() < time);
      // defn of upper_bound next()
      assert(platformIter.peekNext()->time() > time);
      li->interpolate(time, *platformUpdate, *(platformIter.peekNext()), &interpolatedPlatformUpdate);
      platformUpdate = &interpolatedPlatformUpdate;
    }

    // construct the starting coordinate, we may clamp this
    simCore::Coordinate platformCoordPosOnly(simCore::COORD_SYS_ECEF, simCore::Vec3(platformUpdate->x(), platformUpdate->y(), platformUpdate->z()));
    simCore::Coordinate llaCoord;
    if (lastProps_.azelrelativetohostori())
    {
      // calculate host orientation in LLA, used for determining a relative LOB's true angle
      const simCore::Coordinate ecefCoord(simCore::COORD_SYS_ECEF, simCore::Vec3(platformUpdate->x(), platformUpdate->y(), platformUpdate->z()),
                                          simCore::Vec3(platformUpdate->psi(), platformUpdate->theta(), platformUpdate->phi()));
      simCore::CoordinateConverter::convertEcefToGeodetic(ecefCoord, llaCoord);
    }

    // calculate the clamped host platform coord only once, for all lines at this same time
    if (prefs.lobuseclampalt())
    {
      // we provide only ecef
      assert(platformCoordPosOnly.coordinateSystem() == simCore::COORD_SYS_ECEF);
      applyPlatformCoordClamping_(platformCoordPosOnly);
      // and are returned only ecef
      assert(platformCoordPosOnly.coordinateSystem() == simCore::COORD_SYS_ECEF);
    }

    // process endpoints for all lines at same time; all share same host platform position just calc'd
    for (; index < update.datapoints_size() && update.datapoints(index).time() == time; ++index)
    {
      // calculate end point based on update point RAE
      const simData::LobGroupUpdatePoint &curP = update.datapoints(index);

      // find the point relative to the start
      simCore::Vec3 endPoint;

      simCore::Vec3 lobAngles(curP.azimuth(), curP.elevation(), 0.0);
      if (lastProps_.azelrelativetohostori())
      {
        // Offset the host orientation angles via the LOB relative orientation for body-relative mode
        lobAngles = simCore::rotateEulerAngle(llaCoord.orientation(), lobAngles);
      }

      // check for minimum range
      const double range = prefs.userangeoverride() ? prefs.rangeoverridevalue() : curP.range();
      simCore::v3SphtoRec(range, lobAngles.yaw(), lobAngles.pitch(), endPoint);
      simCore::Coordinate endCoord(simCore::COORD_SYS_XEAST, endPoint);

      if (prefs.lobuseclampalt())
        applyEndpointCoordClamping_(endCoord);

      //--- construct the line
      AnimatedLineNode *line = new AnimatedLineNode;
      line->setShiftsPerSecond(0);

      // set starting prefs
      setLineDrawStyle_(time, *line, prefs);

      // set coordinates
      line->setEndPoints(platformCoordPosOnly, endCoord);

      // insert into cache
      lineCache_->addLineAtTime(time, line);
      addChild(line);
    }

    // set the local grid for platform's position and az/el of the last of the lobs
    if (index == numLines)
    {
      const simData::LobGroupUpdatePoint &curP = update.datapoints(update.datapoints_size()-1);
      simCore::Vec3 lobAngles(curP.azimuth(), curP.elevation(), 0.0);
      if (lastProps_.azelrelativetohostori())
      {
        // Offset the host orientation angles via the LOB relative orientation for body-relative mode
        lobAngles = simCore::rotateEulerAngle(llaCoord.orientation(), lobAngles);
      }

      // suppress locator notification until we're done with locator updates
      getLocator()->setLocalOffsets(simCore::Vec3(), lobAngles, time, false);
      // Use position only, otherwise rendering will be adversely affected; locator notification is true now
      // note that if lob is clamped, localgrid will also be clamped
      getLocator()->setCoordinate(platformCoordPosOnly, time);
    }
  }
}

bool LobGroupNode::isActive() const
{
  return hasLastUpdate_ && lastPrefs_.commonprefs().datadraw();
}

const std::string LobGroupNode::getEntityName(EntityNode::NameType nameType, bool allowBlankAlias) const
{
  if (!lastPrefsValid_)
  {
    assert(0);
    return "";
  }

  return getEntityName_(lastPrefs_.commonprefs(), nameType, allowBlankAlias);
}

simData::ObjectId LobGroupNode::getId() const
{
  return lastProps_.id();
}

bool LobGroupNode::getHostId(simData::ObjectId& out_hostId) const
{
  out_hostId = lastProps_.hostid();
  return true;
}

bool LobGroupNode::updateFromDataStore(const simData::DataSliceBase *updateSliceBase, bool force)
{
  const simData::LobGroupUpdateSlice *updateSlice = static_cast<const simData::LobGroupUpdateSlice*>(updateSliceBase);
  assert(updateSlice);
  const simData::LobGroupUpdate* current = updateSlice->current();
  const bool lobChangedToActive = (current != NULL && !hasLastUpdate_);

  // Do any necessary flashing
  simData::DataTable* table = ds_.dataTableManager().findTable(getId(), simData::INTERNAL_LOB_DRAWSTYLE_TABLE);
  if (table != NULL)
  {
    bool flashing = false;
    uint8_t state;
    if (getColumnValue_(simData::INTERNAL_LOB_FLASH_COLUMN, *table, ds_.updateTime(), state) == 0)
      flashing = (state != 0);
    if (flashing != lastFlashingState_)
    {
      getOrCreateStateSet()->getOrCreateUniform(SIMVIS_FLASHING_ENABLE, osg::Uniform::BOOL)->set(flashing);
      lastFlashingState_ = flashing;
    }
  }

  const bool applyUpdate = updateSlice->hasChanged() || force || lobChangedToActive;
  if (applyUpdate)
  {
    if (current)
    {
      // lobGroup gets a pref update immediately after creation; after that, lastPrefsValid_ should always be true
      assert(lastPrefsValid_);
      updateCache_(*current, lastPrefs_);
      lastUpdate_ = *current;
      hasLastUpdate_ = true;

      // update the visibility
      const bool drawnLOBs = hasLastUpdate_ && (lastUpdate_.datapoints_size() > 0);
      const bool drawn = lastPrefs_.commonprefs().datadraw() && lastPrefs_.commonprefs().draw();
      setNodeMask((drawnLOBs && drawn) ? DISPLAY_MASK_LOB_GROUP : DISPLAY_MASK_NONE);
    }
    else
    {
      setNodeMask(DISPLAY_MASK_NONE);
      hasLastUpdate_ = false;
    }
  }
  // Whether updateSlice changed or not, label content may have changed, and for active beams we need to update
  if (isActive())
    updateLabel_(lastPrefs_);

  return applyUpdate;
}

void LobGroupNode::flush()
{
  lineCache_->clearCache(this);
  setNodeMask(DISPLAY_MASK_NONE);
  hasLastUpdate_ = false;
}

double LobGroupNode::range() const
{
  if (!hasLastUpdate_)
    return 0.0;

  if (lastUpdate_.datapoints_size() == 0)
    return 0.0;

  const simData::LobGroupUpdatePoint &curP = lastUpdate_.datapoints(lastUpdate_.datapoints_size()-1);
  return curP.range();
}

void LobGroupNode::applyPlatformCoordClamping_(simCore::Coordinate& platformCoord)
{
  if (!surfaceClamping_)
    return;

  // we are only provided ecef coords
  assert(platformCoord.coordinateSystem() == simCore::COORD_SYS_ECEF);

  // convert to lla first, this is the native coord system for clamping
  simCore::Coordinate platLla;
  simCore::CoordinateConverter::convertEcefToGeodetic(platformCoord, platLla);

  // clamp in ecef means: convert to lla, clamp, convert back to ecef; clamp in lla involves no coord conversion
  surfaceClamping_->clampCoordToMapSurface(platLla);

  // platform position is always our coordinate converter reference origin, in LLA (required for applyEndpointCoordClamping_)
  coordConverter_->setReferenceOrigin(platLla.position());

  // now convert to ecef since that is what the caller requires
  simCore::CoordinateConverter::convertGeodeticToEcef(platLla, platformCoord);
}

void LobGroupNode::applyEndpointCoordClamping_(simCore::Coordinate& endpointCoord)
{
  if (!surfaceClamping_)
    return;

  // convert to lla for surface clamping call
  simCore::Coordinate endLla;
  coordConverter_->convert(endpointCoord, endLla, simCore::COORD_SYS_LLA);
  surfaceClamping_->clampCoordToMapSurface(endLla);
  coordConverter_->convert(endLla, endpointCoord, simCore::COORD_SYS_XEAST);
}

void LobGroupNode::getVisibleEndPoints(std::vector<osg::Vec3d>& ecefVec) const
{
  ecefVec.clear();
  // Line cache only stores lines at current time
  if (isActive())
    lineCache_->getVisibleEndpoints(ecefVec);
}

unsigned int LobGroupNode::objectIndexTag() const
{
  return objectIndexTag_;
}

} // namespace simVis
