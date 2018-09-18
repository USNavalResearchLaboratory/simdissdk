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
#ifndef EXAMPLE_PICKING_CUSTOMRENDER_H
#define EXAMPLE_PICKING_CUSTOMRENDER_H

#include "osg/observer_ptr"
#include "osg/Geometry"
#include "osg/MatrixTransform"
#include "simCore/Calc/MathConstants.h"
#include "simData/DataStore.h"
#include "simVis/CustomRendering.h"
#include "simVis/Locator.h"
#include "simVis/Scenario.h"

namespace ExamplePicking
{

/** String name for the Custom Render "Renderer" property */
static const std::string RENDERER_NAME = "example_picking";

/// Handles the datastore update from the CustomRenderingNode
class RenderEngine : public simVis::CustomRenderingNode::UpdateCallback
{
public:
  RenderEngine()
    : scale_(100.f, 100.f, 1.f)
  {
  }

  virtual bool update(const simData::DataSliceBase* updateSlice, bool force = false)
  {
    // Break out if the node isn't currently valid
    if (node_ == NULL)
      return false;

    // Create the geometry if it hasn't been created yet
    if (transform_ == NULL)
    {
      simVis::LocatorNode* locatorNode = node_->locatorNode();
      locatorNode->removeChildren(0, locatorNode->getNumChildren());

      // In this example do a simple unit circle.  Filled ones are easier to pick.
      osg::Geometry* geom = makeFilledUnitCircle_();
      transform_ = new osg::MatrixTransform;
      transform_->addChild(geom);
      locatorNode->addChild(transform_);
      node_->setCustomActive(true);
      locatorNode->dirtyBound();

      // Configure a render bin that is appropriate for semi-transparent graphics
      transform_->getOrCreateStateSet()->setRenderBinDetails(simVis::BIN_CUSTOM_RENDER, simVis::BIN_TWO_PASS_ALPHA);
    }

    // Alter the transform's scale to demonstrate the rendering effect
    osg::Matrix matrix;
    matrix.makeScale(scale_);
    scale_.x() += 3.0;
    if (scale_.x() > 200.0)
      scale_.x() = 100.0;
    scale_.y() += 2.0;
    if (scale_.y() > 200.0)
      scale_.y() = 100.0;
    transform_->setMatrix(matrix);

    // Adjust the coordinates of the locator to match that of the host
    const simVis::EntityNode* host = node_->host();
    if (host != NULL)
    {
      simCore::Coordinate coord;
      host->getLocator()->getCoordinate(&coord);
      node_->getLocator()->setCoordinate(coord);
      node_->dirtyBound();
    }

    return true;
  }

  /** Configures the host node.  This UpdateCallback only handles a single node. */
  void setNode(simVis::CustomRenderingNode* node)
  {
    node_ = node;
    // Offset the custom rendering entity's center by 100 to the "right" in
    // order to make it easier to pick for Dynamic picking
    if (node_.valid())
      node_->getLocator()->setLocalOffsets(simCore::Vec3(100.0, 0.0, 0.0), simCore::Vec3(0.0, 0.0, 0.0));
  }

private:
  /** Creates a new filled unit circle.  Filled entities are easier to pick. */
  osg::Geometry* makeFilledUnitCircle_() const
  {
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
    (*colors)[0] = osg::Vec4(1, 1, 1, 1);
    geom->setColorArray(colors.get());

    const int NUM_EDGE_POINTS = 80; // one point every (360/80) degrees
    osg::ref_ptr<osg::Vec3Array> fillVerts = new osg::Vec3Array(2 + NUM_EDGE_POINTS);
    (*fillVerts)[0].set(osg::Vec3f(0.f, 0.f, 0.f)); // center point
    for (int ii = 0; ii <= NUM_EDGE_POINTS; ++ii) // Hit first point (circle) twice
    {
      float angle = static_cast<float>(M_PI * 2.f * ii / NUM_EDGE_POINTS);
      float x = static_cast<float>(cos(angle));
      float y = static_cast<float>(sin(angle));
      (*fillVerts)[ii + 1].set(x, y, 0);
    }
    geom->setVertexArray(fillVerts.get());
    geom->setDataVariance(osg::Object::DYNAMIC);
    geom->setUseDisplayList(false);
    geom->setUseVertexBufferObjects(true);
    geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_FAN, 0, fillVerts->size()));

    return geom.release();
  }

  osg::observer_ptr<simVis::CustomRenderingNode> node_;
  osg::ref_ptr<osg::MatrixTransform> transform_;
  osg::Vec3f scale_;
};


/**
 * Listens to the DataStore for new Custom Render Entities, associating an UpdateCallback
 * with the CustomRenderingNode.  The UpdateCallback, defined above, gets called regularly
 * on scenario update in order to draw graphics for the node.
*/
class AttachRenderGraphics : public simData::DataStore::DefaultListener
{
public:
  explicit AttachRenderGraphics(simVis::ScenarioManager* manager)
    : manager_(manager)
  {
  }

  /** Detect our Custom Render nodes */
  virtual void onAddEntity(simData::DataStore *source, simData::ObjectId newId, simData::ObjectType ot)
  {
    // Break out if not a custom rendering; we don't care about those entities here
    if (!manager_.valid() || ot != simData::CUSTOM_RENDERING)
      return;

    simData::DataStore::Transaction txn;
    const simData::CustomRenderingProperties* props = source->customRenderingProperties(newId, &txn);
    // Only attach to OUR custom render objects by comparing renderer engine names
    if (!props || props->renderer() != RENDERER_NAME)
      return;
    txn.complete(&props);

    // Pick out the node from the scene (created by the ScenarioDataStoreAdapter automatically)
    simVis::CustomRenderingNode* node = manager_->find<simVis::CustomRenderingNode>(newId);
    if (node != NULL)
    {
      // A real render engine would need to account for multiple Custom Render nodes here,
      // either by creating a separate updater per entity, or configuring the updater to
      // correctly handle multiple entities.
      RenderEngine* updater = new RenderEngine();
      updater->setNode(node);
      node->setUpdateCallback(updater);
      node->setCustomActive(true);
    }
  }

private:
  osg::observer_ptr<simVis::ScenarioManager> manager_;
};

}

#endif /* EXAMPLE_PICKING_CUSTOMRENDER_H */
