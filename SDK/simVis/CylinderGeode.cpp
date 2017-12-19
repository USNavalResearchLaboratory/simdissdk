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
#include "osgEarth/Registry"
#include "osgEarth/ShaderGenerator"
#include "simVis/Platform.h"
#include "simVis/PlatformModel.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/CylinderGeode.h"

namespace simVis
{
CylinderGeode::CylinderGeode(PlatformNode &hostPlatform)
  : Referenced(),
  transform_(new osg::MatrixTransform)
{
  // Set up the render bin, and turn off lighting
  osg::StateSet* stateSet = transform_->getOrCreateStateSet();
  simVis::setLighting(stateSet, osg::StateAttribute::OFF);
  stateSet->setRenderBinDetails(BIN_CYLINDER, BIN_GLOBAL_SIMSDK);

  // Add to the platform
  if (hostPlatform.getModel() != NULL)
    hostPlatform.getModel()->addScaledChild(transform_.get());
}

CylinderGeode::~CylinderGeode()
{
  removeFromScene_();
  transform_ = NULL;
}

void CylinderGeode::rebuild_()
{

  if (geode_.valid())
  {
    // Remove the drawables from the geode
    geode_->removeDrawables(0, geode_->getNumDrawables());
  }
  else
  {
    geode_ = new osg::Geode;
    // Attach the geode to ourselves
    transform_->addChild(geode_);
  }

  if (currentShape_.length <= 0)
  {
    // Cylinder is off
    transform_->setNodeMask(simVis::DISPLAY_MASK_NONE);
    return;
  }

  transform_->setNodeMask(simVis::DISPLAY_MASK_PLATFORM);

  // Set up wall geometry
  osg::ref_ptr<osg::Geometry> wallGeom = new osg::Geometry;
  // Create and bind vertex array
  osg::ref_ptr<osg::Vec3Array> wallVerts = new osg::Vec3Array;
  wallGeom->setVertexArray(wallVerts.get());
  // Create and bind color array
  osg::ref_ptr<osg::Vec4Array> wallColors = new osg::Vec4Array;
  wallGeom->setColorArray(wallColors.get());
  wallGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

  // Push in the center point of each circle
  simVis::Color colorNear(currentShape_.colorNear);
  simVis::Color colorFar(currentShape_.colorFar);

  /** Number of points in Cylinder's top and bottom caps */
  static const int CAP_RESOLUTION = 32;

  for (int i = 0; i < CAP_RESOLUTION; i++)
  {
    /** Converts the CAP_RESOLUTION to points on a circle, in range [0, 2PI) */
    const double angle = i * M_TWOPI / CAP_RESOLUTION;

    osg::Vec3 topVert(currentShape_.radiusNear * sin(angle), 0, currentShape_.radiusNear * cos(angle));
    osg::Vec3 botVert(currentShape_.radiusFar * sin(angle), -currentShape_.length, currentShape_.radiusFar * cos(angle));

    // Pushes the points from the top and bottom circles in
    wallVerts->push_back(topVert);
    wallColors->push_back(colorNear);
    wallVerts->push_back(botVert);
    wallColors->push_back(colorFar);
  }

  // Pushes the points from the top and bottom circles in
  wallVerts->push_back(*(wallVerts->begin()));
  wallVerts->push_back(*(wallVerts->begin()+1));
  wallColors->push_back(colorNear);
  wallColors->push_back(colorFar);

  // Set the primitive type of each geometry
  wallGeom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, wallVerts->size()));

  // Load the geometries into the geode
  geode_->addDrawable(wallGeom);
  // Turn off backface culling
  geode_->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
}

void CylinderGeode::update(const ShapeData &newShapeData)
{
  if (newShapeData == currentShape_)
    return;

  currentShape_.radiusFar = simCore::sdkMax(0.0, newShapeData.radiusFar);
  currentShape_.radiusNear = simCore::sdkMax(0.0, newShapeData.radiusNear);
  currentShape_.length = simCore::sdkMax(0.0, newShapeData.length);

  currentShape_.colorNear = newShapeData.colorNear;
  currentShape_.colorFar = newShapeData.colorFar;

  rebuild_();
}

void CylinderGeode::setPositionOrientation(const simCore::Vec3& newPosition, const simCore::Vec3& yprRadians)
{
  // Convert the ENU/RightHanded rotations to a rotation matrix.
  osg::Matrixd rot;
  rot.makeRotate(Math::eulerRadToQuat(yprRadians.yaw(), yprRadians.pitch(), yprRadians.roll()));

  // Create a position matrix
  rot.postMultTranslate(osg::Vec3d(newPosition.x(), newPosition.y(), newPosition.z()));

  // Set the transform to the rotation and the position
  transform_->setMatrix(rot);
}

void CylinderGeode::removeFromScene_()
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

CylinderGeode::ShapeData::ShapeData()
 : radiusNear(0),
   radiusFar(0),
   length(0),
   colorNear(1, 1, 1, 1),
   colorFar(1, 1, 1, 1)
{
}

bool CylinderGeode::ShapeData::operator==(const ShapeData& other) const
{
  return simCore::areEqual(other.radiusNear, radiusNear) &&
    simCore::areEqual(other.radiusFar, radiusFar) &&
    simCore::areEqual(other.length, length) &&
    other.colorNear == colorNear &&
    other.colorFar == colorFar;
}

}
