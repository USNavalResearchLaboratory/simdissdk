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
#include "osg/Geode"
#include "osg/Geometry"

#include "osgEarth/LineDrawable"

#include "simCore/Calc/Math.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simNotify/Notify.h"

#include "simVis/Constants.h"
#include "simVis/Locator.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/VelocityVector.h"

namespace simVis
{

// --------------------------------------------------------------------------
VelocityVector::VelocityVector(Locator* hostLocator, float lineWidth)
  : LocatorNode(new Locator(hostLocator, Locator::COMP_POSITION)),
    forceRebuild_(true),
    lineWidth_(lineWidth)
{
  setName("VelocityVector");
  setNodeMask(DISPLAY_MASK_NONE);
}

VelocityVector::~VelocityVector()
{
}

int VelocityVector::rebuild_(const simData::PlatformPrefs& prefs)
{
  // clean the graph so we can rebuild it.
  this->removeChildren(0, this->getNumChildren());

  // Make sure there is data to build a vector
  if (!lastUpdate_.has_time())
  {
    setNodeMask(DISPLAY_MASK_NONE);
    return 1;
  }

  osg::ref_ptr<osgEarth::LineGroup> lineGroup = new osgEarth::LineGroup();
  createVelocityVector_(prefs, lineGroup.get());

  // disable lighting
  simVis::setLighting(lineGroup->getOrCreateStateSet(), osg::StateAttribute::OFF);

  setNodeMask(DISPLAY_MASK_PLATFORM);
  this->addChild(lineGroup.get());
  return 0;
}

/** Helper visitor to set the color of all lines visited */
class SetLineColorVisitor : public osg::NodeVisitor
{
public:
  explicit SetLineColorVisitor(const osg::Vec4f& color)
    : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
      color_(color)
  {
  }

  virtual void apply(osg::Geometry& geometry)
  {
    osgEarth::LineDrawable* line = dynamic_cast<osgEarth::LineDrawable*>(&geometry);
    if (line)
      line->setColor(color_);
  }

private:
  osg::Vec4f color_;
};

void VelocityVector::setPrefs(bool draw, const simData::PlatformPrefs& prefs, bool force)
{
  if (force)
  {
    // cache the force indicator, to be applied when axis draw is enabled
    // note that lastPrefs_ cannot be assumed to be valid
    forceRebuild_ = true;
  }

  // do not process other prefs if we are not drawing the axis
  if (!draw)
  {
    setNodeMask(DISPLAY_MASK_NONE);
  }
  else
  {
    // always rebuild everything first time through, otherwise, only if there is a prefs change
    bool rebuildRequired =
      forceRebuild_ ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, velvecusestaticlength) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, velvecstaticlen) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, velvecstaticlenunits) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, velvectime) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, velvectimeunits);

    if (rebuildRequired)
    {
      if (rebuild_(prefs) == 0)
      {
        // this is guaranteed by rebuild_
        assert(getNodeMask() == DISPLAY_MASK_PLATFORM);
        // force sync with our locator
        syncWithLocator();
      }
    }
    else
    {
      // Update the color if needed
      if (PB_FIELD_CHANGED(&lastPrefs_, &prefs, velveccolor))
      {
        SetLineColorVisitor setLineColor(simVis::Color(prefs.velveccolor(), simVis::Color::RGBA));
        accept(setLineColor);
      }

      setNodeMask(DISPLAY_MASK_PLATFORM);
    }

    forceRebuild_ = false;
  }

  lastPrefs_ = prefs;
}

void VelocityVector::update(const simData::PlatformUpdate& platformUpdate)
{
  lastUpdate_ = platformUpdate;
  if (lastPrefs_.commonprefs().datadraw() && lastPrefs_.commonprefs().draw() && lastPrefs_.drawvelocityvec())
    rebuild_(lastPrefs_);
}

void VelocityVector::createVelocityVector_(const simData::PlatformPrefs& prefs, osg::Group* group) const
{
  osg::ref_ptr<osgEarth::LineDrawable> geom = new osgEarth::LineDrawable(GL_LINES);
  geom->setName("simVis::VelocityVector");

  simCore::Coordinate ecef;
  ecef.setCoordinateSystem(simCore::COORD_SYS_ECEF);
  ecef.setPosition(lastUpdate_.x(), lastUpdate_.y(), lastUpdate_.z());
  ecef.setVelocity(lastUpdate_.vx(), lastUpdate_.vy(), lastUpdate_.vz());
  simCore::Coordinate lla;
  simCore::CoordinateConverter::convertEcefToGeodetic(ecef, lla);

  simCore::Vec3 velocity;
  double scale;
  if (prefs.velvecusestaticlength())
  {
    simCore::v3Norm(lla.velocity(), velocity);
    const osgEarth::Units sizeUnits = simVis::convertUnitsToOsgEarth(prefs.velvecstaticlenunits());
    scale = sizeUnits.convertTo(osgEarth::Units::METERS, prefs.velvecstaticlen());
  }
  else
  {
    velocity = lla.velocity();
    scale = prefs.velvectime();
    const simData::ElapsedTimeFormat timeFormat = prefs.velvectimeunits();
    assert(timeFormat == simData::ELAPSED_HOURS || timeFormat == simData::ELAPSED_MINUTES || timeFormat == simData::ELAPSED_SECONDS);
    if (timeFormat == simData::ELAPSED_MINUTES)
      scale *= 60.0;
    else if (timeFormat == simData::ELAPSED_HOURS)
      scale *= 3600.0;
  }

  simCore::v3Scale(scale, velocity, velocity);

  // draw velocity vector
  geom->allocate(2);
  geom->setVertex(0, osg::Vec3());
  geom->setVertex(1, osg::Vec3(velocity.x(), velocity.y(), velocity.z()));
  geom->setColor(simVis::Color(prefs.velveccolor(), simVis::Color::RGBA));
  // set linewidth
  geom->setLineWidth(lineWidth_);

  // Add the drawable to the geode
  group->addChild(geom.get());
}

}
