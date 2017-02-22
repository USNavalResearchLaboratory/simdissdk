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
#include "osgEarthUtil/LogarithmicDepthBuffer"
#include "simVis/View.h"
#include "simVis/ViewManager.h"
#include "simVis/ViewManagerLogDbAdapter.h"

namespace simVis
{

/** Set the OSG near/far ratio to a small number when LDB is enabled. Otherwise the Triton ocean will get clipped by the near plane */
static const double DEFAULT_NEAR_FAR_RATIO = 0.0005;
static const double LDB_NEAR_FAR_RATIO = 0.000001;

/** Installs on individual views */
class InstallCallback : public ViewManager::Callback
{
public:
  /** Configure with an LDB that gets installed on new views */
  explicit InstallCallback(osgEarth::Util::LogarithmicDepthBuffer* ldb)
    : ldb_(ldb)
  {
  }

  /** Each time view is added or removed, install/uninstall the LDB */
  virtual void operator()(simVis::View* inset, const EventType& e)
  {
    switch (e)
    {
    case VIEW_ADDED:
      ldb_->install(inset->getCamera());
      inset->getCamera()->setNearFarRatio(LDB_NEAR_FAR_RATIO);
      break;
    case VIEW_REMOVED:
      ldb_->uninstall(inset->getCamera());
      inset->getCamera()->setNearFarRatio(DEFAULT_NEAR_FAR_RATIO);
      break;
    }
  }

private:
  /** Pointer to the Log Depth Buffer */
  osgEarth::Util::LogarithmicDepthBuffer* ldb_;
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
  }
  // Remember the manager
  viewManager->addCallback(installCallback_);
  viewManagers_.push_back(viewManager);
}

void ViewManagerLogDbAdapter::uninstall(simVis::ViewManager* viewManager)
{
  // Already installed?
  ViewManagerList::iterator vmlIter = std::find(viewManagers_.begin(), viewManagers_.end(), viewManager);
  if (viewManager == NULL || vmlIter == viewManagers_.end())
    return;

  // Remove it from lists first to avoid callbacks
  viewManager->removeCallback(installCallback_);
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
