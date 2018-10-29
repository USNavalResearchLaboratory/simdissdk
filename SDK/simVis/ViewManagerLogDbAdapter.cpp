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
#include <algorithm>
#include <cassert>
#include "osgEarthUtil/LogarithmicDepthBuffer"
#include "simVis/View.h"
#include "simVis/ViewManager.h"
#include "simVis/ViewManagerLogDbAdapter.h"

namespace simVis
{

/** Set the OSG near/far ratio to a small number when LDB is enabled. Otherwise the Triton ocean will get clipped by the near plane */
static const double DEFAULT_NEAR_FAR_RATIO = 0.0005;
static const double LDB_NEAR_FAR_RATIO = 0.000001;

/** Minimum near plane distance when the LDB is active */
static const double LDB_MIN_NEAR = 1.0;

/**
 * Update callback for an osg::Camera that will automatically adjust the
 * near/far ratio in order to clamp the near plane to a minimum value.
 */
class ClampNearPlaneCallback : public osg::NodeCallback
{
public:
  ClampNearPlaneCallback(double minNear, double minNearFarRatio)
    : minNear_(minNear),
      minNearFarRatio_(minNearFarRatio)
  {
    //nop
  }

public: // osg::NodeCallback
  /** Override near/far ratio */
  virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
  {
    osg::Camera* camera = static_cast<osg::Camera*>(node);
    double vfov, ar, n, f;
    // Camera might be in ortho mode, would return false
    if (camera->getProjectionMatrixAsPerspective(vfov, ar, n, f) && f != 0.0)
    {
      if (n < minNear_)
        camera->setNearFarRatio(minNear_ / f);
      else if (n / f >= minNearFarRatio_)
        camera->setNearFarRatio(minNearFarRatio_);
    }
    traverse(node, nv);
  }

private:
  double minNear_;
  double minNearFarRatio_;
};

/** Installs on individual views */
class InstallCallback : public ViewManager::Callback
{
public:
  /** Configure with an LDB that gets installed on new views */
  explicit InstallCallback(osgEarth::Util::LogarithmicDepthBuffer* ldb)
    : ldb_(ldb),
      clampNearPlaneCallback_(new ClampNearPlaneCallback(LDB_MIN_NEAR, LDB_NEAR_FAR_RATIO))
  {
  }

  /** Each time view is added or removed, install/uninstall the LDB */
  virtual void operator()(simVis::View* inset, const EventType& e)
  {
    switch (e)
    {
    case VIEW_ADDED:
      ldb_->install(inset->getCamera());
      inset->getCamera()->addUpdateCallback(clampNearPlaneCallback_.get());
      break;
    case VIEW_REMOVED:
      ldb_->uninstall(inset->getCamera());
      inset->getCamera()->setNearFarRatio(DEFAULT_NEAR_FAR_RATIO);
      inset->getCamera()->removeUpdateCallback(clampNearPlaneCallback_.get());
      break;
    }
  }

private:
  /** Pointer to the Log Depth Buffer */
  osgEarth::Util::LogarithmicDepthBuffer* ldb_;
  osg::ref_ptr<ClampNearPlaneCallback> clampNearPlaneCallback_;
};

/////////////////////////////////////////////////////////////////////////

ViewManagerLogDbAdapter::ViewManagerLogDbAdapter()
  : osg::Referenced(),
    logDepthBuffer_(new osgEarth::Util::LogarithmicDepthBuffer),
    installCallback_(new InstallCallback(logDepthBuffer_))
{
  logDepthBuffer_->setUseFragDepth(true);
}

ViewManagerLogDbAdapter::~ViewManagerLogDbAdapter()
{
  // Make a copy to avoid iterator invalidation
  ViewManagerList copy = viewManagers_;
  for (ViewManagerList::const_iterator i = copy.begin(); i != copy.end(); ++i)
    uninstall(i->get());
  // Delete the depth buffer
  delete logDepthBuffer_;
}

void ViewManagerLogDbAdapter::install(simVis::ViewManager* viewManager)
{
  // Already installed?
  if (viewManager == NULL || std::find(viewManagers_.begin(), viewManagers_.end(), viewManager) != viewManagers_.end())
    return;

  // Retrieve the views and install on each
  std::vector<simVis::View*> views;
  viewManager->getViews(views);
  for (std::vector<simVis::View*>::const_iterator i = views.begin(); i != views.end(); ++i)
  {
    logDepthBuffer_->install((*i)->getCamera());
    (*i)->getCamera()->setNearFarRatio(LDB_NEAR_FAR_RATIO);
    (*i)->getCamera()->getOrCreateStateSet()->setDefine("SV_USE_LOG_DEPTH_BUFFER");
  }
  // Remember the manager
  viewManager->addCallback(installCallback_.get());
  viewManagers_.push_back(viewManager);
}

void ViewManagerLogDbAdapter::uninstall(simVis::ViewManager* viewManager)
{
  // Already installed?
  ViewManagerList::iterator vmlIter = std::find(viewManagers_.begin(), viewManagers_.end(), viewManager);
  if (viewManager == NULL || vmlIter == viewManagers_.end())
    return;

  // Remove it from lists first to avoid callbacks
  viewManager->removeCallback(installCallback_.get());
  viewManagers_.erase(vmlIter);

  // Retrieve the views and uninstall on each
  std::vector<simVis::View*> views;
  viewManager->getViews(views);
  for (std::vector<simVis::View*>::const_iterator i = views.begin(); i != views.end(); ++i)
  {
    osg::Camera* camera = (*i)->getCamera();
    if (camera)
    {
      logDepthBuffer_->uninstall(camera);
      camera->setNearFarRatio(DEFAULT_NEAR_FAR_RATIO);
      camera->getOrCreateStateSet()->removeDefine("SV_USE_LOG_DEPTH_BUFFER");
    }
  }
}

bool ViewManagerLogDbAdapter::isInstalled() const
{
  return !viewManagers_.empty();
}

bool ViewManagerLogDbAdapter::isInstalled(const simVis::ViewManager* viewManager) const
{
  return std::find(viewManagers_.begin(), viewManagers_.end(), viewManager) != viewManagers_.end();
}

}
