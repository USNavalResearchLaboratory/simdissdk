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
#ifndef SIMVIS_UI_NAVIGATION_MODES
#define SIMVIS_UI_NAVIGATION_MODES 1

#include "simCore/Common/Common.h"
#include "simVis/BoxZoomMouseHandler.h"

namespace simVis
{
class View;

  /**
  * A NavigationMode is used to alter the way the mouse interacts with the
  * SIMDIS globe.  It inherits from the EarthManipulator Settings class;
  * as such, navigation modes are applied by passing them to
  * EarthManipulator::applySettings.
  * The contained ActionOptions subclasses are options which are preloaded
  * with mouse tuning options to mimic the mouse control feel for several
  * mouse actions of the SIMDIS application.
  */
  class SDKVIS_EXPORT NavigationMode : public osgEarth::Util::EarthManipulator::Settings
  {
  protected:
    /**
    * PanOptions is created with mouse action options that are meant to
    * mimic the feel of mouse panning in the original SIMDIS application.
    * Pass this as the options parameter in the bindMouse method for
    * pan actions.
    * Panning should only pan the screen up, down, left, or right at a
    * single time.
    */
    class PanOptions : public osgEarth::Util::EarthManipulator::ActionOptions
    {
    public:
      /**
      * Initialize panning to be a continuous operation in order to
      * mimic SIMDIS panning.
      */
      PanOptions();
    };

    /**
    * RotateOptions is created with mouse action options that are meant to
    * mimic the feel of mouse rotation in the original SIMDIS application.
    * Pass this as the options parameter in the bindMouse method for
    * rotate actions.
    * Rotation should only rotate the screen up, down, left, or right at
    * a single time.
    */
    class RotateOptions : public osgEarth::Util::EarthManipulator::ActionOptions
    {
    public:
      /**
      * Initialize rotation to be a continuous operation in order to
      * mimic SIMDIS rotation.
      */
      RotateOptions();
    };

    /**
    * ContinuousZoomOptions is created with mouse action options that are
    * meant to mimic the feel of continuous mouse zoom in the original
    * SIMDIS application.
    * Pass this as the options parameter in the bindMouse method for
    * zoom actions that should be continuous.
    * Continuous zoom should zoom in when the mouse is above its original
    * click position and zoom out when the mouse is below.
    */
    class ContinuousZoomOptions : public osgEarth::Util::EarthManipulator::ActionOptions
    {
    public:
      /**
      * Initialize zoom to be a continuous operation in order to
      * mimic SIMDIS continuous zoom.
      */
      ContinuousZoomOptions();
    };

    /**
    * FixedZoomOptions is created with mouse action options that are
    * meant to mimic the feel of fixed mouse zoom in the original SIMDIS
    * application.
    * Pass this as the options parameter in the bindMouse method for
    * zoom actions that should be fixed.
    * Fixed zoom generally maps to the mouse wheel (scrolling up will
    * zoom in, scrolling down will zoom out).
    */
    class FixedZoomOptions : public osgEarth::Util::EarthManipulator::ActionOptions
    {
    public:
      /**
      * Initialize zoom to mimic SIMDIS fixed zoom.
      */
      FixedZoomOptions();
    };

    /**
    * FixedZoomOptions is created with mouse action options that are
    * meant to mimic the feel of incremental mouse zoom in the original
    * SIMDIS application.
    * Pass this as the options parameter in the bindMouse method for
    * incremental zoom actions.
    * Incremental fixed zoom, like regular fixed zoom, generally maps
    * to the mouse wheel, but zooms in and out in smaller increments.
    */
    class IncrementalFixedZoomOptions : public osgEarth::Util::EarthManipulator::ActionOptions
    {
    public:
      /**
      * Initialize zoom to mimic SIMDIS incremental zoom.
      */
      IncrementalFixedZoomOptions();
    };

    /**
    * GoToOptions is created with mouse action options that are
    * meant to mimic the feel of the go to mouse action in the original
    * SIMDIS application.
    * Pass this as the options parameter in the bindMouse(Double)Click
    * method for go to actions.
    */
    class GoToOptions : public osgEarth::Util::EarthManipulator::ActionOptions
    {
    public:
      /**
      * Initialize go to speed to mimic SIMDIS go to.
      */
      GoToOptions();
    };

    NavigationMode();

    /// osg::Referenced-derived
    virtual ~NavigationMode();

    /** Adds default bindings for multi-touch */
    void bindMultiTouch_(bool canZoom, bool canRotate);
  };

  /**
  * RotatePanNavigationMode maps rotation/panning to the left mouse
  * button.  This is the default mouse mode of SIMDIS.
  * Pass an instance of RotatePanNavigationMode to
  * EarthManipulator::applySettings in order to enable the rotate/pan
  * navigation mode.<p>
  * The mappings are:
  * <ul>
  * <li>Left mouse button: rotation in perspective, panning in overhead</li>
  * <li>Middle mouse button: continuous zoom</li>
  * <li>Right mouse button: globe spin</li>
  * <li>Left mouse button + alt: box zoom</li>
  * <li>Left mouse button + ctrl: center view</li>
  * <li>Left mouse button + shift: globe spin</li>
  * <li>Double-click left mouse button: center view</li>
  * <li>Scroll wheel: fixed zoom</li>
  * <li>Scroll wheel + alt: incremental fixed zoom</li>
  * <li>Arrow keys: fixed rotate</li>
  * </ul>
  */
  class SDKVIS_EXPORT RotatePanNavigationMode : public NavigationMode
  {
  public:
    /**
    * Initialize the rotate/pan navigation mode.
    * @param view Inset associated with nav mode, used for box zoom key
    * @param enableOverhead true to create the navigation mode with
    * overhead enabled, false to create with perspective mode
    * @param watchMode if true, no rotate actions will be applied, as
    * rotation is disabled in watchMode
    */
    RotatePanNavigationMode(simVis::View* view, bool enableOverhead, bool watchMode);

  protected:
    /// osg::Referenced-derived
    virtual ~RotatePanNavigationMode();

  private:
    /** Initializes the bindings given the parameters. */
    void init_(simVis::View* view, bool enableOverhead, bool watchMode);

    osg::observer_ptr<View> view_;
    osg::ref_ptr<BoxZoomMouseHandler> boxZoom_;
  };

  /**
  * GlobeSpinNavigationMode maps globe grabbing/spinning to the left
  * mouse button.
  * Pass an instance of GlobeSpinNavigationMode to
  * EarthManipulator::applySettings in order to enable the globe spin
  * navigation mode.<p>
  * The mappings are:
  * <ul>
  * <li>Left mouse button: globe spin</li>
  * <li>Middle mouse button: continuous zoom</li>
  * <li>Right mouse button: rotation in perspective, panning in overhead</li>
  * <li>Left mouse button + ctrl: center view</li>
  * <li>Left mouse button + shift: rotation in perspective, panning in overhead</li>
  * <li>Double-click left mouse button: center view</li>
  * <li>Scroll wheel: fixed zoom</li>
  * <li>Scroll wheel + alt: incremental fixed zoom</li>
  * <li>Arrow keys: fixed rotate</li>
  * </ul>
  */
  class SDKVIS_EXPORT GlobeSpinNavigationMode : public NavigationMode
  {
  public:
    /**
    * Initialize the globe spin navigation mode.
    * @param enableOverhead true to create the navigation mode with
    * overhead enabled, false to create with perspective mode
    * @param watchMode if true, no rotate actions will be applied, as
    * rotation is disabled in watchMode
    */
    GlobeSpinNavigationMode(bool enableOverhead, bool watchMode);

  protected:
    /// osg::Referenced-derived
    virtual ~GlobeSpinNavigationMode();
  };

  /**
  * ZoomNavigationMode maps continuous zoom to the left mouse button.
  * Pass an instance of ZoomNavigationMode to
  * EarthManipulator::applySettings in order to enable the continuous
  * zoom navigation mode.<p>
  * The mappings are:
  * <ul>
  * <li>Left mouse button: continuous zoom</li>
  * <li>Middle mouse button: continuous zoom</li>
  * <li>Right mouse button: continuous zoom</li>
  * <li>Left mouse button + ctrl: center view</li>
  * <li>Left mouse button + shift: rotation in perspective, panning in overhead</li>
  * <li>Double-click left mouse button: center view</li>
  * <li>Scroll wheel: fixed zoom</li>
  * <li>Scroll wheel + alt: incremental fixed zoom</li>
  * <li>Arrow keys: fixed rotate</li>
  * </ul>
  */
  class SDKVIS_EXPORT ZoomNavigationMode : public NavigationMode
  {
  public:
    /**
    * Initialize the zoom navigation mode.
    * @param enableOverhead true to create the navigation mode with
    * overhead enabled, false to create with perspective mode
    * @param watchMode if true, no rotate actions will be applied, as
    * rotation is disabled in watchMode
    */
    ZoomNavigationMode(bool enableOverhead, bool watchMode);

  protected:
    /// osg::Referenced-derived
    virtual ~ZoomNavigationMode();
  };

  /**
  * CenterViewNavigationMode maps centering to the left mouse button.
  * Pass an instance of CenterViewNavigationMode to
  * EarthManipulator::applySettings in order to enable the centering
  * navigation mode.<p>
  * The mappings are:
  * <ul>
  * <li>Left mouse button: center view</li>
  * <li>Middle mouse button: continuous zoom</li>
  * <li>Right mouse button: center view</li>
  * <li>Left mouse button + ctrl: center view</li>
  * <li>Left mouse button + shift: rotation in perspective, panning in overhead</li>
  * <li>Double-click left mouse button: center view</li>
  * <li>Scroll wheel: fixed zoom</li>
  * <li>Scroll wheel + alt: incremental fixed zoom</li>
  * <li>Arrow keys: fixed rotate</li>
  * </ul>
  */
  class SDKVIS_EXPORT CenterViewNavigationMode : public NavigationMode
  {
  public:
    /**
    * Initialize the center view navigation mode.
    * @param enableOverhead true to create the navigation mode with
    * overhead enabled, false to create with perspective mode
    * @param watchMode if true, no rotate actions will be applied, as
    * rotation is disabled in watchMode
    */
    CenterViewNavigationMode(bool enableOverhead, bool watchMode);

  protected:
    /// osg::Referenced-derived
    virtual ~CenterViewNavigationMode();
  };

  /**
  * GisNavigationMode provides a similar navigation mode to other non-SIMDIS GIS software.
  * Pass an instance of GisNavigationMode to
  * EarthManipulator::applySettings in order to enable the centering
  * navigation mode.<p>
  * The mappings are:
  * <ul>
  * <li>Left mouse button: pan</li>
  * <li>Left mouse button + shift: rotation</li>
  * <li>Left mouse button + alt: box zoom</li>
  * <li>Middle mouse button: rotation</li>
  * <li>Right mouse button: zoom</li>
  * <li>Double-click left mouse button: center and zoom in</li>
  * <li>Double-click right mouse button: center and zoom out</li>
  * <li>Scroll wheel: zoom</li>
  * <li>Arrow/WASD keys: pan</li>
  * <li>Arrow/WASD keys + alt: pan, slower</li>
  * <li>Arrow/WASD keys + shift: rotate</li>
  * </ul>
  */
  class SDKVIS_EXPORT GisNavigationMode : public NavigationMode
  {
  public:
    /**
    * Initialize the GIS navigation mode.
    * @param view Inset associated with nav mode, used for box zoom key
    * @param enableOverhead true to create the navigation mode with
    * overhead enabled, false to create with perspective mode
    * @param watchMode if true, no rotate actions will be applied, as
    * rotation is disabled in watchMode
    */
    GisNavigationMode(simVis::View* view, bool enableOverhead, bool watchMode);

  protected:
    /// osg::Referenced-derived
    virtual ~GisNavigationMode();

  private:
    /** Initializes the bindings given the parameters. */
    void init_(simVis::View* view, bool enableOverhead, bool watchMode);

    osg::observer_ptr<View> view_;
    osg::ref_ptr<BoxZoomMouseHandler> boxZoom_;
  };

  /**
  * BuilderNavigationMode provides a similar navigation mode to Builder.
  * Pass an instance of BuilderNavigationMode to
  * EarthManipulator::applySettings in order to enable this navigation mode.<p>
  * The mappings are:
  * <ul>
  * <li>Left mouse button: pan</li>
  * <li>Left mouse button + shift: rotation</li>
  * <li>Middle mouse button: zoom</li>
  * <li>Right mouse button: rotation</li>
  * <li>Scroll wheel: zoom</li>
  * <li>Arrow keys: pan</li>
  * <li>Arrow keys + ctrl: zoom</li>
  * <li>Arrow keys + ctrl + shift: rotate</li>
  * </ul>
  */
  class SDKVIS_EXPORT BuilderNavigationMode : public NavigationMode
  {
  public:
    /**
    * Initialize the Builder navigation mode.
    * @param enableOverhead true to create the navigation mode with
    * overhead enabled, false to create with perspective mode
    * @param watchMode if true, no rotate actions will be applied, as
    * rotation is disabled in watchMode
    */
    BuilderNavigationMode(bool enableOverhead, bool watchMode);

  protected:
    /// osg::Referenced-derived
    virtual ~BuilderNavigationMode();

  private:
    /** Initializes the bindings given the parameters. */
    void init_(bool enableOverhead, bool watchMode);
  };

  /**
  * NgtsNavigationMode provides a similar navigation mode to Next Generation Threat System (NGTS).
  * Pass an instance of NgtsNavigationMode to EarthManipulator::applySettings in order to enable
  * this navigation mode.<p>
  * The mappings are nearly identical to RotatePanNavigationMode with following notable exceptions:
  * <ul>
  * <li>Left mouse button: pan in overhead and perspective mode</li>
  * <li>Right mouse button: zoom in overhead, rotate in perspective</li>
  * </ul>
  */
  class SDKVIS_EXPORT NgtsNavigationMode : public RotatePanNavigationMode
  {
  public:
    /**
    * Initialize the NGTS navigation mode.
    * @param enableOverhead true to create the navigation mode with
    * overhead enabled, false to create with perspective mode
    * @param watchMode if true, no rotate actions will be applied, as
    * rotation is disabled in watchMode
    */
    NgtsNavigationMode(simVis::View* view, bool enableOverhead, bool watchMode);

  protected:
    /// osg::Referenced-derived
    virtual ~NgtsNavigationMode();

  private:
    /// Initialize the mouse for overhead mode usage (not in watch mode)
    void initOverhead_();
    /// Initialize the mouse for perspective mode usage (not in watch mode)
    void initPerspective_();
  };

}

#endif // SIMVIS_UI_NAVIGATION_MODES
