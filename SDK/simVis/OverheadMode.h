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
#ifndef SIMVIS_OVERHEAD_MODE_H
#define SIMVIS_OVERHEAD_MODE_H

#include "osg/Callback"
#include "osgGA/GUIEventHandler"
#include "simCore/Common/Common.h"

namespace osgEarth { class GeoTransform; }

namespace simVis
{

class View;

/**
 * Wraps functionality for adding and manipulating a shader that flattens geometry to
 * the surface, e.g. for overhead mode.
 */
class SDKVIS_EXPORT OverheadMode
{
public:
  /**
   * Installs the overhead mode capability on a view, and initializes it
   * to the disabled state.
   */
  static void install(osg::Node* root);

  /** Uninstalls the overhead mode capability on a view. */
  static void uninstall(osg::Node* root);

  /** Enable or disable overhead mode. */
  static void setEnabled(bool value, simVis::View* view);

  /**
   * Override overhead mode on a particular node. For example, you can enable
   * override mode on a view with setEnabled(true), and then disable the shader
   * on a specific node with this call. Good for labels, for example.
   */
  static void enableGeometryFlattening(bool value, osg::Node* node);

  /**
   * The earth-radius at which goemetry is drawing in overhead mode. Static.
   * "z" is the Z component of a unit UP vector; i.e. sin(latitude)
   */
  static double getClampingRadius(double z);

  /**
   * Causes a GeoTransform node to clamp its altitude to zero if the
   * active nodevisitor has overhead mode enabled.
   */
  static void enableGeoTransformClamping(bool value, osgEarth::GeoTransform* xform);

  /** Retrieve the earth-radius at which goemetry is drawing in overhead mode. Static. (meters) */
  static double getClampingRadius();

  /** Returns whether overhead mode is activate in the specified traversal */
  static bool isActive(osg::NodeVisitor* nv);

  /**
   * Prepares a NodeVisitor with the current status of overhead mode.
   * A NodeVisitor needs to know whether overhead mode is enabled, because some
   * components depend on this information (e.g. LocatorNode). The IndicatorCallback
   * below automatically does this for cull visitors, but for other types we have to
   * do it manually by calling this method. (For an intersection visitor, for example)
   */
  static void prepareVisitor(const View* view, osg::NodeVisitor* nv);

public:
  /**
   * Cull callback that will indicate whether overhead mode is active
   * for the subsequent scene graph cull traversal. The mere presence
   * of the callback is sufficient to indicate that overhead mode is ON.
   */
  class IndicatorCallback : public osg::NodeCallback
  {
  public:
    /** Constructs a new IndicatorCallback */
    IndicatorCallback();

    /** Changes whether children of attached node should have overhead processing enabled. */
    void setEnabled(bool value);

    /** Callback operator will configure the appropriate overhead mode token */
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

  protected:
    /** osg::Referenced-derived has protected destructor */
    virtual ~IndicatorCallback();

  private:
    bool enabled_;
  };
};

/**
 * Convenience event handler for toggling overhead mode with a keypress.
 */
class SDKVIS_EXPORT ToggleOverheadMode : public osgGA::GUIEventHandler
{
public:
  /** Constructs a new ToggleOverheadMode that switches overhead mode on the given key press. */
  ToggleOverheadMode(simVis::View* view, int overheadKey, int toggleClampKey);

  /** Changes the hotkey for toggling overhead */
  void setOverheadKey(int key);
  /** Changes the hotkey for toggling clamping in overhead mode */
  void setToggleClampingKey(int key);

public: // osgGA::GUIEventHandler
  /** Toggle overhead mode if key is pressed */
  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

protected:
  /** osg::Referenced-derived has protected destructor */
  virtual ~ToggleOverheadMode();

private:
  osg::observer_ptr<simVis::View> view_;
  int overheadKey_;
  int toggleClampKey_;
};

}

#endif // SIMVIS_OVERHEAD_MODE_H
