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
#include "osgEarth/LineDrawable"
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
    stippleFactor_(1u),
    stipplePattern_(stipple),
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
  stippleFactor_ = rhs.stippleFactor_;
  stipplePattern_ = rhs.stipplePattern_;
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

unsigned int BoxGraphic::stippleFactor() const
{
  return stippleFactor_;
}

unsigned short BoxGraphic::stipplePattern() const
{
  return stipplePattern_;
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

  geom_->setVertex(0, osg::Vec3f(x, y, 0.f));
  geom_->setVertex(1, osg::Vec3f(x + width, y, 0.f));
  geom_->setVertex(2, osg::Vec3f(x + width, y + height, 0.f));
  geom_->setVertex(3, osg::Vec3f(x, y + height, 0.f));
  geom_->setVertex(4, osg::Vec3f(x, y, 0.f));
}

void BoxGraphic::setLineWidth(float lineWidth)
{
  lineWidth_ = lineWidth;
  geom_->setLineWidth(lineWidth);
}

void BoxGraphic::setStippleFactor(unsigned int factor)
{
  stippleFactor_ = factor;
  geom_->setStippleFactor(stippleFactor_);
}

void BoxGraphic::setStipplePattern(unsigned short stipple)
{
  stipplePattern_ = stipple;
  geom_->setStipplePattern(stipplePattern_);
}

void BoxGraphic::setColor(const osg::Vec4& color)
{
  color_ = color;
  geom_->setColor(color);
}

void BoxGraphic::create_()
{
  geom_ = new osgEarth::LineDrawable(GL_LINE_STRIP);
  geom_->setName("simVis::BoxGraphic");
  geom_->setDataVariance(osg::Object::DYNAMIC);
  geom_->allocate(5);

  setGeometry(x_, y_, width_, height_);
  setLineWidth(lineWidth_);
  setStippleFactor(stippleFactor_);
  setStipplePattern(stipplePattern_);
  setColor(color_);

  // Add to the geode
  addChild(geom_.get());
}

}
