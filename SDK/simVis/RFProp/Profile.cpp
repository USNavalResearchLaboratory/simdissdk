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
#include "osg/Texture2D"
#include "osg/Point"
#include "osg/MatrixTransform"
#include "osgEarth/NodeUtils"
#include "osgEarth/ImageUtils"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/Interpolation.h"
#include "simVis/Constants.h"
#include "simVis/Utils.h"
#include "simVis/RFProp/Profile.h"

using namespace simRF;
using namespace simCore;


Profile::Profile(CompositeProfileProvider* data)
 : bearing_(0),
   displayThickness_(1000.0f),
   height_(0.0),
   halfBeamWidth_(osg::DegreesToRadians(5.0)),
   data_(data),
   dirty_(true),
   alpha_(1.0),
   agl_(false),
   mode_(DRAWMODE_2D_HORIZONTAL),
   refCoord_(0, 0, 0),
   sphericalEarth_(true),
   elevAngle_(0.0)
{
  alphaUniform_ = getOrCreateStateSet()->getOrCreateUniform("alpha", osg::Uniform::FLOAT);
  alphaUniform_->set(alpha_);

  transform_ = new osg::MatrixTransform();
  addChild(transform_);
  updateOrientation_();

  init_();
}

void Profile::addProvider(ProfileDataProvider* provider)
{
  if (provider)
    getDataProvider_()->addProvider(provider);
}

const CompositeProfileProvider* Profile::getDataProvider() const
{
  return data_;
}

CompositeProfileProvider* Profile::getDataProvider_()
{
  return data_;
}

void Profile::setDataProvider(CompositeProfileProvider* dataProvider)
{
  if (data_ != dataProvider)
  {
    data_ = dataProvider;
    // if providers change, null the texture to force it to recreate (if necessary)
    texture_ = NULL;
    dirty();
  }
}

Profile::DrawMode Profile::getMode() const
{
  return mode_;
}

void Profile::setMode(DrawMode mode)
{
  if (mode_ != mode)
  {
    mode_ = mode;
    if (mode_ != DRAWMODE_3D_TEXTURE)
    {
      texture_ = NULL;
    }
    dirty();
  }
}

void Profile::dirty()
{
  if (!dirty_)
  {
    ADJUST_UPDATE_TRAV_COUNT(this, 1);
  }
  dirty_ = true;
}

double Profile::getBearing() const
{
  return bearing_;
}

void Profile::setBearing(double bearing)
{
  if (bearing_ != bearing)
  {
    bearing_ = simCore::angFix2PI(bearing);
    updateOrientation_();
  }
}

bool Profile::getAGL() const
{
  return agl_;
}

void Profile::setAGL(bool agl)
{
  if (agl_ != agl)
  {
    agl_ = agl;
    dirty();
  }
}

double Profile::getElevAngle() const
{
  return elevAngle_;
}

void Profile::setElevAngle(double elevAngleRad)
{
  if (elevAngle_ != elevAngleRad)
  {
    elevAngle_ = elevAngleRad;
    if (mode_ == DRAWMODE_RAE)
    {
      // only RAE mode uses elev angle;
      // if angle changes at all, and interpolation is on, this will mean a lot of reprocessing at every update
      dirty();
    }
  }
}

double Profile::getRefLat() const
{
  return refCoord_.y();
}

double Profile::getRefLon() const
{
  return refCoord_.x();
}

double Profile::getRefAlt() const
{
  return refCoord_.z();
}

void Profile::setRefCoord(double latRad, double lonRad, double alt)
{
  if (latRad != refCoord_.y() || lonRad != refCoord_.x() || alt != refCoord_.z())
  {
    refCoord_.set(lonRad, latRad, alt);
    dirty();
  }
}

bool Profile::getSphericalEarth() const
{
  return sphericalEarth_;
}

/**
*Set whether this Profile should conform to a spherical earth
*/
void Profile::setSphericalEarth(bool sphericalEarth)
{
  if (sphericalEarth_ != sphericalEarth)
  {
    sphericalEarth_ = sphericalEarth;
    dirty();
  }
}

void Profile::setTerrainHeights(const std::map<float, float>& terrainHeights)
{
  terrainHeights_.clear();
  terrainHeights_ = terrainHeights;
  dirty();
}

float Profile::getAlpha() const
{
  return alpha_;
}

void Profile::setAlpha(float alpha)
{
  if (alpha_ != alpha)
  {
    alpha_ = osg::clampBetween(alpha, 0.0f, 1.0f);
    alphaUniform_->set(alpha);
  }
}

float Profile::getDisplayThickness() const
{
  return displayThickness_;
}

void Profile::setDisplayThickness(float displayThickness)
{
  if (displayThickness_ != displayThickness)
  {
    displayThickness_ = displayThickness;
    dirty();
  }
}

void Profile::setHeight(double height)
{
  if (height_ != height)
  {
    height_ = height;
    dirty();
  }
}

double Profile::getHeight() const
{
  return height_;
}

double Profile::getHalfBeamWidth() const
{
  return halfBeamWidth_;
}

void Profile::setHalfBeamWidth(double halfBeamWidth)
{
  if (halfBeamWidth_ != halfBeamWidth)
  {
    halfBeamWidth_ = halfBeamWidth;
    dirty();
  }
}

void Profile::adjustSpherical_(osg::Vec3& v, const double *lla, const simCore::Vec3 *tpSphereXYZ)
{
  double pos[3] = { v[0], v[1], v[2] };
  simCore::Vec3 sphereXYZ;
  simCore::tangentPlane2Sphere(lla, pos, sphereXYZ, tpSphereXYZ);
  double alt = v3Length(sphereXYZ) - simCore::EARTH_RADIUS;
  v.z() = v.z() - (alt - v.z()) + refCoord_.z();
}

float Profile::getTerrainHgt_(float gndRng) const
{
  // initialize terrain hgt to default value in case interpolation fails
  float value = 0.f;
  simCore::linearInterpolate(terrainHeights_, gndRng, value);
  return value;
}

ProfileDataProvider::ThresholdType Profile::getThresholdType() const
{
  const CompositeProfileProvider* provider = getDataProvider();
  return (provider && provider->getActiveProvider()) ? provider->getActiveProvider()->getType() : ProfileDataProvider::THRESHOLDTYPE_NONE;
}

void Profile::setThresholdType(ProfileDataProvider::ThresholdType type)
{
  CompositeProfileProvider* provider = getDataProvider_();
  if (provider)
  {
    provider->setActiveProvider(type);
    // if providers change, null the texture to force it to recreate (if necessary)
    texture_ = NULL;
    dirty();
  }
}

void Profile::init_()
{
  // Remove all existing nodes
  transform_->removeChildren(0, transform_->getNumChildren());

  // Clear out the original values
  verts_ = new osg::Vec3Array();
  values_ = new osg::FloatArray();
  geode_ = NULL;
  if (mode_ != DRAWMODE_3D_TEXTURE)
  {
    // if assert fails, check that setMode nulls texture on mode change
    assert(texture_ == NULL);
  }

  // ensure that our provider is valid
  if (data_.valid() && data_->getActiveProvider() != NULL)
  {
    geode_ = new osg::Geode;
    switch (mode_)
    {
      case DRAWMODE_2D_HORIZONTAL:
        init2DHoriz_();
        break;
      case DRAWMODE_2D_VERTICAL:
        init2DVert_();
        break;
      case DRAWMODE_2D_TEE:
        init2DHoriz_();
        init2DVert_();
        break;
      case DRAWMODE_3D:
        init3D_();
        break;
      case DRAWMODE_3D_TEXTURE:
        init3DTexture_();
        break;
      case DRAWMODE_3D_POINTS:
        init3DPoints_();
        break;
      case DRAWMODE_RAE:
        initRAE_();
        break;
      default:
        // if assert fails, a new type has been added, but this switch has not been updated
        assert(0);
    }
    transform_->addChild(geode_);
  }
  dirty_ = false;
}

void Profile::updateOrientation_()
{
  if (transform_)
  {
    // TODO:  Z axis is flipped in order to correctly display RF prop data (SIMSDK-365)
    transform_->setMatrix(osg::Matrixd::rotate(bearing_, osg::Vec3d(0, 0, -1)));
  }
}

void Profile::init2DHoriz_()
{
  assert(data_.valid() && data_->getActiveProvider() != NULL);
  const double minRange = data_->getMinRange();
  const double rangeStep = data_->getRangeStep();
  const double numRanges = data_->getNumRanges();

  const double dt0 = -halfBeamWidth_ + M_PI_2;
  const double dt1 = halfBeamWidth_ + M_PI_2;

  const double cosTheta0 = cos(dt0);
  const double sinTheta0 = sin(dt0);
  const double cosTheta1 = cos(dt1);
  const double sinTheta1 = sin(dt1);

  simCore::Vec3 tpSphereXYZ;
  simCore::geodeticToSpherical(refCoord_.y(), refCoord_.x(), refCoord_.z(), tpSphereXYZ);
  const double lla[3] = { refCoord_.y(), refCoord_.x(), refCoord_.z() };

  const unsigned int startIndex = verts_->size();
  unsigned int heightIndex = data_->getHeightIndex(height_);
  // Error check the height index
  if (heightIndex == CompositeProfileProvider::INVALID_HEIGHT_INDEX)
  {
    // Invalidly defined profile
    assert(0);
    return;
  }

  for (unsigned int i = 0; i < numRanges; i++)
  {
    const double range = minRange + rangeStep * i;
    double height = height_;
    if (agl_ && !terrainHeights_.empty())
    {
      height = height_ + getTerrainHgt_(static_cast<float>(range));
      heightIndex = data_->getHeightIndex(height);
      // Error check the height index
      if (heightIndex == CompositeProfileProvider::INVALID_HEIGHT_INDEX)
      {
        // Invalidly defined profile
        assert(0);
        return;
      }
    }

    // Left vert
    osg::Vec3 v0(range * cosTheta0, range * sinTheta0, height);
    // Right vert
    osg::Vec3 v1(range * cosTheta1, range * sinTheta1, height);

    if (sphericalEarth_)
    {
      adjustSpherical_(v0, lla, &tpSphereXYZ);
      adjustSpherical_(v1, lla, &tpSphereXYZ);
    }

    verts_->push_back(v0);
    verts_->push_back(v1);

    heightIndex = osg::clampBetween(heightIndex, 0u, data_->getNumHeights() - 1);
    const double value = data_->getValueByIndex(heightIndex, i);
    values_->push_back(value);
    values_->push_back(value);
  }

  osg::Geometry* geometry = new osg::Geometry();
  geometry->setUseVertexBufferObjects(true);
  geometry->setDataVariance(osg::Object::DYNAMIC);
  geometry->setVertexArray(verts_);
  geometry->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, values_);
  geometry->setVertexAttribBinding(osg::Drawable::ATTRIBUTE_6, osg::Geometry::BIND_PER_VERTEX);
  geometry->setVertexAttribNormalize(osg::Drawable::ATTRIBUTE_6, false);
  geometry->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

  const unsigned int count = verts_->size() - startIndex;

  geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUAD_STRIP, startIndex, count));

  geode_->addDrawable(geometry);
}

void Profile::init2DVert_()
{
  assert(data_.valid() && data_->getActiveProvider() != NULL);
  const double minRange = data_->getMinRange();
  const double rangeStep = data_->getRangeStep();
  const unsigned int numRanges = data_->getNumRanges();
  // if assert fails, check that init_ ensures that we have a valid provider
  assert(numRanges > 0);

  const double minHeight = data_->getMinHeight();
  const double heightStep = data_->getHeightStep();
  const unsigned int numHeights = data_->getNumHeights();
  // if assert fails, check that init_ ensures that we have a valid provider
  assert(numHeights > 0);

  simCore::Vec3 tpSphereXYZ;
  simCore::geodeticToSpherical(refCoord_.y(), refCoord_.x(), refCoord_.z(), tpSphereXYZ);
  const double lla[3] = { refCoord_.y(), refCoord_.x(), refCoord_.z() };

  const unsigned int startIndex = verts_->size();
  for (unsigned int r = 0; r < numRanges; r++)
  {
    const double x = 0;
    const double y = minRange + rangeStep * r;

    for (unsigned int h = 0; h < numHeights; h++)
    {
      const double height = minHeight + heightStep * h;
      osg::Vec3 v(x, y, height);

      if (sphericalEarth_)
      {
        adjustSpherical_(v, lla, &tpSphereXYZ);
      }

      verts_->push_back(v);

      const float value = data_->getValueByIndex(h, r);
      values_->push_back(value);
    }
  }

  // Now build the indices that will actually be rendered
  osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_QUADS);
  idx->reserve((numRanges - 1) * (numHeights - 1) * 4);

  for (unsigned int r = 0; r < numRanges - 1; r++)
  {
    const unsigned int nextRange = r + 1;
    float valueR = 0.0f;
    float valueL = 0.0f;
    unsigned int endIndex = 0;
    bool optimizedQuad = false;

    // build the graphic for this range cell, from bottom to top
    for (unsigned int h = 0; h < numHeights; ++h)
    {
      const unsigned int indexR = startIndex + (nextRange * numHeights) + h;
      const unsigned int indexL = startIndex + (r * numHeights) + h;
      const float newValueR = values_->at(indexR);
      const float newValueL = values_->at(indexL);

      // we always use bottom verts
      if (h != 0)
      {
        // if there is a value change between bottom and top of cell, this cell must be rendered as its own quad
        // if there is no change in the cell, defer ending the quad, set optimizedQuad flag
        if (newValueR == valueR && newValueL == valueL)
        {
          optimizedQuad = true;
          idx->setElement(endIndex, indexL);
          idx->setElement(endIndex + 1, indexR);
          // if we're at last height iteration, continue will bring us to the end
          continue;
        }

        // there is a transition in this cell, end this quad
        // if we were optimizing the previous quad, we need to end that quad
        if (optimizedQuad)
        {
          // accept the already provisionally completed quad from last iteration, by not continuing to defer
          optimizedQuad = false;
          // start a new quad for this cell - top indices from previous quad become the bottom indices of new quad
          // new lr
          idx->push_back(idx->getElement(endIndex + 1));
          // new ll
          idx->push_back(idx->getElement(endIndex));

          endIndex = idx->getNumIndices();

          // end the quad
          // ul
          idx->push_back(indexL);
          // ur
          idx->push_back(indexR);
        }
        else
        {
          // there is a transition in this cell, end the quad
          idx->setElement(endIndex, indexL);
          idx->setElement(endIndex + 1, indexR);
        }
      }

      const bool lastIteration = (h == (numHeights - 1));
      if (!lastIteration)
      {
        // start a new quad
        // new lr
        idx->push_back(indexR);
        valueR = newValueR;
        // new ll
        idx->push_back(indexL);
        valueL = newValueL;

        endIndex = idx->getNumIndices();
        // pre-set this quad's top indices (to be overwritten (using setElement) next iteration)
        // ul
        idx->push_back(indexL);
        // ur
        idx->push_back(indexR);
      }
    }
  }

  osg::Geometry* geometry = new osg::Geometry();
  geometry->setUseVertexBufferObjects(true);
  geometry->setDataVariance(osg::Object::DYNAMIC);
  geometry->setVertexArray(verts_);

  geometry->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, values_);
  geometry->setVertexAttribBinding(osg::Drawable::ATTRIBUTE_6, osg::Geometry::BIND_PER_VERTEX);
  geometry->setVertexAttribNormalize(osg::Drawable::ATTRIBUTE_6, false);
  geometry->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

  geometry->addPrimitiveSet(idx);
  geode_->addDrawable(geometry);
}

void Profile::init3D_()
{
  assert(data_.valid() && data_->getActiveProvider() != NULL);
  const double minRange = data_->getMinRange();
  const double rangeStep = data_->getRangeStep();
  const unsigned int numRanges = data_->getNumRanges();
  // if assert fails, check that init_ ensures that we have a valid provider
  assert(numRanges > 0);

  const double minHeight = data_->getMinHeight();
  const double heightStep = data_->getHeightStep();
  const unsigned int numHeights = data_->getNumHeights();
  // if assert fails, check that init_ ensures that we have a valid provider
  assert(numHeights > 0);
  if (numHeights == 0)
    return;

  //Build a 3D voxel representation of the profile.  The minimum height is specified by the height_ setting and the maximum height is the height_ + the display thickness
  unsigned int minHeightIndex = data_->getHeightIndex(height_);
  unsigned int maxHeightIndex = data_->getHeightIndex(height_ + displayThickness_);
  // Error check the height index
  if (minHeightIndex == CompositeProfileProvider::INVALID_HEIGHT_INDEX ||
    maxHeightIndex == CompositeProfileProvider::INVALID_HEIGHT_INDEX)
  {
    // Invalidly defined profile
    assert(0);
    return;
  }

  minHeightIndex = osg::clampBetween(minHeightIndex, 0u, numHeights - 1);
  maxHeightIndex = osg::clampBetween(maxHeightIndex, 0u, numHeights - 1);

  simCore::Vec3 tpSphereXYZ;
  simCore::geodeticToSpherical(refCoord_.y(), refCoord_.x(), refCoord_.z(), tpSphereXYZ);
  const double lla[3] = { refCoord_.y(), refCoord_.x(), refCoord_.z() };

  //If we have no valid thickness assume they want to just display a single voxel
  if (minHeightIndex == maxHeightIndex)
  {
    if (minHeightIndex == numHeights - 1)
    {
      //The display height is set to to the max height of the profile, so move the min height back one index
      minHeightIndex = minHeightIndex - 1;
    }
    else
    {
      //The display height is set to to the max height of the profile, so move the min height back one index
      maxHeightIndex = maxHeightIndex + 1;
    }
  }

  //Do nothing if the ranges just aren't valid
  if (minHeightIndex >= numHeights || maxHeightIndex >= numHeights)
  {
    return;
  }

  const unsigned int heightIndexCount = maxHeightIndex - minHeightIndex + 1;
  const unsigned int numVerts = 2 * heightIndexCount * numRanges;
  //osg::Vec3Array* verts = new osg::Vec3Array();
  verts_->reserve(numVerts);

  const unsigned int startIndex = verts_->size();

  osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_QUADS);
  idx->reserve(24 * numVerts);

  const double dt0 = -halfBeamWidth_ + M_PI_2;
  const double dt1 = halfBeamWidth_ + M_PI_2;

  const double cosTheta0 = cos(dt0);
  const double sinTheta0 = sin(dt0);
  const double cosTheta1 = cos(dt1);
  const double sinTheta1 = sin(dt1);

  for (unsigned int r = 0; r < numRanges; r++)
  {
    const double range = minRange + rangeStep * r;
    const double x0 = range * cosTheta0;
    const double y0 = range * sinTheta0;
    const double x1 = range * cosTheta1;
    const double y1 = range * sinTheta1;

    for (unsigned int h = minHeightIndex; h <= maxHeightIndex; h++)
    {
      const double height = minHeight + heightStep * h;
      //Left vert
      osg::Vec3 v0(x0, y0, height);
      //Right vert
      osg::Vec3 v1(x1, y1, height);

      if (sphericalEarth_)
      {
        adjustSpherical_(v0, lla, &tpSphereXYZ);
        adjustSpherical_(v1, lla, &tpSphereXYZ);
      }

      verts_->push_back(v0);
      verts_->push_back(v1);

      double value = data_->getValueByIndex(h, r);
      values_->push_back(value);
      values_->push_back(value);
    }
  }

  //Now build the indices that will actually be rendered
  for (unsigned int r = 0; r < numRanges - 1; r++)
  {
    const unsigned int nextR = r + 1;
    for (unsigned int h = minHeightIndex; h < maxHeightIndex; h++)
    {
      //Compute the indices of the 8 corners of the cube
      const unsigned int v0 = startIndex + r * heightIndexCount * 2 + (h - minHeightIndex) * 2;  // front LR
      const unsigned int v1 = v0 + 1; // front LL
      const unsigned int v2 = v1 + 1; // front UR
      const unsigned int v3 = v2 + 1; // front UL

      const unsigned int v4 = startIndex + nextR * heightIndexCount * 2 + (h - minHeightIndex) * 2; // back LR
      const unsigned int v5 = v4 + 1; // back LL
      const unsigned int v6 = v5 + 1; // back UR
      const unsigned int v7 = v6 + 1; // back UL

      // Front
      idx->push_back(v0); idx->push_back(v2); idx->push_back(v3); idx->push_back(v1);
      // Back
      idx->push_back(v5); idx->push_back(v7); idx->push_back(v6); idx->push_back(v4);

      // Top
      idx->push_back(v2); idx->push_back(v6); idx->push_back(v7); idx->push_back(v3);
      // Bottom
      idx->push_back(v1); idx->push_back(v5); idx->push_back(v4); idx->push_back(v0);

      // Left
      idx->push_back(v1); idx->push_back(v3); idx->push_back(v7); idx->push_back(v5);
      // Right
      idx->push_back(v4); idx->push_back(v6); idx->push_back(v2); idx->push_back(v0);
    }
  }

  osg::Geometry* geometry = new osg::Geometry();
  geometry->setDataVariance(osg::Object::DYNAMIC);
  geometry->setVertexArray(verts_);
  geometry->setUseVertexBufferObjects(true);

  geometry->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, values_);
  geometry->setVertexAttribBinding(osg::Drawable::ATTRIBUTE_6, osg::Geometry::BIND_PER_VERTEX);
  geometry->setVertexAttribNormalize(osg::Drawable::ATTRIBUTE_6, false);


  geometry->addPrimitiveSet(idx);
  geode_->addDrawable(geometry);
}

void Profile::init3DTexture_()
{
  assert(data_.valid() && data_->getActiveProvider() != NULL);
  const double maxRange = data_->getMaxRange();

  const double minHeight = data_->getMinHeight();
  const double maxHeight = data_->getMaxHeight();
  const double heightStep = data_->getHeightStep();
  const unsigned int numHeights = data_->getNumHeights();
  // if assert fails, check that init_ ensures that we have a valid provider
  assert(numHeights > 0);

  //Build a 3D voxel representation of the profile.  The minimum height is specified by the height_ setting and the maximum height is the height_ + the display thickness
  unsigned int minHeightIndex = data_->getHeightIndex(height_);
  unsigned int maxHeightIndex = data_->getHeightIndex(height_ + displayThickness_);
  // Error check the height index
  if (minHeightIndex == CompositeProfileProvider::INVALID_HEIGHT_INDEX ||
    maxHeightIndex == CompositeProfileProvider::INVALID_HEIGHT_INDEX)
  {
    // Invalidly defined profile
    assert(0);
    return;
  }

  minHeightIndex = osg::clampBetween(minHeightIndex, 0u, numHeights - 1);
  maxHeightIndex = osg::clampBetween(maxHeightIndex, 0u, numHeights - 1);

  simCore::Vec3 tpSphereXYZ;
  simCore::geodeticToSpherical(refCoord_.y(), refCoord_.x(), refCoord_.z(), tpSphereXYZ);

  //If we have no valid thickness assume they want to just display a single voxel
  if (minHeightIndex == maxHeightIndex)
  {
    if (minHeightIndex == numHeights - 1)
    {
      //The display height is set to the max height of the profile, so move the min height back one index
      minHeightIndex = minHeightIndex - 1;
    }
    else
    {
      //The display height is set to the max height of the profile, so move the min height back one index
      maxHeightIndex = maxHeightIndex + 1;
    }
  }

  //Do nothing if the ranges just aren't valid
  if (minHeightIndex >= numHeights || maxHeightIndex >= numHeights)
  {
    return;
  }

  // Compute the min and max sampled heights
  const float minSampledHeight = minHeight + heightStep * minHeightIndex;
  const float maxSampledHeight = minHeight + heightStep * maxHeightIndex;

  const float minT = (minSampledHeight - minHeight) / (maxHeight - minHeight);
  const float maxT = (maxSampledHeight - minHeight) / (maxHeight - minHeight);

  const double dt0 = -halfBeamWidth_ + M_PI_2;
  const double dt1 = halfBeamWidth_ + M_PI_2;

  const double cosTheta0 = cos(dt0);
  const double sinTheta0 = sin(dt0);
  const double cosTheta1 = cos(dt1);
  const double sinTheta1 = sin(dt1);

  osg::Vec2Array* tcoords = new osg::Vec2Array();

  // The first two verts are the points at the start of the pie slice
  verts_->push_back(osg::Vec3(0, 0, minSampledHeight));  // 0
  tcoords->push_back(osg::Vec2(0, minT));

  verts_->push_back(osg::Vec3(0, 0, maxSampledHeight));  // 1
  tcoords->push_back(osg::Vec2(0, maxT));

  // Now the left two verts of the pie slice
  verts_->push_back(osg::Vec3(maxRange * cosTheta0, maxRange * sinTheta0, minSampledHeight)); // 2
  tcoords->push_back(osg::Vec2(1.0, minT));

  verts_->push_back(osg::Vec3(maxRange * cosTheta0, maxRange * sinTheta0, maxSampledHeight)); // 3
  tcoords->push_back(osg::Vec2(1.0, maxT));

  // Now the right two verts of the pie slice
  verts_->push_back(osg::Vec3(maxRange * cosTheta1, maxRange * sinTheta1, minSampledHeight)); // 4
  tcoords->push_back(osg::Vec2(1.0, minT));

  verts_->push_back(osg::Vec3(maxRange * cosTheta1, maxRange * sinTheta1, maxSampledHeight)); // 5
  tcoords->push_back(osg::Vec2(1.0, maxT));

#if 0

  osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLES);
  // Right side
  idx->addElement(0);  idx->addElement(4); idx->addElement(5); idx->addElement(0); idx->addElement(5); idx->addElement(1);

  // Left side
  idx->addElement(2);  idx->addElement(0); idx->addElement(1);  idx->addElement(2); idx->addElement(1); idx->addElement(3);

  // Top side
  idx->addElement(1);  idx->addElement(5); idx->addElement(3);

  // Bottom side
  idx->addElement(0);  idx->addElement(2); idx->addElement(4);

  // Cap
  idx->addElement(5); idx->addElement(4); idx->addElement(2); idx->addElement(5); idx->addElement(2);  idx->addElement(3);


  osg::Geometry* geometry = new osg::Geometry();
  geometry->setDataVariance(osg::Object::DYNAMIC);
  geometry->setVertexArray(verts_);
  geometry->setUseVertexBufferObjects(true);
  geometry->setColorArray(colors_);
  geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

  geometry->setTexCoordArray(0, tcoords);
  geometry->addPrimitiveSet(idx);
  geode_->addDrawable(geometry);
#else

  // Add a drawable for each of the sides of the faces.
  // Right side
  {
    osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLES);
    idx->addElement(0);  idx->addElement(4); idx->addElement(5); idx->addElement(0); idx->addElement(5); idx->addElement(1);
    osg::Geometry* geometry = new osg::Geometry();
    geometry->setDataVariance(osg::Object::DYNAMIC);
    geometry->setVertexArray(verts_);
    geometry->setUseVertexBufferObjects(true);

    geometry->setTexCoordArray(0, tcoords);
    geometry->addPrimitiveSet(idx);
    geode_->addDrawable(geometry);
  }

  // Left side
    {
      osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLES);
      idx->addElement(2);  idx->addElement(0); idx->addElement(1);  idx->addElement(2); idx->addElement(1); idx->addElement(3);
      osg::Geometry* geometry = new osg::Geometry();
      geometry->setDataVariance(osg::Object::DYNAMIC);
      geometry->setVertexArray(verts_);
      geometry->setUseVertexBufferObjects(true);
      geometry->setTexCoordArray(0, tcoords);
      geometry->addPrimitiveSet(idx);
      geode_->addDrawable(geometry);
    }

    // Top side
    {
      osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLES);
      idx->addElement(1);  idx->addElement(5); idx->addElement(3);
      osg::Geometry* geometry = new osg::Geometry();
      geometry->setDataVariance(osg::Object::DYNAMIC);
      geometry->setVertexArray(verts_);
      geometry->setUseVertexBufferObjects(true);
      geometry->setTexCoordArray(0, tcoords);
      geometry->addPrimitiveSet(idx);
      geode_->addDrawable(geometry);
    }

    // Bottom side
    {

      osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLES);
      idx->addElement(0);  idx->addElement(2); idx->addElement(4);
      osg::Geometry* geometry = new osg::Geometry();
      geometry->setDataVariance(osg::Object::DYNAMIC);
      geometry->setVertexArray(verts_);
      geometry->setUseVertexBufferObjects(true);
      geometry->setTexCoordArray(0, tcoords);
      geometry->addPrimitiveSet(idx);
      geode_->addDrawable(geometry);
    }

    // Cap
    {
      osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLES);
      idx->addElement(5); idx->addElement(4); idx->addElement(2); idx->addElement(5); idx->addElement(2);  idx->addElement(3);
      osg::Geometry* geometry = new osg::Geometry();
      geometry->setDataVariance(osg::Object::DYNAMIC);
      geometry->setVertexArray(verts_);
      geometry->setUseVertexBufferObjects(true);
      geometry->setTexCoordArray(0, tcoords);
      geometry->addPrimitiveSet(idx);
      geode_->addDrawable(geometry);
    }
#endif

    // Only create the texture if it doesn't already exist, otherwise you can just reuse it
    if (texture_ == NULL)
    {
      texture_ = new osg::Texture2D(createImage_());
      texture_->setResizeNonPowerOfTwoHint(false);
      texture_->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
      texture_->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    }
    geode_->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture_);
}

void Profile::init3DPoints_()
{
  assert(data_.valid() && data_->getActiveProvider() != NULL);
  const double minRange = data_->getMinRange();
  const double rangeStep = data_->getRangeStep();
  const unsigned int numRanges = data_->getNumRanges();

  const double minHeight = data_->getMinHeight();
  const double heightStep = data_->getHeightStep();
  const unsigned int numHeights = data_->getNumHeights();
  // if assert fails, check that init_ ensures that we have a valid provider
  assert(numHeights > 0);

  //Build a 3D voxel representation of the profile.  The minimum height is specified by the height_ setting and the maximum height is the height_ + the display thickness
  unsigned int minHeightIndex = data_->getHeightIndex(height_);
  unsigned int maxHeightIndex = data_->getHeightIndex(height_ + displayThickness_);
  // Error check the height index
  if (minHeightIndex == CompositeProfileProvider::INVALID_HEIGHT_INDEX ||
    maxHeightIndex == CompositeProfileProvider::INVALID_HEIGHT_INDEX)
  {
    // Invalidly defined profile
    assert(0);
    return;
  }

  minHeightIndex = osg::clampBetween(minHeightIndex, 0u, numHeights - 1);
  maxHeightIndex = osg::clampBetween(maxHeightIndex, 0u, numHeights - 1);

  simCore::Vec3 tpSphereXYZ;
  simCore::geodeticToSpherical(refCoord_.y(), refCoord_.x(), refCoord_.z(), tpSphereXYZ);
  const double lla[3] = { refCoord_.y(), refCoord_.x(), refCoord_.z() };

  //If we have no valid thickness assume they want to just display a single voxel
  if (minHeightIndex == maxHeightIndex)
  {
    if (minHeightIndex == numHeights - 1)
    {
      //The display height is set to the max height of the profile, so move the min height back one index
      minHeightIndex = minHeightIndex - 1;
    }
    else
    {
      //The display height is set to the max height of the profile, so move the min height back one index
      maxHeightIndex = maxHeightIndex + 1;
    }
  }

  //Do nothing if the ranges just aren't valid
  if (minHeightIndex >= numHeights || maxHeightIndex >= numHeights)
  {
    return;
  }

  const unsigned int numVerts = (maxHeightIndex - minHeightIndex + 1) * numRanges;
  verts_->reserve(numVerts);

  for (unsigned int r = 0; r < numRanges; r++)
  {
    const double range = minRange + rangeStep * r;

    for (unsigned int h = minHeightIndex; h <= maxHeightIndex; h++)
    {
      const double height = minHeight + heightStep * h;
      osg::Vec3 v(0, range, height);
      if (sphericalEarth_)
      {
        adjustSpherical_(v, lla, &tpSphereXYZ);
      }
      verts_->push_back(v);
      const double value = data_->getValueByIndex(h, r);
      values_->push_back(value);
    }
  }

  osg::Geometry* geometry = new osg::Geometry();
  geometry->setDataVariance(osg::Object::DYNAMIC);
  geometry->setVertexArray(verts_);
  geometry->setUseVertexBufferObjects(true);

  geometry->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, values_);
  geometry->setVertexAttribBinding(osg::Drawable::ATTRIBUTE_6, osg::Geometry::BIND_PER_VERTEX);
  geometry->setVertexAttribNormalize(osg::Drawable::ATTRIBUTE_6, false);

  geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, verts_->size()));
  geode_->addDrawable(geometry);
  geode_->getOrCreateStateSet()->setAttributeAndModes(new osg::Point(3.0f), osg::StateAttribute::ON);
}

osg::Image* Profile::createImage_()
{
  assert(data_.valid() && data_->getActiveProvider() != NULL);
  const unsigned int numRanges = data_->getNumRanges();
  const unsigned int numHeights = data_->getNumHeights();

  osg::Image* image = new osg::Image();
  image->allocateImage(numRanges, numHeights, 1, GL_LUMINANCE, GL_FLOAT);
  image->setInternalTextureFormat(GL_LUMINANCE32F_ARB);

  for (unsigned int r = 0; r < numRanges; r++)
  {
    for (unsigned int h = 0; h < numHeights; h++)
    {
      const double value = data_->getValueByIndex(h, r);
      *(float*)image->data(r, h) = (float)value;
    }
  }
  return image;
}

void Profile::buildVoxel_(const double *lla, const simCore::Vec3 *tpSphereXYZ, unsigned int heightIndex, unsigned int rangeIndex, osg::DrawElementsUInt* idx)
{
  assert(data_.valid() && data_->getActiveProvider() != NULL);
  const double minRange = data_->getMinRange();
  const double rangeStep = data_->getRangeStep();
  const unsigned int numRanges = data_->getNumRanges();
  // if assert fails, check that init_ ensures that we have a valid provider
  assert(numRanges > 0);

  const double minHeight = data_->getMinHeight();
  const double heightStep = data_->getHeightStep();
  const unsigned int numHeights = data_->getNumHeights();
  // if assert fails, check that init_ ensures that we have a valid provider
  assert(numHeights > 0);

  const unsigned int minHeightIndex = heightIndex;
  const unsigned int maxHeightIndex = heightIndex + 1;

  //Uncommenting this block clamps the RAE to the maximum height instead of just not drawing it.
  /*
  minHeightIndex = osg::clampBetween(minHeightIndex, 0u,numHeights-1);
  maxHeightIndex = osg::clampBetween(maxHeightIndex, 0u,numHeights-1);

  //If we have no valid thickness assume they want to just display a single voxel
  if (minHeightIndex == maxHeightIndex)
  {
  if (minHeightIndex == numHeights-1)
  {
  //The display height is set to the max height of the profile, so move the min height back one index
  minHeightIndex = minHeightIndex -1;
  }
  else
  {
  //The display height is set to the max height of the profile, so move the min height back one index
  maxHeightIndex = maxHeightIndex +1;
  }
  }
  */

  //Do nothing if the heights just aren't valid
  if (minHeightIndex >= numHeights || maxHeightIndex >= numHeights)
  {
    return;
  }

  const double h0 = minHeight + heightStep * minHeightIndex;
  const double h1 = h0 + heightStep;

  unsigned int minRangeIndex = rangeIndex;
  unsigned int maxRangeIndex = minRangeIndex + 1;

  minRangeIndex = osg::clampBetween(minRangeIndex, 0u, numRanges - 1);
  maxRangeIndex = osg::clampBetween(maxRangeIndex, 0u, numRanges - 1);

  //If we have no valid thickness assume they want to just display a single voxel
  if (minRangeIndex == maxRangeIndex)
  {
    if (minRangeIndex == numRanges - 1)
    {
      minRangeIndex = minRangeIndex - 1;
    }
    else
    {
      maxRangeIndex = maxRangeIndex + 1;
    }
  }

  //Do nothing if the ranges just aren't valid
  if (minRangeIndex >= numRanges || maxRangeIndex >= numRanges)
  {
    return;
  }

  const double r0 = minRange + rangeStep * minRangeIndex;
  const double r1 = r0 + rangeStep;

  const double dt0 = -halfBeamWidth_ + M_PI_2;
  const double dt1 = halfBeamWidth_ + M_PI_2;

  const double cosTheta0 = cos(dt0);
  const double sinTheta0 = sin(dt0);
  const double cosTheta1 = cos(dt1);
  const double sinTheta1 = sin(dt1);

  //Bottom verts
  osg::Vec3 v0(r0 * cosTheta0, r0 * sinTheta0, h0); // Near right
  osg::Vec3 v1(r0 * cosTheta1, r0 * sinTheta1, h0); // Near left
  osg::Vec3 v2(r1 * cosTheta1, r1 * sinTheta1, h0); // Far left
  osg::Vec3 v3(r1 * cosTheta0, r1 * sinTheta0, h0); // Far right

  //Top verts
  osg::Vec3 v4(r0 * cosTheta0, r0 * sinTheta0, h1); // Near right
  osg::Vec3 v5(r0 * cosTheta1, r0 * sinTheta1, h1); // Near left
  osg::Vec3 v6(r1 * cosTheta1, r1 * sinTheta1, h1); // Far left
  osg::Vec3 v7(r1 * cosTheta0, r1 * sinTheta0, h1); // Far right

  if (sphericalEarth_)
  {
    adjustSpherical_(v0, lla, tpSphereXYZ);
    adjustSpherical_(v1, lla, tpSphereXYZ);
    adjustSpherical_(v2, lla, tpSphereXYZ);
    adjustSpherical_(v3, lla, tpSphereXYZ);
    adjustSpherical_(v4, lla, tpSphereXYZ);
    adjustSpherical_(v5, lla, tpSphereXYZ);
    adjustSpherical_(v6, lla, tpSphereXYZ);
    adjustSpherical_(v7, lla, tpSphereXYZ);
  }

  unsigned int startIndex = verts_->size();
  const unsigned int i0 = startIndex++;
  const unsigned int i1 = startIndex++;
  const unsigned int i2 = startIndex++;
  const unsigned int i3 = startIndex++;
  const unsigned int i4 = startIndex++;
  const unsigned int i5 = startIndex++;
  const unsigned int i6 = startIndex++;
  const unsigned int i7 = startIndex++;

  verts_->push_back(v0);
  verts_->push_back(v1);
  verts_->push_back(v2);
  verts_->push_back(v3);
  verts_->push_back(v4);
  verts_->push_back(v5);
  verts_->push_back(v6);
  verts_->push_back(v7);

  //v0, v1
  double value = data_->getValueByIndex(minHeightIndex, minRangeIndex);
  values_->push_back(value);
  values_->push_back(value);

  //v2, v3
  value = data_->getValueByIndex(minHeightIndex, maxRangeIndex);
  values_->push_back(value);
  values_->push_back(value);

  //v4, v5
  value = data_->getValueByIndex(maxHeightIndex, minRangeIndex);
  values_->push_back(value);
  values_->push_back(value);

  //v6, v7
  value = data_->getValueByIndex(maxHeightIndex, maxRangeIndex);
  values_->push_back(value);
  values_->push_back(value);

  // Bottom
  idx->push_back(i0); idx->push_back(i1); idx->push_back(i2); idx->push_back(i3);
  // Top
  idx->push_back(i4); idx->push_back(i7); idx->push_back(i6); idx->push_back(i5);

  // Front
  idx->push_back(i0); idx->push_back(i4); idx->push_back(i5); idx->push_back(i1);
  // Back
  idx->push_back(i2); idx->push_back(i6); idx->push_back(i7); idx->push_back(i3);

  // Right
  idx->push_back(i3); idx->push_back(i7); idx->push_back(i4); idx->push_back(i0);
  // Left
  idx->push_back(i1); idx->push_back(i5); idx->push_back(i6); idx->push_back(i2);

}

void Profile::initRAE_()
{
  assert(data_.valid() && data_->getActiveProvider() != NULL);
  const unsigned int numRanges = data_->getNumRanges();
  // if assert fails, check that init_ ensures that we have a valid provider
  assert(numRanges > 0);

  const double rangeStep = data_->getRangeStep();

  osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_QUADS);

  simCore::Vec3 tpSphereXYZ;
  const double lla[3] = { refCoord_.y(), refCoord_.x(), refCoord_.z() };
  const double sinElevAngle = sin(elevAngle_);
  simCore::geodeticToSpherical(refCoord_.y(), refCoord_.x(), refCoord_.z(), tpSphereXYZ);
  for (unsigned int i = 0; i < numRanges - 1; ++i)
  {
    const double height = height_ + (i * rangeStep * sinElevAngle);
    const unsigned int heightIndex = data_->getHeightIndex(height);
    // Error check the height index
    if (heightIndex == CompositeProfileProvider::INVALID_HEIGHT_INDEX)
    {
      // Invalidly defined profile
      assert(0);
      return;
    }
    buildVoxel_(lla, &tpSphereXYZ, heightIndex, i, idx);
  }

  osg::Geometry* geometry = new osg::Geometry();
  geometry->setUseVertexBufferObjects(true);
  geometry->setDataVariance(osg::Object::DYNAMIC);
  geometry->setVertexArray(verts_);

  geometry->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, values_);
  geometry->setVertexAttribBinding(osg::Drawable::ATTRIBUTE_6, osg::Geometry::BIND_PER_VERTEX);
  geometry->setVertexAttribNormalize(osg::Drawable::ATTRIBUTE_6, false);

  geometry->addPrimitiveSet(idx);

  geode_->addDrawable(geometry);
}

void Profile::traverse(osg::NodeVisitor& nv)
{
  if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
  {
    if (dirty_)
    {
      init_();
    }
    ADJUST_UPDATE_TRAV_COUNT(this, -1);
  }
  osg::Group::traverse(nv);
}
