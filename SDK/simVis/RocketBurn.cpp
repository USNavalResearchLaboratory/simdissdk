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
#include "osg/Program"
#include "osg/Geometry"
#include "osg/MatrixTransform"
#include "osg/Depth"
#include "osgEarth/ShaderGenerator"
#include "osgEarth/Registry"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/RocketBurn.h"

namespace simVis
{
RocketBurn::RocketBurn(PlatformNode &hostPlatform, osg::Texture2D& texture)
  : Referenced(),
  texture_(&texture),
  transform_(new osg::MatrixTransform),
  shaderGeneratorRun_(false)
{
  // Set up the render bin, turn off depth writes, and turn on depth reads
  osg::StateSet* stateSet = transform_->getOrCreateStateSet();
  stateSet->setRenderBinDetails(BIN_ROCKETBURN, BIN_TRAVERSAL_ORDER_SIMSDK);
  stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::LESS, 0, 1, false));
  // Must be able to blend or the graphics will look awful
  stateSet->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED | osg::StateAttribute::OVERRIDE);

  // Add to the platform
  if (hostPlatform.getModel() != NULL)
    hostPlatform.getModel()->addScaledChild(transform_.get());
}

RocketBurn::~RocketBurn()
{
  removeFromScene_();
  transform_ = NULL;
}

void RocketBurn::rebuild_()
{
  const unsigned int textureUnit = 0; // first available unit
  // the geode is a node which contains the geometry to draw
  // use a billboard to keep the effect pointed at the camera

  if (geode_.valid())
  {
    // Remove the drawables from the geode
    geode_->removeDrawables(0, geode_->getNumDrawables());
  }
  else
  {
    geode_ = new osg::Billboard;
    geode_->setMode(osg::Billboard::POINT_ROT_EYE);
    geode_->getOrCreateStateSet()->setTextureAttributeAndModes(textureUnit, texture_);
    // attach the geode to ourselves
    transform_->addChild(geode_);
  }

  if (currentShape_.length <= 0)
  {
    // rocket burn is off
    transform_->setNodeMask(simVis::DISPLAY_MASK_NONE);
    return;
  }

  transform_->setNodeMask(simVis::DISPLAY_MASK_PLATFORM);

  // Scaling Factor for multiple QUADS to create a Rocket burn
  static const float ROCKETBURN_SCALE_FACTOR = 0.5f;
  double radiusCurrent = currentShape_.radiusNear;
  double currentLength = 0.0;

  while (currentLength < currentShape_.length)
  {
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array; // vertexes to draw
    geometry->setVertexArray(verts.get());

    // map (x,y) pixel coordinate to (s,t) texture coordinate
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    geometry->setTexCoordArray(textureUnit, texcoords.get());

    // colors
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
    geometry->setColorArray(colors.get());
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    simVis::Color currentColor(currentShape_.color);
    if (currentShape_.scaleAlpha)
    {
      // alpha will vary per quad
      currentColor.a() = float(1.0 - currentLength / currentShape_.length);
    }
    colors->push_back(currentColor);

    // add the vertices
    texcoords->push_back(osg::Vec2(1, 0));
    verts->push_back(osg::Vec3(radiusCurrent, 0, -radiusCurrent));

    texcoords->push_back(osg::Vec2(1, 1));
    verts->push_back(osg::Vec3(radiusCurrent, 0, radiusCurrent));

    texcoords->push_back(osg::Vec2(0, 0));
    verts->push_back(osg::Vec3(-radiusCurrent, 0, -radiusCurrent));

    texcoords->push_back(osg::Vec2(0, 1));
    verts->push_back(osg::Vec3(-radiusCurrent, 0, radiusCurrent));

    // tell the geometry that the array data describes triangle strips
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, verts->size()));

    // load the geometry into the geode
    geode_->addDrawable(geometry.get(), osg::Vec3(0, -currentLength, 0));

    // A heuristic algorithm for dividing up the rocket burn.
    currentLength = currentLength + radiusCurrent * ROCKETBURN_SCALE_FACTOR;
    radiusCurrent = currentShape_.radiusNear + ((currentShape_.radiusFar - currentShape_.radiusNear) * (currentLength / currentShape_.length));
  }

  // Generate shaders for the texturing to work (avoid doing more than once)
  if (!shaderGeneratorRun_)
  {
    osgEarth::Registry::shaderGenerator().run(transform_.get());
    shaderGeneratorRun_ = true;
  }
}

void RocketBurn::update(const ShapeData &newShapeData)
{
  if (newShapeData == currentShape_)
    return;

  if (newShapeData.radiusFar > 0)
    currentShape_.radiusFar = newShapeData.radiusFar;

  if (newShapeData.radiusNear > 0)
    currentShape_.radiusNear = newShapeData.radiusNear;

  if (newShapeData.length >= 0)
    currentShape_.length = newShapeData.length;

  currentShape_.color = newShapeData.color;
  currentShape_.scaleAlpha = newShapeData.scaleAlpha;

  rebuild_();
}

void RocketBurn::setPositionOrientation(const osg::Vec3f& newPosition, const osg::Vec3f& yprRadians)
{
  // Convert the ENU/RightHanded rotations to a rotation matrix.
  osg::Matrixd rot;
  rot.makeRotate(Math::eulerRadToQuat(yprRadians.x(), yprRadians.y(), yprRadians.z()));

  // Create a position matrix
  rot.postMultTranslate(newPosition);

  // Set the transform to the rotation and the position
  transform_->setMatrix(rot);
}

void RocketBurn::removeFromScene_()
{
  osg::Node::ParentList parents = transform_->getParents();
  for (osg::Node::ParentList::const_iterator j = parents.begin(); j != parents.end(); ++j)
  {
    osg::observer_ptr<osg::Group> parentAsGroup = (*j)->asGroup();
    if (parentAsGroup.valid())
      parentAsGroup->removeChild(transform_);
  }
}

//////////////////////////////////////////////////////////////////////////

RocketBurn::ShapeData::ShapeData()
 : radiusNear(0),
   radiusFar(0),
   length(0),
   color(1, 1, 1, 1),
   scaleAlpha(false)
{
}

bool RocketBurn::ShapeData::operator==(const ShapeData& other) const
{
  return simCore::areEqual(other.radiusNear, radiusNear) &&
    simCore::areEqual(other.radiusFar, radiusFar) &&
    simCore::areEqual(other.length, length) &&
    other.color == color &&
    other.scaleAlpha == scaleAlpha;
}

}
