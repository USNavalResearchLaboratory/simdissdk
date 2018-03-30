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
VelocityVector::VelocityVector(Locator* hostLocator, const osg::Vec4f& vectorColor, float lineWidth)
  : LocatorNode(new Locator(hostLocator, Locator::COMP_POSITION)),
    forceRebuild_(true),
    lineWidth_(new osg::LineWidth(lineWidth)),
    vectorColor_(vectorColor)
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

  osg::ref_ptr<osg::Geode> geode = new osg::Geode();
  createVelocityVector_(prefs, geode.get());

  // disable lighting
  osg::StateSet* stateSet = geode->getOrCreateStateSet();

  setNodeMask(DISPLAY_MASK_PLATFORM);
  this->addChild(geode.get());
  return 0;
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
      setNodeMask(draw ? DISPLAY_MASK_PLATFORM : DISPLAY_MASK_NONE);

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

void VelocityVector::createVelocityVector_(const simData::PlatformPrefs& prefs, osg::Geode* geode) const
{
  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
  geom->setUseVertexBufferObjects(true);

  osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();
  geom->setVertexArray(vertices.get());

  osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array(osg::Array::BIND_PER_PRIMITIVE_SET);
  geom->setColorArray(colorArray.get());

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
    const Units sizeUnits = simVis::convertUnitsToOsgEarth(prefs.velvecstaticlenunits());
    scale = sizeUnits.convertTo(Units::METERS, prefs.velvecstaticlen());
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
  vertices->push_back(osg::Vec3(0, 0, 0));
  vertices->push_back(osg::Vec3(velocity.x(), velocity.y(), velocity.z()));
  geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, 2));
  colorArray->push_back(vectorColor_);

  // set linewidth
  geom->getOrCreateStateSet()->setAttributeAndModes(lineWidth_.get(), 1);

  // Add the drawable to the geode
  geode->addDrawable(geom);
}

}
