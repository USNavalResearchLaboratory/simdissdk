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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osg/OperationThread"
#include "osgGA/StateSetManipulator"
#include "osgViewer/ViewerEventHandlers"
#include "simVis/CentroidManager.h"
#include "simVis/SceneManager.h"
#include "simVis/Utils.h"
#include "simVis/Viewer.h"

namespace simVis
{

Viewer::Viewer()
{
  init_(WINDOWED, 100, 100, 1024, 768);
}

Viewer::Viewer(osg::ArgumentParser& parser)
  : simVis::ViewManager(parser)
{
  init_(WINDOWED, 100, 100, 1024, 768);
}

Viewer::Viewer(DefaultScreenSize screenSize, int x, int y, int w, int h)
{
  init_(screenSize, x, y, w, h);
}

Viewer::~Viewer() {}

void Viewer::init_(DefaultScreenSize screenSize, int x, int y, int w, int h)
{
  // create a scene manager that all the views will share.
  scene_ = new SceneManager();
  scene_->setName("Scene Manager");
  // Logarithmic depth buffer managing view depth buffer settings
  logDb_ = new ViewManagerLogDbAdapter;
  setLogarithmicDepthBufferEnabled(true);

  // start by adding a default Main view.
  simVis::View* mainView = new simVis::View();
  addView(mainView);
  mainView->setName("Main View");

  // Set up in a window if asked by environment variable
  const char* win = ::getenv("OSG_WINDOW");
  if (win)
  {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    std::istringstream iss(win);
    iss >> x >> y >> width >> height;
    if (!iss.fail() && width > 0 && height > 0)
      mainView->setUpViewInWindow(x, y, width, height, 0u);
  }

  // Apply the windowing request as long as OSG_WINDOW did not override
  if (!mainView->getCamera()->getViewport())
  {
    if (screenSize == FULLSCREEN)
      mainView->setUpViewOnSingleScreen();
    else // screenSize == WINDOWED
      mainView->setUpViewInWindow(x, y, w, h, 0u);
  }

  mainView->setSceneManager(scene_.get());

  // by default, the database pager unreferenced image objects once it downloads them
  // the driver. In composite viewer mode we don't want that since we may be
  // adding and removing views.
  mainView->getScene()->getDatabasePager()->setUnrefImageDataAfterApplyPolicy(true, false);
}

void Viewer::setMapNode(osgEarth::MapNode* mapNode)
{
  // assign it to the scene manager:
  scene_->setMapNode(mapNode);

  // update each of the views' camera manipulator:
  std::vector<simVis::View*> views;
  getViews(views);
  for (std::vector<simVis::View*>::const_iterator i = views.begin(); i != views.end(); ++i)
  {
    // Each view in the views list should be valid and non-nullptr
    assert(*i);
    if (*i == nullptr)
      continue;

    // Not every view necessarily has a camera manipulator (e.g. SuperHud doesn't)
    osgGA::CameraManipulator* manip = (*i)->getCameraManipulator();
    if (manip)
    {
      // Changing the manipulator attach point will reset the view; save and restore the view.
      simVis::Viewpoint vp = (*i)->getViewpoint();
      manip->setNode(nullptr);
      manip->setNode(scene_->getManipulatorAttachPoint());
      (*i)->setViewpoint(vp, 0);
    }
  }
}

void Viewer::setMap(osgEarth::Map* map)
{
  // assign it to the scene manager:
  scene_->setMap(map);
}

int Viewer::run()
{
  bool hasManip = getMainView()->getCameraManipulator() != nullptr;
  osg::Matrix savedViewMatrix;

  // do some final set up before running the frame loop.
  if (scene_.valid())
  {
    if (hasManip)
    {
      Viewpoint saveVP = getMainView()->getViewpoint();
      getMainView()->getCameraManipulator()->setNode(scene_->getManipulatorAttachPoint());
      getMainView()->setViewpoint(saveVP);
    }
  }

  savedViewMatrix = getMainView()->getCamera()->getViewMatrix();

  // temporary
  // getMainView()->setUpViewInWindow(30, 30, 1280, 800, 0);

  if (!hasManip)
  {
    getMainView()->setCameraManipulator(nullptr);
    getMainView()->getCamera()->setViewMatrix(savedViewMatrix);
  }

  // install a persistent update operation that will run the inset manager's
  // update operation.
  //getViewer()->addUpdateOperation( new RunInsetManagerUpdateOperations(insetMan_.get()) );

  return ViewManager::run();
}

View* Viewer::getMainView()
{
  return getNumViews() > 0 ? getView(0) : nullptr;
}

const View* Viewer::getMainView() const
{
  return getNumViews() > 0 ? getView(0) : nullptr;
}

void Viewer::addEventHandler(osgGA::GUIEventHandler* handler)
{
  getMainView()->addEventHandler(handler);
}

void Viewer::removeEventHandler(osgGA::GUIEventHandler* handler)
{
  getMainView()->removeEventHandler(handler);
}

void Viewer::addGlobalEventHandler(osgGA::GUIEventHandler* handler)
{
//  insetMan_->addEventHandler(handler);
  getMainView()->addEventHandler(handler);
}

void Viewer::setNavigationMode(const NavMode& mode)
{
  // update each of the views' camera manipulator:
  std::vector<simVis::View*> views;
  this->getViews(views);

  for (std::vector<simVis::View*>::const_iterator i = views.begin(); i != views.end(); ++i)
  {
    (*i)->setNavigationMode(mode);
  }
}

void Viewer::installDebugHandlers()
{
  installBasicDebugHandlers();
  addEventHandler(new osgViewer::WindowSizeHandler());
}

void Viewer::installBasicDebugHandlers()
{
  osgViewer::StatsHandler* stats = new osgViewer::StatsHandler();
  stats->getCamera()->setAllowEventFocus(false);
  // Fix blocky text in Stats Handler from the shader program issues with text in OSG 3.4.1
  simVis::fixStatsHandlerGl2BlockyText(stats);

  addEventHandler(stats);
  addEventHandler(new osgGA::StateSetManipulator(getMainView()->getCamera()->getOrCreateStateSet()));
}

void Viewer::setUpDatabasePagerThreads(unsigned int totalNumThreads, unsigned int numHttpThreads)
{
  osg::observer_ptr<simVis::View> mainView = getMainView();
  if (mainView.valid())
    mainView->getDatabasePager()->setUpThreads(totalNumThreads, numHttpThreads);
}

unsigned int Viewer::getNumDatabasePagerThreads() const
{
  osg::observer_ptr<const simVis::View> mainView = getMainView();
  if (mainView.valid())
    return mainView->getDatabasePager()->getNumDatabaseThreads();
  return 0;
}

void Viewer::setLogarithmicDepthBufferEnabled(bool enabled)
{
  if (enabled)
    logDb_->install(this);
  else
    logDb_->uninstall(this);
}

bool Viewer::isLogarithmicDepthBufferEnabled() const
{
  return logDb_->isInstalled(this);
}

}
