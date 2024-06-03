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
#ifndef SIMVIS_VIEWER_H
#define SIMVIS_VIEWER_H 1

#include "simCore/Common/Common.h"
#include "simVis/View.h"
#include "simVis/ViewManagerLogDbAdapter.h"
#include "simVis/ViewManager.h"

namespace osgEarth
{
  class Map;
  class MapNode;
}

namespace simVis
{
  class SceneManager;

  /**
  * A viewer window that is automatically set up with the standard
  * SIMDIS functionality and navigation controls. This is a "convenience"
  * object that consolidates the ViewManager and SceneManager,
  * automatically generates a default Main View, and installs an Inset
  * view manager. It is appropriate for simple one-window applications.
  *
  * If you have a multi-window application, or are embedding views in a
  * windowing framework (like Qt) you should NOT use this class; you
  * should instead create your own ViewManager and manage the View setup
  * externally. Refer to the example "ExampleQt.cpp" to see how.
  */
  class SDKVIS_EXPORT Viewer : public simVis::ViewManager
  {
  public:
    /**
    * Constructs a new viewer.
    */
    Viewer();

    /**
    * Constructs a new viewer and attempts to configure it based on command-line arguments.
    * @param arguments Command-line arguments from which to initialize
    */
    explicit Viewer(osg::ArgumentParser& arguments);

    /** Enumeration of different window configurations to apply to main view on start-up. */
    enum DefaultScreenSize {
      /// Start in full-screen mode
      FULLSCREEN,
      /// Start in a window
      WINDOWED
    };

    /** Constructs a new viewer with a given starting screen size.  In fullscreen, params are ignored. */
    Viewer(DefaultScreenSize screenSize, int x, int y, int w, int h);

    /**
    * The viewer's main view (created by default)
    * @return The main view attached to this viewer
    */
    simVis::View* getMainView();

    /** @copydoc getMainView() */
    const simVis::View* getMainView() const;

    /**
    * The scene manager attached to this viewer
    * @return Scene manager.
    */
    SceneManager* getSceneManager() { return scene_.get(); }

    /** @copydoc getSceneManager() */
    const SceneManager* getSceneManager() const { return scene_.get(); }

    /**
    * Adds the specified event handler to the default master view, as well as to all inset views.
    * @param handler Event handler to add
    */
    void addGlobalEventHandler(osgGA::GUIEventHandler* handler);

    /**
    * Adds the specified event handler to the default master view.
    * @param handler Event handler to add
    */
    void addEventHandler(osgGA::GUIEventHandler* handler);

    /**
    * Removes the specified event handler from the default master view.
    * @param handler Event handler to remove
    */
    void removeEventHandler(osgGA::GUIEventHandler* handler);

    /**
    * Activates a navigation motion model (see the NavMode inner enum) on
    * the main view.
    * @param navMode Navigation mode to activate.
    */
    void setNavigationMode(const NavMode& navMode);

    /**
    * Installs a set of event handlers for debugging (stats, state set,
    * window control, etc. (convenience function for testing)
    */
    void installDebugHandlers();

    /**
    * Installs the basic event handlers for debugging (stats, state set)
    * Useful for embedded widget viewers, since it does not add window control
    */
    void installBasicDebugHandlers();

    /**
     * Sets a new MapNode on the scene manager and updates all views accordingly.
     * @param[in ] mapNode New MapNode to install
     */
    void setMapNode(osgEarth::MapNode* mapNode);

    /**
     * Sets a new Map on the scene manager and updates all views accordingly.
     * @param[in ] map New Map to install.
     */
    void setMap(osgEarth::Map* map);

    /**
     * Convenience wrapper to configure the number of threads in the database pager.  The
     * database pager shuts down all active threads and starts new ones.  You may call this
     * more than once in an application, but it may be expensive to call.
     * @param totalNumThreads Number of threads for database pager.  See osgDB::DatabasePager::setUpThreads().
     * @param numHttpThreads Number of HTTP threads for database pager.  See osgDB::DatabasePager::setUpThreads().
     */
    void setUpDatabasePagerThreads(unsigned int totalNumThreads, unsigned int numHttpThreads=1);

    /**
     * Convenience wrapper to return current number of database pager threads.  See
     * osgDB::DatabasePager::getNumDatabaseThreads().
     */
    unsigned int getNumDatabasePagerThreads() const;

    /** Configures the Logarithmic Depth Buffer associated with this instance (enabled by default) */
    void setLogarithmicDepthBufferEnabled(bool enabled);

    /** Returns true if the Logarithmic depth buffer is enabled. */
    bool isLogarithmicDepthBufferEnabled() const;

  public: // ViewManager overrides

    /**
     * Run the frame loop continuously.
     */
    virtual int run();

  protected:
    /// osg::Referenced-derived; destructor body needs to be in the .cpp
    virtual ~Viewer();

  private:
    osg::ref_ptr<SceneManager> scene_;
    osg::ref_ptr<ViewManagerLogDbAdapter> logDb_;

    /** Initializes the scene and main view.  Ignores x/y/w/h if screen size is fullscreen */
    void init_(DefaultScreenSize screenSize, int x, int y, int w, int h);
  };
}

#endif /* SIMVIS_VIEWER_H */
