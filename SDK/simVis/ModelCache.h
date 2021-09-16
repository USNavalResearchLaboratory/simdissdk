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
 * License for source code is in accompanying LICENSE.txt file. If you did
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_MODELCACHE_H
#define SIMVIS_MODELCACHE_H

#include <map>
#include <string>
#include "osg/observer_ptr"
#include "osg/ref_ptr"
#include "osgEarth/Containers"
#include "simCore/Common/Export.h"

namespace osg {
  class Group;
  class Node;
}
namespace simCore { class Clock; }

namespace simVis {

class SequenceTimeUpdater;

/** Provides a loading mechanism for models that caches the results for fast access. */
class SDKVIS_EXPORT ModelCache
{
public:
  ModelCache();
  virtual ~ModelCache();

  /** Retrieves a pointer to a generic box icon used for blank models */
  osg::Node* boxNode() const;

  /**
   * Whether models loaded by getOrCreateIconModel() that contain articulated parts
   * should be shared. Default=FALSE so that each instance can be articulated
   * separately. Set to true if you do not intend to articulate models; this will
   * save memory by sharing the geometry across instances.
   */
  void setShareArticulatedIconModels(bool value);
  /** Retrieves the flag for whether to share articulated models or not */
  bool getShareArticulatedIconModels() const;

  /** Flag whether to use an LOD node to hide the model at an appropriate distance.  Default: true. */
  void setUseLodNode(bool useLodNode);
  /** Retrieves flag for whether to add an LOD node at the root level */
  bool useLodNode() const;

  /** Sets the clock instance to use for SIMDIS Media Player 2 time updates. */
  void setClock(simCore::Clock* clock);
  /** Retrieves the currently set clock instance. */
  simCore::Clock* getClock() const;

  /** Sets the sequence time updater instance.  See simVis::Registry::sequenceTimeUpdater_. */
  void setSequenceTimeUpdater(SequenceTimeUpdater* sequenceTimeUpdater);

  /**
   * Gets or loads a node that represents the specified icon.  The result will either be a 3D
   * model or a billboard icon.  The icon is loaded synchronously.  The URI is passed directly
   * to osgDB::readRefNodeFile(), so using simVis::Registry::findModelFile() is recommended
   * before calling this method to avoid an osgDB file search.
   *
   * This call is synchronous and can impact the frame rate for models that take a long time to
   * load or optimize.  For asynchronous loading, see asyncLoad().
   *
   * @param uri Location of the model to load.  This class does not explicitly pass the string
   *  through the simVis::Registry::findModelFile() method, so be sure to call that if the
   *  URI is not going to be automatically found by OSG built-in mechanisms.
   * @param pIsImage If non-nullptr, will be set to true or false based on whether the URI represents
   *  an image file that needs billboarding.
   * @return A node, or nullptr if no file was found.
   */
  osg::Node* getOrCreateIconModel(const std::string& uri, bool* pIsImage);

  /** Clears the cache. */
  void clear();

  /** Erases a single element from the cache. */
  void erase(const std::string& uri);

  /**
   * Retrieves the asynchronous loader node.  This node must be added to the scene graph for
   * asynchronous loading to work correctly.  The Registry's default model cache is registered with
   * the scene in simVis::SceneManager.  If you create your own ModelCache and wish to use
   * asynchronous loading, please add this node to the scene.  See ModelCache::LoaderNode for details.
   */
  osg::Node* asyncLoaderNode() const;

  /** Callback to use when an asynchronously loaded model is ready for display. */
  class ModelReadyCallback : public osg::Referenced
  {
  public:
    /** Called when the model is ready for display in the scene. */
    virtual void loadFinished(const osg::ref_ptr<osg::Node>& model, bool isImage, const std::string& uri) = 0;
  protected:
    /** Protected destructor due to Referenced derived class. */
    virtual ~ModelReadyCallback() {}
  };

  /**
   * Loads a URI asynchronously.  Equivalent to getOrCreateIconModel(), but occurs in the background
   * without impacting the frame rate.  Before starting an asynchronous load, this method will check
   * the internal cache and immediately execute the callback if the URI is present.  Otherwise, the
   * load is queued in the background.
   * @param uri Raw URI to load.  If necessary, use simVis::Registry::findModelFile() to find the full path.
   * @param callback Callback that is executed when loading completes.
   */
  void asyncLoad(const std::string& uri, ModelReadyCallback* callback);

  /** Helper method that returns true if an articulation node is present under the provided node. */
  static bool isArticulated(osg::Node* node);

private:
  /// Copy constructor not permitted
  ModelCache(const ModelCache& rhs);
  /// Assignment operator not permitted
  ModelCache& operator=(const ModelCache& rhs);

  /// Saves the given URI into the cache
  void saveToCache_(const std::string& uri, osg::Node* node, bool isArticulated, bool isImage);

  /// Entry in the cache
  struct Entry
  {
    /// Node returned to end users
    osg::ref_ptr<osg::Node> node_;
    /// Set true when node_ represents a model with articulations
    bool isArticulated_;
    /// Set true when node_ represents an image icon
    bool isImage_;
  };
  /// osg::Node that is responsible for loading nodes in the background using osg::ProxyNode
  class LoaderNode;

  /// If false, return a separate model instance for any model with articulations
  bool shareArticulatedModels_;
  /// If true (default), add an LOD node at the top of the scene
  bool addLodNode_;
  /// Clock is required for SIMDIS MP2 models
  simCore::Clock* clock_;
  /// Sequence updater is associated with nodes with osg::Sequence, to fix backwards time problems.  See simVis::Registry::sequenceTimeUpdater_
  osg::observer_ptr<SequenceTimeUpdater> sequenceTimeUpdater_;

  /// Typedef for Cache class instance
  typedef osgEarth::Util::LRUCache<std::string, Entry> Cache;
  /// Maps string name to cache entry
  Cache cache_;

  /// Node that is used for when platforms do not exist as a placeholder object
  osg::ref_ptr<osg::Node> boxNode_;
  /// Holds onto the asynchronous loading node
  osg::ref_ptr<LoaderNode> asyncLoader_;
};

/**
 * Model-Ready Callback that will replace a given child in group.  If the parent does not have
 * a child at that index to replace, the new model is added to the end.  This class is provided
 * as a convenience to use ModelCache::asyncLoad() with an osg::Group.
 */
class SDKVIS_EXPORT ReplaceChildReadyCallback : public simVis::ModelCache::ModelReadyCallback
{
public:
  /** On callback, will remove child "childIndex" from "parent" before adding in the model as a child. */
  explicit ReplaceChildReadyCallback(osg::Group* parent, unsigned int childIndex = 0);

  /** Callback virtual method. */
  virtual void loadFinished(const osg::ref_ptr<osg::Node>& model, bool isImage, const std::string& filename);

private:
  osg::observer_ptr<osg::Group> parent_;
  unsigned int childIndex_;
};

}

#endif /* SIMVIS_MODELCACHE_H */
