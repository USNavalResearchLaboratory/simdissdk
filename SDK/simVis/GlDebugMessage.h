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
#ifndef SIMVIS_GLDEBUGMESSAGE_H
#define SIMVIS_GLDEBUGMESSAGE_H

#include <vector>
#include "osg/GL"
#include "osg/observer_ptr"
#include "osg/ref_ptr"
#include "osg/StateAttribute"
#include "simCore/Common/Export.h"

namespace osg {
  class GraphicsContext;
  class StateSet;
}

namespace simVis {

/** Provides an interface for listening glDebugMessageCallback() messages.  Use this with GlDebugMessage class. */
class GlDebugMessageCallback : public osg::Referenced
{
public:
  /**
   * Override this method to be notified when a message is generated.
   * @param source GL_DEBUG_SOURCE_* constant, such as GL_DEBUG_SOURCE_SHADER_COMPILER.
   * @param type GL_DEBUG_TYPE_* constant, such as GL_DEBUG_TYPE_ERROR.
   * @param id ID associated with the message, a user-supplied identifier from glDebugMessageInsert().
   * @param severity GL_DEBUG_SEVERITY_* constant, such as GL_DEBUG_SEVERITY_MEDIUM.
   * @param message Text message describing the error.
   */
  virtual void handleMessage(GLenum source, GLenum type, GLuint id,
    GLenum severity, const std::string& message) = 0;

protected:
  virtual ~GlDebugMessageCallback() {}
};

/**
 * Manager for GL Debug Message output.  Responsible for registering as the debug message handler
 * if so configured, and forwards messages to listener callbacks registered through addCallback().
 * Note that as per OpenGL spec, there are several requirements to get GL messages:
 *
 * 1) GL Debug Output needs to be supported, which is driver-specific.
 * 2) GL_DEBUG_OUTPUT mode needs to be enabled.
 * 3) A callback needs to be registered with OpenGL.
 * 4) Messages have to be generated (driver-specific).
 *
 * This class helps with steps 1-3 by providing a way to determine if GL debug message output is
 * possible (through the attempted retrieval of function points in GL library), and providing
 * functions to control message registration.
 *
 * One method of using this class is to register for messages immediately on graphcis context
 * realization, such as to track down errors reported by OSG.  To do this, add code like the
 * following to your OnRealize method (like the one in ViewManager.cpp) and set a breakpoint
 * as needed in the callback:
 *
 * <code>
 * GlDebugMessage* manager = new GlDebugMessage(graphicsContext);
 * manager->registerCallbacks(true);
 * manager->setDebugOutputMode(true);
 * manager->addCallback(new CerrDebugMessageCallback(CerrDebugMessageCallback::SEVERITY_LOW));
 * </code>
 */
class SDKVIS_EXPORT GlDebugMessage
{
public:
  explicit GlDebugMessage(osg::GraphicsContext* gc);
  virtual ~GlDebugMessage();

  /** Change the effective graphics context, removing callbacks from the previous GC if valid. */
  void setGraphicsContext(osg::GraphicsContext* gc);

  /** Call to register this class for callbacks with graphics context.  Note, this is independent of GL_DEBUG_OUTPUT. */
  void registerCallbacks(bool registerWithGl);

  /** Equivalent to stateSet.setMode(GL_DEBUG_OUTPUT, mode).  Note GL_DEBUG_OUTPUT is off by default. */
  void setDebugOutputMode(osg::StateSet& stateSet, osg::StateAttribute::GLMode mode) const;
  /** Equivalent to glEnable(GL_DEBUG_OUTPUT) or glDisable(GL_DEBUG_OUTPUT), without a stateset. */
  void setDebugOutputMode(bool enabled) const;

  /** Adds a callback for messages, invoked when registerCallbacks(true) is called, GL_DEBUG_OUTPUT is set, and there is a message. */
  void addCallback(GlDebugMessageCallback* callback);
  /** Removes a callback from this manager. */
  void removeCallback(GlDebugMessageCallback* callback);

  /** Convenience wrapper around glDebugMessageControl(); returns non-zero if unsupported. */
  int messageControl(GLenum source, GLenum type, GLenum severity, const std::vector<GLuint>& ids, bool enabled);
  /** Convenience wrapper around glDebugMessageInsert(); returns non-zero if unsupported. */
  int insertMessage(GLenum source, GLenum type, GLuint id, GLenum severity, const std::string& message);

private:
  /** Static method that is called directly by OpenGL for the debug message procedure. */
  static void processMessage_(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei len, const GLchar* msg, const void* userData);

  /** Alert all callbacks of the message. */
  void publishMessage_(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei len, const GLchar* msg) const;

  std::vector<osg::ref_ptr<GlDebugMessageCallback> > callbacks_;
  osg::observer_ptr<osg::GraphicsContext> gc_;
  bool register_;

  typedef void (APIENTRYP PFNGLDEBUGMESSAGECONTROLPROC) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
  typedef void (APIENTRYP PFNGLDEBUGMESSAGEINSERTPROC) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf);
  typedef void (APIENTRYP PFNGLDEBUGMESSAGECALLBACKPROC) (GLDEBUGPROC callback, const void* userParam);

  PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl_;
  PFNGLDEBUGMESSAGEINSERTPROC glDebugMessageInsert_;
  PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback_;
};

/** Prints GL messages to std::cerr */
class SDKVIS_EXPORT CerrDebugMessageCallback : public GlDebugMessageCallback
{
public:
  /** Ascending order of severity */
  enum Severity {
    SEVERITY_NOTIFICATION,
    SEVERITY_LOW,
    SEVERITY_MEDIUM,
    SEVERITY_HIGH
  };

  /** Initialize with minimum severity, such as GL_DEBUG_SEVERITY_MEDIUM. */
  explicit CerrDebugMessageCallback(Severity minSeverity = SEVERITY_LOW);

  /** Sets the minimum severity allowed for printing to std::cerr. */
  void setMinimumSeverity(Severity minSeverity);

  /** Implementation of handleMessage() that filters based on severity and prints to std::cerr. */
  virtual void handleMessage(GLenum source, GLenum type, GLuint id,
    GLenum severity, const std::string& message);

private:
  /** Convert the Severity GLenum to an enumerated and sorted value. */
  Severity severityFromGlEnum_(GLenum severity) const;

  Severity minSeverity_;
};

}

#endif /* SIMVIS_GLDEBUGMESSAGE_H */
