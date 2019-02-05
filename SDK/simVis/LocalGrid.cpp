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

namespace simVis
{
// --------------------------------------------------------------------------
namespace
{
const float CIRCLE_QUANT_LEN = 50.0f; // segment length for quantized circle
const float RADIAL_VERTEX_FACTOR = 2.5f; // ratio of sector points to range rings

// limit range ring and speed ring size to avoid excessive UI responsiveness penalty.
// this number derived from trial and error, and corresponds to a several minute response time.
const double MAX_RING_SIZE_M = 6e+06;

// minimum number of line segments in a polar ring
const unsigned int MIN_NUM_LINE_SEGMENTS = 50;

// Number of points in the subdivided line strip for horizontal and cross-hair grids
const unsigned int NUM_POINTS_PER_LINE_STRIP = 10;

/// Base Class for local grid label types.
class LocalGridLabel : public osgText::Text
{
public:
  explicit LocalGridLabel(const simData::LocalGridPrefs& prefs)
  {
    setFont(simVis::Registry::instance()->getOrCreateFont(prefs.gridlabelfontname()));
    setAxisAlignment(osgText::TextBase::SCREEN);
    setBackdropType(prefs.gridlabeltextoutline() == simData::TO_NONE ? osgText::Text::NONE : osgText::Text::OUTLINE);
    setBackdropColor(simVis::Color(prefs.gridlabeloutlinecolor(), simVis::Color::RGBA));
    const float outlineThickness = simVis::outlineThickness(prefs.gridlabeltextoutline());
    setBackdropOffset(outlineThickness, outlineThickness);
    setColor(simVis::Color(prefs.gridlabelcolor(), simVis::Color::RGBA));
    setCharacterSizeMode(osgText::TextBase::SCREEN_COORDS);
    setCharacterSize(simVis::osgFontSize(prefs.gridlabelfontsize()));
    setAlignment(osgText::TextBase::LEFT_BOTTOM);
  }

  LocalGridLabel(const LocalGridLabel& text, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY)
    : osgText::Text(text, copyop)
  {
  }
};

/// Label class specialized for Cartesian grid types.
class CartesianGridLabel : public LocalGridLabel
{
public:
  CartesianGridLabel(const simData::LocalGridPrefs& prefs, float val) : LocalGridLabel(prefs)
  {
    std::stringstream buf;
    const osgEarth::Units prefSizeUnits = simVis::convertUnitsToOsgEarth(prefs.sizeunits());
    buf << std::fixed << std::setprecision(prefs.gridlabelprecision()) << val << ' ' << prefSizeUnits.getAbbr();
    setText(buf.str());
  }
};

/// Label class specialized for Polar, RangeRing, SpeedRing and SpeedLine grid types.
class RingLabel : public LocalGridLabel
{
public:
  RingLabel(const simData::LocalGridPrefs& prefs, unsigned int ring, bool isMajorAxisLabel)
    : LocalGridLabel(prefs),
    ring_(ring),
    isMajorAxisLabel_(isMajorAxisLabel)
  {
  }

  // this provides for minor-axis copy of major-axis label
  RingLabel(const RingLabel& label, bool isMajorAxisLabel, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY)
    : LocalGridLabel(label, copyop),
    ring_(label.ring_),
    isMajorAxisLabel_(isMajorAxisLabel)
  {
  }

  void update(const simData::LocalGridPrefs& prefs, double sizeM, double timeRadiusSeconds)
  {
    if (!prefs.speedring().displaytime())
    {
      update(prefs, sizeM);
      return;
    }
    if (timeRadiusSeconds <= 0.0)
    {
      setText("");
      return;
    }
    const unsigned int numDivisions = prefs.gridsettings().numdivisions();
    const unsigned int numSubDivisions = prefs.gridsettings().numsubdivisions();
    const unsigned int numRings = osg::maximum(1u, (numDivisions + 1) * (numSubDivisions + 1));

    const double spacingS = timeRadiusSeconds / numRings;
    const double radiusS = spacingS * (ring_ + 1);

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
    setText(buf.str());

    const float spacingM = sizeM / numRings;
    const float radiusM = spacingM * (ring_ + 1);
    updatePosition_(radiusM);
  }

  void update(const simData::LocalGridPrefs& prefs, double sizeM)
  {
    if (sizeM <= 0.0)
    {
      setText("");
      return;
    }
    const unsigned int numDivisions = prefs.gridsettings().numdivisions();
    const unsigned int numSubDivisions = prefs.gridsettings().numsubdivisions();
    const unsigned int numRings = osg::maximum(1u, (numDivisions + 1) * (numSubDivisions + 1));
    const float spacingM = sizeM / numRings;
    const float radiusM = spacingM * (ring_ + 1);

    // displaying distance, not time; convert labels value from meters to local grid units pref
    const osgEarth::Units prefSizeUnits = simVis::convertUnitsToOsgEarth(prefs.sizeunits());
    const double radius = osgEarth::Units::METERS.convertTo(prefSizeUnits, radiusM);
    std::stringstream buf;
    buf << std::fixed << std::setprecision(prefs.gridlabelprecision()) << radius << ' ' << prefSizeUnits.getAbbr();
    setText(buf.str());

    updatePosition_(radiusM);
  }
private:
  void updatePosition_(float radiusM)
  {
    if (isMajorAxisLabel_)
      setPosition(osg::Vec3(0.0f, radiusM, 0.0f));
    else
      setPosition(osg::Vec3(radiusM, 0.0f, 0.0f));
  }

  unsigned int ring_;
  bool isMajorAxisLabel_;
};
/// Geometry for a simple linestrip with fixed NUM_POINTS_PER_LINE_STRIP
class LineStrip : public osg::Geometry
{
public:
  LineStrip()
  {
    setUseVertexBufferObjects(true);
    setUseDisplayList(false);

    osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX, NUM_POINTS_PER_LINE_STRIP);
    setVertexArray(vertexArray.get());

    osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
    (*colorArray)[0] = simVis::Color::White;
    setColorArray(colorArray.get());

    addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, vertexArray->size()));
  }

  void setColor(const osg::Vec4f& color)
  {
    osg::Vec4Array* colorArray = static_cast<osg::Vec4Array*>(getColorArray());
    if (!colorArray)
    {
      assert(0);
      return;
    }
    (*colorArray)[0] = color;
  }

  void update(const osg::Vec3f& start, const osg::Vec3f& end)
  {
    osg::ref_ptr<osg::Vec3Array> vertexArray = dynamic_cast<osg::Vec3Array*>(getVertexArray());
    if (!vertexArray)
    {
      assert(0);
      return;
    }
    assert(vertexArray->getNumElements() == NUM_POINTS_PER_LINE_STRIP);
    VectorScaling::generatePoints(*(vertexArray.get()), start, end);
    vertexArray->dirty();
  }
};
/// Geometry for SpeedLine grid types.
class SpeedLine : public LineStrip
{
public:
  SpeedLine()
  {
    setName("simVis::LocalGridNode::SpeedLine");
  }
  void update(double sizeM)
  {
    LineStrip::update(osg::Vec3(), osg::Vec3(0.f, static_cast<float>(sizeM), 0.f));
  }
};

/// Geometry for axes in Polar and SpeedRing grid types.
class Axis : public LineStrip
{
public:
  explicit Axis(bool isMajorAxis)
    : isMajorAxis_(isMajorAxis)
  {
    if (isMajorAxis_)
      setName("simVis::LocalGridNode::MajorAxis");
    else
      setName("simVis::LocalGridNode::MinorAxis");
  }
  void update(double sizeM)
  {
    if (isMajorAxis_)
      LineStrip::update(osg::Vec3(0.f, -sizeM, 0.f), osg::Vec3(0.f, sizeM, 0.f));
    else
      LineStrip::update(osg::Vec3(-sizeM, 0.f, 0.f), osg::Vec3(sizeM, 0.f, 0.f));
  }
private:
  bool isMajorAxis_;
};

/// Geometry for off-axis sectors in Polar and SpeedRing grid types.
class RadialPoints : public osg::Geometry
{
public:
  RadialPoints(const osg::Vec4f& color, float sectorAngleDeg, unsigned int numRings)
    : sectorAngleDeg_(sectorAngleDeg),
    numRings_(numRings)
  {
    setName("simVis::LocalGridNode::RadialPoints");
    setUseVertexBufferObjects(true);
    setUseDisplayList(false);

    // determine how many vertices we'll actually use
    unsigned int vertexCount = 0;
    // skip 0 and 360
    for (float angle = sectorAngleDeg_; angle < 360.0f; angle += sectorAngleDeg_)
    {
      if (osg::equivalent(static_cast<float>(fmod(angle, 90.f)), 0.f)) // don't overdraw the main X/Y axes
        continue;
      // skip 0
      for (int i = 1; i < numRings_ * RADIAL_VERTEX_FACTOR; ++i)
      {
        if (osg::equivalent(static_cast<float>(fmod(static_cast<float>(i), RADIAL_VERTEX_FACTOR)), 0.f)) // don't overdraw the rings
          continue;
        vertexCount++;
      }
    }
    osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX, vertexCount);
    setVertexArray(vertexArray.get());

    osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
    (*colorArray)[0] = color;
    setColorArray(colorArray.get());

    addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vertexArray->size()));
  }
  void update(double sizeM)
  {
    osg::ref_ptr<osg::Vec3Array> vertexArray = dynamic_cast<osg::Vec3Array*>(getVertexArray());
    if (!vertexArray)
    {
      assert(0);
      return;
    }
    const float spacingM = sizeM / osg::maximum(1u, numRings_);
    const float radialVertexSpacing = spacingM / RADIAL_VERTEX_FACTOR;
    size_t index = 0;
    // skip 0 and 360
    for (float angle = sectorAngleDeg_; angle < 360.0f; angle += sectorAngleDeg_)
    {
      if (osg::equivalent(static_cast<float>(fmod(angle, 90.f)), 0.f)) // don't overdraw the main X/Y axes
        continue;
      const float angleRad = osg::DegreesToRadians(angle);
      const float x = sinf(angleRad);
      const float y = cosf(angleRad);
      // skip 0
      for (int i = 1; i < numRings_ * RADIAL_VERTEX_FACTOR; ++i)
      {
        if (osg::equivalent(static_cast<float>(fmod(static_cast<float>(i), RADIAL_VERTEX_FACTOR)), 0.f)) // don't overdraw the rings
          continue;
        // if assert fails, re-check algorithm for determining vertexCount in constructor
        assert(index < vertexArray->getNumElements());
        (*vertexArray)[index] = osg::Vec3(x * radialVertexSpacing * i, y * radialVertexSpacing * i, 0.f);
        index++;
      }
    }
    vertexArray->dirty();
  }
private:
  float sectorAngleDeg_;
  unsigned int numRings_;
};

/// Geometry for range rings in Polar, RangeRing and SpeedRing grid types.
class RangeRing : public osg::Geometry
{
public:
  explicit RangeRing(unsigned int ring)
    : ring_(ring)
  {
    setName("simVis::LocalGridNode::RangeRing");
    setUseVertexBufferObjects(true);
    setUseDisplayList(false);

    osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    setVertexArray(vertexArray.get());

    osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
    (*colorArray)[0] = simVis::Color::White;
    setColorArray(colorArray.get());

    drawArray_ = new osg::DrawArrays(GL_LINE_LOOP);
    addPrimitiveSet(drawArray_.get());
  }

  void setColor(const osg::Vec4f& color)
  {
    osg::Vec4Array* colorArray = static_cast<osg::Vec4Array*>(getColorArray());
    if (!colorArray)
    {
      assert(0);
      return;
    }
    (*colorArray)[0] = color;
  }

  void update(const simData::LocalGridPrefs& prefs, double sizeM)
  {
    osg::ref_ptr<osg::Vec3Array> vertexArray = dynamic_cast<osg::Vec3Array*>(getVertexArray());
    if (!vertexArray)
    {
      assert(0);
      return;
    }
    if (sizeM <= 0.0)
    {
      drawArray_->setFirst(0);
      drawArray_->setCount(0);
      return;
    }
    const unsigned int numDivisions = prefs.gridsettings().numdivisions();
    const unsigned int numSubDivisions = prefs.gridsettings().numsubdivisions();
    const unsigned int numRings = osg::maximum(1u, (numDivisions + 1) * (numSubDivisions + 1));
    const float spacingM = sizeM / numRings;
    const float radiusM = spacingM * (ring_ + 1);
    const double circum = 2.0 * M_PI * radiusM;
    const unsigned int segs = simCore::sdkMax(MIN_NUM_LINE_SEGMENTS, static_cast<unsigned int>(::ceil(circum / CIRCLE_QUANT_LEN)));
    const float inc = M_TWOPI / segs;

    vertexArray->resize(segs);

    for (unsigned int j = 0; j < segs; ++j)
    {
      const float angle = inc * j;
      const float x = sin(angle);
      const float y = cos(angle);
      assert(j < vertexArray->getNumElements());
      (*vertexArray)[j] = osg::Vec3(x * radiusM, y * radiusM, 0.0f);
    }
    vertexArray->dirty();
    drawArray_->setFirst(0);
    drawArray_->setCount(vertexArray->size());
  }
private:
  osg::ref_ptr<osg::DrawArrays> drawArray_;
  unsigned int ring_;
};
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
  // the underlying locatorNode will be inactive until the node mask is changed
  setNodeMask(DISPLAY_MASK_NONE);
}

LocalGridNode::~LocalGridNode() {}

void LocalGridNode::rebuild_(const simData::LocalGridPrefs& prefs)
{
  // set up the default state set and render bins:
  getOrCreateStateSet()->setRenderBinDetails(BIN_LOCAL_GRID, BIN_GLOBAL_SIMSDK);

  if (!graphicsGroup_)
  {
    graphicsGroup_ = new osg::Geode();
    graphicsGroup_->setName("simVis::LocalGridNode::GraphicsGeode");
    osg::StateSet* ss = graphicsGroup_->getOrCreateStateSet();
    PointSize::setValues(ss, 1.5f, osg::StateAttribute::ON);
    addChild(graphicsGroup_.get());
  }

  if (!labelGroup_)
  {
    labelGroup_ = new osg::Geode();
    labelGroup_->setName("simVis::LocalGridNode::LabelGeode");
    addChild(labelGroup_.get());
  }

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
  case simData::LocalGridPrefs_Type_SPEED_LINE:
  {
    // determine if we can validly display speedrings/speedline
    double sizeM;
    double timeRadiusSeconds;
    const int status = processSpeedParams_(prefs, sizeM, timeRadiusSeconds);
    if (status >= 0)
    {
      createSpeedRings_(prefs, graphicsGroup_.get(), labelGroup_.get(), (prefs.gridtype() == simData::LocalGridPrefs_Type_SPEED_LINE));
      updateSpeedRings_(prefs, sizeM, timeRadiusSeconds);
    }
    break;
  }
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
    const osgEarth::Units sizeUnits = simVis::convertUnitsToOsgEarth(prefs.positionoffsetunits());
    const float x = sizeUnits.convertTo(osgEarth::Units::METERS, pos.x());
    const float y = sizeUnits.convertTo(osgEarth::Units::METERS, pos.y());
    const float z = sizeUnits.convertTo(osgEarth::Units::METERS, pos.z());
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

  if ((lastPrefs_.gridtype() == simData::LocalGridPrefs_Type_SPEED_RINGS) ||
    (lastPrefs_.gridtype() == simData::LocalGridPrefs_Type_SPEED_LINE))
  {
    double sizeM;
    double timeRadiusSeconds;
    // has there been a change in params that will require the speed ring/line to update?
    const int status = processSpeedParams_(lastPrefs_, sizeM, timeRadiusSeconds);
    if (status > 0)
      updateSpeedRings_(lastPrefs_, sizeM, timeRadiusSeconds);
  }
}

// creates a Cartesian grid.
void LocalGridNode::createCartesian_(const simData::LocalGridPrefs& prefs, osg::Geode* geomGroup, osg::Geode* labelGroup) const
{
  const osgEarth::Units sizeUnits = simVis::convertUnitsToOsgEarth(prefs.sizeunits());
  // Note that size is halved; it's provided in diameter, and we need it as radius
  const float size = sizeUnits.convertTo(osgEarth::Units::METERS, prefs.size()) * 0.5f;
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
  for (int s = 0; s < numSubLines; ++s)
  {
    // skip sub lines that are coincident with main division lines
    if (s % (numSubDivisions+1) == 0)
      continue;

    {
      const float x = x0 + subSpacing * s;
      LineStrip* sub1 = new LineStrip();
      sub1->update(osg::Vec3(x, y0, 0.f), osg::Vec3(x, y0 + span, 0.f));
      sub1->setName("simVis::LocalGridNode::GridSubDivision1");
      sub1->setColor(subColor);
      geomGroup->addDrawable(sub1);
    }
    {
      const float y = y0 + subSpacing * s;
      LineStrip* sub2 = new LineStrip();
      sub2->update(osg::Vec3(x0, y, 0.f), osg::Vec3(x0 + span, y, 0.f));
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
      LineStrip* div1 = new LineStrip();
      div1->update(osg::Vec3(x, y0, 0.f), osg::Vec3(x, y0 + span, 0.f));
      div1->setName("simVis::LocalGridNode::GridDivision1");
      div1->setColor(color);
      geomGroup->addDrawable(div1);
    }
    // x-label:
    if (x < 0 && prefs.gridlabeldraw())
    {
      CartesianGridLabel* label = new CartesianGridLabel(prefs, -x);
      label->setPosition(osg::Vec3(-x, 0.f, 0.f));
      labelGroup->addDrawable(label);
    }

    const float y = y0 + divSpacing * p;
    {
      LineStrip* div2 = new LineStrip();
      div2->update(osg::Vec3(x0, y, 0.f), osg::Vec3(x0 + span, y, 0.f));
      div2->setName("simVis::LocalGridNode::GridDivision2");
      div2->setColor(color);
      geomGroup->addDrawable(div2);
    }
    // y-label
    if (y > 0 && prefs.gridlabeldraw())
    {
      CartesianGridLabel* label = new CartesianGridLabel(prefs, y);
      label->setPosition(osg::Vec3(0.f, y, 0.f));
      labelGroup->addDrawable(label);
    }
  }
}

// creates a range-rings local grid with optional polar radials.
void LocalGridNode::createRangeRings_(const simData::LocalGridPrefs& prefs, osg::Geode* geomGroup, osg::Geode* labelGroup, bool includePolarRadials) const
{
  const osgEarth::Units sizeUnits = simVis::convertUnitsToOsgEarth(prefs.sizeunits());
  // Note that size is halved; it's provided in diameter, and we need it as radius
  const float sizeM = sizeUnits.convertTo(osgEarth::Units::METERS, prefs.size()) * 0.5f;

  if (simCore::areEqual(sizeM, 0.0))
    return;

  // if size exceeds this number there is an excessive UI responsiveness penalty
  if (sizeM > MAX_RING_SIZE_M)
  {
    SIM_ERROR << "Range Rings radius exceeds maximum ring size." << std::endl;
    return;
  }

  const int   numDivisions    = prefs.gridsettings().numdivisions();
  const int   numSubDivisions = prefs.gridsettings().numsubdivisions();
  const int numRings          = (numDivisions + 1) * (numSubDivisions + 1);

  const osg::Vec4f& color = osgEarth::Symbology::Color(prefs.gridcolor(), osgEarth::Symbology::Color::RGBA);
  const osg::Vec4f& subColor = osgEarth::Symbology::Color(color * 0.5f, 1.0f);

  // rings:
  for (int i = 0; i < numRings; ++i)
  {
    const bool isMajorRing = ((i + 1) % (numSubDivisions + 1)) == 0;

    RangeRing* rangeRing = new RangeRing(i);
    rangeRing->setColor(isMajorRing ? color : subColor);
    geomGroup->addDrawable(rangeRing);
    rangeRing->update(prefs, sizeM);

    // label:
    if (isMajorRing && prefs.gridlabeldraw())
    {
      RingLabel* label = new RingLabel(prefs, i, true);
      labelGroup->addDrawable(label);
      label->update(prefs, sizeM);

      // add minor axis label as clone
      RingLabel* label2 = new RingLabel(*label, false);
      labelGroup->addChild(label2);
      label2->update(prefs, sizeM);
    }
  }

  // Cross-hair lines don't get drawn for Range Rings, but do for Polar
  if (includePolarRadials)
  {
    Axis* majorAxis = new Axis(true);
    majorAxis->setColor(color);
    geomGroup->addDrawable(majorAxis);
    majorAxis->update(sizeM);

    Axis* minorAxis = new Axis(false);
    minorAxis->setColor(color);
    geomGroup->addDrawable(minorAxis);
    minorAxis->update(sizeM);

    const float sectorAngle = prefs.gridsettings().sectorangle();
    if (sectorAngle > 0.0f)
    {
      RadialPoints* points = new RadialPoints(subColor, sectorAngle, numRings);
      geomGroup->addDrawable(points);
      points->update(sizeM);
    }
  }
}

// creates a speed-rings local grid with optional polar radials.
void LocalGridNode::createSpeedRings_(const simData::LocalGridPrefs& prefs, osg::Geode* graphicsGroup, osg::Geode* labelGroup, bool drawSpeedLine) const
{
  const osg::Vec4f& color = osgEarth::Symbology::Color(prefs.gridcolor(), osgEarth::Symbology::Color::RGBA);
  const osg::Vec4f& subColor = osgEarth::Symbology::Color(color * 0.5f, 1.0f);
  const unsigned int numDivisions = prefs.gridsettings().numdivisions();
  const unsigned int numSubDivisions = prefs.gridsettings().numsubdivisions();
  const unsigned int numRings = (numDivisions + 1) * (numSubDivisions + 1);

  if (drawSpeedLine)
  {
    SpeedLine* speedLine = new SpeedLine();
    speedLine->setColor(color);
    graphicsGroup->addDrawable(speedLine);

    if (!prefs.gridlabeldraw())
      return;
  }
  else
  {
    Axis* majorAxis = new Axis(true);
    majorAxis->setColor(color);
    graphicsGroup->addDrawable(majorAxis);
    Axis* minorAxis = new Axis(false);
    minorAxis->setColor(color);
    graphicsGroup->addDrawable(minorAxis);

    const float sectorAngle = prefs.gridsettings().sectorangle();
    // draw polar radials for speed rings
    if (sectorAngle > 0.0f)
    {
      RadialPoints* points = new RadialPoints(subColor, sectorAngle, numRings);
      graphicsGroup->addDrawable(points);
    }
  }

  for (unsigned int i = 0; i < numRings; ++i)
  {
    const bool isMajorRing = ((i + 1) % (numSubDivisions + 1)) == 0;
    if (!drawSpeedLine)
    {
      RangeRing* speedRing = new RangeRing(i);
      speedRing->setColor(isMajorRing ? color : subColor);
      graphicsGroup->addDrawable(speedRing);
    }
    // labels are only added to major rings
    if (isMajorRing && prefs.gridlabeldraw())
    {
      RingLabel* label = new RingLabel(prefs, i, true);
      labelGroup->addDrawable(label);

      if (!drawSpeedLine)
      {
        // add minor axis label as clone
        RingLabel* label2 = new RingLabel(*label, false);
        labelGroup->addChild(label2);
      }
    }
  }
}

void LocalGridNode::updateSpeedRings_(const simData::LocalGridPrefs& prefs, double sizeM, double timeRadiusSeconds)
{
  if ((prefs.gridtype() != simData::LocalGridPrefs_Type_SPEED_RINGS) && (prefs.gridtype() != simData::LocalGridPrefs_Type_SPEED_LINE))
  {
    assert(0);
    return;
  }
  const unsigned int numLabels = labelGroup_->getNumChildren();
  for (unsigned int i = 0; i < numLabels; i++)
  {
    osg::Node* child = labelGroup_->getChild(i);
    RingLabel* label = dynamic_cast<RingLabel*>(child);
    if (label)
      label->update(prefs, sizeM, timeRadiusSeconds);
  }
  const unsigned int numGraphics = graphicsGroup_->getNumChildren();
  for (unsigned int i = 0; i < numGraphics; i++)
  {
    osg::Node* child = graphicsGroup_->getChild(i);
    RangeRing* ring = dynamic_cast<RangeRing*>(child);
    if (ring)
    {
      ring->update(prefs, sizeM);
      continue;
    }
    SpeedLine* line = dynamic_cast<SpeedLine*>(child);
    if (line)
    {
      line->update(sizeM);
      continue;
    }
    Axis* axis = dynamic_cast<Axis*>(child);
    if (axis)
    {
      axis->update(sizeM);
      continue;
    }
    RadialPoints* pts = dynamic_cast<RadialPoints*>(child);
    if (pts)
    {
      pts->update(sizeM);
      continue;
    }
  }
}

int LocalGridNode::processSpeedParams_(const simData::LocalGridPrefs& prefs, double& sizeM, double& timeRadiusSeconds)
{
  const Locator* hostLocator = (host_.valid() ? host_->getLocator() : NULL);
  if (!hostLocator)
    return -1;
  const PlatformNode* hostPlatform = dynamic_cast<const PlatformNode*>(host_.get());
  // if host is a platform, use its locator. if not, use host locator's parent locator.
  const Locator* hostPlatformLocator = (hostPlatform) ? hostLocator : hostLocator->getParentLocator();
  if (!hostPlatformLocator)
    return -2;

  bool requiresUpdate = false;

  // determine the speed to be used for calculating the rings
  double speedMS = 0.0;
  if (prefs.speedring().useplatformspeed())
  {
    // force rebuild if speed rings are displayed, using platform speed, and host velocity changed
    speedMS = simCore::v3Length(hostPlatformLocator->getCoordinate().velocity());
    if (!simCore::areEqual(hostSpeedMS_, speedMS, 0.01))
    {
      hostSpeedMS_ = speedMS;
      requiresUpdate = true;
    }
  }
  else if (prefs.speedring().speedtouse() > 0.0)
  {
    // using speedToUse, convert to m/s
    const osgEarth::Units prefSpeedUnits = simVis::convertUnitsToOsgEarth(prefs.speedring().speedunits());
    speedMS = prefSpeedUnits.convertTo(osgEarth::Units::METERS_PER_SECOND, prefs.speedring().speedtouse());
    if (simCore::areEqual(speedMS, 0.0))
    {
      // do not display anything if this speed is zero
      return -3;
    }
  }
  else
  {
    // do not display anything if this speed is less than or equal to zero
    return -5;
  }


  // determine the time radius for the speed rings display
  timeRadiusSeconds = 0;
  if (prefs.speedring().usefixedtime())
  {
    // if we are displaying speed rings with fixed time, rebuild the display when host locator time changes
    const double timeS = hostPlatformLocator->getTime();
    if (!simCore::areEqual(hostTimeS_, timeS))
    {
      hostTimeS_ = timeS;
      requiresUpdate = true;
    }
    // as time moves forward, rings should shrink; if platform speed increases, rings should expand, and vice versa
    timeRadiusSeconds = simCore::sdkMax(0.0, fixedTime_ - timeS);
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

    // Verify that there is something to draw
    if (timeRadiusSeconds <= 0.0)
    {
      timeRadiusSeconds = 0;
      return -4;
    }
  }

  sizeM = timeRadiusSeconds * speedMS;

  // if sizeM exceeds this number there is an excessive UI responsiveness penalty
  if (sizeM > MAX_RING_SIZE_M)
  {
    SIM_ERROR << "Speed Rings radius exceeds maximum ring size." << std::endl;
    requiresUpdate = true;
    sizeM = 0.0;
    timeRadiusSeconds = 0.0;
  }

  return requiresUpdate ? 1 : 0;
}

}
