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
#ifndef SIMVIS_UI_OBJECT_SELECTION_HANDLER_H
#define SIMVIS_UI_OBJECT_SELECTION_HANDLER_H

#include "simCore/Common/Common.h"
#include "osgGA/GUIEventHandler"
#include "osg/View"

namespace simVis
{

/// Event handler for clicking to select an "object" in the scene
class SDKVIS_EXPORT ObjectSelectionHandler : public osgGA::GUIEventHandler
{
public:
  /**
  * Parent for callback to determine whether
  * a particular node qualifies as a "selectable" object.
  */
  class SDKVIS_EXPORT SelectCallback : public osg::Referenced
  {
  public:
    /**
    * Return true if the node in question constituted a selectable object.
    * @note Nodes that pass this test will be sent to select().
    */
    virtual bool isSelectable(const osg::Node &node) const { return true; }

    /// Called with the list of all selected nodes.
    virtual void select(osg::NodeList &selectedNodes) = 0;

  protected:
    /// osg::Referenced-derived
    virtual ~SelectCallback() {}
  };

public:
  /// Action required to cause a selection event to occur
  enum SelectAction
  {
    ACTION_CLICK, ///< User must click the object
    ACTION_HOVER  ///< Mouse cursor is over the object
  };

public:
  /// Construct a new handler for selecting objects
  ObjectSelectionHandler();

  /**
  * Install a callback that determines whether a candidate node qualifies as a
  * "selectable" object. Without this, the class will just return all intersected
  * nodes; so we recommend you use this.
  */
  void setSelectCallback(SelectCallback *callback);

  /// Get the installed acceptance callback, or nullptr if none is installed.
  SelectCallback* getSelectCallback() const;

  /**
  * Set the root node under which object selections should occur. In other words,
  * only search this graph for selectable objects. If you leave this unset the
  * search will take place over the entire scene graph under the View in which the
  * user clicks.
  */
  void setRoot(osg::Node *node);

  /// Get the root node under which object selections occur
  osg::Node* getRoot() const;

  /**
  * Set a traversal mask for locating "objects". If you classify your
  * selectable objects with a particular node mask, you can use this to
  * constrain the scene graph search to only objects with that mask.
  */
  void setObjectTraversalMask(const osg::Node::NodeMask &mask);

  /// Gets the traversal mask for object selection
  const osg::Node::NodeMask& getObjectTraversalMask() const;

  /**
  * Set the input specification for selecting an object.
  * By default, this is set to the left mouse button
  */
  void setSelectionInputMasks(
    const osgGA::GUIEventAdapter::MouseButtonMask& buttons,
    const osgGA::GUIEventAdapter::ModKeyMask& modifierKeys);

  /// Set the user action required to enact a selection
  void setSelectAction(const SelectAction &action);

  /// Get the user action required to enact a selection
  const SelectAction& getSelectAction() const;

  /**
  * Set the delay between the mouse stopping over and object and a selection event
  * occurring, in seconds. Only applicable when hover-to-select is true.
  */
  void setHoverDelaySeconds(double seconds);

  /**
  * Get the delay between the mouse stopping over an object and a selection event
  * occurring, in seconds.
  */
  double getHoverDelaySeconds() const;


public: // osgGA::GUIEventHandler
  /** Detects mouse clicks on entities in order to select the items */
  virtual bool handle(const osgGA::GUIEventAdapter &ev, osgGA::GUIActionAdapter &aa);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "ObjectSelectionHandler"; }

protected:
  /// osg::Referenced-derived
  virtual ~ObjectSelectionHandler() {}

private: // methods
  bool isMouseClick_(const osgGA::GUIEventAdapter &ev);
  bool select_(float mx, float my, osg::View *view);

private: // data
  osg::Node::NodeMask traversalMask_;
  osg::observer_ptr<osg::Node> root_;
  osg::ref_ptr<SelectCallback> acceptor_;
  osg::ref_ptr<const osgGA::GUIEventAdapter> mouseDownEvent_;
  int buttonMask_;
  int modKeyMask_;
  SelectAction action_;
  double hoverDelay_s_;

  enum MouseState
  {
    MOUSE_MOVING,
    MOUSE_HOVERING,
    MOUSE_IDLE
  };
  MouseState mouseState_;
};

}

#endif //SIMVIS_UI_OBJECT_SELECTION_HANDLER_H
