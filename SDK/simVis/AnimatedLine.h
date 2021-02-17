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
#ifndef SIMDIS_VISUALIZATION_ANIMATED_LINE_H
#define SIMDIS_VISUALIZATION_ANIMATED_LINE_H

#include "osg/MatrixTransform"
#include "osg/PrimitiveSet"
#include "osgEarth/Revisioning"
#include "simCore/Common/Common.h"
#include "simCore/Calc/MultiFrameCoordinate.h"

namespace osgEarth { class LineDrawable; }

namespace simVis
{
  class Locator;

  /**
   * An animated line in local space. The line is defined by two endpoints, either Coordinates or Locators. Coordinates are simple fixed positions
   * while Locators are movable positions. The animated line automatically updates to the Locator's new position if it moves.
   * NOTE: AnimatedLine has no knowledge of the node that a Locator may represent. In the case that an EnityNode's Locator is passed as an endpoint
   * to the AnimatedLine, the line will not adjust depending on the visible state of the EntityNode. Also, if the EntidyNode is removed from the scene
   * graph, the AnimatedLine still holds a ref_ptr to the Locator, so it will continue to draw at the last location update of the Locator.
   */
  class SDKVIS_EXPORT AnimatedLineNode : public osg::MatrixTransform
  {
  public:
    /**
     * Constructs a new Animated Line node.
     */
    AnimatedLineNode(float lineWidth = 1.0f, bool depthBufferTest = true);

    /**
     * Sets the endpoints of the animated line to two fixed coordinates.
     * @param[in ] first  First endpoint; must be in world coords (e.g. ECEF, LLA)
     * @param[in ] second Second endpoint; can be in world or local coords.
     */
    void setEndPoints(const simCore::Coordinate& first, const simCore::Coordinate& second);

    /**
     * Sets the endpoints to a Locator and a fixed coordinate.
     * @param[in ] first  Locator
     * @param[in ] second Second coordinate of the line; can be in world or local coords.
     */
    void setEndPoints(const Locator* first, const simCore::Coordinate& second);

    /**
     * Sets the endpoints to two Locators.
     * @param[in ] first  First Locator
     * @param[in ] second Second Locator
     */
    void setEndPoints(const Locator* first, const Locator* second);

    /** Retrieves the current resolved position of the start and end of the line, returning 0 on success */
    int getEndPoints(simCore::MultiFrameCoordinate& coord1, simCore::MultiFrameCoordinate& coord2) const;

    /**
     * Sets the first stippling pattern for this line. The first color will
     * appear wherever this pattern has a set bit.
     * @param[in ] pattern Bitwise stippling pattern
     */
    void setStipple1(unsigned short pattern);

    /**
     * Gets the first stippling pattern.
     * @return Bitwise stipple pattern
     */
    unsigned short getStipple1() const { return stipple1_; }

    /**
     * Sets the second stippling pattern for this line. The second color will
     * appear wherever this pattern has a set bit.
     * @param[in ] pattern Bitwise stippling pattern
     */
    void setStipple2(unsigned short pattern);

    /**
     * Gets the second stippling pattern.
     * @return Pattern
     */
    unsigned short getStipple2() const { return stipple2_; }

    /**
     * Sets the first color. This color will appear wherever the first stipple
     * pattern has a bit set.
     * @param[in ] color First color (RGBA, [0..1])
     */
    void setColor1(const osg::Vec4& color);

    /**
     * Gets the first color
     * @return A color (RGBA, [0..1])
     */
    const osg::Vec4& getColor1() const { return color1_; }

    /**
     * Sets the second color. This color will appear wherever the second stipple
     * pattern has a set bit.
     * @param[in ] color Second color (RGBA, [0..1])
     */
    void setColor2(const osg::Vec4& color);

    /**
     * Gets the second color.
     * @return A color (RGBA, [0..1])
     */
    const osg::Vec4& getColor2() const { return color2_; }

    /**
     * Sets the override color. This color will override color1 and color2
     * @param[in ] color Override color (RGBA, [0..1])
     */
    void setColorOverride(const osg::Vec4& color);

    /**
     * Gets the override color
     * @return A color (RGBA, [0..1])
     */
    const osg::Vec4& getColorOverride() const { return colorOverride_; }

    /**
     * Clears the override color
     */
    void clearColorOverride();

    /**
     * Sets the speed at which the line animates.
     * @param[in ] value Number of line shifts per second (default = 5)
     */
    void setShiftsPerSecond(double value);

    /**
     * Gets the animation speed of the line
     * @return Number of shifts per second
     */
    double getShiftsPerSecond() const { return shiftsPerSecond_; }

    /**
     * Sets the line width
     * @param[in ] width Width in pixels
     */
    void setLineWidth(float width);

    /**
     * Gets the line width
     * @return Width in pixels
     */
    float getLineWidth() const;

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "AnimatedLineNode"; }

    /** On the UPDATE_VISITOR traversal, calls update_() to animate the line */
    virtual void traverse(osg::NodeVisitor& nv);

  protected:
    /// osg::Referenced-derived
    virtual ~AnimatedLineNode();

  private:
    unsigned short           stipple1_;
    unsigned short           stipple2_;
    double                   shiftsPerSecond_;
    osgEarth::Util::SimpleMutable<osg::Vec4> color1_;
    osgEarth::Util::SimpleMutable<osg::Vec4> color2_;
    osgEarth::Util::SimpleMutable<osg::Vec4> colorOverride_;
    bool useOverrideColor_;
    float lineWidth_;

    osg::ref_ptr<const Locator> firstLocator_;
    osgEarth::Util::Revision firstLocatorRevision_;

    osg::ref_ptr<const Locator> secondLocator_;
    osgEarth::Util::Revision secondLocatorRevision_;

    osgEarth::Util::SimpleMutable<simCore::MultiFrameCoordinate> firstCoord_;
    /**
     * Second Coordinate might be a relative coordinate (X-East).  This could be
     * associated with a (moving) locator instead of a static point, so we cannot
     * resolve it until later.  Because of this, we use a Coordinate and not a MFC.
     */
    osgEarth::Util::SimpleMutable<simCore::Coordinate> secondCoord_;
    /** ... but we still save the most recently resolved MF coordinate, only set when drawing. */
    osgEarth::Util::SimpleMutable<simCore::MultiFrameCoordinate> secondCoordMF_;
    /** Coordinate converter that is used to put secondCoord_ in a valid (Geo) frame */
    simCore::CoordinateConverter* coordinateConverter_;

    osg::ref_ptr<osg::Group> lineGroup_;

    /**
     * Lines longer than 100 kilometers can show significant jitter at the second point.
     * This is due to using the first point as a reference (location (0,0,0)) and the second point
     * is drawn relative to the first point with floating point resolution (32 bits).  A
     * user zooms in on the second point and sees the line "dance" around the second point.
     * The solution is to divide the line into two halves.  The first half goes from the
     * first point to the mid point.  The second half goes from the second point to the mid
     * point.  The two points are the references to their own line and consequently there
     * is no "dancing" of the line around either point.  The mid points may not line up and
     * may "dance" independently.
     */
    struct HalfALine
    {
      osg::ref_ptr<osg::MatrixTransform> matrix;
      osg::ref_ptr<osgEarth::LineDrawable> line1; ///< Point 1 to midpoint
      osg::ref_ptr<osgEarth::LineDrawable> line2; ///< Point 2 to midpoint

      HalfALine();
      virtual ~HalfALine();

      /**
       * Calculates the vertices from lastPoint (ECEF) to (0, 0, 0) with given number of segments.
       * @param numSegments Number of segments between last point and (0, 0, 0)
       * @param lastPoint The last point in ECEF
       * @param forward If true add the vertices from (0, 0, 0) to last point, if false reverse the order
       */
      void fillSlantLine(unsigned int numSegments, const osg::Vec3d& lastPoint, bool forward);
    };

    // Need continuous start to end for stipple to work correctly
    HalfALine firstHalf_;  ///< Start to Midpoint
    HalfALine secondHalf_;  ///< Midpoint to End

    // track time deltas for smooth animation
    double timeLastShift_;

    /// Flag for controlling depth buffer test
    bool depthBufferTest_;
    /// Turns depth testing off for lines that are close to surface (even if depth testing is requested)
    void fixDepth_(bool isCloseToSurface);

    void initializeGeometry_();
    void update_(double t);

    /** Returns true if a slant between two coordinates intersects earth surface */
    bool doesLineIntersectEarth_(const simCore::MultiFrameCoordinate& coord1, const simCore::MultiFrameCoordinate& coord2) const;

    /** Sets up the line vertices and primitive geometry, choosing straight or bending as appropriate */
    void drawLine_(const simCore::MultiFrameCoordinate& coord1, const simCore::MultiFrameCoordinate& coord2);
    /**
     * Draws a bending line between two simCore::Coordinates.  Coord1 must be LLA or ECEF.  Alters verts_
     * and primset_.  Performs fastest in LLA coordinate frame.
     */
    void drawBendingLine_(const simCore::MultiFrameCoordinate& coord1, const simCore::MultiFrameCoordinate& coord2);
    /** Draws a straight line.  Coord1 must be LLA or ECEF.  Fastest with ECEF coordinate. */
    void drawSlantLine_(const simCore::MultiFrameCoordinate& startPoint, const simCore::MultiFrameCoordinate& endPoint);
  };

} //namespace simVis

#endif // SIMDIS_VISUALIZATION_ANIMATED_LINE_H
