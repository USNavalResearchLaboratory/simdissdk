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
#include "OpenThreads/Mutex"
#include "OpenThreads/ScopedLock"
#include "osg/Version"
#include "osgDB/Callbacks"
#include "osgDB/FileUtils"
#include "osgDB/FileNameUtils"
#include "osgDB/ReadFile"
#include "osgDB/Registry"
#include "osgEarth/Registry"
#include "simNotify/Notify.h"
#include "simCore/String/Utils.h"
#include "simCore/String/Format.h"
#include "simVis/Utils.h"
#include "simVis/ClockOptions.h"
#include "simVis/Constants.h"
#include "simVis/ModelCache.h"
#include "simVis/Registry.h"

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
  : modelCache_(new ModelCache),
    fileSearch_(new simCore::NoSearchFileSearch()),
    sequenceTimeUpdater_(new simVis::SequenceTimeUpdater(NULL))
{
  // Configure the model cache
  modelCache_->setSequenceTimeUpdater(sequenceTimeUpdater_.get());

  // Configure the osgDB::Registry with our own read file callback
  readFileCallback_ = new ReadFileCallback();
  osgDB::Registry* osgDbRegistry = osgDB::Registry::instance();
  osgDbRegistry->setReadFileCallback(readFileCallback_.get());

  // Configure the osgDB::Registry with our own file finder callback
  osg::ref_ptr<FindFileCallback> findFileCallback = new FindFileCallback(*this, osgDbRegistry->getFindFileCallback());
  osgDbRegistry->setFindFileCallback(findFileCallback.get());

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
  const std::string val = simCore::getEnvVar("SIM_NOTIFY_LEVEL");
  if (!val.empty())
  {
    simNotify::setNotifyLevel(simNotify::stringToSeverity(val));
  }

  memoryChecking_ = false;
  const std::string memoryCheckingEnv = simCore::getEnvVar("SIM_MEMORY_CHECKING");
  if (simCore::caseCompare(memoryCheckingEnv, "On") == 0)
  {
    SIM_INFO << "SIM_MEMORY_CHECKING enabled"  << std::endl;
    memoryChecking_ = true;
  }

  // prime a font when no fonts can be found.  This font will be invisible, but at least the program can limp along.
  osgText::Font* cantFindFont = new osgText::Font();
#if OSG_VERSION_GREATER_OR_EQUAL(3, 4, 1) && OSG_VERSION_LESS_THAN(3, 7, 0) && defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
  // Remove the osgText::Font's program to avoid a bug where LDB does not apply due to conflict in programs
  if (cantFindFont->getStateSet())
    cantFindFont->getStateSet()->removeAttribute(osg::StateAttribute::PROGRAM);
#endif
  fontCache_[CANT_FIND_FONT] = cantFindFont;

  // prime a default font which will be returned if the requested font cannot be found
  osgText::Font* defaultFont = getOrCreateFont(DEFAULT_FONT);
  osgEarth::Registry* osgEarthRegistry = osgEarth::Registry::instance();
  if (osgEarthRegistry->getDefaultFont() == NULL)
    osgEarthRegistry->setDefaultFont(defaultFont);
}

simVis::Registry::~Registry()
{
  delete modelCache_;
  modelCache_ = NULL;
}

static OpenThreads::Mutex s_instMutex;

simVis::Registry* simVis::Registry::instance()
{
  static Registry* s_inst = NULL;
  if (!s_inst)
  {
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_instMutex);
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

namespace {
  /** URLRewriter that is intended to block network access by always rewriting URLs to empty string */
  class RewriteToEmptyString : public osgEarth::URLRewriter
  {
  public:
    virtual std::string rewrite(const std::string& url)
    {
      return "";
    }
  };
}

void simVis::Registry::setNetworkDisabled()
{
  // Turn off HTTPClient
  osgEarth::HTTPClient::setURLRewriter(new RewriteToEmptyString);

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
  modelCache_->setShareArticulatedIconModels(value);
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
  modelCache_->clear();
}

osg::Node* simVis::Registry::getOrCreateIconModel(const std::string& location, bool* pIsImage) const
{
  // if doing a memory check, return NULL to load in a box instead of a complex icon
  if (memoryChecking_)
    return NULL;

  // Attempt to locate the filename
  std::string uri = findModelFile(location);
  if (uri.empty())
    return NULL;
  return modelCache_->getOrCreateIconModel(uri, pIsImage);
}

simVis::ModelCache* simVis::Registry::modelCache() const
{
  return modelCache_;
}

osgText::Font* simVis::Registry::getOrCreateFont(const std::string& name) const
{
  FontCache::const_iterator it = fontCache_.find(name);
  if (it != fontCache_.end())
    return it->second.get();

  // Check SIMDIS location first
  std::string filename = this->findFontFile(name);
  // If didn't find it in SIMDIS check standard system locations
  if (filename.empty())
    filename = name;

  // Create a new font with a wider image margin, to prevent image artifacts
  // See http://forum.openscenegraph.org/viewtopic.php?t=3156
  osg::ref_ptr<osgText::Font> font = osgText::readRefFontFile(filename);

  if (!font.valid())
  {
    if (fontCache_.find(DEFAULT_FONT) != fontCache_.end())
      return fontCache_.find(DEFAULT_FONT)->second.get();

    SIM_ERROR << "Could not find any fonts.  Check the value for the environment variable SIMDIS_FONTPATH\n";
    return fontCache_.find(CANT_FIND_FONT)->second.get();
  }

#if OSG_VERSION_GREATER_OR_EQUAL(3, 4, 1) && OSG_VERSION_LESS_THAN(3, 7, 0) && defined(OSG_GL_FIXED_FUNCTION_AVAILABLE)
  // Remove the osgText::Font's program to avoid a bug where LDB does not apply due to conflict in programs
  if (font->getStateSet())
    font->getStateSet()->removeAttribute(osg::StateAttribute::PROGRAM);
#endif

#if OSG_VERSION_LESS_THAN(3, 5, 8)
  // Change the glyph image margin to 2 to prevent issues with glyph textures bleeding into
  // successive glyphs.  This shows up as a vertical line on either side of the glyph.  This
  // function went away in OSG 3.5.8.
  font->setGlyphImageMargin(2);
#endif
  fontCache_[name] = font.get();

  return font.get();
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
    std::string tempString = simCore::getEnvVar("SIMDIS_FONTPATH");
    if (!tempString.empty())
      filePaths.push_back(tempString);
    tempString = simCore::getEnvVar("SIMDIS_SDK_FILE_PATH");
    if (!tempString.empty())
    {
      filePaths.push_back(tempString);
      filePaths.push_back(tempString + "/fonts");
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

void simVis::Registry::setClock(simCore::Clock* clock)
{
  modelCache_->setClock(clock);
}

simCore::Clock* simVis::Registry::getClock() const
{
  return modelCache_->getClock();
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
