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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osg/MatrixTransform"
#include "osg/Texture2D"
#include "osgEarth/GLUtils"
#include "osgEarth/ImageUtils"
#include "osgEarth/PointDrawable"
#include "osgEarth/NodeUtils"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Calculations.h"
#include "simCore/Calc/Interpolation.h"
#include "simVis/Constants.h"
#include "simVis/Utils.h"
#include "simVis/RFProp/CompositeProfileProvider.h"
#include "simVis/RFProp/Profile.h"

namespace simRF {

Profile::Profile(CompositeProfileProvider* data)
 : bearing_(0),
   halfBeamWidth_(0.0),
   data_(data),
   dirty_(false)
{
  setHalfBeamWidth(5.0 * simCore::DEG2RAD);
  updateOrientation_();
  init_();
}

Profile::~Profile()
{
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

double Profile::getHalfBeamWidth() const
{
  return halfBeamWidth_;
}

void Profile::setHalfBeamWidth(double halfBeamWidth)
{
  // ensure that halfBeamWidth has a reasonable value
  halfBeamWidth = osg::clampBetween(halfBeamWidth, -M_PI, M_PI);
  if (halfBeamWidth_ == halfBeamWidth)
    return;
  halfBeamWidth_ = halfBeamWidth;
  const double dt0 = -halfBeamWidth_ + M_PI_2;
  const double dt1 = halfBeamWidth_ + M_PI_2;
  cosTheta0_ = cos(dt0);
  sinTheta0_ = sin(dt0);
  cosTheta1_ = cos(dt1);
  sinTheta1_ = sin(dt1);

  dirty();
}

void Profile::setTerrainHeights(const std::map<float, float>& terrainHeights)
{
  terrainHeights_.clear();
  terrainHeights_ = terrainHeights;
  dirty();
}

void Profile::addProvider(ProfileDataProvider* provider)
{
  if (data_.valid() && provider)
    data_->addProvider(provider);
}

const CompositeProfileProvider* Profile::getDataProvider() const
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

void Profile::setProfileContext(std::shared_ptr<ProfileContext> profileContext)
{
  profileContext_ = profileContext;
  if (profileContext_)
  {
    // ensure that this profile matches the shared context
    setThresholdType(profileContext->type_);
    dirty();
  }
}

void Profile::setThresholdType(ProfileDataProvider::ThresholdType type)
{
  if (data_.valid())
    data_->setActiveProvider(type);
  // null the texture to force it to recreate
  texture_ = NULL;
  dirty();
}

void Profile::dirty()
{
  if (!dirty_)
  {
    ADJUST_UPDATE_TRAV_COUNT(this, 1);
    dirty_ = true;
  }
}

void Profile::traverse(osg::NodeVisitor& nv)
{
  if (nv.getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
  {
    if (dirty_)
      init_();
    ADJUST_UPDATE_TRAV_COUNT(this, -1);
  }
  osg::Group::traverse(nv);
}

void Profile::init_()
{
  // Remove all existing nodes
  removeChildren(0, getNumChildren());

  // Clear out the original values
  verts_ = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
  values_ = new osg::FloatArray(osg::Array::BIND_PER_VERTEX);
  values_->setNormalize(false);
  group_ = NULL;

  if (!profileContext_)
    return;

  if (profileContext_->mode_ != DRAWMODE_3D_TEXTURE)
    texture_ = NULL;

  // ensure that our provider is valid
  if (data_.valid() && data_->getActiveProvider() != NULL)
  {
    group_ = new osg::Group;
    switch (profileContext_->mode_)
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
    addChild(group_);
  }
  dirty_ = false;
}

void Profile::updateOrientation_()
{
  setMatrix(osg::Matrixd::rotate(bearing_, osg::Vec3d(0., 0., -1.)));
}

float Profile::getTerrainHgt_(float gndRng) const
{
  // initialize terrain hgt to default value in case interpolation fails
  float value = 0.f;
  simCore::linearInterpolate(terrainHeights_, gndRng, value);
  return value;
}

void Profile::adjustSpherical_(osg::Vec3& v, const simCore::Vec3& tpSphereXYZ)
{
  if (!profileContext_)
  {
    // dev error; can't init a profile without a profileContext_
    assert(0);
    return;
  }
  simCore::Vec3 sphereXYZ;
  simCore::tangentPlane2Sphere(profileContext_->refLLA_, simCore::Vec3(v[0], v[1], v[2]), sphereXYZ, &tpSphereXYZ);
  const double alt = v3Length(sphereXYZ) - simCore::EARTH_RADIUS;
  v.z() = v.z() - (alt - v.z()) + profileContext_->refLLA_.alt();
}

void Profile::init2DHoriz_()
{
  if (!data_.valid() || !data_->getActiveProvider() || !profileContext_)
  {
    // dev error; can't init a profile without valid active provider and profileContext_
    assert(0);
    return;
  }
  const double minRange = data_->getMinRange();
  const double rangeStep = data_->getRangeStep();
  const double numRanges = data_->getNumRanges();
  const unsigned int startIndex = verts_->size();
  unsigned int heightIndex = data_->getHeightIndex(profileContext_->heightM_);
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
  simCore::geodeticToSpherical(profileContext_->refLLA_.lat(), profileContext_->refLLA_.lon(), profileContext_->refLLA_.alt(), tpSphereXYZ);

  // init the flag that indicates whether this profile has seen valid data
  bool validDataStarted = false;
  for (unsigned int i = 0; i < numRanges; i++)
  {
    const double range = minRange + rangeStep * i;
    double height = profileContext_->heightM_;
    if (profileContext_->agl_ && !terrainHeights_.empty())
    {
      height += getTerrainHgt_(static_cast<float>(range));
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

    if (profileContext_->sphericalEarth_)
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

  group_->addChild(geometry);
}

// Used to tesselate the 2D Vertical with triangle strip
void Profile::tesselate2DVert_(unsigned int numRanges, unsigned int numHeights, unsigned int startIndex, osg::ref_ptr<osg::FloatArray> values, osg::Geometry* geometry) const
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
  if (!data_.valid() || !data_->getActiveProvider() || !profileContext_)
  {
    // dev error; can't init a profile without valid active provider and profileContext_
    assert(0);
    return;
  }
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
  simCore::geodeticToSpherical(profileContext_->refLLA_.lat(), profileContext_->refLLA_.lon(), profileContext_->refLLA_.alt(), tpSphereXYZ);

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

      if (profileContext_->sphericalEarth_)
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

  group_->addChild(geometry);
}

void Profile::init3D_()
{
  if (!data_.valid() || !data_->getActiveProvider() || !profileContext_)
  {
    // dev error; can't init a profile without valid active provider and profileContext_
    assert(0);
    return;
  }
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
  unsigned int minHeightIndex = data_->getHeightIndex(profileContext_->heightM_);
  // Error check the height index
  if (minHeightIndex == CompositeProfileProvider::INVALID_HEIGHT_INDEX)
  {
    // Invalidly defined profile
    assert(0);
    return;
  }

  minHeightIndex = osg::clampBetween(minHeightIndex, 0u, numHeights - 1);
  unsigned int maxHeightIndex = simCore::sdkMin(minHeightIndex + profileContext_->displayThickness_, numHeights - 1);
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
  simCore::geodeticToSpherical(profileContext_->refLLA_.lat(), profileContext_->refLLA_.lon(), profileContext_->refLLA_.alt(), tpSphereXYZ);

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

      if (profileContext_->sphericalEarth_)
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

  osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLES);
  const size_t idxSize = 36 * (maxHeightIndex - minHeightIndex) * (numRanges - 1);
  idx->reserve(idxSize);

  //Now build the indices that will actually be rendered
  for (unsigned int r = 0; r < numRanges - 1; r++)
  {
    const unsigned int nextR = r + 1;
    for (unsigned int h = minHeightIndex; h < maxHeightIndex; h++)
    {
      // 36 indices / cube
      //Compute the indices of the 8 corners of the cube
      const unsigned int v0 = startIndex + r * heightIndexCount * 2 + (h - minHeightIndex) * 2;  // front LR
      const unsigned int v1 = v0 + 1; // front LL
      const unsigned int v2 = v1 + 1; // front UR
      const unsigned int v3 = v2 + 1; // front UL

      const unsigned int v4 = startIndex + nextR * heightIndexCount * 2 + (h - minHeightIndex) * 2; // back LR
      const unsigned int v5 = v4 + 1; // back LL
      const unsigned int v6 = v5 + 1; // back UR
      const unsigned int v7 = v6 + 1; // back UL

      // Front face
      idx->push_back(v1); idx->push_back(v0); idx->push_back(v3);
      idx->push_back(v0); idx->push_back(v2); idx->push_back(v3);

      // Back face
      idx->push_back(v6); idx->push_back(v4); idx->push_back(v5);
      idx->push_back(v7); idx->push_back(v6); idx->push_back(v5);

      // Top face
      idx->push_back(v3); idx->push_back(v2); idx->push_back(v7);
      idx->push_back(v2); idx->push_back(v6); idx->push_back(v7);

      // Bottom face
      idx->push_back(v1); idx->push_back(v5); idx->push_back(v4);
      idx->push_back(v1); idx->push_back(v4); idx->push_back(v0);

      // Left face
      idx->push_back(v5); idx->push_back(v1); idx->push_back(v7);
      idx->push_back(v1); idx->push_back(v3); idx->push_back(v7);

      // Right face
      idx->push_back(v0); idx->push_back(v4); idx->push_back(v6);
      idx->push_back(v0); idx->push_back(v6); idx->push_back(v2);
    }
  }
  // assertion fail means algorithm changed without correction to the reserve()
  assert(idxSize == idx->size());
  geometry->addPrimitiveSet(idx);

  geometry->setDataVariance(osg::Object::DYNAMIC);
  geometry->setVertexArray(verts_.get());
  geometry->setUseVertexBufferObjects(true);

  geometry->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, values_.get());

  group_->addChild(geometry);
}

void Profile::init3DTexture_()
{
  if (!data_.valid() || !data_->getActiveProvider() || !profileContext_)
  {
    // dev error; can't init a profile without valid active provider and profileContext_
    assert(0);
    return;
  }
  const double maxRange = data_->getMaxRange();

  const double minHeight = data_->getMinHeight();
  const double maxHeight = data_->getMaxHeight();
  const double heightStep = data_->getHeightStep();
  const unsigned int numHeights = data_->getNumHeights();
  // if assert fails, check that init_ ensures that we have a valid provider
  assert(numHeights > 0);

  //Build a 3D voxel representation of the profile.  The minimum height is specified by the height_ setting and the maximum height is the height_ + the display thickness
  unsigned int minHeightIndex = data_->getHeightIndex(profileContext_->heightM_);
  // Error check the height index
  if (minHeightIndex == CompositeProfileProvider::INVALID_HEIGHT_INDEX)
  {
    // Invalidly defined profile
    assert(0);
    return;
  }

  minHeightIndex = osg::clampBetween(minHeightIndex, 0u, numHeights - 1);
  unsigned int maxHeightIndex = simCore::sdkMin(minHeightIndex + profileContext_->displayThickness_, numHeights - 1);

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
    group_->addChild(geometry);

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
    group_->addChild(geometry);

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
    group_->addChild(geometry);
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
    group_->addChild(geometry);
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
    group_->addChild(geometry);
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
  group_->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture_);
}

void Profile::init3DPoints_()
{
  if (!data_.valid() || !data_->getActiveProvider() || !profileContext_)
  {
    // dev error; can't init a profile without valid active provider and profileContext_
    assert(0);
    return;
  }
  const double minRange = data_->getMinRange();
  const double rangeStep = data_->getRangeStep();
  const unsigned int numRanges = data_->getNumRanges();

  const double minHeight = data_->getMinHeight();
  const double heightStep = data_->getHeightStep();
  const unsigned int numHeights = data_->getNumHeights();
  // if assert fails, check that init_ ensures that we have a valid provider
  assert(numHeights > 0);

  //Build a 3D voxel representation of the profile.  The minimum height is specified by the height_ setting and the maximum height is the height_ + the display thickness
  unsigned int minHeightIndex = data_->getHeightIndex(profileContext_->heightM_);
  // Error check the height index
  if (minHeightIndex == CompositeProfileProvider::INVALID_HEIGHT_INDEX)
  {
    // Invalidly defined profile
    assert(0);
    return;
  }

  minHeightIndex = osg::clampBetween(minHeightIndex, 0u, numHeights - 1);
  unsigned int maxHeightIndex = simCore::sdkMin(minHeightIndex + profileContext_->displayThickness_, numHeights - 1);
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
  simCore::geodeticToSpherical(profileContext_->refLLA_.lat(), profileContext_->refLLA_.lon(), profileContext_->refLLA_.alt(), tpSphereXYZ);

  const unsigned int numVerts = (maxHeightIndex - minHeightIndex + 1) * numRanges;
  values_->reserve(numVerts);

  osgEarth::PointDrawable* points = new osgEarth::PointDrawable();
  points->setDataVariance(osg::Object::DYNAMIC);
  points->reserve(numVerts);

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
      if (profileContext_->sphericalEarth_)
      {
        adjustSpherical_(v, tpSphereXYZ);
      }
      points->pushVertex(v);
      values_->push_back(value);
    }
  }
  points->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, values_.get());
  points->finish();

  group_->addChild(points);
  osgEarth::GLUtils::setPointSize(points->getOrCreateStateSet(), 3.f, osg::StateAttribute::ON);
}

osg::Image* Profile::createImage_()
{
  if (!data_.valid() || !data_->getActiveProvider())
  {
    // dev error; can't init a profile without valid active provider
    assert(0);
    return NULL;
  }
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

//----------------------------------------------------------------------------

// VoxelProcessor for data provided as RAH, with height values in the height data structures of the profile provider
class Profile::RahVoxelProcessor : public Profile::VoxelProcessor
{
public:
  RahVoxelProcessor(const simRF::CompositeProfileProvider& data, double height)
    : data_(data)
  {
    minRange_ = data.getMinRange();
    rangeStep_ = data.getRangeStep();
    numRanges_ = data.getNumRanges();
    minHeight_ = data.getMinHeight();
    heightStep_ = data.getHeightStep();
    numHeights_ = data.getNumHeights();
    clearIndexCache();

    // interpret user-selected height_ as height at near range, determine elev angle to that height, use that elev angle to draw rae.
    const double rangeToUse = (minRange_ > 0) ? minRange_ : minRange_ + rangeStep_;
    heightRangeRatio_ = height / rangeToUse;
  }

  virtual ~RahVoxelProcessor()
  {
  }

  virtual bool isValid() const
  {
    if (numRanges_ < 2 || rangeStep_ <= 0.)
    {
      // if assert fails, check that init_ ensures that we have a valid provider
      assert(0);
      return false;
    }
    return true;
  }

  virtual int calculateVoxel(unsigned int rangeIndex, VoxelRange& voxelRange, VoxelHeight& nearHeight, VoxelHeight& farHeight) const
  {
    if (rangeIndex >= (numRanges_ - 1))
    {
      // developer error - check caller
      assert(0);
      return -1;
    }
    voxelRange.indexNear = rangeIndex;
    voxelRange.indexFar = simCore::sdkMin(voxelRange.indexNear + 1, numRanges_ - 1);
    voxelRange.valNear = minRange_ + (rangeStep_ * voxelRange.indexNear);
    voxelRange.valFar = voxelRange.valNear + rangeStep_;

    const int rvNear = calculateVoxelHeight_(voxelRange.valNear, nearHeight);
    if (rvNear < 0)
      return rvNear;
    const int rvFar = calculateVoxelHeight_(voxelRange.valFar, farHeight);
    if (rvFar < 0)
      return rvFar;
    return rvNear + rvFar;
  }

  virtual void setIndexCache(unsigned int i2, unsigned int i3, unsigned int i6, unsigned int i7)
  {
    if (i2 == std::numeric_limits<unsigned int>::max() ||
      i3 == std::numeric_limits<unsigned int>::max() ||
      i6 == std::numeric_limits<unsigned int>::max() ||
      i7 == std::numeric_limits<unsigned int>::max())
    {
      // developer error - indices must be valid
      assert(0);
      cachedIndicesAreValid_ = false;
      return;
    }

    cache_.i2 = i2;
    cache_.i3 = i3;
    cache_.i6 = i6;
    cache_.i7 = i7;
    cachedIndicesAreValid_ = true;
  }

  virtual void clearIndexCache()
  {
    cachedIndicesAreValid_ = false;
    cache_.i2 = std::numeric_limits<unsigned int>::max();
    cache_.i3 = std::numeric_limits<unsigned int>::max();
    cache_.i6 = std::numeric_limits<unsigned int>::max();
    cache_.i7 = std::numeric_limits<unsigned int>::max();
  }

  virtual int indexCache(VoxelIndexCache& cache) const
  {
    if (!cachedIndicesAreValid_)
      return 1;
    // setIndexCache guarantees these assertions
    assert(cache_.i2 != std::numeric_limits<unsigned int>::max());
    assert(cache_.i3 != std::numeric_limits<unsigned int>::max());
    assert(cache_.i6 != std::numeric_limits<unsigned int>::max());
    assert(cache_.i7 != std::numeric_limits<unsigned int>::max());
    cache = cache_;
    return 0;
  }

private:
  int calculateVoxelHeight_(double rangeVal, VoxelHeight& voxelHeight) const
  {
    // calculate ht at near range and elev
    voxelHeight.valBottom = (rangeVal * heightRangeRatio_);
    // find the nearest index for this calculated ht
    voxelHeight.indexBottom = data_.getHeightIndex(voxelHeight.valBottom);
    if (voxelHeight.indexBottom == CompositeProfileProvider::INVALID_HEIGHT_INDEX || voxelHeight.indexBottom >= numHeights_)
    {
      // Invalidly defined profile
      assert(0);
      return -1;
    }
    voxelHeight.valBottom = minHeight_ + (heightStep_ * voxelHeight.indexBottom);
    voxelHeight.indexTop = simCore::sdkMin(voxelHeight.indexBottom + 1, numHeights_ - 1);
    voxelHeight.valTop = minHeight_ + (heightStep_ * voxelHeight.indexTop);

    // return value indicates whether this voxel has reached the top of the data
    return (voxelHeight.indexTop == (numHeights_ - 1)) ? 1 : 0;
  }

protected:
  const simRF::CompositeProfileProvider& data_;
  double heightRangeRatio_;
  double minRange_;
  double rangeStep_;
  unsigned int numRanges_;
  double minHeight_;
  double heightStep_;
  unsigned int numHeights_;
  VoxelIndexCache cache_;
  bool cachedIndicesAreValid_;
};

//----------------------------------------------------------------------------

// VoxelProcessor for data provided as RAE, with elevation values in the height data structures of the profile provider
class Profile::RaeVoxelProcessor : public Profile::RahVoxelProcessor
{
public:
  RaeVoxelProcessor(const simRF::CompositeProfileProvider& data, double height)
    : Profile::RahVoxelProcessor(data, height)
  {
    const double minEl = data.getMinHeight();
    const double elStep = data.getHeightStep();

    // user's height selection is actually an elevation angle
    double elValBottom = height;
    // nearest elev index to the elev selected by the user
    elIndexBottom_ = data.getHeightIndex(elValBottom);
    // reset to el value at that index
    elValBottom = minEl + elStep * elIndexBottom_;
    // note that sin here implies range is slant range; if ground range, then tan() is required
    heightRangeRatio_ = sin(elValBottom * simCore::DEG2RAD);

    elIndexTop_ = simCore::sdkMin(elIndexBottom_ + 1, numHeights_ - 1);
    const double elValTop = minEl + elStep * elIndexTop_;
    // note that sin here implies range is slant range; if ground range, then tan() is required
    heightRangeRatioTop_ = sin(elValTop * simCore::DEG2RAD);
  }

  virtual ~RaeVoxelProcessor()
  {
  }

  virtual int calculateVoxel(unsigned int rangeIndex, VoxelRange& voxelRange, VoxelHeight& nearHeight, VoxelHeight& farHeight) const
  {
    if (rangeIndex >= (numRanges_ - 1))
    {
      assert(0);
      return -1;
    }
    voxelRange.indexNear = rangeIndex;
    voxelRange.indexFar = simCore::sdkMin(voxelRange.indexNear + 1, numRanges_ - 1);
    voxelRange.valNear = minRange_ + (rangeStep_ * voxelRange.indexNear);
    voxelRange.valFar = voxelRange.valNear + rangeStep_;

    // calculate ht at near range and elevIndex
    nearHeight.valBottom = (voxelRange.valNear * heightRangeRatio_);
    nearHeight.indexBottom = elIndexBottom_;
    // calculate ht at far range and elev
    farHeight.valBottom = (voxelRange.valFar * heightRangeRatio_);
    farHeight.indexBottom = elIndexBottom_;

    // calculate ht at near range and elevIndex
    nearHeight.valTop = (voxelRange.valNear * heightRangeRatioTop_);
    nearHeight.indexTop = elIndexTop_;
    // calculate ht at far range and elev
    farHeight.valTop = (voxelRange.valFar * heightRangeRatioTop_);
    farHeight.indexTop = elIndexTop_;
    return 0;
  }

protected:
  unsigned int elIndexBottom_;
  unsigned int elIndexTop_;
  double heightRangeRatioTop_;
};

//----------------------------------------------------------------------------

int Profile::buildVoxel_(VoxelProcessor& vProcessor, const simCore::Vec3& tpSphereXYZ, unsigned int rangeIndex, osg::Geometry* geometry)
{
  VoxelProcessor::VoxelRange voxelRange;
  VoxelProcessor::VoxelHeight nearVoxelHeight;
  VoxelProcessor::VoxelHeight farVoxelHeight;
  const int rv = vProcessor.calculateVoxel(rangeIndex, voxelRange, nearVoxelHeight, farVoxelHeight);
  if (rv < 0)
  {
    vProcessor.clearIndexCache();
    return 1;
  }
  // rv > 0 indicates: either near or far edge of this voxel is drawn at max height, stop drawing successive voxels.

  // determine if we have valid cached indices to optimize this voxel
  VoxelProcessor::VoxelIndexCache indexCache;
  const bool usingCachedIndices = (0 == vProcessor.indexCache(indexCache));

  // process values
  //v0, v1
  const double value01 = usingCachedIndices ? (values_->asVector())[indexCache.i2] : data_->getValueByIndex(nearVoxelHeight.indexBottom, voxelRange.indexNear);
  //v2, v3
  const double value23 = data_->getValueByIndex(farVoxelHeight.indexBottom, voxelRange.indexFar);
  //v4, v5
  const double value45 = usingCachedIndices ? (values_->asVector())[indexCache.i6] : data_->getValueByIndex(nearVoxelHeight.indexTop, voxelRange.indexNear);
  //v6, v7
  const double value67 = data_->getValueByIndex(farVoxelHeight.indexTop, voxelRange.indexFar);

  if (value01 <= AREPS_GROUND_VALUE && value23 <= AREPS_GROUND_VALUE && value45 <= AREPS_GROUND_VALUE && value67 <= AREPS_GROUND_VALUE)
  {
    // voxel has no data
    vProcessor.clearIndexCache();
    return rv;
  }

  unsigned int startIndex = verts_->size();

  if (!usingCachedIndices)
  {
    //v0, v1
    values_->push_back(value01);
    values_->push_back(value01);
    osg::Vec3 v0(voxelRange.valNear * cosTheta0_, voxelRange.valNear * sinTheta0_, nearVoxelHeight.valBottom); // Near right
    osg::Vec3 v1(voxelRange.valNear * cosTheta1_, voxelRange.valNear * sinTheta1_, nearVoxelHeight.valBottom); // Near left
    if (profileContext_->sphericalEarth_)
    {
      adjustSpherical_(v0, tpSphereXYZ);
      adjustSpherical_(v1, tpSphereXYZ);
    }
    verts_->push_back(v0);
    verts_->push_back(v1);
  }
  const unsigned int i0 = usingCachedIndices ? indexCache.i3 : startIndex++;
  const unsigned int i1 = usingCachedIndices ? indexCache.i2 : startIndex++;


  //v2, v3
  values_->push_back(value23);
  values_->push_back(value23);
  osg::Vec3 v2(voxelRange.valFar * cosTheta1_, voxelRange.valFar * sinTheta1_, farVoxelHeight.valBottom); // Far left
  osg::Vec3 v3(voxelRange.valFar * cosTheta0_, voxelRange.valFar * sinTheta0_, farVoxelHeight.valBottom); // Far right
  if (profileContext_->sphericalEarth_)
  {
    adjustSpherical_(v2, tpSphereXYZ);
    adjustSpherical_(v3, tpSphereXYZ);
  }
  verts_->push_back(v2);
  verts_->push_back(v3);
  const unsigned int i2 = startIndex++;
  const unsigned int i3 = startIndex++;


  if (!usingCachedIndices)
  {
    //v4, v5
    values_->push_back(value45);
    values_->push_back(value45);
    osg::Vec3 v4(voxelRange.valNear * cosTheta0_, voxelRange.valNear * sinTheta0_, nearVoxelHeight.valTop); // Near right
    osg::Vec3 v5(voxelRange.valNear * cosTheta1_, voxelRange.valNear * sinTheta1_, nearVoxelHeight.valTop); // Near left
    if (profileContext_->sphericalEarth_)
    {
      adjustSpherical_(v4, tpSphereXYZ);
      adjustSpherical_(v5, tpSphereXYZ);
    }
    verts_->push_back(v4);
    verts_->push_back(v5);
  }
  const unsigned int i4 = usingCachedIndices ? indexCache.i7 : startIndex++;
  const unsigned int i5 = usingCachedIndices ? indexCache.i6 : startIndex++;


  //v6, v7
  values_->push_back(value67);
  values_->push_back(value67);
  osg::Vec3 v6(voxelRange.valFar * cosTheta1_, voxelRange.valFar * sinTheta1_, farVoxelHeight.valTop); // Far left
  osg::Vec3 v7(voxelRange.valFar * cosTheta0_, voxelRange.valFar * sinTheta0_, farVoxelHeight.valTop); // Far right
  if (profileContext_->sphericalEarth_)
  {
    adjustSpherical_(v6, tpSphereXYZ);
    adjustSpherical_(v7, tpSphereXYZ);
  }
  verts_->push_back(v6);
  verts_->push_back(v7);
  const unsigned int i6 = startIndex++;
  const unsigned int i7 = startIndex++;


  // Create a triangle strip set to wrap the voxel
  osg::DrawElementsUInt* idx = new osg::DrawElementsUInt(GL_TRIANGLE_STRIP);
  idx->reserve(14);
  // Back Bottom
  idx->push_back(i3);
  idx->push_back(i2);

  // Back to top
  idx->push_back(i7);
  idx->push_back(i6);

  // Top to left
  idx->push_back(i5);
  idx->push_back(i2);

  // Left to bottom
  idx->push_back(i1);
  idx->push_back(i3);

  // Bottom to right
  idx->push_back(i0);
  idx->push_back(i7);

  // Right to top
  idx->push_back(i4);
  idx->push_back(i5);

  // Top to front
  idx->push_back(i0);
  idx->push_back(i1);

  geometry->addPrimitiveSet(idx);

  // cache the far indices for next voxel segment
  vProcessor.setIndexCache(i2, i3, i6, i7);
  return rv;
}

void Profile::initRAE_()
{
  if (!data_.valid() || !data_->getActiveProvider() || !profileContext_)
  {
    // dev error; can't init a profile without valid active provider and profileContext_
    assert(0);
    return;
  }
  RahVoxelProcessor vProcessor(*(data_.get()), profileContext_->heightM_);
  if (!vProcessor.isValid())
    return;

  simCore::Vec3 tpSphereXYZ;
  simCore::geodeticToSpherical(profileContext_->refLLA_.lat(), profileContext_->refLLA_.lon(), profileContext_->refLLA_.alt(), tpSphereXYZ);

  const unsigned int numRanges = data_->getNumRanges();
  const size_t numVoxels = (numRanges - 1);
  const size_t vertsPerVoxel = 8;
  verts_->reserve(numVoxels * vertsPerVoxel);
  values_->reserve(numVoxels * vertsPerVoxel);

  osg::Geometry* geometry = new osg::Geometry();

  // create an RAE visualization by using elev angle and range data to generate height
  for (unsigned int r = 0; r < (numRanges - 1); ++r)
  {
    // build voxel that spans from rangeIndex r to rangeIndex r+1
    const int rv = buildVoxel_(vProcessor, tpSphereXYZ, r, geometry);
    if (rv != 0)
      break;
  }

  geometry->setUseVertexBufferObjects(true);
  geometry->setDataVariance(osg::Object::DYNAMIC);
  geometry->setVertexArray(verts_.get());
  geometry->setVertexAttribArray(osg::Drawable::ATTRIBUTE_6, values_.get());
  group_->addChild(geometry);
}

}
