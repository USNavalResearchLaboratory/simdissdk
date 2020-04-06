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
  firstLocator_ = NULL;
  secondLocator_ = NULL;
  // Assertion failure means bad input from developer for setting initial endpoint
  assert(firstCoord_->isValid());
}

void AnimatedLineNode::setEndPoints(const Locator* first, const simCore::Coordinate& second)
{
  secondCoord_ = second;
  firstLocator_ = first;
  secondLocator_ = NULL;
}

void AnimatedLineNode::setEndPoints(const Locator* first, const Locator* second)
{
  firstLocator_ = first;
  secondLocator_ = second;
}

int AnimatedLineNode::getEndPoints(simCore::MultiFrameCoordinate& coord1, simCore::MultiFrameCoordinate& coord2) const
{
  if (!line1_.valid())
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

  // First geometry:
  {
    line1_ = new osgEarth::LineDrawable(GL_LINE_STRIP);
    line1_->setName("simVis::AnimatedLine");
    line1_->setDataVariance(osg::Object::DYNAMIC);
    line1_->allocate(2);
    line1_->setColor(color1_);
    line1_->setLineWidth(lineWidth_);
    line1_->setStipplePattern(stipple1_);
    line1_->dirty();
    geode_->addChild(line1_.get());
  }

  // Second geometry:
  {
    line2_ = new osgEarth::LineDrawable(GL_LINE_STRIP);
    line2_->setName("simVis::AnimatedLine");
    line2_->setDataVariance(osg::Object::DYNAMIC);
    line2_->allocate(2);
    line2_->setColor(color2_);
    line2_->setLineWidth(lineWidth_);
    line2_->setStipplePattern(stipple2_);
    line2_->dirty();
    geode_->addChild(line2_.get());
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
      this->setMatrix(osg::Matrix::translate(ecef1.x(), ecef1.y(), ecef1.z()));
      simCore::Vec3 ecef2;
      secondLocator_->getLocatorPosition(&ecef2);
      secondLocator_->sync(secondLocatorRevision_);

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
        // Need to update the local matrix
        this->setMatrix(osg::Matrix::translate(ecef1.x(), ecef1.y(), ecef1.z()));

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
      this->setMatrix(osg::Matrix::translate(ecef.x(), ecef.y(), ecef.z()));

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
      drawLine_(firstCoord_, secondCoordMF);
    }
  }

  // update the colors if we need to:
  if (colorOverride_.changed())
  {
    if (useOverrideColor_)
    {
      line1_->setColor(colorOverride_);
      line2_->setColor(colorOverride_);
    }
    else
    {
      line1_->setColor(color1_);
      line2_->setColor(color2_);
    }
  }

  if (color1_.changed() && !useOverrideColor_)
  {
    line1_->setColor(color1_);
  }

  if (color2_.changed() && !useOverrideColor_)
  {
    line2_->setColor(color2_);
  }

  // LineDrawable is efficient in cases of no change
  line1_->setLineWidth(lineWidth_);
  line2_->setLineWidth(lineWidth_);

  // animate the line:
  const double dt        = t - timeLastShift_;
  const double numShifts = dt * fabs(shiftsPerSecond_);

  if (numShifts >= 1.0)
  {
    // note: lines are tessellated end-to-start,
    // so we bit-shift in the opposite direction to achieve
    // propert stippling direction.
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

    line1_->setStipplePattern(stipple1_);
    line2_->setStipplePattern(stipple2_);

    timeLastShift_ = t;
  }
  else
  {
    // process changes to stipple even if line is not animating
    line1_->setStipplePattern(stipple1_);
    line2_->setStipplePattern(stipple2_);
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
  line1_->clear();
  line2_->clear();

  // Calculate the reference point in ECEF
  const osg::Vec3d zeroPoint = this->getMatrix().getTrans();

  // Convert back to ECEF and add the vertex relative to the 0 point
  simCore::CoordinateConverter cc;
  const simCore::Coordinate& outEcef = endPoint.ecefCoordinate();

  // Calculate the length of the vector
  const osg::Vec3d lastPoint = osg::Vec3d(outEcef.x(), outEcef.y(), outEcef.z()) - zeroPoint;
  const double distance = lastPoint.length();
  const double segmentLength = simCore::sdkMin(distance, MAX_SEGMENT_LENGTH);

  // make sure there's enough room. Don't bother shrinking.
  const unsigned int numSegs = simCore::sdkMax(MIN_NUM_SEGMENTS, simCore::sdkMin(MAX_NUM_SEGMENTS, static_cast<unsigned int>(distance / segmentLength)));
  line1_->reserve(numSegs + 1);
  line2_->reserve(numSegs + 1);

  // Add points to the vertex list, from back to front, for consistent stippling.  Order
  // matters because it affects the line direction during stippling.
  for (unsigned int k = 0; k <= numSegs; ++k)
  {
    // Add in the subdivided line point
    const double percentOfFull = static_cast<double>((numSegs-k)) / numSegs; // From 1 to 0
    osg::Vec3f point = lastPoint * percentOfFull;
    line1_->pushVertex(point);
    line2_->pushVertex(point);
  }

  // Finish up
  line1_->dirty();
  line2_->dirty();
}

void AnimatedLineNode::dirtyGeometryBounds_()
{
  for (unsigned int i = 0; i < geode_->getNumDrawables(); ++i)
  {
    geode_->getDrawable(i)->dirtyBound();
  }
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
  double azimuth = 0.0;
  const double distance = simCore::sodanoInverse(lla1.lat(), lla1.lon(), lla1.alt(), lla2.lat(), lla2.lon(), &azimuth);

  // Calculate the reference point in ECEF
  const osg::Vec3d zeroPoint = this->getMatrix().getTrans();

  // purely vertical line will be drawn as a single segment
  if (distance <= 0.0)
  {
    // Convert back to ECEF and add the vertex
    const simCore::Coordinate& outEcef = coord2.ecefCoordinate();
    const osg::Vec3f p2 = osg::Vec3f(outEcef.x(), outEcef.y(), outEcef.z()) - zeroPoint;

    for (unsigned int i = 0; i < 2; ++i)
    {
      osgEarth::LineDrawable* line = geode_->getLineDrawable(i);
      line->clear();
      line->pushVertex(p2);
      line->pushVertex(osg::Vec3f());
      line->dirty();
    }

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

  // make sure there's enough room. Don't bother shrinking.
  const unsigned int numSegs = simCore::sdkMax(MIN_NUM_SEGMENTS, simCore::sdkMin(MAX_NUM_SEGMENTS, static_cast<unsigned int>(distance / segmentLength)));
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array();
  verts->reserve(numSegs + 1);

  // Add points to the vertex list, from back to front, for consistent stippling.  Order
  // matters because it affects the line direction during stippling.
  for (unsigned int k = 0; k < numSegs; ++k)
  {
    const float percentOfFull = (float)(numSegs-k) / (float)numSegs; // From 1 to 0 (almost 0)

    // Calculate the LLA value of the point, and replace the altitude
    double lat = 0.0;
    double lon = 0.0;
    simCore::sodanoDirect(lla1.lat(), lla1.lon(), lla1.alt(), distance * percentOfFull, azimuth, &lat, &lon);
    const double alt = lla1.alt() + percentOfFull * (lla2.alt() - lla1.alt());

    // Convert back to ECEF and add the vertex
    simCore::Vec3 ecefPos;
    simCore::CoordinateConverter::convertGeodeticPosToEcef(simCore::Vec3(lat, lon, alt), ecefPos);
    verts->push_back(osg::Vec3f(ecefPos.x(), ecefPos.y(), ecefPos.z()) - zeroPoint);
  }

  // Finish up
  verts->push_back(osg::Vec3f());

  geode_->getLineDrawable(0)->importVertexArray(verts.get());
  geode_->getLineDrawable(1)->importVertexArray(verts.get());
}

}
