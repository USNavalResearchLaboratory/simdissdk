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
#include "simVis/Registry.h"
#include "simVis/Utils.h"
#include "simVis/ClockOptions.h"
#include "simVis/Constants.h"

#include "simNotify/Notify.h"
#include "simCore/String/Utils.h"
#include "simCore/String/Format.h"

#include "OpenThreads/Mutex"
#include "OpenThreads/ScopedLock"

#include "osgDB/Callbacks"
#include "osgDB/FileUtils"
#include "osgDB/FileNameUtils"
#include "osgDB/ReadFile"
#include "osgDB/Registry"

#include "osg/AutoTransform"
#include "osg/LOD"
#include "osg/MatrixTransform"
#include "osg/Sequence"
#include "osg/Depth"
#include "osg/Sequence"
#include "osgUtil/Optimizer"

#include "osgSim/DOFTransform"
#include "osgSim/MultiSwitch"

#include "osgEarth/Registry"
#include "osgEarth/NodeUtils"
#include "osgEarth/ShaderGenerator"
#include "osgEarthAnnotation/AnnotationUtils"

#include <stdlib.h>
#include <cstring>

using namespace OpenThreads;

//----------------------------------------------------------------------------

namespace
{
  const std::string DEFAULT_FONT = "arial.ttf";
  const std::string CANT_FIND_FONT = "CouldNotFind";
}


/**
 * osgDB read callback that will reject filenames with an http: prefix.
 */
class simVis::Registry::ReadFileCallback : public osgDB::ReadFileCallback
{
public:
  /** Constructor for the ReadFileCallback */
  ReadFileCallback()
    : blockNetwork_(false)
  {
  }

  /** Turns off network access */
  void setNetworkDisabled()
  {
    blockNetwork_ = true;
  }

  /** @see osgDB::ReadFileCallback::openArchive() */
  virtual osgDB::ReaderWriter::ReadResult openArchive(const std::string& filename, osgDB::ReaderWriter::ArchiveStatus status, unsigned int indexBlockSizeHint, const osgDB::Options* useObjectCache)
  {
    if (blockNetwork_ && osgDB::containsServerAddress(filename))
      return osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;
    return osgDB::Registry::instance()->openArchiveImplementation(filename, status, indexBlockSizeHint, useObjectCache);
  }

  /** @see osgDB::ReadFileCallback::readObject() */
  virtual osgDB::ReaderWriter::ReadResult readObject(const std::string& filename, const osgDB::Options* options)
  {
    if (blockNetwork_ && osgDB::containsServerAddress(filename))
      return osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;
    return osgDB::Registry::instance()->readObjectImplementation(filename, options);
  }

  /** @see osgDB::ReadFileCallback::readImage() */
  virtual osgDB::ReaderWriter::ReadResult readImage(const std::string& filename, const osgDB::Options* options)
  {
    if (blockNetwork_ && osgDB::containsServerAddress(filename))
      return osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;
    return osgDB::Registry::instance()->readImageImplementation(filename, options);
  }

  /** @see osgDB::ReadFileCallback::readHeightField() */
  virtual osgDB::ReaderWriter::ReadResult readHeightField(const std::string& filename, const osgDB::Options* options)
  {
    if (blockNetwork_ && osgDB::containsServerAddress(filename))
      return osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;
    return osgDB::Registry::instance()->readHeightFieldImplementation(filename, options);
  }

  /** @see osgDB::ReadFileCallback::readNode() */
  virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& filename, const osgDB::Options* options)
  {
    if (blockNetwork_ && osgDB::containsServerAddress(filename))
      return osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;
    return osgDB::Registry::instance()->readNodeImplementation(filename, options);
  }

  /** @see osgDB::ReadFileCallback::readShader() */
  virtual osgDB::ReaderWriter::ReadResult readShader(const std::string& filename, const osgDB::Options* options)
  {
    if (blockNetwork_ && osgDB::containsServerAddress(filename))
      return osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;
    return osgDB::Registry::instance()->readShaderImplementation(filename, options);
  }

protected:
  /// osg::Referenced-derived
  virtual ~ReadFileCallback()
  {
  }

private:
  bool blockNetwork_;
};

/** Find-file callback that falls back to the File Search in simVis::Registry */
class simVis::Registry::FindFileCallback : public osgDB::FindFileCallback
{
public:
  FindFileCallback(const simVis::Registry& registry, osgDB::FindFileCallback* searchFirst)
    : registry_(registry),
      searchFirst_(searchFirst)
  {
  }

  virtual std::string findDataFile(const std::string& filename, const osgDB::Options* options, osgDB::CaseSensitivity caseSensitivity)
  {
    std::string rv;
    // Search the original one (presumably faster) first
    if (searchFirst_.valid())
      rv = searchFirst_->findDataFile(filename, options, caseSensitivity);
    else
      rv = osgDB::Registry::instance()->findDataFileImplementation(filename, options, caseSensitivity);
    if (rv.empty())
      rv = registry_.findFile_(filename, simCore::FileSearch::OTHER);
    return rv;
  }

  virtual std::string findLibraryFile(const std::string& filename, const osgDB::Options* options, osgDB::CaseSensitivity caseSensitivity)
  {
    return osgDB::Registry::instance()->findLibraryFileImplementation(filename, options, caseSensitivity);
  }

protected:
  virtual ~FindFileCallback()
  {
  }

private:
  const simVis::Registry& registry_;
  osg::ref_ptr<osgDB::FindFileCallback> searchFirst_;
};

//----------------------------------------------------------------------------

simVis::Registry::Registry()
  : clock_(NULL),
    fileSearch_(new simCore::NoSearchFileSearch()),
    sequenceTimeUpdater_(new simVis::SequenceTimeUpdater(NULL))
{
  // Configure the osgDB::Registry with our own read file callback
  readFileCallback_ = new ReadFileCallback();
  osgDB::Registry* osgDbRegistry = osgDB::Registry::instance();
  osgDbRegistry->setReadFileCallback(readFileCallback_);

  // Configure the osgDB::Registry with our own file finder callback
  osg::ref_ptr<FindFileCallback> findFileCallback = new FindFileCallback(*this, osgDbRegistry->getFindFileCallback());
  osgDbRegistry->setFindFileCallback(findFileCallback);

  // models may be specified without extension, use this list to attempt to resolve.
  // these should be reconciled with list in simVis::isImageFile (Utils.cpp)
  modelExtensions_.push_back("3db");
  modelExtensions_.push_back("opt");
  modelExtensions_.push_back("ive");
  modelExtensions_.push_back("flt");

  // these may be used for models, but are not model-specific formats
  modelExtensions_.push_back("png");
  modelExtensions_.push_back("bmp");
  modelExtensions_.push_back("jpg");

  // initialize the default NOTIFY level from the environment variable
  const char* val = getenv("SIM_NOTIFY_LEVEL");
  if (val)
  {
    simNotify::setNotifyLevel(simNotify::stringToSeverity(val));
  }

  memoryChecking_ = false;
  if (getenv("SIM_MEMORY_CHECKING"))
  {
    if (simCore::caseCompare(getenv("SIM_MEMORY_CHECKING"), "On") == 0)
    {
      SIM_INFO << "SIM_MEMORY_CHECKING enabled"  << std::endl;
      memoryChecking_ = true;
    }
  }

  // by default, articulated models aren't shared so that you can
  // articulate each instance independently.
  shareArticulatedModels_ = false;

  // prime a font when no fonts can be found.  This font will be invisible, but at least the program can limp along.
  fontCache_[CANT_FIND_FONT] = new osgText::Font();
  // prime a default font which will be returned if the requested font cannot be found
  osgText::Font* defaultFont = getOrCreateFont(DEFAULT_FONT);
  osgEarth::Registry* osgEarthRegistry = osgEarth::Registry::instance();
  if (osgEarthRegistry->getDefaultFont() == NULL)
    osgEarthRegistry->setDefaultFont(defaultFont);
}

simVis::Registry::~Registry()
{
}

static OpenThreads::Mutex s_instMutex;

simVis::Registry* simVis::Registry::instance()
{
  static Registry* s_inst = NULL;
  if (!s_inst)
  {
    ScopedLock<Mutex> lock(s_instMutex);
    {
      if (!s_inst) // double-check pattern
      {
        s_inst = new Registry();
      }
    }
  }
  return s_inst;
}

bool simVis::Registry::isMemoryCheck() const
{
  return memoryChecking_;
}

void simVis::Registry::setNetworkDisabled()
{
  // first, set "cache only" mode in osgEarth. This will prevent any osgEarth
  // code from trying to access the network.
  osgEarth::Registry::instance()->setOverrideCachePolicy(osgEarth::CachePolicy::CACHE_ONLY);

  // next, intercept the osgDB read implementation so that it will reject any network URLs.
  readFileCallback_->setNetworkDisabled();
}

void simVis::Registry::getModelSearchPaths(FilePathList& out_list) const
{
  out_list = modelPaths_;
}

void simVis::Registry::setModelSearchPaths(const FilePathList& list)
{
  modelPaths_ = list;
}

void simVis::Registry::getModelSearchExtensions(FileExtensionList& out_list) const
{
  out_list = modelExtensions_;
}

void simVis::Registry::setModelSearchExtensions(const FileExtensionList& list)
{
  modelExtensions_ = list;
}

void simVis::Registry::setShareArticulatedIconModels(bool value)
{
  shareArticulatedModels_ = value;
}

std::string simVis::Registry::findModelFile(const std::string& name) const
{
  osgEarth::Threading::ScopedMutexLock lock(fileSearchMutex_);

  if (!name.empty())
  {
    // Check to see if the file has been previously located
    FilenameCache::const_iterator it = modelFilenameCache_.find(name);
    if (it != modelFilenameCache_.end())
      return it->second;

    // First check the extension
    std::string fileSearchName = fileSearch_->findFile(name, simCore::FileSearch::MODEL);
    // Fall back on osgDB if it doesn't find it
    if (fileSearchName.empty())
      fileSearchName = osgDB::findDataFile(fileSearchName, simVis::DEFAULT_CASE_SENSITIVITY);

    // first check whether the name is serviceable as-is:
    if (osgDB::containsServerAddress(fileSearchName) || osgDB::fileExists(fileSearchName))
    {
      modelFilenameCache_[name] = fileSearchName;
      return fileSearchName;
    }

    // Now check via osgDB which has different search paths than fileSearch_
    if (osgDB::getFileExtension(name).empty())
    {
      // if the name has no extension, try tacking on extensions and trying to find the paths.
      for (FileExtensionList::const_iterator i = modelExtensions_.begin(); i != modelExtensions_.end(); ++i)
      {
        std::string filename = name + "." + *i;
        std::string result = osgDB::findFileInPath(filename, modelPaths_, simVis::DEFAULT_CASE_SENSITIVITY);
        if (!result.empty())
        {
          modelFilenameCache_[name] = result;
          return result;
        }
      }
    }
    else
    {
      // if the name already has an extension, try to find it as-is:
      std::string result = osgDB::findFileInPath(name, modelPaths_, simVis::DEFAULT_CASE_SENSITIVITY);
      if (!result.empty())
      {
        modelFilenameCache_[name] = result;
        return result;
      }
    }
  }

  // no luck
  modelFilenameCache_[name].clear();
  return std::string();
}

void simVis::Registry::clearModelCache()
{
  modelFilenameCache_.clear();
  modelCache_.clear();
}

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

osg::Node* simVis::Registry::getOrCreateIconModel(const std::string& location, bool* pIsImage) const
{
  // if doing a memory check, return NULL to load in a box instead of a complex icon
  if (memoryChecking_)
    return NULL;

  // Attempt to locate the filename
  osg::Node* result = NULL;
  std::string uri = findModelFile(location);
  if (uri.empty())
    return NULL;

  // first check the cache.
  ModelCache::const_iterator i = modelCache_.find(uri);
  if (i != modelCache_.end())
  {
    const ModelCacheEntry& entry = i->second;
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

    osg::Image* image = osgDB::readImageFile(uri);
    if (image)
    {
      // create the geometry representing the icon:
      osg::Geometry* geom = AnnotationUtils::createImageGeometry(
        image,
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
        result = osgDB::readNodeFile(uri, options.get());
        // The time dependent icons MUST not be cached to prevent them from leaking into the next scenario which may or may not need them
        cacheIt = false;
      }
    }

    // Apply rendering hints to the new node, appropriate for 2D images
    if (result)
    {
      osg::StateSet* stateSet = result->getOrCreateStateSet();
      // As per SIMSDK-157, blending needs to be on to avoid jaggies
      stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);

      // and we need to explicitly turn on depth writes
      stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS, 0, 1, true), osg::StateAttribute::ON);
    }
  }
  else // is model
  {
    // Convert the URI to Windows backslashes if needed
    std::string localUri = simCore::toNativeSeparators(uri);
    SIM_DEBUG << "Loading icon model: " << localUri << std::endl;

    result = osgDB::readNodeFile(localUri);

    // Need to apply a sequence time fix for osg::Sequence to deal with decreasing simulation times
    if (result)
    {
      AddUpdateCallbackToSequence fixSequenceTimeUpdates(sequenceTimeUpdater_);
      result->accept(fixSequenceTimeUpdates);

      // Set all render bins for the loaded model to Inherited.  This allows us to later on put
      // the model into a traversal order bin.  This helps with model display of alpha textures.
      SetRenderBinsToInherit setRenderBinsToInherit;
      result->accept(setRenderBinsToInherit);
    }
  }

  // process and cache the result.
  if (!result)
    return NULL;

  // Set up an LOD for performance's sake that eliminates the object from drawing if eye is too far
  osg::LOD* lod = new osg::LOD;
  // Some models like MP2 could change size over time, so give a default radius and don't accept 0.0
  const float radius = simCore::sdkMin(result->getBound().radius(), 32.f); // meters
  // LOD scale of 5000 times radius was based on visibility tests with a 2k pixel screen maximized
  lod->addChild(result, 0.f, radius * 5000.f); // Minimum value: 160km (5000 * 32) before phase out
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
  osgEarth::Registry::shaderGenerator().run(result, stateCache.get());

  // cache it.
  if (cacheIt)
  {
    ModelCacheEntry& entry = modelCache_[uri];
    entry.node_ = result;
    entry.isImage_ = isImage;
    entry.isArticulated_ = isArticulated_(result);
  }
  return result;
}

osgText::Font* simVis::Registry::getOrCreateFont(const std::string& name) const
{
  FontCache::const_iterator it = fontCache_.find(name);
  if (it != fontCache_.end())
    return it->second;

  // Check SIMDIS location first
  std::string filename = this->findFontFile(name);
  // If didn't find it in SIMDIS check standard system locations
  if (filename.empty())
    filename = name;

  // Create a new font with a wider image margin, to prevent image artifacts
  // See http://forum.openscenegraph.org/viewtopic.php?t=3156
  osg::ref_ptr<osgText::Font> font = osgText::readFontFile(filename);

  if (!font.valid())
  {
    if (fontCache_.find(DEFAULT_FONT) != fontCache_.end())
      return fontCache_.find(DEFAULT_FONT)->second;

    SIM_ERROR << "Could not find any fonts.  Check the value for the environment variable SIMDIS_FONTPATH\n";
    return fontCache_.find(CANT_FIND_FONT)->second;
  }

  font->setGlyphImageMargin(2);
  fontCache_[name] = font;

  return font;
}

std::string simVis::Registry::findFontFile(const std::string& name) const
{
  osgEarth::Threading::ScopedMutexLock lock(fileSearchMutex_);

  if (!name.empty())
  {
    // Check to see if the file has been previously located
    FilenameCache::const_iterator it = fontFilenameCache_.find(name);
    if (it != fontFilenameCache_.end())
      return it->second;

    // First check the extension
    std::string fileSearchName = fileSearch_->findFile(name, simCore::FileSearch::OTHER);
    // If it fails, fall back on osgText, which itself falls back on osgDB::findDataFile()
    if (fileSearchName.empty())
      fileSearchName = osgText::findFontFile(name);

    // first check whether the name is serviceable as-is:
    if (osgDB::containsServerAddress(fileSearchName) || osgDB::fileExists(fileSearchName))
    {
      fontFilenameCache_[name] = fileSearchName;
      return fileSearchName;
    }


    // Try to find it in the typical path list using SIMDIS_FONTPATH
    osgDB::FilePathList filePaths;

    // search for font in the SIMDIS_FONTPATH directory, falling back on SIMDIS_SDK_FILE_PATH (/fonts)
    const char* tempString = getenv("SIMDIS_FONTPATH");
    if (tempString)
      filePaths.push_back(tempString);
    tempString = getenv("SIMDIS_SDK_FILE_PATH");
    if (tempString)
    {
      std::string sdkFilePath = tempString;
      filePaths.push_back(sdkFilePath);
      filePaths.push_back(sdkFilePath + "/fonts");
    }

    // Search the disk
    std::string result = osgDB::findFileInPath(name, filePaths, simVis::DEFAULT_CASE_SENSITIVITY);
    // Fall back to the osgText search instead of relying on findFileInPath() alone
    if (result.empty())
      result = osgText::findFontFile(name);

    // Save the cached name if the font was found
    if (!result.empty())
    {
      fontFilenameCache_[name] = result;
      return result;
    }
  }

  // no luck
  fontFilenameCache_[name].clear();
  return std::string();
}

void simVis::Registry::putObject(const std::string& key, osg::Referenced* obj)
{
  osgEarth::Threading::ScopedWriteLock lock(weakObjectCacheMutex_);
  weakObjectCache_[key] = obj;
}

osg::Referenced* simVis::Registry::getObject(const std::string& key) const
{
  osgEarth::Threading::ScopedReadLock lock(const_cast<Registry*>(this)->weakObjectCacheMutex_);
  WeakObjectCache::const_iterator i = weakObjectCache_.find(key);
  if (i != weakObjectCache_.end())
    return i->second.get();
  else
    return NULL;
}

bool simVis::Registry::isArticulated_(osg::Node* node) const
{
  osgSim::DOFTransform* dof = osgEarth::findTopMostNodeOfType<osgSim::DOFTransform>(node);
  osgSim::MultiSwitch* ms = osgEarth::findTopMostNodeOfType<osgSim::MultiSwitch>(node);
  osg::Sequence* seq = osgEarth::findTopMostNodeOfType<osg::Sequence>(node);
  return (dof != NULL || ms != NULL || seq != NULL);
}

void simVis::Registry::setClock(simCore::Clock* clock)
{
  clock_ = clock;
}

simCore::Clock* simVis::Registry::getClock() const
{
  return clock_;
}

void simVis::Registry::setFileSearch(simCore::FileSearchPtr fileSearch)
{
  osgEarth::Threading::ScopedMutexLock lock(fileSearchMutex_);

  if (fileSearch == NULL)
    fileSearch_.reset(new simCore::NoSearchFileSearch());
  else
    fileSearch_ = fileSearch;
}

std::string simVis::Registry::findFile_(const std::string& filename, simCore::FileSearch::SearchFileType fileType) const
{
  osgEarth::Threading::ScopedMutexLock lock(fileSearchMutex_);
  if (fileSearch_ == NULL)
    return "";
  return fileSearch_->findFile(filename, fileType);
}

void simVis::Registry::setFrameStamp(osg::FrameStamp* frameStamp)
{
  sequenceTimeUpdater_->setFrameStamp(frameStamp);
}
