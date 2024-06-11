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
#include <algorithm>
#include <iostream>
#include "osg/GLDefines"
#include "osg/GraphicsContext"
#include "osg/StateSet"
#include "simVis/GlDebugMessage.h"

namespace simVis {

GlDebugMessage::GlDebugMessage(osg::GraphicsContext* gc)
  : register_(false),
  glDebugMessageControl_(nullptr),
  glDebugMessageInsert_(nullptr),
  glDebugMessageCallback_(nullptr)
{
  setGraphicsContext(gc);
}

GlDebugMessage::~GlDebugMessage()
{
  // Disable callbacks
  setGraphicsContext(nullptr);
}

void GlDebugMessage::setGraphicsContext(osg::GraphicsContext* gc)
{
  if (gc_.get() == gc)
    return;

  // Remove hooks from old graphics context
  osg::ref_ptr<osg::GraphicsContext> oldGc;
  if (gc_.lock(oldGc) && oldGc->makeCurrent())
  {
    if (glDebugMessageCallback_)
      glDebugMessageCallback_(nullptr, nullptr);
  }
  glDebugMessageControl_ = nullptr;
  glDebugMessageInsert_ = nullptr;
  glDebugMessageCallback_ = nullptr;

  // Pull out the new function pointers
  gc_ = gc;
  if (gc_.valid() && gc_->makeCurrent())
  {
    glDebugMessageControl_ = reinterpret_cast<PFNGLDEBUGMESSAGECONTROLPROC>(osg::getGLExtensionFuncPtr("glDebugMessageControl", "glDebugMessageControlARB"));
    glDebugMessageInsert_ = reinterpret_cast<PFNGLDEBUGMESSAGEINSERTPROC>(osg::getGLExtensionFuncPtr("glDebugMessageInsert", "glDebugMessageInsertARB"));
    glDebugMessageCallback_ = reinterpret_cast<PFNGLDEBUGMESSAGECALLBACKPROC>(osg::getGLExtensionFuncPtr("glDebugMessageCallback", "glDebugMessageCallbackARB"));
  }

  // Force re-registration
  register_ = !register_;
  registerCallbacks(!register_);
}

void GlDebugMessage::registerCallbacks(bool registerWithGl)
{
  if (register_ == registerWithGl)
    return;
  register_ = registerWithGl;

  // Make the context current
  osg::ref_ptr<osg::GraphicsContext> gc;
  if (!glDebugMessageCallback_ || !gc_.lock(gc) || !gc->makeCurrent())
    return;

  if (registerWithGl)
    glDebugMessageCallback_(&GlDebugMessage::processMessage_, this);
  else
    glDebugMessageCallback_(nullptr, nullptr);
}

void GlDebugMessage::setDebugOutputMode(osg::StateSet& stateSet, osg::StateAttribute::GLMode mode) const
{
  stateSet.setMode(GL_DEBUG_OUTPUT, mode);
}

void GlDebugMessage::setDebugOutputMode(bool enabled) const
{
  osg::ref_ptr<osg::GraphicsContext> gc;
  if (!gc_.lock(gc) || !gc->makeCurrent())
    return;
  if (enabled)
    glEnable(GL_DEBUG_OUTPUT);
  else
    glDisable(GL_DEBUG_OUTPUT);
}

void GlDebugMessage::addCallback(GlDebugMessageCallback* callback)
{
  // Avoid adding more than once
  osg::ref_ptr<GlDebugMessageCallback> refCallback(callback);
  if (std::find(callbacks_.begin(), callbacks_.end(), refCallback) != callbacks_.end())
    return;
  callbacks_.push_back(refCallback);
}

void GlDebugMessage::removeCallback(GlDebugMessageCallback* callback)
{
  callbacks_.erase(std::remove(callbacks_.begin(), callbacks_.end(), callback), callbacks_.end());
}

int GlDebugMessage::messageControl(GLenum source, GLenum type, GLenum severity, const std::vector<GLuint>& ids, bool enabled)
{
  osg::ref_ptr<osg::GraphicsContext> gc;
  if (!glDebugMessageControl_ || ids.empty() || !gc_.lock(gc) || !gc->makeCurrent())
    return 1;
  glDebugMessageControl_(source, type, severity, ids.size(), &ids[0], enabled ? GL_TRUE : GL_FALSE);
  return 0;
}

int GlDebugMessage::insertMessage(GLenum source, GLenum type, GLuint id, GLenum severity, const std::string& message)
{
  osg::ref_ptr<osg::GraphicsContext> gc;
  if (!glDebugMessageInsert_ || !gc_.lock(gc) || !gc->makeCurrent())
    return 1;
  glDebugMessageInsert_(source, type, id, severity, message.length(), message.c_str());
  return 0;
}

void GlDebugMessage::processMessage_(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei len, const GLchar* msg, const void* userData)
{
  if (userData)
    static_cast<const GlDebugMessage*>(userData)->publishMessage_(source, type, id, severity, len, msg);
}

void GlDebugMessage::publishMessage_(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei len, const GLchar* msg) const
{
  if (callbacks_.empty())
    return;
  const std::string stringMessage(msg, len);
  for (const auto& callback : callbacks_)
    callback->handleMessage(source, type, id, severity, stringMessage);
}

////////////////////////////////////////////////////////////////

CerrDebugMessageCallback::CerrDebugMessageCallback(Severity minSeverity)
  : minSeverity_(minSeverity)
{
}

void CerrDebugMessageCallback::handleMessage(GLenum source, GLenum type, GLuint id,
    GLenum severity, const std::string& message)
{
  if (severityFromGlEnum_(severity) < minSeverity_)
    return;
  std::cerr << message << "\n";
}

void CerrDebugMessageCallback::setMinimumSeverity(Severity minSeverity)
{
  minSeverity_ = minSeverity;
}

#ifndef GL_DEBUG_SEVERITY_NOTIFICATION
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
#endif

CerrDebugMessageCallback::Severity CerrDebugMessageCallback::severityFromGlEnum_(GLenum severity) const
{
  switch (severity)
  {
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    return SEVERITY_NOTIFICATION;
  case GL_DEBUG_SEVERITY_LOW:
    return SEVERITY_LOW;
  case GL_DEBUG_SEVERITY_MEDIUM:
    return SEVERITY_MEDIUM;
  case GL_DEBUG_SEVERITY_HIGH:
    return SEVERITY_HIGH;
  }
  // Fall back on lowest severity
  return SEVERITY_NOTIFICATION;
}

}
