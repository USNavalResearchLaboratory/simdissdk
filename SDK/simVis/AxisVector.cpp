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
#include "simNotify/Notify.h"
#include "simCore/Calc/Math.h"
#include "simVis/Constants.h"
#include "simVis/Utils.h"
#include "simVis/AxisVector.h"

namespace simVis
{

// --------------------------------------------------------------------------
namespace
{
/// Number of points in the subdivided line strip
const int AXIS_NUM_POINTS_PER_LINE_STRIP = 4;

}

// --------------------------------------------------------------------------
AxisVector::AxisVector()
  : MatrixTransform(),
    lineWidth_(new osg::LineWidth(2.f)),
    colors_(new osg::Vec4Array(osg::Array::BIND_PER_PRIMITIVE_SET)),
    axisLengths_(1.f, 1.f, 1.f)
{
  setName("AxisVector");

  // Initialize contents of colors array for 3 axes
  colors_->push_back(simVis::Color::Yellow);
  colors_->push_back(simVis::Color::Fuchsia);
  colors_->push_back(simVis::Color::Aqua);
  init_();
}

AxisVector::AxisVector(const AxisVector &rhs, const osg::CopyOp& copyOp)
  : MatrixTransform(rhs, copyOp),
    lineWidth_(static_cast<osg::LineWidth*>(copyOp(rhs.lineWidth_.get()))),
    colors_(static_cast<osg::Vec4Array*>(copyOp(colors_.get()))),
    axisLengths_(rhs.axisLengths_)
{
}

AxisVector::~AxisVector()
{
}

void AxisVector::init_()
{
  osg::ref_ptr<osg::Geode> geode = new osg::Geode();
  createAxisVectors_(geode.get());
  addChild(geode.get());
}

void AxisVector::setAxisLengths(osg::Vec3f axisLengths, bool force)
{
  if (force || axisLengths != axisLengths_)
  {
    MatrixTransform::setMatrix(osg::Matrix::scale(axisLengths));
    axisLengths_ = axisLengths;
  }
}

void AxisVector::setAxisLengths(float xLength, float yLength, float zLength, bool force)
{
  setAxisLengths(osg::Vec3f(xLength, yLength, zLength), force);
}

osg::Vec3f AxisVector::axisLengths() const
{
  return axisLengths_;
}

void AxisVector::setLineWidth(float lineWidth)
{
  lineWidth_->setWidth(lineWidth);
}

float AxisVector::lineWidth() const
{
  return lineWidth_->getWidth();
}

void AxisVector::setColors(const simVis::Color& x, const simVis::Color& y, const simVis::Color& z)
{
  // Avoid rebuilding unnecessarily
  if (x == xColor() && y == yColor() && z == zColor())
    return;
  (*colors_)[0] = x;
  (*colors_)[1] = y;
  (*colors_)[2] = z;
  colors_->dirty();
}

simVis::Color AxisVector::xColor() const
{
  return (*colors_)[0];
}

simVis::Color AxisVector::yColor() const
{
  return (*colors_)[1];
}

simVis::Color AxisVector::zColor() const
{
  return (*colors_)[2];
}

void AxisVector::setPositionOrientation(const osg::Vec3f& pos, const osg::Vec3f& vec)
{
  osg::Matrixf rot;
  if (vec != osg::Vec3f())
  {
    // determine a rotation matrix that rotates x-axis vector to the specified vector
    rot.makeRotate(osg::X_AXIS, vec);
  }
  rot.postMultTranslate(pos);
  rot.preMultScale(axisLengths_);
  setMatrix(rot);
}

void AxisVector::createAxisVectors_(osg::Geode* geode) const
{
  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
  geom->setName("simVis::AxisVector");
  geom->setUseVertexBufferObjects(true);

  osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
  geom->setVertexArray(vertices.get());

  geom->setColorArray(colors_.get());

  // Keep track of location in the primitive set array
  int primitiveSetStart = 0;

  // draw x axis vector
  addLineStrip_(*geom, *vertices, primitiveSetStart, osg::Vec3(0, 0, 0), osg::Vec3(1, 0, 0), AXIS_NUM_POINTS_PER_LINE_STRIP);

  // draw y axis vector
  addLineStrip_(*geom, *vertices, primitiveSetStart, osg::Vec3(0, 0, 0), osg::Vec3(0, 1, 0), AXIS_NUM_POINTS_PER_LINE_STRIP);

  // draw z axis vector
  addLineStrip_(*geom, *vertices, primitiveSetStart, osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 1), AXIS_NUM_POINTS_PER_LINE_STRIP);

  // set linewidth
  geom->getOrCreateStateSet()->setAttributeAndModes(lineWidth_.get(), 1);

  // Add the drawable to the geode
  geode->addDrawable(geom);
}


void AxisVector::addLineStrip_(osg::Geometry& geom, osg::Vec3Array& vertices, int& primitiveSetStart,
  const osg::Vec3& start, const osg::Vec3& end, int numPointsPerLine) const
{
  // Avoid divide-by-zero problems
  if (numPointsPerLine < 2)
    return;

  // Add line strips for each line
  VectorScaling::generatePoints(vertices, start, end, numPointsPerLine);
  geom.addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, primitiveSetStart, numPointsPerLine));
  primitiveSetStart += numPointsPerLine;
}

}
