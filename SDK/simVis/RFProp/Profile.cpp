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
#include "osg/MatrixTransform"
#include "osg/Texture2D"
#include "osgEarth/ImageUtils"
#include "osgEarth/NodeUtils"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/Interpolation.h"
#include "simVis/Constants.h"
#include "simVis/PointSize.h"
#include "simVis/Utils.h"
#include "simVis/RFProp/CompositeProfileProvider.h"
#include "simVis/RFProp/Profile.h"

namespace simRF {

Profile::Profile(CompositeProfileProvider* data)
 : bearing_(0),
   displayThickness_(1000.0f),
   height_(0.0),
   halfBeamWidth_(0.0),
   data_(data),
   dirty_(true),
   alpha_(1.0),
   agl_(false),
   mode_(DRAWMODE_2D_HORIZONTAL),
   refCoord_(0., 0., 0.),
   sphericalEarth_(true),
   elevAngle_(0.0)
{
  setHalfBeamWidth(5.0 * simCore::DEG2RAD);
  alphaUniform_ = getOrCreateStateSet()->getOrCreateUniform("alpha", osg::Uniform::FLOAT);
  alphaUniform_->set(alpha_);

  transform_ = new osg::MatrixTransform();
  addChild(transform_);
  updateOrientation_();

  init_();
}

Profile::~Profile()
{
}

void Profile::addProvider(ProfileDataProvider* provider)
{
  if (provider)
    getDataProvider_()->addProvider(provider);
}

const CompositeProfileProvider* Profile::getDataProvider() const
{
  return data_.get();
}

CompositeProfileProvider* Profile::getDataProvider_()
{
  return data_.get();
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
  return refCoord_.lat();
}

double Profile::getRefLon() const
{
  return refCoord_.lon();
}

double Profile::getRefAlt() const
{
  return refCoord_.alt();
}

void Profile::setRefCoord(double latRad, double lonRad, double alt)
{
  if (latRad != refCoord_.lat() || lonRad != refCoord_.lon() || alt != refCoord_.alt())
  {
    refCoord_.set(latRad, lonRad, alt);
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
    const double dt0 = -halfBeamWidth_ + M_PI_2;
    const double dt1 = halfBeamWidth_ + M_PI_2;
    cosTheta0_ = cos(dt0);
    sinTheta0_ = sin(dt0);
    cosTheta1_ = cos(dt1);
    sinTheta1_ = sin(dt1);

    dirty();
  }
}

void Profile::adjustSpherical_(osg::Vec3& v, const simCore::Vec3& tpSphereXYZ)
{
  simCore::Vec3 sphereXYZ;
  simCore::tangentPlane2Sphere(refCoord_, simCore::Vec3(v[0], v[1], v[2]), sphereXYZ, &tpSphereXYZ);
  const double alt = v3Length(sphereXYZ) - simCore::EARTH_RADIUS;
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
  verts_ = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
  values_ = new osg::FloatArray(osg::Array::BIND_PER_VERTEX);
  values_->setNormalize(false);
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
  const unsigned int startIndex = verts_->size();
  unsigned int heightIndex = data_->getHeightIndex(height_);
  // Error check the height index
  if (heightIndex == CompositeProfileProvider::INVALID_HEIGHT_INDEX || heightIndex > (data_->getNumHeights() - 1))
  {
    // getHeightIndex guarantees to return either error condition (which indicates invalidly defined profile), or a valid height index
    assert(0);
    return;
  }

  verts_->reserve(2 * numRanges);
  values_->reserve(2 * numRanges);

  simCore::Vec3 tpSphereXYZ;
  simCore::geodeticToSpherical(refCoord_.lat(), refCoord_.lon(), refCoord_.alt(), tpSphereXYZ);

  // init the flag that indicates whether this profile has seen valid data
  bool validDataStarted = false;
  for (unsigned int i = 0; i < numRanges; i++)
  {
    const double range = minRange + rangeStep * i;
    double height = height_;
    if (agl_ && !terrainHeights_.empty())
    {
      height = height_ + getTerrainHgt_(static_cast<float>(range));
      heightIndex = data_->getHeightIndex(height);
      // Error check the height index
      if (heightIndex == CompositeProfileProvider::INVALID_HEIGHT_INDEX  || heightIndex > (data_->getNumHeights() - 1))
      {
        // getHeightIndex guarantees to return either error condition (which indicates invalidly defined profile), or a valid height index
        assert(0);
        return;
      }
    }

    const double value = data_->getValueByIndex(heightIndex, i);
    if (!validDataStarted)
    {
      // values <= AREPS_GROUND_VALUE are sentinel values, not actual values. some profiles can have long stretch of no-data, especially at low range.
      if (value <= AREPS_GROUND_VALUE)
      {
        // ignore no-data values until valid data is received
        continue;
      }
      // once valid data has started, skipping no-data points might affect the triangle strip. in these cases, shaders will make those vertices transparent.
      validDataStarted = true;
    }

    // Left vert
    osg::Vec3 v0(range * cosTheta0_, range * sinTheta0_, height);
    // Right vert
    osg::Vec3 v1(range * cosTheta1_, range * sinTheta1_, height);

    if (sphericalEarth_)
    {
      adjustSpherical_(v0, tpSphereXYZ);
      adjustSpherical_(v1, tpSphereXYZ);
    }

    verts_->push_back(v1);
    verts_->push_back(v0);
    values_->push_back(value);
    values_->push_back(value);
  }

  osg::Geometry* geometry = new osg::Geometry();
  geometry->setUseVertexBufferObjects(true);
  geometry->setDataVariance(osg::Object::DYNAMIC);
  geometry->setVertexArray(verts_.get());
  geometry->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, values_.get());

  // GL_CULL_FACE is OFF because 2D Horizontal is a strip and not a 3D object
  geometry->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

  const unsigned int count = verts_->size() - startIndex;

  geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, startIndex, count));

  geode_->addDrawable(geometry);
}

// Used to tesselate the 2D Vertical with triangle strip
const void Profile::tesselate2DVert_(unsigned int numRanges, unsigned int numHeights, unsigned int startIndex, osg::ref_ptr<osg::FloatArray> values, osg::Geometry* geometry)
{
  for (unsigned int h = 0; h < numHeights - 1; ++h)
  {
    osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLE_STRIP);
    idx->reserve(2 * numRanges);

    // init the flag that indicates whether this profile has seen valid data
    bool validDataStarted = false;

    // Create row strips
    for (unsigned int r = 0; r < numRanges; ++r)
    {
      const unsigned int indexBottom = startIndex + (r * numHeights) + h;
      const unsigned int indexTop = indexBottom + 1;
      if (!validDataStarted)
      {
        const double valueBottom = values->at(indexBottom);
        const double valueTop = values->at(indexTop);
        // some profiles can have large patch of no-data at beginning
        if (valueBottom <= AREPS_GROUND_VALUE && valueTop <= AREPS_GROUND_VALUE)
        {
          // ignore no-data values until valid data is received
          continue;
        }
        validDataStarted = true;
      }

      idx->push_back(indexBottom);
      idx->push_back(indexTop);
    }
    // Add individual row primitive set
    geometry->addPrimitiveSet(idx);
  }
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
  simCore::geodeticToSpherical(refCoord_.lat(), refCoord_.lon(), refCoord_.alt(), tpSphereXYZ);

  // 2DVert draw mode can be combined with 2DHorz mode; cache the starting point for vertices relevant to this draw mode
  const unsigned int startIndex = verts_->size();
  const unsigned int numVerts = numHeights * numRanges;
  verts_->reserve(numVerts + startIndex);
  values_->reserve(numVerts + startIndex);

  for (unsigned int r = 0; r < numRanges; r++)
  {
    const double range = minRange + rangeStep * r;

    for (unsigned int h = 0; h < numHeights; h++)
    {
      const double height = minHeight + heightStep * h;
      osg::Vec3 v(0., range, height);

      if (sphericalEarth_)
      {
        adjustSpherical_(v, tpSphereXYZ);
      }

      verts_->push_back(v);

      const float value = data_->getValueByIndex(h, r);
      values_->push_back(value);
    }
  }

  osg::Geometry* geometry = new osg::Geometry();
  geometry->setUseVertexBufferObjects(true);
  geometry->setDataVariance(osg::Object::DYNAMIC);
  geometry->setVertexArray(verts_.get());

  geometry->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, values_.get());
  geometry->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);

  // Call to tesselate the 2D Vertical
  tesselate2DVert_(numRanges, numHeights, startIndex, values_, geometry);

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

  simCore::Vec3 tpSphereXYZ;
  simCore::geodeticToSpherical(refCoord_.lat(), refCoord_.lon(), refCoord_.alt(), tpSphereXYZ);

  const unsigned int heightIndexCount = maxHeightIndex - minHeightIndex + 1;
  const unsigned int numVerts = 2 * heightIndexCount * numRanges;
  verts_->reserve(numVerts);
  values_->reserve(numVerts);

  const unsigned int startIndex = verts_->size();
  for (unsigned int r = 0; r < numRanges; r++)
  {
    const double range = minRange + rangeStep * r;
    const double x0 = range * cosTheta0_;
    const double y0 = range * sinTheta0_;
    const double x1 = range * cosTheta1_;
    const double y1 = range * sinTheta1_;

    for (unsigned int h = minHeightIndex; h <= maxHeightIndex; h++)
    {
      const double height = minHeight + heightStep * h;
      //Left vert
      osg::Vec3 v0(x0, y0, height);
      //Right vert
      osg::Vec3 v1(x1, y1, height);

      if (sphericalEarth_)
      {
        adjustSpherical_(v0, tpSphereXYZ);
        adjustSpherical_(v1, tpSphereXYZ);
      }

      verts_->push_back(v0);
      verts_->push_back(v1);

      const double value = data_->getValueByIndex(h, r);
      values_->push_back(value);
      values_->push_back(value);
    }
  }

  osg::Geometry* geometry = new osg::Geometry();

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

      osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLE_STRIP);
      idx->reserve(14);
      // Wrap the voxel with a triangle strip
      // Back Bottom
      idx->push_back(v5); idx->push_back(v4);

      // Back to top
      idx->push_back(v6); idx->push_back(v7);

      // Top to left
      idx->push_back(v3); idx->push_back(v5);

      // Left to bottom
      idx->push_back(v1); idx->push_back(v4);

      // Bottom to right
      idx->push_back(v0); idx->push_back(v6);

      // Right to top
      idx->push_back(v2); idx->push_back(v3);

      // Top to front
      idx->push_back(v0); idx->push_back(v1);

      geometry->addPrimitiveSet(idx);
    }
  }

  geometry->setDataVariance(osg::Object::DYNAMIC);
  geometry->setVertexArray(verts_.get());
  geometry->setUseVertexBufferObjects(true);

  geometry->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, values_.get());

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

  // TODO: determine how to support spherical earth like other draw modes
  //simCore::Vec3 tpSphereXYZ;
  //simCore::geodeticToSpherical(refCoord_.lat(), refCoord_.lon(), refCoord_.alt(), tpSphereXYZ);

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

  int lt = 0; // Left side top of cap
  int lb = 0; // Left side bottom of cap
  int rt = 0; // Right side top of cap
  int rb = 0; // Right side bottom of cap

  // Calculate length of step for tessellation
  const double pieLength = simCore::sdkMin(maxRange, simVis::MAX_SEGMENT_LENGTH);
  const unsigned int numSegs = simCore::sdkMax(simVis::MIN_NUM_SEGMENTS, simCore::sdkMin(simVis::MAX_NUM_SEGMENTS, static_cast<unsigned int>(maxRange / pieLength)));
  const double maxRangeStep = maxRange / numSegs;
  double texStep = 1.0 / numSegs;

  std::vector<std::pair<int, int> > topVerts;
  topVerts.reserve(numSegs); // Top side vertex pairs

  std::vector<std::pair<int, int> > botVerts;
  botVerts.reserve(numSegs); // Bottom side vertex pairs

  const size_t numVerts = 2 + (numSegs * 2) + (numSegs * 2);
  osg::Vec2Array* tcoords = new osg::Vec2Array();
  tcoords->reserve(numVerts);
  verts_->reserve(numVerts);

  // The first two verts are the points at the start of the pie slice
  verts_->push_back(osg::Vec3(0, 0, minSampledHeight));  // 0
  tcoords->push_back(osg::Vec2(0, minT));

  verts_->push_back(osg::Vec3(0, 0, maxSampledHeight));  // 1
  tcoords->push_back(osg::Vec2(0, maxT));

  int vertCount = 2; // Current vertex count, to keep track

  { // Right side (drawn in opposite order of left side, required to make triangles face out the correct way)
    osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLE_STRIP);
    idx->reserve(2 + (2 * numSegs));
    // Add first two vertices
    idx->addElement(1);
    idx->addElement(0);

    // Add triangles, alternating between top and bottom vertices
    for (unsigned int i = 0; i < numSegs; ++i)
    {
      const double thisStep = maxRangeStep * (i + 1);
      const double thisTexStep = texStep * (i + 1);

      // Top vertex
      verts_->push_back(osg::Vec3(thisStep * cosTheta0_, thisStep * sinTheta0_, maxSampledHeight)); // 3
      tcoords->push_back(osg::Vec2(thisTexStep, maxT));
      idx->addElement(vertCount);
      topVerts.push_back(std::pair<int, int>(vertCount, 0));
      ++vertCount;

      // Bottom vertex
      verts_->push_back(osg::Vec3(thisStep * cosTheta0_, thisStep * sinTheta0_, minSampledHeight)); // 2
      tcoords->push_back(osg::Vec2(thisTexStep, minT));
      idx->addElement(vertCount);
      botVerts.push_back(std::pair<int, int>(vertCount, 0));
      ++vertCount;
    }

    // Create and add drawable to scene
    osg::Geometry* geometry = new osg::Geometry();
    geometry->setDataVariance(osg::Object::DYNAMIC);
    geometry->setVertexArray(verts_.get());
    geometry->setUseVertexBufferObjects(true);
    geometry->setTexCoordArray(0, tcoords);
    geometry->addPrimitiveSet(idx);
    geode_->addDrawable(geometry);

    // Save right end vertices for cap
    rb = vertCount - 1; // Right bottom
    rt = vertCount - 2; // Right top
  }

  { // Left side (drawn in opposite order of right side, required to make triangles face out the correct way)
    osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLE_STRIP);
    idx->reserve(2 + (2 * numSegs));
    // Add first two vertices
    idx->addElement(0);
    idx->addElement(1);

    // Add triangles, alternating between top and bottom vertices
    for (unsigned int i = 0; i < numSegs; ++i)
    {
      const double thisStep = maxRangeStep * (i + 1);
      const double thisTexStep = texStep * (i + 1);
      verts_->push_back(osg::Vec3(thisStep * cosTheta1_, thisStep * sinTheta1_, minSampledHeight)); // 2
      tcoords->push_back(osg::Vec2(thisTexStep, minT));
      idx->addElement(vertCount);
      botVerts[i].second = vertCount;
      ++vertCount;

      verts_->push_back(osg::Vec3(thisStep * cosTheta1_, thisStep * sinTheta1_, maxSampledHeight)); // 3
      tcoords->push_back(osg::Vec2(thisTexStep, maxT));
      idx->addElement(vertCount);
      topVerts[i].second = vertCount;
      ++vertCount;
    }

    // Create and add drawable to scene
    osg::Geometry* geometry = new osg::Geometry();
    geometry->setDataVariance(osg::Object::DYNAMIC);
    geometry->setVertexArray(verts_.get());
    geometry->setUseVertexBufferObjects(true);
    geometry->setTexCoordArray(0, tcoords);
    geometry->addPrimitiveSet(idx);
    geode_->addDrawable(geometry);

    // Save left end vertices for cap
    lb = vertCount - 2; // Left bottom
    lt = vertCount - 1; // Left top
  }

  { // Top side (drawn in opposite order of bottom side, required to make triangles face out the correct way)
    osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLE_STRIP);
    idx->reserve(3 + (2 * numSegs));
    // Add the first triangle
    idx->addElement(1); idx->addElement(topVerts[0].first); idx->addElement(topVerts[0].second);
    // Add the rest
    for (unsigned int i = 1; i < numSegs; ++i)
    {
      idx->addElement(topVerts[i].first);
      idx->addElement(topVerts[i].second);
    }

    // Create and add drawable to scene
    osg::Geometry* geometry = new osg::Geometry();
    geometry->setDataVariance(osg::Object::DYNAMIC);
    geometry->setVertexArray(verts_.get());
    geometry->setUseVertexBufferObjects(true);
    geometry->setTexCoordArray(0, tcoords);
    geometry->addPrimitiveSet(idx);
    geode_->addDrawable(geometry);
  }

  { // Bottom side (drawn in opposite order of top side, required to make triangles face out the correct way)
    osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLE_STRIP);
    idx->reserve(3 + (2 * numSegs));
    // Add the first triangle
    idx->addElement(0); idx->addElement(botVerts[0].second); idx->addElement(botVerts[0].first);
    // Add the rest
    for (unsigned int i = 1; i < numSegs; ++i)
    {
      idx->addElement(botVerts[i].second);
      idx->addElement(botVerts[i].first);
    }

    // Create and add drawable to scene
    osg::Geometry* geometry = new osg::Geometry();
    geometry->setDataVariance(osg::Object::DYNAMIC);
    geometry->setVertexArray(verts_.get());
    geometry->setUseVertexBufferObjects(true);
    geometry->setTexCoordArray(0, tcoords);
    geometry->addPrimitiveSet(idx);
    geode_->addDrawable(geometry);
  }

  { // Cap (end of the shape, the pie "crust")
    osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLE_STRIP);
    idx->addElement(lb);  idx->addElement(lt); idx->addElement(rb);  idx->addElement(rt);
    // Create and add drawable to scene
    osg::Geometry* geometry = new osg::Geometry();
    geometry->setDataVariance(osg::Object::DYNAMIC);
    geometry->setVertexArray(verts_.get());
    geometry->setUseVertexBufferObjects(true);
    geometry->setTexCoordArray(0, tcoords);
    geometry->addPrimitiveSet(idx);
    geode_->addDrawable(geometry);
  }

  // Only create the texture if it doesn't already exist, otherwise you can just reuse it
  if (texture_ == NULL)
  {
    texture_ = new osg::Texture2D(createImage_());
    texture_->setResizeNonPowerOfTwoHint(false);
    texture_->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    texture_->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
    simVis::fixTextureForGlCoreProfile(texture_.get());
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

  simCore::Vec3 tpSphereXYZ;
  simCore::geodeticToSpherical(refCoord_.lat(), refCoord_.lon(), refCoord_.alt(), tpSphereXYZ);

  const unsigned int numVerts = (maxHeightIndex - minHeightIndex + 1) * numRanges;
  verts_->reserve(numVerts);
  values_->reserve(numVerts);

  for (unsigned int r = 0; r < numRanges; r++)
  {
    const double range = minRange + rangeStep * r;

    for (unsigned int h = minHeightIndex; h <= maxHeightIndex; h++)
    {
      const double value = data_->getValueByIndex(h, r);
      // values <= AREPS_GROUND_VALUE are sentinel values, not actual values.
      if (value <= AREPS_GROUND_VALUE)
        continue;

      const double height = minHeight + heightStep * h;
      osg::Vec3 v(0, range, height);
      if (sphericalEarth_)
      {
        adjustSpherical_(v, tpSphereXYZ);
      }
      verts_->push_back(v);
      values_->push_back(value);
    }
  }

  osg::Geometry* geometry = new osg::Geometry();
  geometry->setDataVariance(osg::Object::DYNAMIC);
  geometry->setVertexArray(verts_.get());
  geometry->setUseVertexBufferObjects(true);

  geometry->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, values_.get());

  geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, verts_->size()));
  geode_->addDrawable(geometry);
  simVis::PointSize::setValues(geode_->getOrCreateStateSet(), 3.f, osg::StateAttribute::ON);
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

struct Profile::VoxelParameters
{
  VoxelParameters(simRF::CompositeProfileProvider& data)
  {
    minRange = data.getMinRange();
    rangeStep = data.getRangeStep();
    numRanges = data.getNumRanges();
    minHeight = data.getMinHeight();
    heightStep = data.getHeightStep();
    numHeights = data.getNumHeights();
  }

  bool isValid() const
  {
    if (numRanges < 2 || rangeStep <= 0.)
    {
      // if assert fails, check that init_ ensures that we have a valid provider
      assert(0);
      return false;
    }
    return true;
  }

  double minRange;
  double rangeStep;
  unsigned int numRanges;
  double minHeight;
  double heightStep;
  unsigned int numHeights;
};

const void Profile::buildVoxel_(const VoxelParameters& vParams, const simCore::Vec3& tpSphereXYZ, double heightRangeRatio, unsigned int rangeIndex, osg::Geometry* geometry)
{
  // determine range values and indices
  const unsigned int minRangeIndex = rangeIndex;
  const unsigned int maxRangeIndex = simCore::sdkMin(minRangeIndex + 1, vParams.numRanges - 1);
  if (minRangeIndex >= (vParams.numRanges - 1) || maxRangeIndex > vParams.numRanges)
    return;
  const double rNear = vParams.minRange + (vParams.rangeStep * minRangeIndex);
  const double rFar = rNear + vParams.rangeStep;


  // calculate ht at near range and elev
  double htValNearBottom = height_ + (rNear * heightRangeRatio);
  // find the nearest index for this calculated ht
  const unsigned int htIndexNearBottom = data_->getHeightIndex(htValNearBottom);
  if (htIndexNearBottom == CompositeProfileProvider::INVALID_HEIGHT_INDEX)
  {
    // Invalidly defined profile
    assert(0);
    return;
  }
  if (htIndexNearBottom >= vParams.numHeights)
    return;
  // revise to use the value at the index
  htValNearBottom = vParams.minHeight + (vParams.heightStep * htIndexNearBottom);


  // calculate ht at far range and elev
  double htValFarBottom = height_ + (rFar * heightRangeRatio);
  // find the nearest index for this calculated ht
  const unsigned int htIndexFarBottom = data_->getHeightIndex(htValFarBottom);
  if (htIndexFarBottom == CompositeProfileProvider::INVALID_HEIGHT_INDEX)
  {
    // Invalidly defined profile
    assert(0);
    return;
  }
  if (htIndexFarBottom >= vParams.numHeights)
    return;
  // revise to use the value at the index
  htValFarBottom = vParams.minHeight + (vParams.heightStep * htIndexFarBottom);

  // clamp near-top height
  const unsigned int htIndexNearTop = simCore::sdkMin(htIndexNearBottom + 1, vParams.numHeights - 1);
  const double htValNearTop = vParams.minHeight + vParams.heightStep * htIndexNearTop;


  // clamp far-top height
  const unsigned int htIndexFarTop = simCore::sdkMin(htIndexFarBottom + 1, vParams.numHeights - 1);
  const double htValFarTop = vParams.minHeight + vParams.heightStep * htIndexFarTop;


  // Bottom verts
  osg::Vec3 v0(rNear * cosTheta0_, rNear * sinTheta0_, htValNearBottom); // Near right
  osg::Vec3 v1(rNear * cosTheta1_, rNear * sinTheta1_, htValNearBottom); // Near left
  osg::Vec3 v2(rFar * cosTheta1_, rFar * sinTheta1_, htValFarBottom); // Far left
  osg::Vec3 v3(rFar * cosTheta0_, rFar * sinTheta0_, htValFarBottom); // Far right

  // Top verts
  osg::Vec3 v4(rNear * cosTheta0_, rNear * sinTheta0_, htValNearTop); // Near right
  osg::Vec3 v5(rNear * cosTheta1_, rNear * sinTheta1_, htValNearTop); // Near left
  osg::Vec3 v6(rFar * cosTheta1_, rFar * sinTheta1_, htValFarTop); // Far left
  osg::Vec3 v7(rFar * cosTheta0_, rFar * sinTheta0_, htValFarTop); // Far right

  if (sphericalEarth_)
  {
    adjustSpherical_(v0, tpSphereXYZ);
    adjustSpherical_(v1, tpSphereXYZ);
    adjustSpherical_(v2, tpSphereXYZ);
    adjustSpherical_(v3, tpSphereXYZ);
    adjustSpherical_(v4, tpSphereXYZ);
    adjustSpherical_(v5, tpSphereXYZ);
    adjustSpherical_(v6, tpSphereXYZ);
    adjustSpherical_(v7, tpSphereXYZ);
  }

  // TODO: design should allow for re-using 4 vertices and 4 values from previous range voxel


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
  double value = data_->getValueByIndex(htIndexNearBottom, minRangeIndex);
  values_->push_back(value);
  values_->push_back(value);

  //v2, v3
  value = data_->getValueByIndex(htIndexFarBottom, maxRangeIndex);
  values_->push_back(value);
  values_->push_back(value);

  //v4, v5
  value = data_->getValueByIndex(htIndexNearTop, minRangeIndex);
  values_->push_back(value);
  values_->push_back(value);

  //v6, v7
  value = data_->getValueByIndex(htIndexFarTop, maxRangeIndex);
  values_->push_back(value);
  values_->push_back(value);

  // Create a triangle strip set to wrap the voxel
  osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLE_STRIP);
  idx->reserve(14);
  // Back Bottom
  idx->push_back(i3); idx->push_back(i2);

  // Back to top
  idx->push_back(i7); idx->push_back(i6);

  // Top to left
  idx->push_back(i5); idx->push_back(i2);

  // Left to bottom
  idx->push_back(i1); idx->push_back(i3);

  // Bottom to right
  idx->push_back(i0); idx->push_back(i7);

  // Right to top
  idx->push_back(i4); idx->push_back(i5);

  // Top to front
  idx->push_back(i0); idx->push_back(i1);

  geometry->addPrimitiveSet(idx);
}

void Profile::initRAE_()
{
  assert(data_.valid() && data_->getActiveProvider() != NULL);

  const VoxelParameters vParams(*(data_.get()));
  if (!vParams.isValid())
    return;
  // using sin(elev) implies that range in the data is slant range; elev angle is asin(ht/slantRange)) or atan(ht/groundRange)
  const double heightRangeRatio = sin(elevAngle_);

  simCore::Vec3 tpSphereXYZ;
  simCore::geodeticToSpherical(refCoord_.lat(), refCoord_.lon(), refCoord_.alt(), tpSphereXYZ);

  const size_t numVoxels = (vParams.numRanges - 1);
  const size_t vertsPerVoxel = 8;
  verts_->reserve(numVoxels * vertsPerVoxel);
  values_->reserve(numVoxels * vertsPerVoxel);

  osg::Geometry* geometry = new osg::Geometry();

  // create an RAE visualization by using elev angle and range data to generate height
  for (unsigned int r = 0; r < (vParams.numRanges - 1); ++r)
  {
    // build voxel that spans from rangeIndex r to rangeIndex r+1
    buildVoxel_(vParams, tpSphereXYZ, heightRangeRatio, r, geometry);
  }

  geometry->setUseVertexBufferObjects(true);
  geometry->setDataVariance(osg::Object::DYNAMIC);
  geometry->setVertexArray(verts_.get());
  geometry->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, values_.get());
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

}
