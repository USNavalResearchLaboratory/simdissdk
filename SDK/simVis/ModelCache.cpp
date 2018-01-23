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
#include <cassert>
#include "osg/LOD"
#include "osg/Node"
#include "osg/NodeVisitor"
#include "osg/Sequence"
#include "osgSim/DOFTransform"
#include "osgSim/MultiSwitch"
#include "osgUtil/Optimizer"
#include "osgEarth/NodeUtils"
#include "osgEarth/Registry"
#include "osgEarth/ShaderGenerator"
#include "osgEarthAnnotation/AnnotationUtils"
#include "simNotify/Notify.h"
#include "simCore/String/Utils.h"
#include "simVis/ClockOptions.h"
#include "simVis/Utils.h"
#include "simVis/ModelCache.h"

namespace simVis {

/** Local helper visitor to add a given callback to all sequences in the graph */
class AddUpdateCallbackToSequence : public osg::NodeVisitor
{
public:
  explicit AddUpdateCallbackToSequence(osg::Callback* callback)
    : NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
      callback_(callback)
  {
  }

  void apply(osg::Sequence& sequence)
  {
    sequence.addUpdateCallback(callback_);
    traverse(sequence);
  }

private:
  osg::Callback* callback_;
};

////////////////////////////////////////////////////////////////////////////

/** Visitor that clears out the render bins of all nodes; used to fix up a regularly occurring alpha issue */
class SetRenderBinsToInherit : public osg::NodeVisitor
{
public:
  SetRenderBinsToInherit()
    : NodeVisitor(TRAVERSE_ALL_CHILDREN)
  {
  }
  virtual void apply(osg::Node& node)
  {
    osg::StateSet* ss = node.getStateSet();
    if (ss)
      ss->setRenderBinToInherit();
    traverse(node);
  }
};

////////////////////////////////////////////////////////////////////////////

ModelCache::ModelCache()
  : shareArticulatedModels_(false),
    clock_(NULL)
{
}

ModelCache::~ModelCache()
{
}

osg::Node* ModelCache::getOrCreateIconModel(const std::string& uri, bool* pIsImage)
{
  // Attempt to locate the filename
  osg::ref_ptr<osg::Node> result;

  // first check the cache.
  auto i = cache_.find(uri);
  if (i != cache_.end())
  {
    const Entry& entry = i->second;
    if (pIsImage)
      *pIsImage = entry.isImage_;

    if (entry.isArticulated_ && !shareArticulatedModels_)
    {
      // clone nodes so we get independent articulations
      return osg::clone(entry.node_.get(), osg::CopyOp::DEEP_COPY_NODES);
    }

    // shared scene graph:
    return entry.node_.get();
  }

  // Treat models and icons differently
  const bool isImage = simVis::isImageFile(uri);
  // Only cache icons re-usable between scenarios
  bool cacheIt = true;

  if (isImage)
  {
    // For an image, build a "billboard" that will always face the user
    // and auto-scale to the screen.
    using namespace osgEarth::Annotation;

    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(uri);
    if (image)
    {
      // create the geometry representing the icon:
      osg::Geometry* geom = AnnotationUtils::createImageGeometry(
        image.get(),
        osg::Vec2s(0, 0), // pixel offsets from center
        0,                // texture image unit
        0.0,              // heading
        1.0);             // scale

      osg::Geode* geode = new osg::Geode();
      geode->addDrawable(geom);
      result = geode;
    }
    else
    {
      // See if it is an MP2 file

      // Need a clock to drive the time dependent icons; failure means missing a call to setClock
      assert(clock_);
      if (clock_ != NULL)
      {
        osg::ref_ptr<simVis::ClockOptions> options = new simVis::ClockOptions(clock_);
        result = osgDB::readRefNodeFile(uri, options.get());
        // The time dependent icons MUST not be cached to prevent them from leaking into the next scenario which may or may not need them
        cacheIt = false;
      }
    }

    // Apply rendering hints to the new node, appropriate for 2D images
    if (result.valid())
    {
      osg::StateSet* stateSet = result->getOrCreateStateSet();
      // As per SIMSDK-157, blending needs to be on to avoid jaggies
      stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
    }
  }
  else // is model
  {
    // Convert the URI to Windows backslashes if needed
    std::string localUri = simCore::toNativeSeparators(uri);
    SIM_DEBUG << "Loading icon model: " << localUri << std::endl;

    result = osgDB::readRefNodeFile(localUri);

    // Need to apply a sequence time fix for osg::Sequence to deal with decreasing simulation times
    if (result.valid())
    {
      osg::ref_ptr<SequenceTimeUpdater> stuRef;
      if (sequenceTimeUpdater_.lock(stuRef))
      {
        AddUpdateCallbackToSequence fixSequenceTimeUpdates(sequenceTimeUpdater_.get());
        result->accept(fixSequenceTimeUpdates);
      }

      // Set all render bins for the loaded model to Inherited.  This allows us to later on put
      // the model into a traversal order bin.  This helps with model display of alpha textures.
      SetRenderBinsToInherit setRenderBinsToInherit;
      result->accept(setRenderBinsToInherit);
    }
  }

  // process and cache the result.
  if (!result.valid())
    return NULL;

  // Set up an LOD for performance's sake that eliminates the object from drawing if eye is too far
  osg::LOD* lod = new osg::LOD;
  // Use a pixel-size LOD.  Range LOD scales relative to eye distance, but models that get distorted
  // significantly in only 2 dimensions will have significant LOD issues with that approach.
  lod->setRangeMode(osg::LOD::PIXEL_SIZE_ON_SCREEN);
  lod->addChild(result.get(), 2.f, std::numeric_limits<float>::max());
  result = lod;

  // Save the is-image flag
  if (pIsImage)
    *pIsImage = isImage;

  // Perform vertex cache ordering optimization.
  osgUtil::Optimizer o;
  o.optimize(
    result,
    osgUtil::Optimizer::DEFAULT_OPTIMIZATIONS |
    osgUtil::Optimizer::VERTEX_PRETRANSFORM |
    osgUtil::Optimizer::VERTEX_POSTTRANSFORM |
    osgUtil::Optimizer::INDEX_MESH
    );

  // generate shaders.
  osg::ref_ptr<osgEarth::StateSetCache> stateCache = new osgEarth::StateSetCache();
  osgEarth::Registry::shaderGenerator().run(result.get(), stateCache.get());

  // cache it.
  if (cacheIt)
  {
    Entry& entry = cache_[uri];
    entry.node_ = result.get();
    entry.isImage_ = isImage;
    entry.isArticulated_ = ModelCache::isArticulated(result);
  }
  return result.get();
}

void ModelCache::setShareArticulatedIconModels(bool value)
{
  shareArticulatedModels_ = value;
}

bool ModelCache::getShareArticulatedIconModels() const
{
  return shareArticulatedModels_;
}

void ModelCache::setClock(simCore::Clock* clock)
{
  clock_ = clock;
}

simCore::Clock* ModelCache::getClock() const
{
  return clock_;
}

void ModelCache::setSequenceTimeUpdater(SequenceTimeUpdater* sequenceTimeUpdater)
{
  sequenceTimeUpdater_ = sequenceTimeUpdater;
}

void ModelCache::clear()
{
  cache_.clear();
}

bool ModelCache::isArticulated(osg::Node* node)
{
  osgSim::DOFTransform* dof = osgEarth::findTopMostNodeOfType<osgSim::DOFTransform>(node);
  if (dof)
    return true;
  osgSim::MultiSwitch* ms = osgEarth::findTopMostNodeOfType<osgSim::MultiSwitch>(node);
  if (ms)
    return true;
  return osgEarth::findTopMostNodeOfType<osg::Sequence>(node) != NULL;
}

}
