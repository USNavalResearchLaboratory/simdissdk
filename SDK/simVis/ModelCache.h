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
#ifndef SIMVIS_MODELCACHE_H
#define SIMVIS_MODELCACHE_H

#include <map>
#include <string>
#include "osg/observer_ptr"
#include "osg/ref_ptr"
#include "simCore/Common/Export.h"

namespace osg { class Node; }
namespace simCore { class Clock; }

namespace simVis {

class SequenceTimeUpdater;

/** Provides a loading mechanism for models that caches the results for fast access. */
class SDKVIS_EXPORT ModelCache
{
public:
  ModelCache();
  virtual ~ModelCache();

  /**
   * Whether models loaded by getOrCreateIconModel() that contain articulated parts
   * should be shared. Default=FALSE so that each instance can be articulated
   * separately. Set to true if you do not intend to articulate models; this will
   * save memory by sharing the geometry across instances.
   */
  void setShareArticulatedIconModels(bool value);
  /** Retrieves the flag for whether to share articulated models or not */
  bool getShareArticulatedIconModels() const;

  /** Sets the clock instance to use for SIMDIS Media Player 2 time updates. */
  void setClock(simCore::Clock* clock);
  /** Retrieves the currently set clock instance. */
  simCore::Clock* getClock() const;

  /** Sets the sequence time updater instance.  See simVis::Registry::sequenceTimeUpdater_. */
  void setSequenceTimeUpdater(SequenceTimeUpdater* sequenceTimeUpdater);

  /**
   * Gets or loads a node that represents the specified icon.  The result will either be a 3D
   * model or a billboard icon.  The icon is loaded synchronously.  The URI is passed directly
   * to osgDB::readRefNodeFile(), so use simVis::Registry::findModelFile() is recommended
   * before calling this method.
   * @param uri Location of the model to load.  This class does not explicitly pass the string
   *  through the simVis::Registry::findModelFile() method, so be sure to call that if the
   *  URI is not going to be automatically found by OSG built-in mechanisms.
   * @param pIsImage If non-NULL, will be set to true or false based on whether the URI represents
   *  an image file that needs billboarding.
   * @return A node, or NULL if no file was found.
   */
  osg::Node* getOrCreateIconModel(const std::string& uri, bool* pIsImage);

  /** Clears the cache. */
  void clear();

  /** Helper method that returns true if an articulation node is present under the provided node. */
  static bool isArticulated(osg::Node* node);

private:
  /// Copy constructor not permitted
  ModelCache(const ModelCache&);

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

  /// If false, return a separate model instance for any model with articulations
  bool shareArticulatedModels_;
  /// Clock is required for SIMDIS MP2 models
  simCore::Clock* clock_;
  /// Sequence updater is associated with nodes with osg::Sequence, to fix backwards time problems.  See simVis::Registry::sequenceTimeUpdater_
  osg::observer_ptr<SequenceTimeUpdater> sequenceTimeUpdater_;
  /// Maps string name to cache entry
  std::map<std::string, Entry> cache_;
};

}

#endif /* SIMVIS_MODELCACHE_H */
