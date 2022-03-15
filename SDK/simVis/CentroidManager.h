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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_CENTROIDMANAGER_H
#define SIMVIS_CENTROIDMANAGER_H

#include "osg/ref_ptr"
#include "osg/Group"
#include "simCore/Common/Common.h"
#include "simVis/AveragePositionNode.h"

namespace osg { class ObserverSet; }

namespace simVis {

class EntityNode;
class View;
class ViewManager;

/**
* Manages a centroid node for any and all views in the scene.
* Each view can have at most one centroid. If a new centroid is
* requested for a view that already has one, the old centroid is lost.
*/
class SDKVIS_EXPORT CentroidManager : public osg::Group
{
public:
  CentroidManager();

  /**
  * Create a centroid using the given nodes. Returns a pointer to the
  * created centroid. Will return nullptr if the given view is invalid or
  * if nodes is empty or filled with invalid pointers.
  * @param inNodes Vector of EntityNodes about which to center the view
  * @param view View to tether to the centroid
  * @return Pointer to created centroid node
  */
  AveragePositionNode* createCentroid(const std::vector<EntityNode*>& inNodes, View* view);

  /**
  * Create a centroid using the given nodes and center the given view on it.
  * @param nodes Vector of EntityNodes about which to center the view
  * @param view View to tether to the centroid
  */
  void centerViewOn(const std::vector<EntityNode*>& nodes, View* view);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "CentroidManager"; }

protected:
  /** Protect osg::Referenced-derived destructor */
  virtual ~CentroidManager();

private:
  /** ViewManager callback to watch for deleted views */
  class ViewsWatcher;
  /** Container linking an AveragePositionNode to a View's ObserverSet. */
  struct CentroidInfo
  {
    osg::ref_ptr<osg::ObserverSet> viewObs;
    osg::ref_ptr<AveragePositionNode> node;
  };

  /**
  * Install a view callback on the given ViewManager.
  * @param vm ViewManager on which to install a callback
  */
  void initViewCallback_(ViewManager& vm);
  /** Triggered by a view removal. Removes the view if necessary. */
  void removeView_(View* view);

  /**
  * Maps a view to its centroid info. Keys are NOT owned. Must check
  * the validity of CentroidInfo.viewObs before dereferencing the key.
  */
  std::map<View*, CentroidInfo> centroids_;
};

}

#endif /* SIMVIS_CENTROIDMANAGER_H */
