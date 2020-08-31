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
#include <cassert>
#include "osg/BoundingSphere"
#include "osg/LineSegment"
#include "osg/Geode"
#include "osg/Geometry"
#include "osgEarth/ECEF"
#include "osgEarth/LineDrawable"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/CoordinateConverter.h"
#include "simCore/Calc/MultiFrameCoordinate.h"
#include "simNotify/Notify.h"
#include "simVis/Constants.h"
#include "simVis/Locator.h"
#include "simVis/OverheadMode.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/AnimatedLine.h"

#define LC "[AnimatedLine] "

//--------------------------------------------------------------------------

// Local utility functions
namespace
{
  /** Depth to offset to figure out whether line intersects sphere (Mariana trench depth in meters) */
  static const double OCEAN_DEPTH_TEST_OFFSET = 11033.;

  // rotates a 16-bit value to the left by the specified # of bits, where bits is [0,15]
  void rol(unsigned short& v, unsigned short bits)
  {
    assert(bits < 16);
    if (bits > 0 && v != 0)
    {
      v = (v >> (16-bits)) | (v << bits);
      assert(v != 0);
    }
  }

  // rotates a 16-bit value to the right by the specified # of bits, where bits is [0,15]
  void ror(unsigned short& v, unsigned short bits)
  {
    assert(bits < 16);
    if (bits > 0 && v != 0)
    {
      v = (v << (16-bits)) | (v >> bits);
      assert(v != 0);
    }
  }

  // rounds and fmods a floating point number to the nearest integer in interval [0, 15]
  unsigned short shortRound(double n)
  {
    // if assert fails, frame stamp times are going backwards
    assert(n >= 0);
    if (n > 16.0)
      n = fmod(n, 16.0);
    const unsigned short result = static_cast<unsigned short>(simCore::rint(n));
    return (result == 16) ? 0 : result;
  }
}

//--------------------------------------------------------------------------

namespace simVis {

AnimatedLineNode::HalfALine::HalfALine()
{
}

AnimatedLineNode::HalfALine::~HalfALine()
{
}

void AnimatedLineNode::HalfALine::fillSlantLine(unsigned int numSegments, const osg::Vec3d& lastPoint, bool forward)
{
  line1->reserve(numSegments + 1);
  line2->reserve(numSegments + 1);

  // Add points to the vertex list, from back to front, for consistent stippling.  Order
  // matters because it affects the line direction during stippling.
  for (unsigned int k = 0; k <= numSegments; ++k)
  {
    // Add in the subdivided line point
    const double percentOfFull = forward ? (static_cast<double>(numSegments - k) / numSegments) : (static_cast<double>(k) / numSegments);
    osg::Vec3f point = lastPoint * percentOfFull;
    line1->pushVertex(point);
    line2->pushVertex(point);
  }

  // Finish up
  line1->dirty();
  line2->dirty();
}

AnimatedLineNode::AnimatedLineNode(float lineWidth, bool depthBufferTest) :
stipple1_(0xFF00),
stipple2_(0x00FF),
shiftsPerSecond_(10.0),
color1_(simVis::Color::Blue),
color2_(simVis::Color::Yellow),
colorOverride_(osg::Vec4()),    // transparent
useOverrideColor_(false),
lineWidth_(lineWidth),
coordinateConverter_(new simCore::CoordinateConverter),
timeLastShift_(0.0),
depthBufferTest_(depthBufferTest)
{
  // animation requires an update traversal.
  this->setNumChildrenRequiringUpdateTraversal(1);

  // build and attach the line geometry
  initializeGeometry_();

  OverheadMode::enableGeometryFlattening(true, this);
}

AnimatedLineNode::~AnimatedLineNode()
{
  delete coordinateConverter_;
}

void AnimatedLineNode::setEndPoints(const simCore::Coordinate& first, const simCore::Coordinate& second)
{
  firstCoord_ = simCore::MultiFrameCoordinate(first);
  secondCoord_ = second;
  firstLocator_ = nullptr;
  secondLocator_ = nullptr;
  // Assertion failure means bad input from developer for setting initial endpoint
  assert(firstCoord_->isValid());
}

void AnimatedLineNode::setEndPoints(const Locator* first, const simCore::Coordinate& second)
{
  secondCoord_ = second;
  firstLocator_ = first;
  secondLocator_ = nullptr;
}

void AnimatedLineNode::setEndPoints(const Locator* first, const Locator* second)
{
  firstLocator_ = first;
  secondLocator_ = second;
}

int AnimatedLineNode::getEndPoints(simCore::MultiFrameCoordinate& coord1, simCore::MultiFrameCoordinate& coord2) const
{
  if (!firstHalf_.line1.valid())
    return 1;
  coord1 = firstCoord_;
  coord2 = secondCoordMF_;
  return (coord1.isValid() && coord2.isValid()) ? 0 : 1;
}

void AnimatedLineNode::setStipple1(unsigned short value)
{
  stipple1_ = value;
  // Need to reset the time shift to recalculate shifting correctly, per SIMDIS-3104
  timeLastShift_ = 0.0;
}

void AnimatedLineNode::setStipple2(unsigned short value)
{
  stipple2_ = value;
  // Need to reset the time shift to recalculate shifting correctly, per SIMDIS-3104
  timeLastShift_ = 0.0;
}

void AnimatedLineNode::setColor1(const osg::Vec4& value)
{
  color1_ = value;
}

void AnimatedLineNode::setColor2(const osg::Vec4& value)
{
  color2_ = value;
}

void AnimatedLineNode::setColorOverride(const osg::Vec4& value)
{
  colorOverride_ = value;
  useOverrideColor_ = true;
}

void AnimatedLineNode::clearColorOverride()
{
  colorOverride_ = osg::Vec4();    // no color,  .changed() will be true
  useOverrideColor_ = false;
}

void AnimatedLineNode::setLineWidth(float width)
{
  lineWidth_ = width;
}

float AnimatedLineNode::getLineWidth() const
{
  return lineWidth_;
}

void AnimatedLineNode::setShiftsPerSecond(double value)
{
  shiftsPerSecond_ = value;
  // Need to reset the time shift to recalculate shifting correctly, per SIMDIS-3104
  timeLastShift_ = 0.0;
}

void AnimatedLineNode::initializeGeometry_()
{
  // build the initial geometry from scratch.
  this->removeChildren(0, this->getNumChildren());

  // Geode to hold the geometry.
  geode_ = new osgEarth::LineGroup();
  firstHalf_.matrix = new osg::MatrixTransform;
  secondHalf_.matrix = new osg::MatrixTransform;
  geode_->addChild(firstHalf_.matrix);
  geode_->addChild(secondHalf_.matrix);

  // First geometry:
  {
    firstHalf_.line1 = new osgEarth::LineDrawable(GL_LINE_STRIP);
    firstHalf_.line1->setName("simVis::AnimatedLine");
    firstHalf_.line1->setDataVariance(osg::Object::DYNAMIC);
    firstHalf_.line1->allocate(2);
    firstHalf_.line1->setColor(color1_);
    firstHalf_.line1->setLineWidth(lineWidth_);
    firstHalf_.line1->setStipplePattern(stipple1_);
    firstHalf_.line1->dirty();
    firstHalf_.matrix->addChild(firstHalf_.line1.get());

    firstHalf_.line2 = new osgEarth::LineDrawable(GL_LINE_STRIP);
    firstHalf_.line2->setName("simVis::AnimatedLine");
    firstHalf_.line2->setDataVariance(osg::Object::DYNAMIC);
    firstHalf_.line2->allocate(2);
    firstHalf_.line2->setColor(color2_);
    firstHalf_.line2->setLineWidth(lineWidth_);
    firstHalf_.line2->setStipplePattern(stipple2_);
    firstHalf_.line2->dirty();
    firstHalf_.matrix->addChild(firstHalf_.line2.get());
  }

  // Second geometry:
  {
    secondHalf_.line1 = new osgEarth::LineDrawable(GL_LINE_STRIP);
    secondHalf_.line1->setName("simVis::AnimatedLine");
    secondHalf_.line1->setDataVariance(osg::Object::DYNAMIC);
    secondHalf_.line1->allocate(2);
    secondHalf_.line1->setColor(color1_);
    secondHalf_.line1->setLineWidth(lineWidth_);
    secondHalf_.line1->setStipplePattern(stipple1_);
    secondHalf_.line1->dirty();
    secondHalf_.matrix->addChild(secondHalf_.line1.get());

    secondHalf_.line2 = new osgEarth::LineDrawable(GL_LINE_STRIP);
    secondHalf_.line2->setName("simVis::AnimatedLine");
    secondHalf_.line2->setDataVariance(osg::Object::DYNAMIC);
    secondHalf_.line2->allocate(2);
    secondHalf_.line2->setColor(color2_);
    secondHalf_.line2->setLineWidth(lineWidth_);
    secondHalf_.line2->setStipplePattern(stipple2_);
    secondHalf_.line2->dirty();
    secondHalf_.matrix->addChild(secondHalf_.line2.get());
  }

  // top-level state set sets up lighting, etc.
  osg::StateSet* stateSet = geode_->getOrCreateStateSet();
  stateSet->setMode(GL_BLEND, 1);

  fixDepth_(false);
  this->addChild(geode_.get());
}

void AnimatedLineNode::fixDepth_(bool isCloseToSurface)
{
  osg::StateSet* stateSet = geode_->getOrCreateStateSet();

  // Turn off depth buffer only if requested, or if not-requested and near surface (Z-fighting)
  if (depthBufferTest_ && !isCloseToSurface)
  {
    // Turn on the depth buffer test and render early
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
    stateSet->setRenderBinDetails(BIN_ANIMATEDLINE, BIN_GLOBAL_SIMSDK);

    // Remove horizon clip plane.  Because the depth test is on, there is no need to clip against
    // the horizon plane.  Lines can extend past horizon and earth will clip them correctly.
    stateSet->setMode(simVis::CLIPPLANE_VISIBLE_HORIZON_GL_MODE, osg::StateAttribute::OFF);
  }
  else
  {
    // Turn off the depth buffer test and render late
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    stateSet->setRenderBinDetails(BIN_ANIMATEDLINE_FLAT, BIN_GLOBAL_SIMSDK);

    // Add a horizon clip plane.  This is needed because the depth test is off and we need to make
    // sure the line does not extend over the horizon.  Note that this mode is useful for lines that
    // are expected to go above/below ground, or near ground, to avoid Z-fighting issues.  In these
    // cases the lines won't clip against the earth due to depth test off, so we add the horizon
    // clip plane to make sure we don't see them "through" the earth when eye is on other side.
    stateSet->setMode(simVis::CLIPPLANE_VISIBLE_HORIZON_GL_MODE, osg::StateAttribute::ON);
  }
}

void AnimatedLineNode::traverse(osg::NodeVisitor& nv)
{
  if (nv.getVisitorType() == nv.UPDATE_VISITOR)
  {
    update_(nv.getFrameStamp()->getReferenceTime());
  }

  osg::MatrixTransform::traverse(nv);
}

void AnimatedLineNode::update_(double t)
{
  const bool firstLocatorValid = firstLocator_.valid();
  const bool secondLocatorValid = secondLocator_.valid();

  // case 1: Locator => Locator.
  if (firstLocatorValid && secondLocatorValid)
  {
    if (firstLocator_->outOfSyncWith(firstLocatorRevision_) ||
        secondLocator_->outOfSyncWith(secondLocatorRevision_))
    {
      // Pull out the 2 ECEF coordinates, set up local matrix
      simCore::Vec3 ecef1;
      firstLocator_->getLocatorPosition(&ecef1);
      firstLocator_->sync(firstLocatorRevision_);
      firstHalf_.matrix->setMatrix(osg::Matrix::translate(ecef1.x(), ecef1.y(), ecef1.z()));
      simCore::Vec3 ecef2;
      secondLocator_->getLocatorPosition(&ecef2);
      secondLocator_->sync(secondLocatorRevision_);
      secondHalf_.matrix->setMatrix(osg::Matrix::translate(ecef2.x(), ecef2.y(), ecef2.z()));

      // Perform the bendy
      drawLine_(simCore::MultiFrameCoordinate(simCore::Coordinate(simCore::COORD_SYS_ECEF, ecef1)), simCore::MultiFrameCoordinate(simCore::Coordinate(simCore::COORD_SYS_ECEF, ecef2)));
    }
  }

  // case 2: Locator => Coordinate.
  else if (firstLocatorValid && !secondLocatorValid)
  {
    const bool locatorMoved = firstLocator_->outOfSyncWith(firstLocatorRevision_);

    if (secondCoord_.changed() || locatorMoved)
    {
      simCore::Vec3 ecef1;
      firstLocator_->getLocatorPosition(&ecef1);
      const simCore::MultiFrameCoordinate coord1(simCore::Coordinate(simCore::COORD_SYS_ECEF, ecef1));
      if (locatorMoved)
      {
        firstHalf_.matrix->setMatrix(osg::Matrix::translate(ecef1.x(), ecef1.y(), ecef1.z()));

        // Update the coordinate reference origin.  Note that we could optimize this by
        // only setting the reference origin when the second coordinate is non-Geo (ECEF/LLA),
        // but there's an edge case where this could fail if the second coordinate changes
        // via setEndPoints() but locator stays in same place.  This optimization is not
        // being done right now because it overly complicates the code for a minor fix.
        //
        // Could also be optimized in Coord Converter to avoid doing complex math to initialize
        // the matrices until a calculation is done that requires it.
        coordinateConverter_->setReferenceOrigin(coord1.llaCoordinate().position());
      }

      // Resolve the second coordinate (may or may not be relative, so we need a CoordinateConverter)
      simCore::MultiFrameCoordinate secondCoordMF;
      secondCoordMF.setCoordinate(secondCoord_, *coordinateConverter_);
      const simCore::Coordinate& ecef2 = secondCoordMF.ecefCoordinate();
      secondHalf_.matrix->setMatrix(osg::Matrix::translate(ecef2.x(), ecef2.y(), ecef2.z()));
      drawLine_(coord1, secondCoordMF);
    }

    firstLocator_->sync(firstLocatorRevision_);
  }

  // case 3: Coordinate => Coordinate.
  else if (!firstLocatorValid && !secondLocatorValid)
  {
    const bool anchorChanged = firstCoord_.changed();
    if (anchorChanged)
    {
      // Reset the matrix
      const simCore::Coordinate& ecef = firstCoord_->ecefCoordinate();
      firstHalf_.matrix->setMatrix(osg::Matrix::translate(ecef.x(), ecef.y(), ecef.z()));

      // Need to also update the Coordinate Converter with new reference origin.  Suffers
      // the same issue as case 2 for performance here, but is less likely to be a problem
      // in this case because there is no way to have anchorChanged without also changing
      // the secondCoord_ using the public interface.
      coordinateConverter_->setReferenceOrigin(firstCoord_->llaCoordinate().position());
    }

    // Need to recalculate points
    if (secondCoord_.changed() || anchorChanged)
    {
      // Resolve the second coordinate (may or may not be relative, so we need CoordinateConverter)
      simCore::MultiFrameCoordinate secondCoordMF;
      secondCoordMF.setCoordinate(secondCoord_, *coordinateConverter_);
      const simCore::Coordinate& ecef = secondCoordMF.ecefCoordinate();
      secondHalf_.matrix->setMatrix(osg::Matrix::translate(ecef.x(), ecef.y(), ecef.z()));
      drawLine_(firstCoord_, secondCoordMF);
    }
  }

  // update the colors if we need to:
  if (colorOverride_.changed())
  {
    if (useOverrideColor_)
    {
      firstHalf_.line1->setColor(colorOverride_);
      firstHalf_.line2->setColor(colorOverride_);
      secondHalf_.line1->setColor(colorOverride_);
      secondHalf_.line2->setColor(colorOverride_);
    }
    else
    {
      firstHalf_.line1->setColor(color1_);
      firstHalf_.line2->setColor(color2_);
      secondHalf_.line1->setColor(color1_);
      secondHalf_.line2->setColor(color2_);
    }
  }

  if (color1_.changed() && !useOverrideColor_)
  {
    firstHalf_.line1->setColor(color1_);
    secondHalf_.line1->setColor(color1_);
  }

  if (color2_.changed() && !useOverrideColor_)
  {
    firstHalf_.line2->setColor(color2_);
    secondHalf_.line2->setColor(color2_);
  }

  // LineDrawable is efficient in cases of no change
  firstHalf_.line1->setLineWidth(lineWidth_);
  firstHalf_.line2->setLineWidth(lineWidth_);
  secondHalf_.line1->setLineWidth(lineWidth_);
  secondHalf_.line2->setLineWidth(lineWidth_);

  // animate the line:
  const double dt        = t - timeLastShift_;
  const double numShifts = dt * fabs(shiftsPerSecond_);

  if (numShifts >= 1.0)
  {
    // note: lines are tessellated end-to-start,
    // so we bit-shift in the opposite direction to achieve
    // proper stippling direction.
    const unsigned short bits = shortRound(numShifts);
    if (shiftsPerSecond_ > 0.0)
    {
      rol(stipple1_, bits);
      rol(stipple2_, bits);
    }
    else
    {
      ror(stipple1_, bits);
      ror(stipple2_, bits);
    }

    firstHalf_.line1->setStipplePattern(stipple1_);
    firstHalf_.line2->setStipplePattern(stipple2_);
    secondHalf_.line1->setStipplePattern(stipple1_);
    secondHalf_.line2->setStipplePattern(stipple2_);
    timeLastShift_ = t;
  }
  else
  {
    // process changes to stipple even if line is not animating
    firstHalf_.line1->setStipplePattern(stipple1_);
    firstHalf_.line2->setStipplePattern(stipple2_);
    secondHalf_.line1->setStipplePattern(stipple1_);
    secondHalf_.line2->setStipplePattern(stipple2_);
  }
}

bool AnimatedLineNode::doesLineIntersectEarth_(const simCore::MultiFrameCoordinate& coord1, const simCore::MultiFrameCoordinate& coord2) const
{
  if (!coord1.isValid() || !coord2.isValid())
  {
    // Precondition for calling this function is that the coordinates are valid.
    assert(false);
    return false;
  }

  // Get into geocentric frame
  const simCore::Coordinate& lla1 = coord1.llaCoordinate();

  // Use the scaled earth radius at the latitude, for determining whether to draw straight line
  double earthRadius = simCore::calculateEarthRadius(lla1.lat());
  // Shrink the sphere to bottom of ocean if the lla1 is underground
  if (lla1.alt() < 0)
    earthRadius -= OCEAN_DEPTH_TEST_OFFSET; // Depth of the Mariana Trench in meters (matches SIMDIS 9 behavior)
  osg::BoundingSphere earthSphere(osg::Vec3(), earthRadius);

  // Get ECEF coordinates and make a line
  const simCore::Coordinate& ecef1 = coord1.ecefCoordinate();
  const simCore::Coordinate& ecef2 = coord2.ecefCoordinate();
  osg::ref_ptr<osg::LineSegment> lineSeg = new osg::LineSegment(
    osg::Vec3(ecef1.x(), ecef1.y(), ecef1.z()),
    osg::Vec3(ecef2.x(), ecef2.y(), ecef2.z()));

  // Test against sphere
  return lineSeg->intersect(earthSphere);
}

void AnimatedLineNode::drawLine_(const simCore::MultiFrameCoordinate& coord1, const simCore::MultiFrameCoordinate& coord2)
{
  // firstCoord_ is already initialized.  Because secondCoord_ might be in tangent plane or a
  // locator, it needs to be explicitly updated when its target is dirty.  Because of this, we
  // can cache the secondCoord in a MultiFrameCoordinate only after it's been resolved.  That's
  // here.  We store it even if it is not valid.
  secondCoordMF_ = coord2;

  // Both coordinates must be valid
  if (!coord1.isValid() || !coord2.isValid())
    return;

  // Do horizon checking to determine if the coordinates will hit the earth
  // with a slant line.  If so, then draw a bending line, else draw a straight line.
  const bool drawSlant = !doesLineIntersectEarth_(coord1, coord2);
  if (drawSlant)
    drawSlantLine_(coord1, coord2);
  else
    drawBendingLine_(coord1, coord2);

  // Prevent terrain interference with lines ~1m from the surface
  fixDepth_(simCore::areEqual(coord1.llaCoordinate().alt(), 0.0, 1.0) &&
    simCore::areEqual(coord2.llaCoordinate().alt(), 0.0, 1.0));
}

void AnimatedLineNode::drawSlantLine_(const simCore::MultiFrameCoordinate& startPoint, const simCore::MultiFrameCoordinate& endPoint)
{
  if (!startPoint.isValid() || !endPoint.isValid())
  {
    // Precondition for calling this function is that the coordinates are valid.
    assert(false);
    return;
  }

  // Reserve 2 points for the output
  firstHalf_.line1->clear();
  firstHalf_.line2->clear();
  secondHalf_.line1->clear();
  secondHalf_.line2->clear();

  // Calculate the length of the vector
  simCore::Vec3 delta;
  simCore::v3Subtract(endPoint.ecefCoordinate().position(), startPoint.ecefCoordinate().position(), delta);
  const double length = simCore::v3Length(delta);

  // Calculate the number of segments
  const double segmentLength = simCore::sdkMin(length, MAX_SEGMENT_LENGTH);
  unsigned int numSegs = simCore::sdkMax(MIN_NUM_SEGMENTS, simCore::sdkMin(MAX_NUM_SEGMENTS, static_cast<unsigned int>(length / segmentLength)));
  // Easier to divide an even number of segments into two lines
  if ((numSegs % 2) == 1)
    ++numSegs;

  osg::Vec3d midPoint(delta.x() / 2.0, delta.y() / 2.0, delta.z() / 2.0);
  firstHalf_.fillSlantLine(numSegs / 2, midPoint, true);
  secondHalf_.fillSlantLine(numSegs / 2, -midPoint, false);
}

void AnimatedLineNode::drawBendingLine_(const simCore::MultiFrameCoordinate& coord1, const simCore::MultiFrameCoordinate& coord2)
{
  if (!coord1.isValid() || !coord2.isValid())
  {
    // Precondition for calling this function is that the coordinates are valid.
    assert(false);
    return;
  }

  // Get into geodetic frame
  const simCore::Coordinate& lla1 = coord1.llaCoordinate();
  const simCore::Coordinate& lla2 = coord2.llaCoordinate();

  // Use Sodano method to calculate azimuth and distance
  double azfwd = 0.0;
  const double distance = simCore::sodanoInverse(lla1.lat(), lla1.lon(), lla1.alt(), lla2.lat(), lla2.lon(), &azfwd);

  // purely vertical line will be drawn as a slant line
  if (distance <= 0.0)
  {
    drawSlantLine_(coord1, coord2);
    return;
  }

  // if total distance of the line is less than the max segment length, use that
  double segmentLength = simCore::sdkMin(distance, MAX_SEGMENT_LENGTH);
  // When lines are at/close to surface, we might need to tessellate more closely
  if (fabs(lla1.alt()) < SUBDIVIDE_BY_GROUND_THRESHOLD && fabs(lla2.alt()) < SUBDIVIDE_BY_GROUND_THRESHOLD)
  {
    // if the total distance of the line is less than the max segment length, use that
    segmentLength = simCore::sdkMin(distance, MAX_SEGMENT_LENGTH_GROUNDED);
  }

  unsigned int numSegs = simCore::sdkMax(MIN_NUM_SEGMENTS, simCore::sdkMin(MAX_NUM_SEGMENTS, static_cast<unsigned int>(distance / segmentLength)));
  // Easier to divide an even number of segments into two lines
  if ((numSegs % 2) == 1)
    ++numSegs;

  // Point 1 to midpoint
  {
    const osg::Vec3d zeroPoint = firstHalf_.matrix->getMatrix().getTrans();

    osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
    verts->reserve(numSegs/2 + 1);

    // start at zero
    verts->push_back(osg::Vec3f());

    // Add points to the vertex list, from back to front, for consistent stippling.  Order
    // matters because it affects the line direction during stippling.
    for (unsigned int k = 1; k <= numSegs/2; ++k)
    {
      const float percentOfFull = (float)k / (float)numSegs; // almost .0 to 0.5

      // Calculate the LLA value of the point, and replace the altitude
      double lat = 0.0;
      double lon = 0.0;
      simCore::sodanoDirect(lla1.lat(), lla1.lon(), lla1.alt(), distance * percentOfFull, azfwd, &lat, &lon);
      const double alt = lla1.alt() + percentOfFull * (lla2.alt() - lla1.alt());

      // Convert back to ECEF and add the vertex
      simCore::Vec3 ecefPos;
      simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(lat, lon, alt), ecefPos);
      verts->push_back(osg::Vec3f(ecefPos.x(), ecefPos.y(), ecefPos.z()) - zeroPoint);
    }



    firstHalf_.line1->importVertexArray(verts.get());
    firstHalf_.line2->importVertexArray(verts.get());
  }

  // Point 2 to midpoint
  {
    const osg::Vec3d zeroPoint = secondHalf_.matrix->getMatrix().getTrans();

    osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
    verts->reserve(numSegs / 2 + 1);

    // Add points to the vertex list, from back to front, for consistent stippling.  Order
    // matters because it affects the line direction during stippling.
    for (unsigned int k =  numSegs / 2; k < numSegs ; ++k)
    {
      const float percentOfFull = (float)(k) / (float)numSegs; // From .5 to almost 1.0

      // Calculate the LLA value of the point, and replace the altitude
      double lat = 0.0;
      double lon = 0.0;
      simCore::sodanoDirect(lla1.lat(), lla1.lon(), lla1.alt(), distance * percentOfFull, azfwd, &lat, &lon);
      const double alt = lla1.alt() + percentOfFull * (lla2.alt() - lla1.alt());

      // Convert back to ECEF and add the vertex
      simCore::Vec3 ecefPos;
      simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(lat, lon, alt), ecefPos);
      verts->push_back(osg::Vec3f(ecefPos.x(), ecefPos.y(), ecefPos.z()) - zeroPoint);
    }

    // Finish up
    verts->push_back(osg::Vec3f());

    secondHalf_.line1->importVertexArray(verts.get());
    secondHalf_.line2->importVertexArray(verts.get());
  }
}

}
