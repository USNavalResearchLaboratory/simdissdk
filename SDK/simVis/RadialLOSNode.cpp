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
#include "simVis/RadialLOSNode.h"
#include "simVis/Utils.h"
#include "simVis/Constants.h"
#include "simNotify/Notify.h"
#include "osgEarth/DrapeableNode"
#include "osgEarth/Terrain"
#include "osg/Depth"
#include "osg/Point"
#include <cassert>

#define LC "[RadialLOSNode] "

using namespace simVis;

//----------------------------------------------------------------------------

RadialLOSNode::RadialLOSNode(osgEarth::MapNode* mapNode) :
GeoPositionNode(mapNode),
visibleColor_(0.0f, 1.0f, 0.0f, 0.5f),
obstructedColor_(1.0f, 0.0f, 0.0f, 0.5f),
samplePointColor_(1.0f, 1.0f, 1.0f, 1.0f),
active_(false)
{
  callbackHook_ = new TerrainCallbackHook(this);

  geode_ = new osg::Geode();

  osg::StateSet* stateSet = geode_->getOrCreateStateSet();
  stateSet->setMode(GL_BLEND,    1);
  simVis::setLighting(stateSet, 0);
  stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

  osgEarth::DrapeableNode* drapeable = new osgEarth::DrapeableNode();
  getPositionAttitudeTransform()->addChild(drapeable);
  drapeable->addChild(geode_);

  mapNode->getTerrain()->addTerrainCallback(callbackHook_);
}


void RadialLOSNode::setMapNode(osgEarth::MapNode* mapNode)
{
  osgEarth::MapNode* oldMap = getMapNode();
  if (mapNode == oldMap)
    return;

  oldMap->getTerrain()->removeTerrainCallback(callbackHook_);
  mapNode->getTerrain()->addTerrainCallback(callbackHook_);

  GeoPositionNode::setMapNode(mapNode);

  // re-apply the position
  setCoordinate(coord_);
}


bool RadialLOSNode::setCoordinate(const simCore::Coordinate& coord)
{
  if (!getMapNode())
    return false;

  // convert it to a GeoPoint:
  osgEarth::GeoPoint point;
  if (!convertCoordToGeoPoint(coord, point, getMapNode()->getMapSRS()))
    return false;

  // update the position of the annotation:
  setPosition(point);

  // update the LOS model and recompute it:
  if (los_.compute(getMapNode(), coord))
  {
    refreshGeometry_();
  }

  coord_ = coord;
  bound_ = osgEarth::GeoCircle(point, los_.getMaxRange().as(osgEarth::Units::METERS));

  return true;
}


void RadialLOSNode::setDataModel(const RadialLOS& los)
{
  if (!getMapNode())
    return;

  RadialLOS newLOS = los;
  if (newLOS.compute(getMapNode(), coord_))
  {
    los_ = newLOS;
    refreshGeometry_();
    losPrevious_ = los_;
  }
}

void RadialLOSNode::setMaxRange(const Distance& value)
{
  los_.setMaxRange(value);
  los_.compute(getMapNode(), coord_);
}

void RadialLOSNode::setCentralAzimuth(const Angle& value)
{
  los_.setCentralAzimuth(value);
  los_.compute(getMapNode(), coord_);
}

void RadialLOSNode::setFieldOfView(const Angle& value)
{
  los_.setFieldOfView(value);
  los_.compute(getMapNode(), coord_);
}

void RadialLOSNode::setRangeResolution(const Distance& value)
{
  los_.setRangeResolution(value);
  los_.compute(getMapNode(), coord_);
}

void RadialLOSNode::setAzimuthalResolution(const Angle& value)
{
  los_.setAzimuthalResolution(value);
  los_.compute(getMapNode(), coord_);
}

void RadialLOSNode::updateDataModel(const osgEarth::GeoExtent& extent,
                                    osg::Node*                 patch)
{
  if (getMapNode())
  {
    osgEarth::GeoCircle circle = extent.computeBoundingGeoCircle();
    if (bound_.intersects(circle))
    {
      if (los_.update(getMapNode(), extent, patch))
      {
        refreshGeometry_();
      }
    }
  }
}


void RadialLOSNode::setVisibleColor(const osg::Vec4& value)
{
  if (value != visibleColor_)
  {
    visibleColor_ = value;
    refreshGeometry_();
  }
}


void RadialLOSNode::setObstructedColor(const osg::Vec4& value)
{
  if (value != obstructedColor_)
  {
    obstructedColor_ = value;
    refreshGeometry_();
  }
}


void RadialLOSNode::setSamplePointColor(const osg::Vec4& value)
{
  if (value != samplePointColor_)
  {
    samplePointColor_ = value;
    refreshGeometry_();
  }
}

void RadialLOSNode::setActive(bool active)
{
  if (active != active_)
  {
    active_ = active;
    refreshGeometry_();
  }
}

void RadialLOSNode::refreshGeometry_()
{
  this->dirtyBound();

  // check for a change that requires a complete rebuild.
  bool rebuildGeometry =
    geode_->getNumDrawables() == 0 ||
    !losPrevious_.isSet()          ||
    los_.getRadials().size()      != losPrevious_->getRadials().size() ||
    los_.getNumSamplesPerRadial() != losPrevious_->getNumSamplesPerRadial();

  // check whether we need to re-position the verts:
  bool updateVertexPositions =
    !rebuildGeometry &&
    losPrevious_.isSet() &&
    los_.getRadials().size() > 0 && (
      los_.getCentralAzimuth() != losPrevious_->getCentralAzimuth() ||
      los_.getFieldOfView() != losPrevious_->getFieldOfView() ||
      los_.getAzimuthalResolution() != losPrevious_->getAzimuthalResolution() ||
      los_.getMaxRange() != losPrevious_->getMaxRange() ||
      los_.getRangeResolution() != losPrevious_->getRangeResolution());

  const RadialLOS::RadialVector& radials = los_.getRadials();
  unsigned int samplesPerRadial = los_.getNumSamplesPerRadial();

  // remove all geometry for an empty set or inactive node.
  if (radials.size() == 0 || samplesPerRadial == 0 || !active_)
  {
    geode_->removeDrawables(0, geode_->getNumDrawables());
    return;
  }

  //const RadialLOS::Radial& r0 = radials.front();
  unsigned int numVerts = 1u + radials.size() * samplesPerRadial;

  osg::Vec3Array*        verts  = NULL;
  osg::Vec4Array*        colors = NULL;
  osg::DrawElementsUInt* tris   = NULL;
  osg::DrawElementsUInt* points = NULL;

  if (rebuildGeometry)
  {
    geode_->removeDrawables(0, geode_->getNumDrawables());

    // a state set that will render the geometries in order,
    osg::StateSet* stateSet = new osg::StateSet();
    stateSet->setRenderBinDetails(0, BIN_TRAVERSAL_ORDER_SIMSDK);
    stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::LEQUAL, 0.0, 1.0, false), osg::StateAttribute::ON);
    stateSet->setAttributeAndModes(new osg::Point(2.0f), osg::StateAttribute::ON);

    // set up and pre-allocate the geometry arrays:
    osg::Geometry* geom = new osg::Geometry();
    geom->setStateSet(stateSet);
    geom->setDataVariance(osg::Object::DYNAMIC);
    geom->setUseVertexBufferObjects(true);

    verts = new osg::Vec3Array(numVerts);
    geom->setVertexArray(verts);
    (*verts)[0].set(0, 0, 0);

    colors = new osg::Vec4Array(numVerts);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    tris = new osg::DrawElementsUInt(GL_TRIANGLES);
    geom->addPrimitiveSet(tris);

    geode_->addDrawable(geom);


    // the points overlay geometry.
    osg::Geometry* pointsGeom = new osg::Geometry();
    pointsGeom->setStateSet(stateSet);
    pointsGeom->setDataVariance(osg::Object::DYNAMIC);
    pointsGeom->setUseVertexBufferObjects(true);

    // shares the same vert array
    pointsGeom->setVertexArray(verts);

    osg::Vec4Array* pointsColors = new osg::Vec4Array(1);
    (*pointsColors)[0] = samplePointColor_;
    pointsGeom->setColorArray(pointsColors);
    pointsGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

    points = new osg::DrawElementsUInt(GL_POINTS);
    pointsGeom->addPrimitiveSet(points);

    geode_->addDrawable(pointsGeom);
  }
  else
  {
    osg::Geometry* geom = geode_->getDrawable(0)->asGeometry();

    if (updateVertexPositions)
    {
      verts = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
      verts->dirty();
    }

    // prepare to update the triangle colors:
    colors = dynamic_cast<osg::Vec4Array*>(geom->getColorArray());
    colors->dirty();

    // update the sample point color:
    osg::Geometry* pointsGeom = geode_->getDrawable(1)->asGeometry();
    osg::Vec4Array* samplePointColors = dynamic_cast<osg::Vec4Array*>(pointsGeom->getColorArray());
    (*samplePointColors)[0] = samplePointColor_;
    samplePointColors->dirty();
  }

  // go through the radials and generate verts and colors.
  unsigned int vertIndex = 1; // skip the center coord which never changes

  (*colors)[0] = visibleColor_;

  for (unsigned int radialIndex = 0; radialIndex < radials.size(); ++radialIndex)
  {
    const RadialLOS::Radial& radial = radials[radialIndex];

    double x = 0.0, y = 0.0;
    if (rebuildGeometry || updateVertexPositions)
    {
      x = sin(radial.azim_rad_);
      y = cos(radial.azim_rad_);
    }

    for (unsigned int sampleIndex = 0; sampleIndex < samplesPerRadial; ++sampleIndex)
    {
      const RadialLOS::Sample& sample = radial.samples_[sampleIndex];

      double range = sample.range_m_;
      //TODO: consider the Z value here:

      (*colors)[vertIndex] = sample.visible_ ? visibleColor_ : obstructedColor_;

      // recalculate the verts for this radial if necessary:
      if (rebuildGeometry || updateVertexPositions)
      {
        (*verts)[vertIndex].set(x*range, y*range, 0.0);
      }

      // generate the primitive sets for this radial:
      if (rebuildGeometry)
      {
        // generate triangle indices:
        if (radialIndex < radials.size() - 1)
        {
          // for the first sample, draw the origin tris as well
          if (sampleIndex == 0)
          {
            tris->push_back(0); // origin point
            tris->push_back(1 + (radialIndex     * samplesPerRadial) + sampleIndex);
            tris->push_back(1 + ((radialIndex+1) * samplesPerRadial) + sampleIndex);
          }

          if (sampleIndex < samplesPerRadial - 1)
          {
            tris->push_back(1 + (radialIndex     * samplesPerRadial) + sampleIndex);
            tris->push_back(1 + ((radialIndex+1) * samplesPerRadial) + sampleIndex);
            tris->push_back(1 + ((radialIndex+1) * samplesPerRadial) + (sampleIndex + 1));

            tris->push_back(1 + (radialIndex     * samplesPerRadial) + sampleIndex);
            tris->push_back(1 + ((radialIndex+1) * samplesPerRadial) + (sampleIndex + 1));
            tris->push_back(1 + (radialIndex     * samplesPerRadial) + (sampleIndex + 1));
          }
        }

        // the point primset.
        points->push_back(1 + (radialIndex * samplesPerRadial) + sampleIndex);
      }

      vertIndex++;
    }
  }
}

void RadialLOSNode::onTileAdded_(const osgEarth::TileKey& key, osg::Node* tile)
{
  updateDataModel(key.getExtent(), tile);
}
