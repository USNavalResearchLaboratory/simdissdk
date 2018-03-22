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
#include <iomanip>

#include "osg/Notify"
#include "osg/Geode"
#include "osg/Geometry"
#include "osgText/Text"

#include "osgEarth/Registry"
#include "osgEarth/ShaderGenerator"
#include "osgEarthSymbology/Color"

#include "simCore/Calc/Math.h"
#include "simNotify/Notify.h"
#include "simCore/Time/String.h"
#include "simCore/Time/TimeClass.h"

#include "simVis/Constants.h"
#include "simVis/Locator.h"
#include "simVis/Platform.h"
#include "simVis/PointSize.h"
#include "simVis/Registry.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/LocalGrid.h"

#define LC "[LocalGrid] "

using namespace osgEarth::Symbology;

namespace simVis
{
// --------------------------------------------------------------------------
namespace
{
const float CIRCLE_QUANT_LEN = 50.0f; // segment length for quantized circle

// limit range ring and speed ring size to avoid excessive UI responsiveness penalty.
// this number derived from trial and error, and corresponds to a several minute response time.
const double MAX_RING_SIZE_M = 6e+06;

// minimum number of line segments in a polar ring
const int MIN_NUM_LINE_SEGMENTS = 50;

// Number of points in the subdivided line strip for horizontal and cross-hair grids
const int NUM_POINTS_PER_LINE_STRIP = 10;

}

// --------------------------------------------------------------------------
LocalGridNode::LocalGridNode(Locator* hostLocator, const EntityNode* host, int referenceYear)
  : LocatorNode(new Locator(hostLocator, Locator::COMP_POSITION | Locator::COMP_HEADING)),
    forceRebuild_(true),
    hostSpeedMS_(0.0),
    hostTimeS_(0.0),
    referenceYear_(referenceYear),
    fixedTime_(0.0),
    host_(host)
{
  setName("LocalGrid");
  setNodeMask(DISPLAY_MASK_NONE);
}

void LocalGridNode::rebuild_(const simData::LocalGridPrefs& prefs)
{
  // clean the graph so we can rebuild it.
  this->removeChildren(0, this->getNumChildren());
  osg::ref_ptr<osg::Geode> geode = new osg::Geode();

  // build for the appropriate grid type:
  switch (prefs.gridtype())
  {
  case simData::LocalGridPrefs_Type_CARTESIAN:
    createCartesian_(prefs, geode.get());
    break;

  case simData::LocalGridPrefs_Type_POLAR:
    createRangeRings_(prefs, geode.get(), true);
    break;

  case simData::LocalGridPrefs_Type_RANGE_RINGS:
    createRangeRings_(prefs, geode.get(), false);
    break;

  case simData::LocalGridPrefs_Type_SPEED_RINGS:
    createSpeedRings_(prefs, geode.get(), false);
    break;

  case simData::LocalGridPrefs_Type_SPEED_LINE:
    createSpeedRings_(prefs, geode.get(), true);
    break;
  }

  // shader needed to draw text properly
  osgEarth::Registry::shaderGenerator().run(geode.get());

  // disable lighting
  osg::StateSet* stateSet = geode->getOrCreateStateSet();
  PointSize::setValues(stateSet, 1.5f, osg::StateAttribute::ON);
  stateSet->setRenderBinDetails(BIN_LOCAL_GRID, BIN_GLOBAL_SIMSDK);

  this->addChild(geode);
};

void LocalGridNode::validatePrefs(const simData::LocalGridPrefs& prefs)
{
  // because fixed time validation provides feedback to user, it needs to be processed when interaction occurs, not just when grid is turned on
  // note that lastPrefs_ is only valid when !forceRebuild
  const std::string time = prefs.speedring().fixedtime();
  if (!time.empty() && (forceRebuild_ || PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, speedring, fixedtime)))
  {
    simCore::OrdinalTimeFormatter formatter;
    simCore::TimeStamp timeStamp;
    if (0 != formatter.fromString(time, timeStamp, referenceYear_))
    {
      SIM_ERROR << "Local Grid Fixed Time " << time << " is not a valid DDD YYYY HH:MM:SS.sss time." << std::endl;
    }
    else
      fixedTime_ = timeStamp.secondsSinceRefYear(referenceYear_).Double();
  }

  // send messages to console on empty time only if useFixedTime is on
  const bool useFixedTime = prefs.speedring().usefixedtime();
  if (useFixedTime && time.empty())
  {
    SIM_ERROR << "Local Grid Fixed Time field is empty. Can not create fixed time speed ring." << std::endl;
    fixedTime_ = 0.0;
  }
}

void LocalGridNode::setPrefs(const simData::LocalGridPrefs& prefs, bool force)
{
  if (force)
  {
    // cache the force indicator, to be applied when grid draw is enabled
    // note that lastPrefs_ cannot be assumed to be valid
    forceRebuild_ = true;
  }

  if (!prefs.drawgrid())
  {
    setNodeMask(DISPLAY_MASK_NONE);
    // do not process other prefs changes if we are not drawing the grid; we need to detect those changes (below) when grid is enabled
    lastPrefs_.set_drawgrid(false);
  }
  else
  {
    setNodeMask(DISPLAY_MASK_LOCAL_GRID);

    // always rebuild everything first time through, otherwise, only if there is a prefs change
    const bool rebuildRequired =
      forceRebuild_ ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, gridtype)  ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, gridcolor) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, size)      ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, sizeunits) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, gridlabeldraw) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, gridlabelfontsize) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, gridlabelfontname) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, gridlabelcolor) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, gridlabeltextoutline) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, gridlabeloutlinecolor) ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, gridlabelprecision) ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, gridsettings, numdivisions)    ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, gridsettings, numsubdivisions) ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, gridsettings, sectorangle)     ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, speedring, usefixedtime)       ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, speedring, fixedtime)          ||  // note that fixed time validation occurs above, but this will cause a rebuild
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, speedring, timeformat)         ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, speedring, radius)             ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, speedring, useplatformspeed)   ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, speedring, speedtouse)         ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, speedring, speedunits)         ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, speedring, displaytime);

    if (rebuildRequired)
    {
      if ((prefs.gridtype() == simData::LocalGridPrefs_Type_SPEED_RINGS) ||
          (prefs.gridtype() == simData::LocalGridPrefs_Type_SPEED_LINE))
      {
        calcSpeedParams_(prefs);
      }
      rebuild_(prefs);
    }

    const bool locatorChangeRequired =
      forceRebuild_ ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, drawgrid) ||    // if draw was toggled on, force a locator sync
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, followyaw)             ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, followpitch)           ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, followroll)            ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, gridpositionoffset, x)    ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, gridpositionoffset, y)    ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, gridpositionoffset, z)    ||
      PB_FIELD_CHANGED(&lastPrefs_, &prefs, positionoffsetunits) ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, gridorientationoffset, yaw)   ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, gridorientationoffset, pitch) ||
      PB_SUBFIELD_CHANGED(&lastPrefs_, &prefs, gridorientationoffset, roll);

    // sync our prefs state before updating locator
    lastPrefs_ = prefs;

    if (locatorChangeRequired)
    {
      configureLocator_(prefs);
    }

    forceRebuild_ = false;
  }
}

void LocalGridNode::configureLocator_(const simData::LocalGridPrefs& prefs)
{
  osg::ref_ptr<simVis::Locator> locator = this->getLocator();
  // suppress notification, leave that to endUpdate below
  locator->setComponentsToInherit(
    simVis::Locator::COMP_POSITION                    |
    (prefs.followyaw()   ? simVis::Locator::COMP_HEADING : 0) |
    (prefs.followpitch() ? simVis::Locator::COMP_PITCH   : 0) |
    (prefs.followroll()  ? simVis::Locator::COMP_ROLL    : 0), false);

  // positional offset:
  simCore::Vec3 posOffset;
  if (prefs.has_gridpositionoffset())
  {
    const simData::Position& pos = prefs.gridpositionoffset();
    const Units sizeUnits = simVis::convertUnitsToOsgEarth(prefs.positionoffsetunits());
    const float x = sizeUnits.convertTo(Units::METERS, pos.x());
    const float y = sizeUnits.convertTo(Units::METERS, pos.y());
    const float z = sizeUnits.convertTo(Units::METERS, pos.z());
    posOffset.set(x, y, z);
  }

  // orientation offset:
  simCore::Vec3 oriOffset;
  if (prefs.has_gridorientationoffset())
  {
    const simData::BodyOrientation& ori = prefs.gridorientationoffset();
    oriOffset.set(ori.yaw(), ori.pitch(), ori.roll());
  }

  // Suppress single notify on setLocalOffsets...
  locator->setLocalOffsets(posOffset, oriOffset, locator->getParentLocator()->getTime(), false);
  // ...instead send explicit notify for it and setComponentsToInherit above
  locator->endUpdate();
}

void LocalGridNode::syncWithLocator()
{
  // if not drawing, we don't need to update this
  if (!host_.valid() || getNodeMask() != DISPLAY_MASK_LOCAL_GRID)
    return;

  // call the base class to update the matrix.
  LocatorNode::syncWithLocator();

  if ((lastPrefs_.gridtype() != simData::LocalGridPrefs_Type_SPEED_RINGS) && (lastPrefs_.gridtype() != simData::LocalGridPrefs_Type_SPEED_LINE))
    return;

  if (calcSpeedParams_(lastPrefs_))
    rebuild_(lastPrefs_);
}

// determine host's update time and calculate host speed as required for current speed-rings mode
bool LocalGridNode::calcSpeedParams_(const simData::LocalGridPrefs& prefs)
{
  if (!prefs.speedring().useplatformspeed() && !prefs.speedring().usefixedtime())
    return false;
  const Locator* hostLocator = host_->getLocator();
  if (!hostLocator)
    return false;
  const PlatformNode* hostPlatform = dynamic_cast<const PlatformNode*>(host_.get());
  // if host is a platform, use its locator. if not, use host locator's parent locator.
  const Locator* hostPlatformLocator = (hostPlatform) ? hostLocator : hostLocator->getParentLocator();
  if (!hostPlatformLocator)
    return false;
 
  bool rebuild = false;
  // force rebuild if speed rings are displayed, using platform speed, and host velocity changed
  if (prefs.speedring().useplatformspeed())
  {
    const double speedMS = simCore::v3Length(hostPlatformLocator->getCoordinate().velocity());
    if (!simCore::areEqual(hostSpeedMS_, speedMS, 0.01))
    {
      hostSpeedMS_ = speedMS;
      rebuild = true;
    }
  }

  // if we are displaying speed rings with fixed time, rebuild the display when host locator time changes
  if (prefs.speedring().usefixedtime())
  {
    const double timeS = hostPlatformLocator->getTime();
    if (!simCore::areEqual(hostTimeS_, timeS))
    {
      hostTimeS_ = timeS;
      rebuild = true;
    }
  }
  return rebuild;
}

// creates a prototype Text template for the grid labels.
osgText::Text* LocalGridNode::createTextPrototype_(const simData::LocalGridPrefs& prefs, double value, const std::string& units, int precision) const
{
  std::stringstream buf;
  buf << std::fixed << std::setprecision(precision) << value << ' ' << units;
  std::string str = buf.str();
  return createTextPrototype_(prefs, str);
}

// creates a prototype Text template for the grid labels.
osgText::Text* LocalGridNode::createTextPrototype_(const simData::LocalGridPrefs& prefs, const std::string& str) const
{
  osgText::Text* label = new osgText::Text();
  label->setFont(simVis::Registry::instance()->getOrCreateFont(prefs.gridlabelfontname()));
  label->setAxisAlignment(osgText::TextBase::XY_PLANE);
  label->setBackdropType(prefs.gridlabeltextoutline() == simData::TO_NONE ? osgText::Text::NONE : osgText::Text::OUTLINE);
  label->setBackdropColor(simVis::Color(prefs.gridlabeloutlinecolor(), simVis::Color::RGBA));
  float outlineThickness = simVis::outlineThickness(prefs.gridlabeltextoutline());
  label->setBackdropOffset(outlineThickness, outlineThickness);
  label->setColor(simVis::Color(prefs.gridlabelcolor(), simVis::Color::RGBA));
  label->setCharacterSizeMode(osgText::TextBase::SCREEN_COORDS);
  // Scale font up, based on an eyeball estimate compared to SIMDIS 9
  label->setCharacterSize(prefs.gridlabelfontsize() * 1.8);
  label->setAlignment(osgText::TextBase::LEFT_BOTTOM);
  label->setText(str);

  return label;
}

// creates a Cartesian grid.
void LocalGridNode::createCartesian_(const simData::LocalGridPrefs& prefs, osg::Geode* geode) const
{
  // create one geometry for divisions, and another for sub-divisions
  osg::ref_ptr<osg::Geometry> geomSub = new osg::Geometry();
  osg::ref_ptr<osg::Geometry> geomDiv = new osg::Geometry();

  geomSub->setUseVertexBufferObjects(true);
  geomDiv->setUseVertexBufferObjects(true);

  osg::ref_ptr<osg::Vec3Array> vertSub = new osg::Vec3Array();
  osg::ref_ptr<osg::Vec3Array> vertDiv = new osg::Vec3Array();
  geomSub->setVertexArray(vertSub.get());
  geomDiv->setVertexArray(vertDiv.get());

  osg::ref_ptr<osg::Vec4Array> colorArraySub = new osg::Vec4Array();
  osg::ref_ptr<osg::Vec4Array> colorArrayDiv = new osg::Vec4Array();
  geomSub->setColorArray(colorArraySub.get());
  geomDiv->setColorArray(colorArrayDiv.get());

  geomSub->setColorBinding(osg::Geometry::BIND_OVERALL);
  geomDiv->setColorBinding(osg::Geometry::BIND_OVERALL);

  const Units sizeUnits = simVis::convertUnitsToOsgEarth(prefs.sizeunits());
  // Note that size is halved; it's provided in diameter, and we need it as radius
  const float size = sizeUnits.convertTo(Units::METERS, prefs.size()) * 0.5f;
  const int numDivisions    = prefs.gridsettings().numdivisions();
  const int numSubDivisions = prefs.gridsettings().numsubdivisions();
  const int numDivLines = (numDivisions * 2) + 3;
  const int numSubLines = (numDivLines-1) * (numSubDivisions + 1);

  float span = 2 * size;
  float divSpacing = span / (float)(numDivLines-1);
  float subSpacing = span / (float)(numSubLines);
  float x0 = -0.5 * span;
  float y0 = -0.5 * span;

  const osg::Vec4f color = osgEarth::Symbology::Color(prefs.gridcolor(), osgEarth::Symbology::Color::RGBA);
  osg::Vec4f subColor = color * 0.5;
  subColor.a() = 1.0;

  colorArraySub->push_back(subColor);
  colorArrayDiv->push_back(color);

  // Keep track of location in the primitive set array
  int primitiveSetStart = 0;

  // first draw the subdivision lines
  for (int s=0; s<numSubLines; ++s)
  {
    // skip sub lines that are coincident with main division lines
    if (s % (numSubDivisions+1) == 0)
      continue;

    float x = x0 + subSpacing * (float)s;
    addLineStrip_(*geomSub, *vertSub, primitiveSetStart, osg::Vec3(x, y0, 0), osg::Vec3(x, y0 + span, 0), NUM_POINTS_PER_LINE_STRIP);

    float y = y0 + subSpacing * (float)s;
    addLineStrip_(*geomSub, *vertSub, primitiveSetStart, osg::Vec3(x0, y, 0), osg::Vec3(x0 + span, y, 0), NUM_POINTS_PER_LINE_STRIP);
  }

  // second draw the main division lines and the text labels
  primitiveSetStart = 0;
  const std::string abbrev = sizeUnits.getAbbr();
  for (int p=0; p<numDivLines; ++p)
  {
    float x = x0 + divSpacing * (float)p;
    addLineStrip_(*geomDiv, *vertDiv, primitiveSetStart, osg::Vec3(x, y0, 0), osg::Vec3(x, y0 + span, 0), NUM_POINTS_PER_LINE_STRIP);

    // x-label:
    if (x < 0 && prefs.gridlabeldraw())
    {
      osg::ref_ptr<osgText::Text> label = createTextPrototype_(prefs,
        osgEarth::Units::convert(osgEarth::Units::METERS, sizeUnits, -x),
        abbrev, prefs.gridlabelprecision());
      label->setPosition(osg::Vec3(-x, 0, 0));
      geode->addDrawable(label);
    }

    float y = y0 + divSpacing * (float)p;
    addLineStrip_(*geomDiv, *vertDiv, primitiveSetStart, osg::Vec3(x0, y, 0), osg::Vec3(x0 + span, y, 0), NUM_POINTS_PER_LINE_STRIP);

    // y-label
    if (y > 0 && prefs.gridlabeldraw())
    {
      osg::ref_ptr<osgText::Text> label = createTextPrototype_(prefs,
        osgEarth::Units::convert(osgEarth::Units::METERS, sizeUnits, y),
        abbrev, prefs.gridlabelprecision());
      label->setPosition(osg::Vec3(0, y, 0));
      geode->addDrawable(label);
    }
  }

  // Add the drawable to the geode
  geode->addDrawable(geomSub);
  geode->addDrawable(geomDiv);
}

// creates a range-rings local grid with optional polar radials.
void LocalGridNode::createRangeRings_(const simData::LocalGridPrefs& prefs, osg::Geode* geode, bool includePolarRadials) const
{
  const Units sizeUnits = simVis::convertUnitsToOsgEarth(prefs.sizeunits());
  // Note that size is halved; it's provided in diameter, and we need it as radius
  const float size = sizeUnits.convertTo(Units::METERS, prefs.size()) * 0.5f;

  if (simCore::areEqual(size, 0.0))
    return;

  // if size exceeds this number there is an excessive UI responsiveness penalty
  if (size > MAX_RING_SIZE_M)
  {
    SIM_ERROR << "Range Rings radius exceeds maximum ring size." << std::endl;
    return;
  }

  const float sectorAngle     = prefs.gridsettings().sectorangle();
  const int   numDivisions    = prefs.gridsettings().numdivisions();
  const int   numSubDivisions = prefs.gridsettings().numsubdivisions();
  const int numRings          = (numDivisions + 1) * (numSubDivisions + 1);
  const float spacing         = size / osg::maximum(1, numRings);

  osg::Vec4f color = osgEarth::Symbology::Color(prefs.gridcolor(), osgEarth::Symbology::Color::RGBA);
  osg::Vec4f subColor = color * 0.5;
  subColor.a() = 1.0;

  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
  geom->setUseVertexBufferObjects(true);

  osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array();
  geom->setVertexArray(vertexArray.get());

  osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
  geom->setColorArray(colorArray.get());
  geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

  // rings:
  int primitiveSetStart = 0;
  const std::string abbrev = sizeUnits.getAbbr();
  for (int i = 0; i < numRings; ++i)
  {
    double radiusM = spacing * static_cast<double>(i + 1);
    float circum = 2.0 * M_PI * radiusM;
    const int segs = simCore::sdkMax(MIN_NUM_LINE_SEGMENTS, static_cast<int>(::ceil(circum / CIRCLE_QUANT_LEN)));
    float inc = M_TWOPI / static_cast<float>(segs);

    bool isMajorRing = ((i + 1) % (numSubDivisions + 1)) == 0;

    for (int j = 0; j < segs; ++j)
    {
      float angle = inc * j;
      float x = sin(angle);
      float y = cos(angle);
      vertexArray->push_back(osg::Vec3(x * radiusM, y * radiusM, 0));
      colorArray->push_back(isMajorRing ? color : subColor);
    }
    geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_LOOP, primitiveSetStart, segs));
    primitiveSetStart += segs;

    // label:
    if (isMajorRing && prefs.gridlabeldraw())
    {
      osg::ref_ptr<osgText::Text> label = createTextPrototype_(prefs,
        osgEarth::Units::convert(osgEarth::Units::METERS, sizeUnits, radiusM), abbrev, prefs.gridlabelprecision());
      label->setPosition(osg::Vec3(0.0f, radiusM, 0.0f));
      geode->addDrawable(label);

      osg::ref_ptr<osgText::Text> label2 = static_cast<osgText::Text*>(label->clone(osg::CopyOp::SHALLOW_COPY));
      label2->setPosition(osg::Vec3(radiusM, 0.0f, 0.0f));
      geode->addDrawable(label2);
    }
  }

  // Cross-hair lines don't get drawn for Range Rings, but do for Polar
  if (includePolarRadials)
  {
    addLineStrip_(*geom, *vertexArray, primitiveSetStart, osg::Vec3(0.f, -size, 0.f), osg::Vec3(0.f, size, 0.f), NUM_POINTS_PER_LINE_STRIP);
    addLineStrip_(*geom, *vertexArray, primitiveSetStart, osg::Vec3(-size, 0.f, 0.f), osg::Vec3(size, 0.f, 0.f), NUM_POINTS_PER_LINE_STRIP);
    for (int k = 0; k < NUM_POINTS_PER_LINE_STRIP * 2; ++k)
      colorArray->push_back(color);
  }

  if (includePolarRadials && sectorAngle > 0.0f)
  {
    int verts = 0;
    // drawing 2.5 vertices per ring
    float radialVertexFactor = 2.5f;
    float radialVertexSpacing = size / static_cast<float>(numRings * radialVertexFactor);
    for (float angle = sectorAngle; angle < 360.0f; angle += sectorAngle)
    {
      if (!osg::equivalent(fmod(angle, 90), 0)) // so we don't overdraw the main X/Y axes
      {
        float x = sin(osg::DegreesToRadians(angle));
        float y = cos(osg::DegreesToRadians(angle));
        for (int i = 0; i < numRings * radialVertexFactor; ++i)
        {
          vertexArray->push_back(osg::Vec3(x * radialVertexSpacing * static_cast<float>(i), y * radialVertexSpacing * static_cast<float>(i), 0));
          colorArray->push_back(subColor);
          verts++;
        }
      }
    }
    geom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, primitiveSetStart, verts));
  }

  geode->addDrawable(geom);
}

// creates a speed-rings local grid with optional polar radials.
void LocalGridNode::createSpeedRings_(const simData::LocalGridPrefs& prefs, osg::Geode* geode, bool drawSpeedLine) const
{
  // determine the speed to be used for calculating the rings
  double speedMS = 10.0f; // m/s; this default should never be needed, should be overridden by DefaultDataStoreValues
  if (prefs.speedring().useplatformspeed())
  {
    speedMS = hostSpeedMS_;
  }
  else if (prefs.speedring().has_speedtouse())
  {
    // using speedToUse, convert to m/s
    const Units prefSpeedUnits = simVis::convertUnitsToOsgEarth(prefs.speedring().speedunits());
    speedMS = prefSpeedUnits.convertTo(osgEarth::Units::METERS_PER_SECOND, prefs.speedring().speedtouse());
  }

  if (simCore::areEqual(speedMS, 0.0))
  {
    // do not display anything if speed is zero
    return;
  }

  // determine the time radius for the speed rings display
  double timeRadiusSeconds = 0;
  if (prefs.speedring().usefixedtime())
  {
    // as time moves forward, rings should shrink; if platform speed increases, rings should expand, and vice versa
    timeRadiusSeconds = fixedTime_ - hostTimeS_;
  }
  else
  {
    // not using fixed time, so use radius if specified, default to something if not specified
    const simCore::TimeFormat timeFormat = static_cast<simCore::TimeFormat>(prefs.speedring().timeformat());
    timeRadiusSeconds = prefs.speedring().radius();
    // convert values from specified timeFormat to UNITS_SECONDS
    if (timeFormat == simCore::TIMEFORMAT_MINUTES)
      timeRadiusSeconds *= simCore::SECPERMIN;
    else if (timeFormat == simCore::TIMEFORMAT_HOURS)
      timeRadiusSeconds *= simCore::SECPERHOUR;
  }

  // Verify that there is something to draw
  if (timeRadiusSeconds <= 0.0)
    return;

  const double sizeM = timeRadiusSeconds * speedMS;

  // if sizeM exceeds this number there is an excessive UI responsiveness penalty
  if (sizeM > MAX_RING_SIZE_M)
  {
    SIM_ERROR << "Speed Rings radius exceeds maximum ring size." << std::endl;
    return;
  }

  const float sectorAngle     = prefs.gridsettings().sectorangle();
  const int   numDivisions    = prefs.gridsettings().numdivisions();
  const int   numSubDivisions = prefs.gridsettings().numsubdivisions();
  const int numRings          = (numDivisions + 1) * (numSubDivisions + 1);
  const double spacingM       = sizeM / osg::maximum(1, numRings);

  osg::Vec4f color = osgEarth::Symbology::Color(prefs.gridcolor(), osgEarth::Symbology::Color::RGBA);
  osg::Vec4f subColor = color * 0.5f;
  subColor.a() = 1.0f;

  // label prefs and values
  const bool displayLabels    = prefs.gridlabeldraw();
  const bool displayTime      = prefs.speedring().displaytime();
  const Units prefSizeUnits   = simVis::convertUnitsToOsgEarth(prefs.sizeunits());
  const double spacingS = timeRadiusSeconds / osg::maximum(1, numRings);

  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
  geom->setUseVertexBufferObjects(true);

  osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array();
  geom->setVertexArray(vertexArray.get());

  osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array();
  geom->setColorArray(colorArray.get());
  geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

  // rings or speed line:
  int primitiveSetStart = 0;
  for (int i = 0; i < numRings; ++i)
  {
    double radiusM = spacingM * static_cast<double>(i + 1);
    bool isMajorRing = ((i + 1) % (numSubDivisions + 1)) == 0;

    // concentric rings:
    if (!drawSpeedLine)
    {
      double circum = 2.0 * M_PI * radiusM;
      const int segs = simCore::sdkMax(MIN_NUM_LINE_SEGMENTS, static_cast<int>(::ceil(circum / CIRCLE_QUANT_LEN)));
      double inc = M_TWOPI / segs;

      for (int j = 0; j < segs; ++j)
      {
        double angle = inc * j;
        double x = sin(angle);
        double y = cos(angle);
        vertexArray->push_back(osg::Vec3(static_cast<float>(x * radiusM), static_cast<float>(y * radiusM), 0.0f));
        colorArray->push_back(isMajorRing ? color : subColor);
      }
      geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_LOOP, primitiveSetStart, segs));
      primitiveSetStart += segs;
    }

    // label formatting:
    if (displayLabels && isMajorRing)
    {
      osg::ref_ptr<osgText::Text> label = NULL;
      if (displayTime)
      {
        double radiusS = spacingS * static_cast<double>(i + 1);
        const simData::ElapsedTimeFormat timeFormat = prefs.speedring().timeformat();
        assert(timeFormat == simData::ELAPSED_HOURS || timeFormat == simData::ELAPSED_MINUTES || timeFormat == simData::ELAPSED_SECONDS);
        std::stringstream buf;

        // show HH:MM:SS.SS
        if (timeFormat == simData::ELAPSED_HOURS)
        {
          simCore::HoursTimeFormatter formatter;
          formatter.toStream(buf, radiusS, prefs.gridlabelprecision());
        }

        // show MM:SS.SS
        else if (timeFormat == simData::ELAPSED_MINUTES)
        {
          simCore::MinutesTimeFormatter formatter;
          formatter.toStream(buf, radiusS, prefs.gridlabelprecision());
        }

        // show SS.SS
        else
        {
          simCore::SecondsTimeFormatter formatter;
          formatter.toStream(buf, radiusS, prefs.gridlabelprecision());
        }
        std::string str = buf.str();
        label = createTextPrototype_(prefs, str);
      }
      else
      {
        // displaying distance, not time; convert labels value from meters to local grid units pref
        double radius = osgEarth::Units::METERS.convertTo(prefSizeUnits, radiusM);
        label = createTextPrototype_(prefs, radius, prefSizeUnits.getAbbr(), prefs.gridlabelprecision());
      }

      // text label for major axis/speed line:
      label->setPosition(osg::Vec3(0.0f, radiusM, 0.0f));
      geode->addDrawable(label);

      // text label for minor axis:
      if (!drawSpeedLine)
      {
        osg::ref_ptr<osgText::Text> label2 = static_cast<osgText::Text*>(label->clone(osg::CopyOp::SHALLOW_COPY));
        label2->setPosition(osg::Vec3(radiusM, 0.0f, 0.0f));
        geode->addDrawable(label2);
      }
    }
  }

  if (!drawSpeedLine)
  {
    // speed ring axes:
    addLineStrip_(*geom, *vertexArray, primitiveSetStart, osg::Vec3(0.f, -sizeM, 0.f), osg::Vec3(0.f, sizeM, 0.f), NUM_POINTS_PER_LINE_STRIP);
    addLineStrip_(*geom, *vertexArray, primitiveSetStart, osg::Vec3(-sizeM, 0.f, 0.f), osg::Vec3(sizeM, 0.f, 0.f), NUM_POINTS_PER_LINE_STRIP);
    for (int i = 0; i < NUM_POINTS_PER_LINE_STRIP * 2; i++)
      colorArray->push_back(color);
  }
  else
  {
    addLineStrip_(*geom, *vertexArray, primitiveSetStart, osg::Vec3(0.f, 0.f, 0.f), osg::Vec3(0.f, sizeM, 0.f), NUM_POINTS_PER_LINE_STRIP);
    for (int k = 0; k < NUM_POINTS_PER_LINE_STRIP; ++k)
      colorArray->push_back(color);
  }

  // draw polar radials for speed rings
  if (!drawSpeedLine && sectorAngle > 0.0f)
  {
    int verts = 0;
    // drawing 2.5 vertices per ring
    float radialVertexFactor = 2.5f;
    float radialVertexSpacing = spacingM / radialVertexFactor;
    for (float angle = sectorAngle; angle < 360.0f; angle += sectorAngle)
    {
      if (!osg::equivalent(fmod(angle, 90), 0)) // so we don't overdraw the main X/Y axes
      {
        float x = sinf(osg::DegreesToRadians(angle));
        float y = cosf(osg::DegreesToRadians(angle));
        for (int i = 0; i < numRings * radialVertexFactor; ++i)
        {
          vertexArray->push_back(osg::Vec3(x * radialVertexSpacing * static_cast<float>(i), y * radialVertexSpacing * static_cast<float>(i), 0));
          colorArray->push_back(subColor);
          verts++;
        }
      }
    }
    geom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, primitiveSetStart, verts));
  }
  geode->addDrawable(geom);
}

void LocalGridNode::addLineStrip_(osg::Geometry& geom, osg::Vec3Array& vertices, int& primitiveSetStart,
  const osg::Vec3& start, const osg::Vec3& end, int numPointsPerLine) const
{
  // Avoid divide-by-zero problems
  if (numPointsPerLine < 2)
    return;

  const osg::Vec3 delta = (end - start);
  for (int k = 0; k < numPointsPerLine; ++k)
  {
    // Translate [0,numPointsPerLine) into [0,1]
    const float pct = static_cast<float>(k) / (numPointsPerLine - 1);
    vertices.push_back(start + delta * pct);
  }

  // Add line strips for each line
  geom.addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, primitiveSetStart, numPointsPerLine));
  primitiveSetStart += numPointsPerLine;
}

}
