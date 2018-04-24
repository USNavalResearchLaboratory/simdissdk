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
#include "osg/Geode"
#include "osg/Geometry"
#include "osg/LineWidth"
#include "osgEarth/Version"
#include "simVis/LineDrawable.h"
#include "osgEarthUtil/Ephemeris"
#include "simNotify/Notify.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simVis/PlatformModel.h"
#include "simVis/Registry.h"
#include "simVis/Constants.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/EphemerisVector.h"

using namespace osgEarth::Symbology;

namespace simVis
{

/** Number of vertices to use when drawing the line vectors (should be at least 2) */
static const int NUM_LINE_VERTICES = 4;
/** Ephemeris vector display mask when visible; uses label to avoid affecting platform bounds */
static const simVis::DisplayMask DISPLAY_MASK_EPHEMERIS = DISPLAY_MASK_LABEL;
/** Interval in minutes for updating ephemeris vectors on time, when they aren't rebuilt due to other means */
static const int REBUILD_TIMEOUT = 15; // minutes of scenario time before vector gets rebuilt even when platform not moving

#define VECTOR_MOON 0
#define VECTOR_SUN  1

/** Every XX minutes of scenario time, make sure the ephemeris vector is rebuilt for new positions */
class EphemerisVector::RebuildOnTimer : public osg::Callback
{
public:
  explicit RebuildOnTimer(int updateMinutes=REBUILD_TIMEOUT)
    : Callback(),
      maxDelta_(updateMinutes * 60)
  {
  }

  virtual bool run(osg::Object* object, osg::Object* data)
  {
    EphemerisVector* ephemeris = dynamic_cast<EphemerisVector*>(object);
    if (ephemeris != NULL)
    {
      // Clock should always be set when using ephemeris vectors
      const simCore::Clock* clock = simVis::Registry::instance()->getClock();
      assert(clock != NULL);
      if (clock != NULL)
      {
        const simCore::TimeStamp now = clock->currentTime();
        const simCore::Seconds delta = now - ephemeris->lastUpdateTime_;
        if (fabs(delta.Double()) > maxDelta_)
          ephemeris->rebuild_(ephemeris->lastPrefs_);
      }
    }
    return traverse(object, data);
  }

private:
  /// Maximum delta between last update time and new update required (in seconds)
  int maxDelta_;
};

// --------------------------------------------------------------------------
EphemerisVector::EphemerisVector(const simVis::Color& moonColor, const simVis::Color& sunColor, float lineWidth)
  : Group(),
    coordConvert_(new simCore::CoordinateConverter),
    ephemeris_(new osgEarth::Util::Ephemeris),
    lastUpdateTime_(simCore::INFINITE_TIME_STAMP),
    hasLastPrefs_(false)
{
  setName("EphemerisVector");
  setNodeMask(DISPLAY_MASK_NONE);

  // scratch area for updating vectors
  vertices_ = new osg::Vec3Array();

  // Group to hold the vector lines:
  geomGroup_ = new osgEarth::LineGroup();
  addChild(geomGroup_);

  // Create and add the moon and sun lines
  geomGroup_->addChild(createVector_(moonColor, lineWidth));
  geomGroup_->addChild(createVector_(sunColor, lineWidth));

  // Add a callback to redraw ephemeris vectors when time passes in scenario
  addUpdateCallback(new RebuildOnTimer);
}

EphemerisVector::~EphemerisVector()
{
  delete coordConvert_;
  coordConvert_ = NULL;
}

void EphemerisVector::setModelNode(const PlatformModelNode* hostPlatformModel)
{
  modelNode_ = hostPlatformModel;
}

osg::Node* EphemerisVector::createVector_(const simVis::Color& color, float lineWidth) const
{
  osgEarth::LineDrawable* geom = new osgEarth::LineDrawable(GL_LINE_STRIP);
  geom->setName("simVis::EphemerisVector");
  for (int k = 0; k < NUM_LINE_VERTICES; ++k)
    geom->pushVertex(osg::Vec3(k, 0, 0));
  geom->dirty();
  geom->setColor(color);
  geom->setLineWidth(lineWidth);
  return geom;
}

void EphemerisVector::rebuild_(const simData::PlatformPrefs& prefs)
{
  // Make sure there is data to build a vector
  if (!lastUpdate_.has_time() || (!prefs.drawmoonvec() && !prefs.drawsunvec()))
  {
    setNodeMask(0);
    return;
  }

  // Clock should always be set when using ephemeris vectors
  const simCore::Clock* clock = simVis::Registry::instance()->getClock();
  assert(clock != NULL);
  if (clock == NULL)
  {
    setNodeMask(0);
    return;
  }

  // Pull out the DateTime that we can then send to the Ephemeris calculations
  const simCore::TimeStamp timeStamp = clock->currentTime();
  lastUpdateTime_ = timeStamp;
  const osgEarth::DateTime dateTime(timeStamp.secondsSinceRefYear(1970));

  // Reset the coordinate conversion center point
  const simCore::Coordinate asEcef(simCore::COORD_SYS_ECEF, simCore::Vec3(lastUpdate_.x(), lastUpdate_.y(), lastUpdate_.z()));
  simCore::Coordinate asLla;
  coordConvert_->convert(asEcef, asLla, simCore::COORD_SYS_LLA);
  coordConvert_->setReferenceOrigin(asLla.position());

  // Figure out how long the lines should be based on the standard algorithm
  const float lineLength = VectorScaling::lineLength(modelNode_.get(), static_cast<float>(prefs.axisscale()));

  // Draw the moon vector
  osgEarth::LineDrawable* moonGeom = geomGroup_->getLineDrawable(VECTOR_MOON);
  if (prefs.drawmoonvec())
  {
#if OSGEARTH_VERSION_LESS_THAN(2,10,0)
    rebuildLine_(VECTOR_MOON, ephemeris_->getMoonPositionECEF(dateTime), lineLength);
#else
    osgEarth::Util::CelestialBody moon = ephemeris_->getMoonPosition(dateTime);
    rebuildLine_(moonGeom, moon.geocentric, lineLength);
#endif
    moonGeom->setNodeMask(DISPLAY_MASK_EPHEMERIS);
  }
  else
    moonGeom->setNodeMask(0);

  // Draw the sun vector
  osgEarth::LineDrawable* sunGeom = geomGroup_->getLineDrawable(VECTOR_SUN);
  if (prefs.drawsunvec())
  {
#if OSGEARTH_VERSION_LESS_THAN(2,10,0)
    rebuildLine_(sunGeom, ephemeris_->getSunPositionECEF(dateTime), lineLength);
#else
    osgEarth::Util::CelestialBody sun = ephemeris_->getSunPosition(dateTime);
    rebuildLine_(sunGeom, sun.geocentric, lineLength);
#endif
    sunGeom->setNodeMask(DISPLAY_MASK_EPHEMERIS);
  }
  else
    sunGeom->setNodeMask(0);

  // Always show this group, at this point
  setNodeMask(DISPLAY_MASK_EPHEMERIS);
}

void EphemerisVector::rebuildLine_(osgEarth::LineDrawable* geom, const osg::Vec3& ephemerisPosition, float lineLength) const
{
  // Get the tangent plane (XEast) coordinates of the moon relative to platform-centric system
  simCore::Coordinate asTp;
  coordConvert_->convert(simCore::Coordinate(simCore::COORD_SYS_ECEF, simCore::Vec3(ephemerisPosition.x(), ephemerisPosition.y(), ephemerisPosition.z())),
    asTp, simCore::COORD_SYS_XEAST);

  // Figure out the end point, relative to the platform
  osg::Vec3d relToPlatform(asTp.x(), asTp.y(), asTp.z());
  relToPlatform.normalize();
  relToPlatform *= lineLength;

  // Generate all the points from center of platform to end of line
  vertices_->clear();
  VectorScaling::generatePoints(*vertices_.get(), osg::Vec3(0, 0, 0), relToPlatform, NUM_LINE_VERTICES);
  geom->importVertexArray(vertices_.get());
}

void EphemerisVector::setPrefs(const simData::PlatformPrefs& prefs)
{
  // do not process other prefs if we are not drawing the axis
  if (!prefs.drawmoonvec() && !prefs.drawsunvec())
  {
    setNodeMask(DISPLAY_MASK_NONE);
  }
  else
  {
    // Rebuild the vector if one of the scaling factors changes, or draw flags change
    if (!hasLastPrefs_ ||
      VectorScaling::fieldsChanged(lastPrefs_, prefs) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, drawmoonvec) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, drawsunvec))
      rebuild_(prefs);
    setNodeMask(DISPLAY_MASK_EPHEMERIS);
  }
  lastPrefs_ = prefs;
  hasLastPrefs_ = true;
}

void EphemerisVector::update(const simData::PlatformUpdate& platformUpdate)
{
  lastUpdate_ = platformUpdate;
  // Updates only trigger a redraw if we're already drawing, and the host is set to draw
  if (lastPrefs_.commonprefs().datadraw() && lastPrefs_.commonprefs().draw() &&
    (lastPrefs_.drawmoonvec() || lastPrefs_.drawsunvec()))
    rebuild_(lastPrefs_);
}
}
