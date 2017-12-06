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
*               EW Modeling and Simulation, Code 5770
*               4555 Overlook Ave.
*               Washington, D.C. 20375-5339
*
* For more information please send email to simdis@enews.nrl.navy.mil
*
* The U.S. Government retains all rights to use, duplicate, distribute,
* disclose, or release this software.
****************************************************************************
*
*/

#include "osg/Geometry"
#include "osg/LineStipple"
#include "osg/LineWidth"
#include "simVis/BoxGraphic.h"

namespace simVis
{

BoxGraphic::BoxGraphic(double x, double y, double width, double height,
  float lineWidth, unsigned short stipple, const osg::Vec4& color)
  : osg::Geode(),
    x_(x),
    y_(y),
    width_(width),
    height_(height),
    lineWidth_(lineWidth),
    stipple_(stipple),
    color_(color)
{
  create_();
}

BoxGraphic::BoxGraphic(const BoxGraphic& rhs)
{
  x_ = rhs.x_;
  y_ = rhs.y_;
  width_ = rhs.width_;
  height_ = rhs.height_;
  lineWidth_ = rhs.lineWidth_;
  color_ = rhs.color_;
  create_();
}

BoxGraphic::~BoxGraphic()
{}

double BoxGraphic::x() const
{
  return x_;
}

double BoxGraphic::y() const
{
  return y_;
}

double BoxGraphic::width() const
{
  return width_;
}

double BoxGraphic::height() const
{
  return height_;
}

float BoxGraphic::lineWidth() const
{
  return lineWidth_;
}

unsigned short BoxGraphic::stipple() const
{
  return stipple_;
}

osg::Vec4 BoxGraphic::color() const
{
  return color_;
}

void BoxGraphic::setGeometry(double x, double y, double width, double height)
{
  x_ = x;
  y_ = y;
  width_ = width;
  height_ = height;

  verts_->reserve(4);
  verts_->clear();

  verts_->push_back(osg::Vec3d(x, y, 0));
  verts_->push_back(osg::Vec3d(x + width, y, 0));
  verts_->push_back(osg::Vec3d(x + width, y + height, 0));
  verts_->push_back(osg::Vec3d(x, y + height, 0));

  verts_->dirty();
  primset_->setCount(verts_->size());
  primset_->dirty();
  for (unsigned int i = 0; i < getNumDrawables(); ++i)
  {
    getDrawable(i)->dirtyBound();
  }
}

void BoxGraphic::setLineWidth(float lineWidth)
{
  lineWidth_ = lineWidth;
  osg::ref_ptr<osg::LineWidth> lineWidthAttr = new osg::LineWidth();
  lineWidthAttr->setWidth(lineWidth);
  osg::StateSet* stateSet = geometry_->getOrCreateStateSet();
  stateSet->setAttributeAndModes(lineWidthAttr.get(), 1);
}

void BoxGraphic::setStipplePattern(unsigned short stipple)
{
  stipple_ = stipple;
  osg::ref_ptr<osg::LineStipple> stippleAttr = new osg::LineStipple();
  stippleAttr->setFactor(2);
  stippleAttr->setPattern(stipple);
  osg::StateSet* stateSet = geometry_->getOrCreateStateSet();
  stateSet->setAttributeAndModes(stippleAttr.get(), 1);
}


void BoxGraphic::setColor(const osg::Vec4& color)
{
  color_ = color;
  osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array(1);
  (*colorArray)[0] = color;
  geometry_->setColorArray(colorArray);
  geometry_->setColorBinding(osg::Geometry::BIND_OVERALL);
  osg::VertexBufferObject* vbo = colorArray->getVertexBufferObject();
  if (vbo)
    vbo->setUsage(GL_DYNAMIC_DRAW_ARB);
}

void BoxGraphic::create_()
{
  geometry_ = new osg::Geometry;
  geometry_->setUseVertexBufferObjects(true);
  geometry_->setDataVariance(osg::Object::DYNAMIC);

  verts_ = new osg::Vec3Array(2);
  (*verts_)[0].set(0.0f, 0.0f, 0.0f);
  (*verts_)[1].set(0.0f, 0.0f, 0.0f);
  primset_ = new osg::DrawArrays(GL_LINE_LOOP, 0, 4);

  geometry_->setVertexArray(verts_);
  osg::VertexBufferObject* vbo = verts_->getVertexBufferObject();
  if (vbo)
    vbo->setUsage(GL_DYNAMIC_DRAW_ARB);
  geometry_->addPrimitiveSet(primset_);

  setGeometry(x_, y_, width_, height_);
  setLineWidth(lineWidth_);
  setStipplePattern(stipple_);
  setColor(color_);

  // Add to the geode
  addDrawable(geometry_);
}

}
