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
#include "simVis/LineDrawable.h"
#include "simVis/BoxGraphic.h"

namespace simVis
{

BoxGraphic::BoxGraphic(double x, double y, double width, double height,
  float lineWidth, unsigned short stipple, const osg::Vec4& color)
  : osg::Group(),
    x_(x),
    y_(y),
    width_(width),
    height_(height),
    lineWidth_(lineWidth),
    factor_(1u),
    stipple_(stipple),
    color_(color)
{
  create_();
}

BoxGraphic::BoxGraphic(const BoxGraphic& rhs) :
osg::Group(rhs)
{
  x_ = rhs.x_;
  y_ = rhs.y_;
  width_ = rhs.width_;
  height_ = rhs.height_;
  lineWidth_ = rhs.lineWidth_;
  factor_ = rhs.factor_;
  stipple_ = rhs.stipple_;
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

unsigned int BoxGraphic::factor() const
{
  return factor_;
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

  geom_->setVertex(0, osg::Vec3d(x, y, 0));
  geom_->setVertex(1, osg::Vec3d(x + width, y, 0));
  geom_->setVertex(2, osg::Vec3d(x + width, y + height, 0));
  geom_->setVertex(3, osg::Vec3d(x, y + height, 0));
}

void BoxGraphic::setLineWidth(float lineWidth)
{
  lineWidth_ = lineWidth;
  geom_->setLineWidth(lineWidth);
}

void BoxGraphic::setStippleFactor(unsigned int factor)
{
  factor_ = factor;
  geom_->setStippleFactor(factor);
}

void BoxGraphic::setStipplePattern(unsigned short stipple)
{
  stipple_ = stipple;
  geom_->setStipplePattern(stipple);
}

void BoxGraphic::setColor(const osg::Vec4& color)
{
  color_ = color;
  geom_->setColor(color);
}

void BoxGraphic::create_()
{
  geom_ = new osgEarth::LineDrawable(GL_LINE_LOOP);
  geom_->setName("simVis::BoxGraphic");
  geom_->setDataVariance(osg::Object::DYNAMIC);
  geom_->allocate(4);

  // since we're not adding it to a LineGroup, call this:
  geom_->installShader();

  setGeometry(x_, y_, width_, height_);
  setLineWidth(lineWidth_);
  setStippleFactor(factor_);
  setStipplePattern(stipple_);
  setColor(color_);

  // Add to the geode
  addChild(geom_.get());
}

}
