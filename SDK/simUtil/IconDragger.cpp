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
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osg/AutoTransform"
#include "osg/BlendFunc"
#include "osg/Image"
#include "osg/Texture2D"
#include "osgEarth/GeoPositionNodeAutoScaler"
#include "osgEarth/GLUtils"
#include "simVis/Utils.h"
#include "IconDragger.h"

namespace simUtil {

IconDragger::IconDragger(osgEarth::MapNode* mapNode, osg::Image* image)
  : Dragger(mapNode),
  texture_(new osg::Texture2D)
{
  // Create the quad geometry, centered on 0,0
  geometry_ = osg::createTexturedQuadGeometry(
    osg::Vec3(-size_ / 2.0f, -size_ / 2.0f, 0.0f),
    osg::Vec3(size_, 0.0f, 0.0f),
    osg::Vec3(0.0f, size_, 0.0f)
  );
  // Data variance required for size and color changes
  geometry_->setDataVariance(osg::Object::DYNAMIC);

  // Apply the base color to the geometry (white by default)
  osg::Vec4Array* colors = new osg::Vec4Array(1);
  (*colors)[0] = color_;
  geometry_->setColorArray(colors, osg::Array::BIND_OVERALL);

  texture_->setResizeNonPowerOfTwoHint(false);
  texture_->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
  texture_->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
  setImage(image);

  // Set up reasonable stateset values for 2D icons
  osg::StateSet* stateSet = geometry_->getOrCreateStateSet();
  stateSet->setTextureAttributeAndModes(0, texture_.get(), osg::StateAttribute::ON);
  stateSet->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON);
  stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
  osgEarth::GLUtils::setLighting(stateSet, osg::StateAttribute::OFF);

  // Run the shader generator, but also isolate it from Map for simpler shader state
  osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateSet);
  vp->setInheritShaders(false);
  osgEarth::Registry::shaderGenerator().run(geometry_.get());

  // Billboard the icon instead of laying flat on globe
  osg::AutoTransform* autoTransform = new osg::AutoTransform();
  autoTransform->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
  autoTransform->addChild(geometry_.get());
  getPositionAttitudeTransform()->addChild(autoTransform);
  addCullCallback(new osgEarth::GeoPositionNodeAutoScaler());

  updateColor_();
}

void IconDragger::setImage(osg::Image* image)
{
  if (!texture_.valid())
    return;
  texture_->setImage(image);
  // GL Core profile fixes require a valid image
  if (image)
    simVis::fixTextureForGlCoreProfile(texture_.get());
}

const osg::Vec4f& IconDragger::getColor() const
{
  return color_;
}

void IconDragger::setColor(const osg::Vec4f& color)
{
  if (color_ != color)
  {
    color_ = color;
    updateColor_();
  }
}

const osg::Vec4f& IconDragger::getPickColor() const
{
  return pickColor_;
}

void IconDragger::setPickColor(const osg::Vec4f& pickColor)
{
  if (pickColor_ != pickColor)
  {
    pickColor_ = pickColor;
    updateColor_();
  }
}

float IconDragger::getSize() const
{
  return size_;
}

void IconDragger::setSize(float size)
{
  if (size_ == size)
    return;
  size_ = size;

  // Update the vertex array to match the new size
  osg::Vec3Array* verts = static_cast<osg::Vec3Array*>(geometry_->getVertexArray());
  if (verts && verts->size() == 4)
  {
    const float half = size_ / 2.0f;
    (*verts)[0].set(-half, -half, 0.0f);
    (*verts)[1].set(half, -half, 0.0f);
    (*verts)[2].set(half, half, 0.0f);
    (*verts)[3].set(-half, half, 0.0f);
    verts->dirty();
    geometry_->dirtyBound();
  }
}

void IconDragger::enter()
{
  updateColor_();
}

void IconDragger::leave()
{
  updateColor_();
}

void IconDragger::updateColor_()
{
  osg::Vec4Array* colors = static_cast<osg::Vec4Array*>(geometry_->getColorArray());
  if (colors && colors->size() > 0)
  {
    (*colors)[0] = getHovered() ? pickColor_ : color_;
    colors->dirty();
  }
}

}
