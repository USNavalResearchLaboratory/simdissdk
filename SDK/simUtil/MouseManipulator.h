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
#ifndef SIMUTIL_MOUSEMANIPULATOR_H
#define SIMUTIL_MOUSEMANIPULATOR_H

#include <memory>
#include "osg/ref_ptr"
#include "simCore/Common/Common.h"

namespace osgGA {
  class GUIActionAdapter;
  class GUIEventAdapter;
  class GUIEventHandler;
}

namespace simUtil {

/**
 * Strategy for dealing with mouse clicks, pushes, etc.  Used by the
 * simUtil::MouseDispatcher code as a strategy, with the MouseHandler as
 * the context.
 *
 * Implement your own MouseManipulator to provide a method for dealing with
 * mouse clicks, to integrate with priority into a MouseDispatcher.  If you
 * don't care about priority integration, then an osgGA::GUIEventHandler
 * might be more appropriate.
 */
class SDKUTIL_EXPORT MouseManipulator
{
public:
  virtual ~MouseManipulator() {}

  /** Mouse button pushed, returns non-zero on handled */
  virtual int push(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) = 0;
  /** Mouse button released, returns non-zero on handled */
  virtual int release(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) = 0;
  /** Mouse being moved, returns non-zero on handled */
  virtual int move(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) = 0;
  /** Mouse being dragged, returns non-zero on handled */
  virtual int drag(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) = 0;
  /** Mouse button double clicked, returns non-zero on handled */
  virtual int doubleClick(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) = 0;
  /** Mouse wheel scrolled, returns non-zero on handled */
  virtual int scroll(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) = 0;
  /** Frame event, returns non-zero on handled */
  virtual int frame(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) = 0;

  /** Touch event started, returns non-zero on handled */
  virtual int touchBegan(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) = 0;
  /** Touch event updated with coordinates moved, returns non-zero on handled */
  virtual int touchMoved(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) = 0;
  /** Touch event ended, returns non-zero on handled */
  virtual int touchEnded(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) = 0;

  /**
   * Called by the MouseDispatcher when the mouse manipulator is activated.  This is useful
   * for mutually exclusive mouse manipulators only.  Only one mutually exclusive mouse
   * manipulator is active at a time for receiving events.  Override this method to know
   * when the dispatcher activates this mouse manipulator.  Note that mutually exclusive
   * mouse manipulators must be explicitly activated and start in a deactivated state.
   */
  virtual void activate() = 0;
  /**
   * Called by the MouseDispatcher when the mouse manipulator is deactivated.  This is
   * useful for mutually exclusive mouse manipulators only.  Override this method to know
   * when the dispatcher deactivates this mouse manipulator.
   */
  virtual void deactivate() = 0;
};
typedef std::shared_ptr<MouseManipulator> MouseManipulatorPtr;


/**
 * Adapter for the MouseManipulator class that serves as a pass-through.  You can derive
 * from this class if you only plan to implement a couple of methods. By default, touch
 * events are processed as left mouse button push, drag, and release events. You can override
 * this behavior by either changing setTouchEmulatesMouse(false), and/or overriding the
 * touchBegan()/touchMoved()/touchEnded() routines.
 */
class SDKUTIL_EXPORT MouseManipulatorAdapter : public MouseManipulator
{
public:
  /** Instantiate this adapter around the provided GUIEventHandler. This may be nullptr. */
  explicit MouseManipulatorAdapter(osgGA::GUIEventHandler* handler=nullptr, bool touchEmulatesMouse=true);
  virtual ~MouseManipulatorAdapter();

  // From MouseManipulator:
  virtual int push(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int release(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int move(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int drag(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int doubleClick(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int scroll(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int frame(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int touchBegan(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int touchMoved(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int touchEnded(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual void activate() override;
  virtual void deactivate() override;

  /** Retrieves the underlying GUI Event Handler that, if non-nullptr, defines the default behavior for adapter. */
  osgGA::GUIEventHandler* handler() const;
  /** Changes the underlying GUI Event Handler used for default behavior in the adapter. */
  void setHandler(osgGA::GUIEventHandler* handler);

  /** Changes whether touch events simulate mouse events */
  void setTouchEmulatesMouse(bool emulateMouse);
  /** Returns true if touch simulates mouse events */
  bool touchEmulatesMouse() const;

private:
  // Not implemented
  MouseManipulatorAdapter(const MouseManipulatorAdapter& rhs);

  osg::ref_ptr<osgGA::GUIEventHandler> handler_;
  bool touchEmulatesMouse_;
};


/** Utility template class that applies a setEnabled() on activate and deactive. */
template <typename T>
class /* HEADER-ONLY */ MouseManipulatorExclusiveAdapter : public MouseManipulatorAdapter
{
public:
  explicit MouseManipulatorExclusiveAdapter(T* guiEventAdapter)
    : MouseManipulatorAdapter(guiEventAdapter)
  {
  }

  // From MouseManipulatorAdapter:
  virtual void activate() override
  {
    static_cast<T*>(handler())->setEnabled(true);
  }
  virtual void deactivate() override
  {
    static_cast<T*>(handler())->setEnabled(false);
  }
};


/** Proxy implementation of the MouseManipulator that forwards commands to another instance. */
class SDKUTIL_EXPORT MouseManipulatorProxy : public MouseManipulator
{
public:
  /** Instantiate with no real subject */
  MouseManipulatorProxy();
  /** Instantiate with a pointer to the real subject */
  explicit MouseManipulatorProxy(const MouseManipulatorPtr& realManipulator);

  // From MouseManipulator:
  virtual int push(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int release(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int move(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int drag(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int doubleClick(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int scroll(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int frame(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int touchBegan(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int touchMoved(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual int touchEnded(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override;
  virtual void activate() override;
  virtual void deactivate() override;

  /** Retrieves the real subject of the Proxy */
  MouseManipulatorPtr subject() const;
  /** Changes the subject of the Proxy */
  void setSubject(MouseManipulatorPtr manipulator);

private:
  MouseManipulatorPtr manipulator_;
};

}

#endif /* SIMUTIL_MOUSEMANIPULATOR_H */
