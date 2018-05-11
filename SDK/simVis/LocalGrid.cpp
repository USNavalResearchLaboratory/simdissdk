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
#include "simVis/LineDrawable.h"
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
const unsigned int MIN_NUM_LINE_SEGMENTS = 50;

// Number of points in the subdivided line strip for horizontal and cross-hair grids
const unsigned int NUM_POINTS_PER_LINE_STRIP = 10;
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

  // set up the default state set and render bins:
  getOrCreateStateSet()->setRenderBinDetails(BIN_LOCAL_GRID, BIN_GLOBAL_SIMSDK);

  graphicsGroup_ = new osg::Geode();
  graphicsGroup_->setName("simVis::LocalGridNode::GraphicsGeode");
  osg::StateSet* ss = graphicsGroup_->getOrCreateStateSet();
  PointSize::setValues(ss, 1.5f, osg::StateAttribute::ON);
  osgEarth::LineDrawable::installShader(ss);
  addChild(graphicsGroup_.get());

  labelGroup_ = new osg::Geode();
  labelGroup_->setName("simVis::LocalGridNode::LabelGeode");
  addChild(labelGroup_.get());
}

LocalGridNode::~LocalGridNode() {}

void LocalGridNode::rebuild_(const simData::LocalGridPrefs& prefs)
{
  // LocalGrid is constructed with 2 children; they are not removed
  assert(getNumChildren() == 2);
  graphicsGroup_->removeChildren(0, graphicsGroup_->getNumChildren());
  labelGroup_->removeChildren(0, labelGroup_->getNumChildren());

  // build for the appropriate grid type:
  switch (prefs.gridtype())
  {
  case simData::LocalGridPrefs_Type_CARTESIAN:
    createCartesian_(prefs, graphicsGroup_.get(), labelGroup_.get());
    break;

  case simData::LocalGridPrefs_Type_POLAR:
    createRangeRings_(prefs, graphicsGroup_.get(), labelGroup_.get(), true);
    break;

  case simData::LocalGridPrefs_Type_RANGE_RINGS:
    createRangeRings_(prefs, graphicsGroup_.get(), labelGroup_.get(), false);
    break;

  case simData::LocalGridPrefs_Type_SPEED_RINGS:
    createSpeedRings_(prefs, graphicsGroup_.get(), labelGroup_.get(), false);
    break;

  case simData::LocalGridPrefs_Type_SPEED_LINE:
    createSpeedRings_(prefs, graphicsGroup_.get(), labelGroup_.get(), true);
    break;
  }

  // have to run ShaderGenerator after adding labels
  if (labelGroup_->getNumChildren() > 0)
    osgEarth::Registry::shaderGenerator().run(labelGroup_.get());
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
void LocalGridNode::createCartesian_(const simData::LocalGridPrefs& prefs, osg::Geode* geomGroup, osg::Geode* labelGroup) const
{
  const Units sizeUnits = simVis::convertUnitsToOsgEarth(prefs.sizeunits());
  // Note that size is halved; it's provided in diameter, and we need it as radius
  const float size = sizeUnits.convertTo(Units::METERS, prefs.size()) * 0.5f;
  const int numDivisions    = prefs.gridsettings().numdivisions();
  const int numSubDivisions = prefs.gridsettings().numsubdivisions();
  const int numDivLines = (numDivisions * 2) + 3;
  const int numSubLines = (numDivLines-1) * (numSubDivisions + 1);

  const float span = 2.f * size;
  const float divSpacing = span / (numDivLines-1);
  const float subSpacing = span / (numSubLines);
  const float x0 = -0.5f * span;
  const float y0 = -0.5f * span;

  const osg::Vec4f& color = osgEarth::Symbology::Color(prefs.gridcolor(), osgEarth::Symbology::Color::RGBA);
  const osg::Vec4f& subColor = osgEarth::Symbology::Color(color * 0.5f, 1.0f);

  // first draw the subdivision lines
  for (int s=0; s<numSubLines; ++s)
  {
    // skip sub lines that are coincident with main division lines
    if (s % (numSubDivisions+1) == 0)
      continue;

    {
      const float x = x0 + subSpacing * s;
      osgEarth::LineDrawable* sub1 = new osgEarth::LineDrawable(GL_LINE_STRIP);
      VectorScaling::generatePoints(*sub1, osg::Vec3(x, y0, 0.f), osg::Vec3(x, y0 + span, 0.f), NUM_POINTS_PER_LINE_STRIP);
      sub1->setName("simVis::LocalGridNode::GridSubDivision1");
      sub1->setColor(subColor);
      geomGroup->addDrawable(sub1);
    }
    {
      const float y = y0 + subSpacing * s;
      osgEarth::LineDrawable* sub2 = new osgEarth::LineDrawable(GL_LINE_STRIP);
      VectorScaling::generatePoints(*sub2, osg::Vec3(x0, y, 0.f), osg::Vec3(x0 + span, y, 0.f), NUM_POINTS_PER_LINE_STRIP);
      sub2->setName("simVis::LocalGridNode::GridSubDivision2");
      sub2->setColor(subColor);
      geomGroup->addDrawable(sub2);
    }
  }

  // second draw the main division lines and the text labels
  const std::string abbrev = sizeUnits.getAbbr();
  for (int p=0; p < numDivLines; ++p)
  {
    const float x = x0 + divSpacing * p;
    {
      osgEarth::LineDrawable* div1 = new osgEarth::LineDrawable(GL_LINE_STRIP);
      VectorScaling::generatePoints(*div1, osg::Vec3(x, y0, 0.f), osg::Vec3(x, y0 + span, 0.f), NUM_POINTS_PER_LINE_STRIP);
      div1->setName("simVis::LocalGridNode::GridDivision1");
      div1->setColor(color);
      geomGroup->addDrawable(div1);
    }
    // x-label:
    if (x < 0 && prefs.gridlabeldraw())
    {
      osg::ref_ptr<osgText::Text> label = createTextPrototype_(prefs,
        osgEarth::Units::convert(osgEarth::Units::METERS, sizeUnits, -x),
        abbrev, prefs.gridlabelprecision());
      label->setPosition(osg::Vec3(-x, 0.f, 0.f));
      labelGroup->addDrawable(label.get());
    }

    const float y = y0 + divSpacing * p;
    {
      osgEarth::LineDrawable* div2 = new osgEarth::LineDrawable(GL_LINE_STRIP);
      VectorScaling::generatePoints(*div2, osg::Vec3(x0, y, 0.f), osg::Vec3(x0 + span, y, 0.f), NUM_POINTS_PER_LINE_STRIP);
      div2->setName("simVis::LocalGridNode::GridDivision2");
      div2->setColor(color);
      geomGroup->addDrawable(div2);
    }
    // y-label
    if (y > 0 && prefs.gridlabeldraw())
    {
      osg::ref_ptr<osgText::Text> label = createTextPrototype_(prefs,
        osgEarth::Units::convert(osgEarth::Units::METERS, sizeUnits, y),
        abbrev, prefs.gridlabelprecision());
      label->setPosition(osg::Vec3(0.f, y, 0.f));
      labelGroup->addDrawable(label.get());
    }
  }
}

// creates a range-rings local grid with optional polar radials.
void LocalGridNode::createRangeRings_(const simData::LocalGridPrefs& prefs, osg::Geode* geomGroup, osg::Geode* labelGroup, bool includePolarRadials) const
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

  const osg::Vec4f& color = osgEarth::Symbology::Color(prefs.gridcolor(), osgEarth::Symbology::Color::RGBA);
  const osg::Vec4f& subColor = osgEarth::Symbology::Color(color * 0.5f, 1.0f);

  // rings:
  const std::string abbrev = sizeUnits.getAbbr();
  for (int i = 0; i < numRings; ++i)
  {
    const float radiusM = spacing * (i + 1);
    const float circum = 2.0 * M_PI * radiusM;
    const unsigned int segs = simCore::sdkMax(MIN_NUM_LINE_SEGMENTS, static_cast<unsigned int>(::ceil(circum / CIRCLE_QUANT_LEN)));
    const float inc = M_TWOPI / segs;

    const bool isMajorRing = ((i + 1) % (numSubDivisions + 1)) == 0;

    osgEarth::LineDrawable* ring = new osgEarth::LineDrawable(GL_LINE_LOOP);
    ring->setName("simVis::LocalGridNode::Ring");
    ring->allocate(segs);
    ring->setColor(isMajorRing ? color : subColor);

    for (unsigned int j = 0; j < segs; ++j)
    {
      const float angle = inc * j;
      const float x = sin(angle);
      const float y = cos(angle);
      ring->setVertex(j, osg::Vec3(x * radiusM, y * radiusM, 0.f));
    }
    geomGroup->addDrawable(ring);

    // label:
    if (isMajorRing && prefs.gridlabeldraw())
    {
      osg::ref_ptr<osgText::Text> label = createTextPrototype_(prefs,
        osgEarth::Units::convert(osgEarth::Units::METERS, sizeUnits, radiusM), abbrev, prefs.gridlabelprecision());
      label->setPosition(osg::Vec3(0.0f, radiusM, 0.0f));
      labelGroup->addDrawable(label.get());

      osg::ref_ptr<osgText::Text> label2 = static_cast<osgText::Text*>(label->clone(osg::CopyOp::SHALLOW_COPY));
      label2->setPosition(osg::Vec3(radiusM, 0.0f, 0.0f));
      labelGroup->addDrawable(label2.get());
    }
  }

  // Cross-hair lines don't get drawn for Range Rings, but do for Polar
  if (includePolarRadials)
  {
    {
      osgEarth::LineDrawable* radial1 = new osgEarth::LineDrawable(GL_LINE_STRIP);
      VectorScaling::generatePoints(*radial1, osg::Vec3(0.f, -size, 0.f), osg::Vec3(0.f, size, 0.f), NUM_POINTS_PER_LINE_STRIP);
      radial1->setName("simVis::LocalGridNode::Radial1");
      radial1->setColor(color);
      geomGroup->addDrawable(radial1);
    }
    {
      osgEarth::LineDrawable* radial2 = new osgEarth::LineDrawable(GL_LINE_STRIP);
      VectorScaling::generatePoints(*radial2, osg::Vec3(-size, 0.f, 0.f), osg::Vec3(size, 0.f, 0.f), NUM_POINTS_PER_LINE_STRIP);
      radial2->setName("simVis::LocalGridNode::Radial2");
      radial2->setColor(color);
      geomGroup->addDrawable(radial2);
    }
  }

  if (includePolarRadials && sectorAngle > 0.0f)
  {
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
    geom->setName("simVis::LocalGridNode::RadialPoints");
    geom->setUseVertexBufferObjects(true);
    geom->setUseDisplayList(false);

    osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    geom->setVertexArray(vertexArray.get());

    osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
    (*colorArray)[0] = subColor;
    geom->setColorArray(colorArray.get());

    // drawing 2.5 vertices per ring
    const float radialVertexFactor = 2.5f;
    const float radialVertexSpacing = size / (numRings * radialVertexFactor);
    vertexArray->reserve((360.f / sectorAngle) * numRings * radialVertexFactor);

    for (float angle = sectorAngle; angle < 360.0f; angle += sectorAngle)
    {
      if (!osg::equivalent(fmod(angle, 90), 0)) // so we don't overdraw the main X/Y axes
      {
        const float x = sin(osg::DegreesToRadians(angle));
        const float y = cos(osg::DegreesToRadians(angle));
        for (int i = 0; i < numRings * radialVertexFactor; ++i)
        {
          vertexArray->push_back(osg::Vec3(x * radialVertexSpacing * i, y * radialVertexSpacing * i, 0.f));
        }
      }
    }
    geom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vertexArray->size()));
    geomGroup->addDrawable(geom.get());
  }
}

// creates a speed-rings local grid with optional polar radials.
void LocalGridNode::createSpeedRings_(const simData::LocalGridPrefs& prefs, osg::Geode* geomGroup, osg::Geode* labelGroup, bool drawSpeedLine) const
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
  const int   numRings        = (numDivisions + 1) * (numSubDivisions + 1);
  const float spacingM        = sizeM / osg::maximum(1, numRings);

  const osg::Vec4f& color = osgEarth::Symbology::Color(prefs.gridcolor(), osgEarth::Symbology::Color::RGBA);
  const osg::Vec4f& subColor = osgEarth::Symbology::Color(color * 0.5f, 1.0f);

  // label prefs and values
  const bool displayLabels    = prefs.gridlabeldraw();
  const bool displayTime      = prefs.speedring().displaytime();
  const Units prefSizeUnits   = simVis::convertUnitsToOsgEarth(prefs.sizeunits());
  const double spacingS = timeRadiusSeconds / osg::maximum(1, numRings);

  // rings or speed line:
  for (int i = 0; i < numRings; ++i)
  {
    const float radiusM = spacingM * (i + 1);
    const bool isMajorRing = ((i + 1) % (numSubDivisions + 1)) == 0;

    // concentric rings:
    if (!drawSpeedLine)
    {
      const double circum = 2.0 * M_PI * radiusM;
      const unsigned int segs = simCore::sdkMax(MIN_NUM_LINE_SEGMENTS, static_cast<unsigned int>(::ceil(circum / CIRCLE_QUANT_LEN)));
      const float inc = M_TWOPI / segs;

      osgEarth::LineDrawable* ring = new osgEarth::LineDrawable(GL_LINE_LOOP);
      ring->setName("simVis::LocalGridNode::SpeedRing");
      ring->allocate(segs);
      ring->setColor(isMajorRing ? color : subColor);

      for (unsigned int j = 0; j < segs; ++j)
      {
        const float angle = inc * j;
        const float x = sin(angle);
        const float y = cos(angle);
        ring->setVertex(j, osg::Vec3(x * radiusM, y * radiusM, 0.0f));
      }
      geomGroup->addDrawable(ring);
    }

    // label formatting:
    if (displayLabels && isMajorRing)
    {
      osg::ref_ptr<osgText::Text> label = NULL;
      if (displayTime)
      {
        const double radiusS = spacingS * (i + 1);
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
        const double radius = osgEarth::Units::METERS.convertTo(prefSizeUnits, radiusM);
        label = createTextPrototype_(prefs, radius, prefSizeUnits.getAbbr(), prefs.gridlabelprecision());
      }

      // text label for major axis/speed line:
      label->setPosition(osg::Vec3(0.0f, radiusM, 0.0f));
      labelGroup->addDrawable(label.get());

      // text label for minor axis:
      if (!drawSpeedLine)
      {
        osg::ref_ptr<osgText::Text> label2 = static_cast<osgText::Text*>(label->clone(osg::CopyOp::SHALLOW_COPY));
        label2->setPosition(osg::Vec3(radiusM, 0.0f, 0.0f));
        labelGroup->addDrawable(label2.get());
      }
    }
  }

  const float sizeMfloat = static_cast<float>(sizeM);
  if (!drawSpeedLine)
  {
    // speed ring axes:
    {
      osgEarth::LineDrawable* radial1 = new osgEarth::LineDrawable(GL_LINE_STRIP);
      VectorScaling::generatePoints(*radial1, osg::Vec3(0.f, -sizeMfloat, 0.f), osg::Vec3(0.f, sizeMfloat, 0.f), NUM_POINTS_PER_LINE_STRIP);
      radial1->setName("simVis::LocalGridNode::SpeedRingAxis1");
      radial1->setColor(color);
      geomGroup->addDrawable(radial1);
    }
    {
      osgEarth::LineDrawable* radial2 = new osgEarth::LineDrawable(GL_LINE_STRIP);
      VectorScaling::generatePoints(*radial2, osg::Vec3(-sizeMfloat, 0.f, 0.f), osg::Vec3(sizeMfloat, 0.f, 0.f), NUM_POINTS_PER_LINE_STRIP);
      radial2->setName("simVis::LocalGridNode::SpeedRingAxis2");
      radial2->setColor(color);
      geomGroup->addDrawable(radial2);
    }
  }
  else
  {
    osgEarth::LineDrawable* speedLine = new osgEarth::LineDrawable(GL_LINE_STRIP);
    VectorScaling::generatePoints(*speedLine, osg::Vec3(), osg::Vec3(0.f, sizeMfloat, 0.f), NUM_POINTS_PER_LINE_STRIP);
    speedLine->setName("simVis::LocalGridNode::SpeedLine");
    speedLine->setColor(color);
    geomGroup->addDrawable(speedLine);
  }

  // draw polar radials for speed rings
  if (!drawSpeedLine && sectorAngle > 0.0f)
  {
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
    geom->setName("simVis::LocalGridNode::RadialPoints");
    geom->setUseVertexBufferObjects(true);
    geom->setUseDisplayList(false);

    osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    geom->setVertexArray(vertexArray.get());

    osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
    (*colorArray)[0] = subColor;
    geom->setColorArray(colorArray.get());

    // drawing 2.5 vertices per ring
    const float radialVertexFactor = 2.5f;
    const float radialVertexSpacing = spacingM / radialVertexFactor;
    vertexArray->reserve((360.f / sectorAngle) * numRings * radialVertexFactor);

    for (float angle = sectorAngle; angle < 360.0f; angle += sectorAngle)
    {
      if (!osg::equivalent(fmod(angle, 90), 0)) // so we don't overdraw the main X/Y axes
      {
        const float x = sinf(osg::DegreesToRadians(angle));
        const float y = cosf(osg::DegreesToRadians(angle));
        for (int i = 0; i < numRings * radialVertexFactor; ++i)
        {
          vertexArray->push_back(osg::Vec3(x * radialVertexSpacing * i, y * radialVertexSpacing * i, 0.f));
        }
      }
    }
    geom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vertexArray->size()));
    geomGroup->addDrawable(geom.get());
  }
}

}
