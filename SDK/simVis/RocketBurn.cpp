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
#include "osg/Program"
#include "osg/Geometry"
#include "osg/MatrixTransform"
#include "osg/Depth"
#include "OpenThreads/Mutex"
#include "OpenThreads/ScopedLock"
#include "osgEarth/ShaderGenerator"
#include "osgEarth/Registry"
#include "simVis/DisableDepthOnAlpha.h"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Shaders.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/RocketBurn.h"

namespace simVis
{
// statics
osg::observer_ptr<osg::StateSet> RocketBurn::s_stateSet_;

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
  // Drastically lower the threshold for disabling depth on alpha, to improve typical use case
  simVis::DisableDepthOnAlpha::setAlphaThreshold(stateSet, 0.001f);

  // Add to the platform
  if (hostPlatform.getModel() != nullptr)
    hostPlatform.getModel()->addScaledChild(transform_.get());
}

RocketBurn::~RocketBurn()
{
  removeFromScene_();
  transform_ = nullptr;
}

// A RocketBurn visual is a series of poofs (textured quads).
// Calculate all poof parameters before updating the geometry
// so we know how much memory to allocate:
struct Poof
{
  float radius;
  float length;
  float alpha;
};

void RocketBurn::rebuild_()
{
  // hard-coded texture image unit.
  // (If you decide to parameterize this, see the note below about
  // moving the associated Uniform into the transform's stateset.)
  const unsigned int textureUnit = 0;

  // Lazy initialization on the group
  if (!group_.valid())
  {
    group_ = new osg::Group();

    // program goes on the group since it's globally shared
    osg::ref_ptr<osg::StateSet> stateSet;
    if (s_stateSet_.lock(stateSet) == false)
    {
      static OpenThreads::Mutex s_mutex;
      OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex);
      if (s_stateSet_.lock(stateSet) == false)
      {
        s_stateSet_ = stateSet = new osg::StateSet();
        // Load the virtual program and attach the ATTRIBUTE_6 parameter to the shader's radius attribute
        osgEarth::VirtualProgram* vp = osgEarth::VirtualProgram::getOrCreate(stateSet.get());
        simVis::Shaders shaders;
        shaders.load(vp, shaders.rocketBurn());
        vp->addBindAttribLocation("sim_RocketBurn_radius", osg::Drawable::ATTRIBUTE_6);

        // Note: textureUnit is a const delcared above. If you parameterize it,
        // you will need to move this uniform to the transform_'s stateset! -gw
        stateSet->addUniform(new osg::Uniform("sim_RocketBurn_tex", static_cast<int>(textureUnit)));
      }
    }
    group_->setStateSet(stateSet.get());
    transform_->addChild(group_.get());
  }

  if (currentShape_.length <= 0)
  {
    // rocket burn is off
    transform_->setNodeMask(simVis::DISPLAY_MASK_NONE);
    return;
  }

  transform_->setNodeMask(simVis::DISPLAY_MASK_PLATFORM);

  // texture information goes on the transform since it can change
  // across RocketBurn instances
  transform_->getOrCreateStateSet()->setTextureAttributeAndModes(textureUnit, texture_);

  // Scaling Factor for multiple QUADS to create a Rocket burn
  static const float ROCKETBURN_SCALE_FACTOR = 0.5f;
  double currentRadius = currentShape_.radiusNear;
  double currentLength = 0.0;

  osg::ref_ptr<osg::Vec3Array> verts;
  osg::ref_ptr<osg::FloatArray> radii;
  osg::ref_ptr<osg::Vec4Array> colors;
  osg::ref_ptr<osg::DrawElementsUShort> elements;

  if (!geometry_.valid())
  {
    // Holds the poof visuals. Mark as DYNAMIC to support
    // runtime updates to the buffers.
    geometry_ = new osg::Geometry();
    geometry_->setDataVariance(osg::Object::DYNAMIC);
    geometry_->setName("simVis::RocketBurn");

    verts = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    geometry_->setVertexArray(verts.get());
    colors = new osg::Vec4Array(osg::Array::BIND_PER_VERTEX);
    geometry_->setColorArray(colors.get());
    radii = new osg::FloatArray(osg::Array::BIND_PER_VERTEX);
    geometry_->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, radii.get(), osg::Array::BIND_PER_VERTEX);
    elements = new osg::DrawElementsUShort(GL_TRIANGLES);
    geometry_->addPrimitiveSet(elements.get());

    group_->addChild(geometry_.get());
  }
  else
  {
    // Buffers already exist...find them
    verts = static_cast<osg::Vec3Array*>(geometry_->getVertexArray());
    colors = static_cast<osg::Vec4Array*>(geometry_->getColorArray());
    radii = static_cast<osg::FloatArray*>(geometry_->getVertexAttribArray(osg::Drawable::ATTRIBUTE_6));
    elements = static_cast<osg::DrawElementsUShort*>(geometry_->getPrimitiveSet(0));
  }

  std::vector<Poof> poofs;
  if (!verts->empty())
    poofs.reserve(verts->size() / 4);

  while (currentLength < currentShape_.length)
  {
    Poof p;
    p.radius = currentRadius;
    p.length = currentLength;
    p.alpha = currentShape_.scaleAlpha ?
      static_cast<float>(1.0 - currentLength / currentShape_.length) :
      currentShape_.color.a();

    poofs.push_back(p);

    // A heuristic algorithm for dividing up the rocket burn.
    currentLength = currentLength + currentRadius * ROCKETBURN_SCALE_FACTOR;
    currentRadius = currentShape_.radiusNear + ((currentShape_.radiusFar - currentShape_.radiusNear) * (currentLength / currentShape_.length));
  }

  // Clear all buffers and reserve new space if necessary.
  // Memory will only allocate if more space is needed.
  verts->clear();
  verts->reserveArray(poofs.size() * 4u);
  colors->clear();
  colors->reserveArray(poofs.size() * 4u);
  radii->clear();
  radii->reserveArray(poofs.size() * 4u);
  elements->clear();
  elements->reserveElements(poofs.size() * 6u);

  for (unsigned int i = 0; i < poofs.size(); ++i)
  {
    Poof& poof = poofs[i];

    // two triangles comprise a quad:
    unsigned int k = i * 4u;
    elements->addElement(k);
    elements->addElement(k + 1);
    elements->addElement(k + 2);
    elements->addElement(k + 2);
    elements->addElement(k + 3);
    elements->addElement(k);

    // offsets the poof along the length of the burn:
    osg::Vec3f currentVert(0.f, -poof.length, 0.f);

    // custom alpha per poof:
    osg::Vec4f currentColor(currentShape_.color);
    currentColor.a() = poof.alpha;

    // 4 verts per poof (4 corners to be expanded by the shader)
    for (unsigned int j = 0; j < 4u; ++j)
    {
      verts->push_back(currentVert);
      colors->push_back(currentColor);
      radii->push_back(poof.radius);
    }
  }

  // Mark all arrays dirty so they re-sync with the GPU
  verts->dirty();
  colors->dirty();
  radii->dirty();
  elements->dirty();
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
