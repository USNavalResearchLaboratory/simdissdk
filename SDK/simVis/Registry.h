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
#ifndef SIMVIS_REGISTRY_H
#define SIMVIS_REGISTRY_H

#include <list>
#include "OpenThreads/ReentrantMutex"
#include "osg/observer_ptr"
#include "osg/ref_ptr"
#include "osgDB/FileUtils"
#include "osgEarth/ThreadingUtils"
#include "simCore/Common/Common.h"
#include "simCore/Common/FileSearch.h"

namespace osg { class FrameStamp; }
namespace osgText { class Font; }
namespace simCore { class Clock; }

namespace simVis
{

/** Bring osgDB::FilePathList into the simVis namespace for convenience */
typedef osgDB::FilePathList FilePathList;
/** A list of strings */
typedef std::list<std::string> FileExtensionList;
/** Model cache for loading models */
class ModelCache;

// Handles time updates on osg::Sequence
class SequenceTimeUpdater;

/**
* Central singleton repository for package-wide settings and defaults.
*/
class SDKVIS_EXPORT Registry
{
public:

  /**
  * Enable "NO NETWORK" mode, in which the application will never attempt
  * to access the network to fetch data. Best practice will be to call this
  * at the start of your application.
  */
  void setNetworkDisabled();

  /**
  * Gets a copy of the path list that the system will use to find platform models.
  * @param out_list List to populate with the model search paths (output param)
  */
  void getModelSearchPaths(FilePathList& out_list) const;

  /**
  * Sets the path list the system will use to find platform models.
  * @param list Model search paths
  */
  void setModelSearchPaths(const FilePathList& list);

  /**
  * Gets a copy of the list of extensions to look for when searching for a platform model.
  * @param out_list List to populate with search extensions (output param)
  */
  void getModelSearchExtensions(FileExtensionList& out_list) const;

  /**
  * Sets the list of extensions to look for when searching for a platform model.
  * @param list Model file extensions list
  */
  void setModelSearchExtensions(const FileExtensionList& list);

  /**
  * Searches for the named model, using the model search path list and the extensions list.
  * This method is thread safe
  * @param name Name of the model to locate (path or filename)
  * @return Absolute URL of the located model, or empty string if not found
  */
  std::string findModelFile(const std::string& name) const;

  /**
   * Model filenames are cached for faster access.  Models themselves are also cached to
   * prevent duplicate loads on non-articulated models.  This clears both caches, such as
   * when loading a new data file.
   */
  void clearModelCache();

  /**
  * Gets or loads a node that represent the specified icon.
  * The result will be either a 3D model or a billboard icon depending on the
  * Not multi-thread safe.
  * file type of the icon file found.
  * @param[in ] name Location of the file
  * @param[inout] pIsImage Pointer to a boolean that will be set to true if the loaded model refers
  *     to an image, or false if using a model.  Only set for if the return value is non-NULL.
  * @return A node, or NULL if no file was found.
  */
  osg::Node* getOrCreateIconModel(const std::string& name, bool* pIsImage = NULL) const;

  /** Retrieve a pointer to the model cache. */
  ModelCache* modelCache() const;

  /**
  * Searches for the named font, using the data search path list and the extensions list.
  * This method is thread safe
  * @param name Name of the font to locate (path or filename)
  * @return Absolute URL of the located font, or empty string if not found
  */
  std::string findFontFile(const std::string& name) const;

  /**
  * Gets or loads a font that is specified by the given name
  * Not multi-thread safe.
  * @param[in ] name name of the font
  * @return     the requested font or a default font if no file was found.
  */
  osgText::Font* getOrCreateFont(const std::string& name) const;

  /**
  * Adds an object to the "weak" object cache
  */
  void putObject(const std::string& key, osg::Referenced* obj);

  /**
  * Gets and object from the weak object cache.
  */
  osg::Referenced* getObject(const std::string& key) const;

  /**
  * True means the program can take short cuts to minimize memory usage as part of
  * memory checking.
  */
  bool isMemoryCheck() const;

  /**
  * Whether models loaded by getOrCreateIconModel() that contain articulated parts
  * should be shared. Default=FALSE so that each instance can be articulated
  * separately. Set to true if you do not intend to articulate models; this will
  * save memory by sharing the geometry across instances.
  */
  void setShareArticulatedIconModels(bool value);

  /** Retrieves the flag for whether to share articulated models or not */
  bool getShareArticulatedIconModels() const;

  /** Set a clock for time-dependent icons */
  void setClock(simCore::Clock* clock);
  /** Retrieves a clock for time-dependent icons */
  simCore::Clock* getClock() const;

  /**
  * Use this method to provide a method for location files.  The fileSearch
  * must handle both local files and URLs.  If fileSearch is not set the Registry
  * will use osg to locate the file. Blocks access to fileSearch_, so it's thread safe
  */
  void setFileSearch(simCore::FileSearchPtr fileSearch);

  /**
   * Sets the Frame Stamp to use for updating Sequences.  Sequences require a strictly
   * increasing simulation time.  simVis::Registry manages an update callback attached
   * to sequences to swap Reference Time from this framestamp with the Simultation time
   * so that SIMDIS and the SDK can have a Simulation Time that decreases (required for
   * work with osgEarthTriton for example).
   *
   * This method is automatically called when creating a new simVis::ViewManager.
   */
  void setFrameStamp(osg::FrameStamp* frameStamp);

public:

  /**
  * The static singleton registry instance
  */
  static Registry* instance();

protected:
  virtual ~Registry();

private:
  /** Constructor for the registry is private.  Use instance() to access. */
  Registry();

  /** Hide copy constructor */
  Registry(const Registry& rhs);
  /** Hide copy operator */
  Registry& operator=(const Registry& rhs);

  /// use the fileSearch_ to find the specified file. Returns the passed in filename if fileSearch_ is not set. This method is thread safe
  std::string findFile_(const std::string& filename, simCore::FileSearch::SearchFileType fileType) const;

  FilePathList modelPaths_;
  FileExtensionList modelExtensions_;
  ModelCache* modelCache_;

  // A mapping between the supplied file name and the actual file name
  typedef std::map<std::string, std::string> FilenameCache;
  /// Cache of all found model filenames
  mutable FilenameCache modelFilenameCache_;
  /// Cache of all found font filenames
  mutable FilenameCache fontFilenameCache_;

  // A mapping between the supplied font name and the actual osgText::Font
  typedef std::map<std::string, osg::ref_ptr<osgText::Font> > FontCache;
  mutable FontCache fontCache_;

  typedef std::map<std::string, osg::observer_ptr<osg::Referenced> > WeakObjectCache;
  WeakObjectCache weakObjectCache_;
  osgEarth::Threading::ReadWriteMutex weakObjectCacheMutex_;

  // If true it means abort icon loads to speed up the program
  bool memoryChecking_;

  simCore::FileSearchPtr fileSearch_;
  mutable OpenThreads::ReentrantMutex fileSearchMutex_;

  /// We maintain a callback method that blocks HTTP results, and uses our file search
  class ReadFileCallback;
  class FindFileCallback;
  osg::ref_ptr<ReadFileCallback> readFileCallback_;

  /**
   * Holds a pointer to an Update Callback that also holds the overall frame stamp.  This
   * is required to reduce the number of time calculations when updating sequences.  Sequences
   * need updating because osg::Sequence cannot handle a Simulation Time that goes back in
   * time, so we attach an updater to each osg::Sequence in loaded models.  We cannot rely
   * on the time stamp during update traversal because the reference time in it gets reset
   * per-inset as insets are added.  We need a consistent and strictly increasing reference
   * time stamp, so we rely on ViewManager to provide it.  Without this, Sequence nodes do
   * not render correctly.
   */
  osg::ref_ptr<SequenceTimeUpdater> sequenceTimeUpdater_;
};

} // namespace simVis

#endif // SIMVIS_REGISTRY_H
