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
#ifndef SIMUTIL_RESIZEVIEWMANIPULATOR_H
#define SIMUTIL_RESIZEVIEWMANIPULATOR_H

#include "osg/observer_ptr"
#include "osg/ref_ptr"
#include "osg/Vec2d"
#include "simCore/Common/Export.h"
#include "simVis/View.h"
#include "simUtil/MouseManipulator.h"

namespace osg { class Group; }

namespace simUtil {

/** Mouse manipulator that is responsible for letting the end user resize views. */
class SDKUTIL_EXPORT ResizeViewManipulator : public simUtil::MouseManipulatorAdapter
{
public:
  /** Constructor */
  ResizeViewManipulator(simVis::View* mainView, osg::Group* hudGroup);
  virtual ~ResizeViewManipulator();

  /// Override simUtil::MouseManipulatorAdapter::push()
  virtual int push(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
  /// Override simUtil::MouseManipulatorAdapter::release()
  virtual int release(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
  /// Override simUtil::MouseManipulatorAdapter::move()
  virtual int move(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
  /// Override simUtil::MouseManipulatorAdapter::drag()
  virtual int drag(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

  /// Note that the order matches the entries in the colors_ mask in rubber band
  enum DragPoint
  {
    TOP_LEFT = 0,
    TOP,
    TOP_RIGHT,
    RIGHT,
    BOTTOM_RIGHT,
    BOTTOM,
    BOTTOM_LEFT,
    LEFT,
    CENTER,
    NONE
  };

  /// Changes the flag as to whether this mouse manipulator is active (enabled)
  void setEnabled(bool enabled);
  /// Returns true if this mouse manipulator is active and capturing values
  bool isEnabled() const;
  /// Retrieve the most recent drag point
  DragPoint mostRecentDragPoint() const;

  /// Observer for when the drag point changes.  Use this to implement a mouse cursor change, for example
  class Listener
  {
  public:
    virtual ~Listener() {}
    /** Fired off when the drag point of the resize view manipulator changes */
    virtual void dragPointChanged(DragPoint dragPoint) = 0;
  };

  /// Shared pointer for Listener
  typedef std::tr1::shared_ptr<Listener> ListenerPtr;

  /// Adds a new Listener for notifications
  void addListener(ListenerPtr listener);
  /// Removes a Listener from vector
  void removeListener(ListenerPtr listener);

private:
  /// Drags the current activeView_'s dragPoint_ to the given x/y
  void drag_(const osg::Vec2d& newXY);
  /// Calculates a drag point based on the extents provided and the position provided
  DragPoint calculateDragPoint_(const simVis::View& view, const osg::Vec2d& mouseXY) const;

  /// Converts the extents to a ratio for the provided view
  void toRatioExtents_(const simVis::View& view, simVis::View::Extents& extents) const;
  /// Converts the extents to absolute pixels for the provided view
  void toAbsoluteExtents_(const simVis::View& view, simVis::View::Extents& extents, bool* wasRatio) const;

  /// Changes the internal drag point field, firing off observer as needed
  void setDragPoint_(DragPoint dragPoint);

  bool enabled_;
  osg::observer_ptr<simVis::View> mainView_;
  osg::observer_ptr<osg::Group> hudGroup_;
  class RubberBand;
  RubberBand* rubberBand_;
  osg::observer_ptr<simVis::View> activeView_;
  simVis::View::Extents originalExtents_;
  DragPoint dragPoint_;
  osg::Vec2d anchorMousePosition_;
  std::vector<ListenerPtr> listeners_;
};

}

#endif /* SIMUTIL_RESIZEVIEWMANIPULATOR_H */
