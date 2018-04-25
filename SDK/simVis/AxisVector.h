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
#ifndef SIMVIS_AXIS_VECTOR_H
#define SIMVIS_AXIS_VECTOR_H

#include "osg/MatrixTransform"
#include "osg/Vec3"
#include "simCore/Common/Common.h"
#include "simData/DataTypes.h"
#include "simVis/Types.h"

namespace osg { class Geometry; }
namespace osgEarth { class LineGroup; }

namespace simVis
{

/**
 * X, Y, and Z Axis display.  Unit length, though can be scaled.
 */
class SDKVIS_EXPORT AxisVector : public osg::MatrixTransform
{
public:
  /** Declare boilerplate code */
  META_Node(simVis, AxisVector);
  /** Constructor */
  AxisVector();
  /** OSG Copy constructor */
  AxisVector(const AxisVector &rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY);

  /** Helper method to configure a scale matrix; will optimize away if not set, unless force is true */
  void setAxisLengths(osg::Vec3f axisLengths, bool force=false);
  /** Helper method to configure a scale matrix; will optimize away if not set, unless force is true */
  void setAxisLengths(float xLength, float yLength, float zLength, bool force=false);
  /** Last line length set from setAxisLength */
  osg::Vec3f axisLengths() const;

  /** Set width of the lines for the axes */
  void setLineWidth(float lineWidth);
  /** Retrieve width of the lines for the axes */
  float lineWidth() const;

  /** Change the axis colors */
  void setColors(const simVis::Color& x, const simVis::Color& y, const simVis::Color& z);
  /** Retrieve the X axis color */
  simVis::Color xColor() const;
  /** Retrieve the Y axis color */
  simVis::Color yColor() const;
  /** Retrieve the Z axis color */
  simVis::Color zColor() const;

  /**
  * Position the axes at the specified pos, orienting the x-axis along the specified vec
  * @param[in] pos the position for the axes origin
  * @param[in] vec the orientation for the x-axis
  */
  void setPositionOrientation(const osg::Vec3f& pos, const osg::Vec3f& vec);

protected:

  /// osg::Referenced-derived
  virtual ~AxisVector();

private:
  /// create the geometry
  void init_();

  /// create the axis vector lines
  void createAxisVectors_(osg::Geode* geode) const;

  /// width of axis vector lines
  float lineWidth_;
  /// most recent value for axis size
  osg::Vec3f axisLengths_;
  /// holds the 3 axis vectors
  osg::ref_ptr<osgEarth::LineGroup> geode_;
};

} // namespace simVis

#endif // SIMVIS_AXIS_VECTOR_H
