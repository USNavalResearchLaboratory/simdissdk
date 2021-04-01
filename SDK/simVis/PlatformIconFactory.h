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
#ifndef SIMVIS_PLATFORMICONFACTORY_H
#define SIMVIS_PLATFORMICONFACTORY_H

#include <memory>
#include "osg/ref_ptr"
#include "simCore/Common/Common.h"

namespace osg { class Node; }
namespace simData { class PlatformPrefs; }

namespace simVis {

/**
 * Factory for creating performance-optimized 2D icons for platforms.  2D icons can be rendered quickly in
 * OSG with minimal state changes, such that tens of thousands of icons can be rendered very cheaply during
 * the Draw phase, so long as their state changes are minimized.
 *
 * Icons created in this factory will not fit cleanly into the typical PlatformModelNode hierarchy, which
 * relies on creating a generic node path in order to fill all available Platform features.  This is instead
 * an alternate path that is less flexible, but offers dramatically improved performance.  The canApply()
 * method will return false if this class won't be able to implement requested features.
 *
 * To use, simply call getOrCreate() and use the returned node.  If it returns null, then use the old
 * slower hierarchy because the user has a feature request that can't be used in this path.  When the
 * returned node is no longer used, inform via notifyRemove() to clean up memory.
 *
 * This class handles dynamic scale and scaling.  It does not handle icon rotations, and it expects to
 * be encapsulated in a BillboardAutoTransform (managed externally) for correct icon orientations.
 *
 * A singleton is provided for convenience, since most access will want to access the same factory.
 */
class SDKVIS_EXPORT PlatformIconFactory
{
public:
  PlatformIconFactory();
  virtual ~PlatformIconFactory();

  /** Returns a single global instance, singleton pattern. */
  static PlatformIconFactory* instance();

  /** Sets an enabled flag.  If disabled, getOrCreate() always return nullptr. Enabled by default. */
  void setEnabled(bool enabled);
  /** Returns the enabled flag. */
  bool isEnabled() const;

  /** Factory method for creating an node based on prefs.  May return null if prefs can't be implemented using this path. */
  osg::Node* getOrCreate(const simData::PlatformPrefs& prefs);

  /** Returns true if the icon needs to be reevaluated after new prefs apply. */
  bool hasRelevantChanges(const simData::PlatformPrefs& oldPrefs, const simData::PlatformPrefs& newPrefs) const;

private:
  /**
   * Call this method after the node from getOrCreate() loses a parent.  We use this to track
   * when a particular settings container has no more instantiations, to clean up memory.
   */
  void notifyRemove_(osg::Node* old);
  /** Returns false if we definitely cannot generate an optimized icon given the prefs. */
  bool canApply_(const simData::PlatformPrefs& prefs) const;

  class RemoveNotifier;
  struct IconContainer;

  /// Calls notifyRemove_() when one of the icons is deleted
  std::unique_ptr<RemoveNotifier> removeNotifier_;
  /// Holds a map from prefs values to unique icon instantiations
  std::unique_ptr<IconContainer> icons_;
  /// Increasing ID value; split icons into nested render bins based on this order
  uint64_t nextOrder_;
  /// Indicates whether getOrCreate() always returns nullptr
  bool enabled_;
};

}

#endif /* SIMVIS_PLATFORMICONFACTORY_H */
