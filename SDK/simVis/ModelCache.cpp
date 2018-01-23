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

/** Extension to use for the pseudo-loader for model cache */
static const std::string MODEL_LOADER_EXT = "simvis_modelcache";
/** Key to use for User Values to flag whether loaded node should be cached */
static const std::string CACHE_HINT_KEY = "CacheHint";
/** Key to use for User Values to flag whether loaded node is an image */
static const std::string IMAGE_HINT_KEY = "ImageHint";

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

/** Options class that holds onto the Clock and SequenceTimeUpdater from the Model Cache. */
class ModelCacheLoaderOptions : public osgDB::ReaderWriter::Options
{
public:
  ModelCacheLoaderOptions()
    : addLodNode(true),
      runShaderGenerator(true),
      clock(NULL)
  {
    optimizeFlags = osgUtil::Optimizer::DEFAULT_OPTIMIZATIONS |
      osgUtil::Optimizer::VERTEX_PRETRANSFORM |
      osgUtil::Optimizer::VERTEX_POSTTRANSFORM |
      osgUtil::Optimizer::INDEX_MESH;

  }

  /** Set true to create an LOD node that swaps out when item is too small on screen. */
  bool addLodNode;
  /** Change the flags sent to optimizer.  Set to 0 to disable optimization. */
  unsigned int optimizeFlags;
  /** Set true to run the osgEarth shader generator on the resulting node. */
  bool runShaderGenerator;
  /** Clock object used for SIMDIS Media Player 2 playlist nodes. */
  simCore::Clock* clock;
  /** Pointer to the class that fixes osg::Sequence; see simVis::Registry::sequenceTimeUpdater_. */
  osg::observer_ptr<SequenceTimeUpdater> sequenceTimeUpdater;

protected:
  virtual ~ModelCacheLoaderOptions() {}
};

////////////////////////////////////////////////////////////////////////////

/** Pseudo-loader for ModelCache that does the heavy lifting, including reading node and optimizing it. */
class ModelCacheLoader : public osgDB::ReaderWriter
{
public:
  META_Object(simVis, ModelCacheLoader);

  /** Default constructor */
  ModelCacheLoader()
  {
    supportsExtension(MODEL_LOADER_EXT, "simVis Model Cache Pseudo-Loader");
  }
  /** OSG copy constructor, needed for META_Object. */
  ModelCacheLoader(const ModelCacheLoader& copy, const osg::CopyOp& copyOp)
    : ReaderWriter(copy, copyOp)
  {
  }

  /** Called by OSG when a filename is requested to be read into a node. */
  virtual ReadResult readNode(const std::string& filename, const osgDB::ReaderWriter::Options* options) const
  {
    const std::string ext = osgDB::getLowerCaseFileExtension(filename);
    if (!acceptsExtension(ext))
      return ReadResult::FILE_NOT_HANDLED;

    // Strip the extension
    const std::string tmpName = osgDB::getNameLessExtension(filename);
    if (tmpName.empty())
      return ReadResult::FILE_NOT_HANDLED;

    return readNode_(tmpName, dynamic_cast<const ModelCacheLoaderOptions*>(options));
  }

private:
  /** Helper method reads the raw filename and respects the incoming options. */
  ReadResult readNode_(const std::string& filename, const ModelCacheLoaderOptions* options) const
  {
    // ref_ptr that will hold the return value
    osg::ref_ptr<osg::Node> result;
    // Only cache icons re-usable between scenarios
    bool cacheIt = true;
    const bool isImage = simVis::isImageFile(filename);
    if (isImage)
      result = readImageNode_(filename, options, cacheIt);
    else // is model
      result = readModelNode_(filename, options);

    // process and cache the result.
    if (!result.valid())
      return ReadResult::FILE_NOT_HANDLED;

    // Apply post-load options
    applyPostLoadOptions_(result, options);
    // Save the is-image and cache flag hints
    result->setUserValue(IMAGE_HINT_KEY, isImage);
    result->setUserValue(CACHE_HINT_KEY, cacheIt);

    return result.release();
  }

  /** Helper method that applies the various post-read-node operations to a node. */
  void applyPostLoadOptions_(osg::ref_ptr<osg::Node>& result, const ModelCacheLoaderOptions* options) const
  {
    if (!result || !options)
      return;

    // Set up an LOD for performance's sake that eliminates the object from drawing if eye is too far.
    if (options->addLodNode)
    {
      osg::ref_ptr<osg::LOD> lod = new osg::LOD;
      // Use a pixel-size LOD.  Range LOD scales relative to eye distance, but models that get distorted
      // significantly in only 2 dimensions will have significant LOD issues with that approach.
      lod->setRangeMode(osg::LOD::PIXEL_SIZE_ON_SCREEN);
      lod->addChild(result.get(), 2.f, std::numeric_limits<float>::max());
      result = lod;
    }

    // Perform vertex cache ordering optimization.
    if (options->optimizeFlags != 0)
    {
      osgUtil::Optimizer o;
      o.optimize(result, options->optimizeFlags);
    }

    // generate shaders.
    if (options->runShaderGenerator)
    {
      osg::ref_ptr<osgEarth::StateSetCache> stateCache = new osgEarth::StateSetCache();
      osgEarth::Registry::shaderGenerator().run(result.get(), stateCache.get());
    }
  }

  /** Helper method to process the filename into an image.  Also handles SIMDIS MP2 files. */
  osg::ref_ptr<osg::Node> readImageNode_(const std::string& filename, const ModelCacheLoaderOptions* options, bool& cacheIt) const
  {
    osg::ref_ptr<osg::Node> result;

    // For an image, build a "billboard" that will always face the user
    // and auto-scale to the screen.
    using namespace osgEarth::Annotation;

    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(filename);
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
      assert(options && options->clock);
      if (options && options->clock)
      {
        osg::ref_ptr<simVis::ClockOptions> cOpts = new simVis::ClockOptions(options->clock);
        result = osgDB::readRefNodeFile(filename, cOpts.get());
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
    return result;
  }

  /** Helper method to process the filename into an model. */
  osg::ref_ptr<osg::Node> readModelNode_(const std::string& filename, const ModelCacheLoaderOptions* options) const
  {
    // Convert the URI to Windows backslashes if needed
    std::string localUri = simCore::toNativeSeparators(filename);
    osg::ref_ptr<osg::Node> result = osgDB::readRefNodeFile(localUri);

    // Need to apply a sequence time fix for osg::Sequence to deal with decreasing simulation times
    if (result.valid())
    {
      osg::ref_ptr<SequenceTimeUpdater> seqUpdater;
      if (options && options->sequenceTimeUpdater.lock(seqUpdater))
      {
        AddUpdateCallbackToSequence fixSequenceTimeUpdates(seqUpdater.get());
        result->accept(fixSequenceTimeUpdates);
      }

      // Set all render bins for the loaded model to Inherited.  This allows us to later on put
      // the model into a traversal order bin.  This helps with model display of alpha textures.
      SetRenderBinsToInherit setRenderBinsToInherit;
      result->accept(setRenderBinsToInherit);
    }
    return result;
  }

};

REGISTER_OSGPLUGIN(simvis_modelcache, ModelCacheLoader)

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

  // Set up an options struct for the pseudo loader
  osg::ref_ptr<ModelCacheLoaderOptions> opts = new ModelCacheLoaderOptions;
  opts->clock = clock_;
  opts->sequenceTimeUpdater = sequenceTimeUpdater_.get();
  // Farm off to the pseudo-loader
  osg::ref_ptr<osg::Node> result = osgDB::readRefNodeFile(uri + "." + MODEL_LOADER_EXT, opts.get());
  if (!result)
    return NULL;

  // Store the image hint
  bool isImage = false;
  result->getUserValue(IMAGE_HINT_KEY, isImage);
  if (pIsImage)
    *pIsImage = isImage;

  // Respect the cache hint
  bool cacheIt = false;
  result->getUserValue(CACHE_HINT_KEY, cacheIt);
  if (cacheIt)
  {
    Entry& entry = cache_[uri];
    entry.node_ = result.get();
    entry.isImage_ = isImage;
    entry.isArticulated_ = ModelCache::isArticulated(result);
  }

  return result.release();
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
