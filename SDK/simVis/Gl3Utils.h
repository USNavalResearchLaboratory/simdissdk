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
#ifndef SIMVIS_GL3UTILS_H
#define SIMVIS_GL3UTILS_H

#include <cstring>
#include "osg/GLExtensions"
#include "osg/GraphicsContext"
#include "osg/Light"
#include "osg/Point"
#include "osg/PointSprite"
#include "osg/OperationThread"
#include "simNotify/Notify.h"

namespace simVis {

/**
 * Disables the validity of the GL_LIGHTING and GL_RESCALE_NORMAL modes when the
 * passed-in graphics context is a core profile without compatibility mode support.
 * This is useful to prevent error spam from OSG when running under the Core Profile.
 *
 * This is best done during the viewer realize operation.
 *
 * @param graphicsContext Graphics context to inspect and set mode validity on.
 */
inline void applyCoreProfileValidity(osg::GraphicsContext* graphicsContext)
{
  // Can only call some methods on a current context
  if (!graphicsContext || !graphicsContext->getState() || !graphicsContext->makeCurrent())
    return;
  osg::State* state = graphicsContext->getState();
  const unsigned int contextId = state->getContextID();
  // One of the most reliable ways to test for core profile is to check for the
  // compatibility profile.  If it exists, then we're not in core profile mode.
  const float glVersion = osg::getGLVersionNumber();
  const bool isCoreProfile = (glVersion >= 3.2f) && !osg::isGLExtensionSupported(contextId, "GL_ARB_compatibility");

  // For core profile, disable certain incompatible modes that are seen in osgEarth and loaded models
  if (isCoreProfile)
  {
    state->setModeValidity(GL_LIGHTING, false);
    state->setModeValidity(GL_LIGHT0, false);
    state->setModeValidity(GL_RESCALE_NORMAL, false);
    state->setModeValidity(GL_POINT_SMOOTH, false);
  }
  else
  {
#ifndef OSG_GL_FIXED_FUNCTION_AVAILABLE
    // Point sprite needs to be explicitly enabled for compatibility profile to match OSG expectations.
    // If this line goes away, stars in osgEarth will not be visible, when in compatibility profile
    // under an OSG built for core profile.
    state->applyMode(GL_POINT_SPRITE_ARB, true);
#endif
  }
}

/**
 * Certain older Mesa drivers support the Geometry shader, but do not support various
 * flags relating to geometry shader program fields.  This function will disable the
 * geometry shader code that spams errors on the console when Mesa drivers are detected
 * that do not explicitly support the associated GL extension, or are not high enough
 * version of OpenGL.
 *
 * This is best done during the viewer realize operation.
 *
 * @param graphicsContext Graphics context to inspect and potentially remove geometry shader support on.
 */
inline void applyMesaGeometryShaderFix(osg::GraphicsContext* graphicsContext)
{
  // Some Mesa drivers are reporting GL 3.3 support, but cannot support GL_GEOMETRY_VERTICES_OUT_EXT and
  // friends with glProgramParameteri() because neither the GL_ARB_geometry_shader4 nor GL_EXT_geometry_shader4
  // extensions are defined.

  if (!graphicsContext || !graphicsContext->getState() || !graphicsContext->makeCurrent())
    return;
  graphicsContext->makeCurrent();
  const char* glVersionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
  if (glVersionString == NULL)
    return;

  // Only apply the fix to Mesa.  It might apply to non-Mesa drivers too, but we'll just fix Mesa for
  // now until it can be shown that this issue impacts other drivers too.
  const unsigned int contextId = graphicsContext->getState()->getContextID();
  osg::ref_ptr<osg::GLExtensions> glExtensions = osg::GLExtensions::Get(contextId, true);
  if (strstr(glVersionString, "Mesa") != 0 && glExtensions.valid())
  {
    // Disable geometry shader support until GL 4.1, or explicit support for shader4
    if (glExtensions->isGeometryShader4Supported &&
      !(osg::isGLExtensionSupported(contextId, "GL_EXT_geometry_shader4") ||
        osg::isGLExtensionSupported(contextId, "GL_OES_geometry_shader") ||
        osg::isGLExtensionOrVersionSupported(contextId, "GL_ARB_geometry_shader4", 4.1f)))
    {
      SIM_INFO << "Applying Mesa work-around for Geometry shader support.\n";
      glExtensions->isGeometryShader4Supported = false;
    }
  }
}

/**
 * Convenience Realize Operation that applies core profile and Mesa fixes.  To use:
 *
 *    viewer_->setRealizeOperation(new simVis::Gl3RealizeOperation());
 *
 * This is provided as a convenience.  ViewManager-based viewers will automatically
 * initialize their viewers to perform these operations.
 */
class Gl3RealizeOperation : public osg::Operation
{
public:
  explicit Gl3RealizeOperation(osg::Operation* nested = NULL)
    : nested_(nested)
  {
  }

  /** Detect mesa Geometry Shader bug with GL3 and disable geometry portions if present */
  virtual void operator() (osg::Object* obj)
  {
    osg::GraphicsContext* gc = dynamic_cast<osg::GraphicsContext*>(obj);
    simVis::applyCoreProfileValidity(gc);
    simVis::applyMesaGeometryShaderFix(gc);
    if (nested_.valid())
      nested_->operator()(obj);
  }

protected:
  virtual ~Gl3RealizeOperation()
  {
  }

private:
  osg::ref_ptr<osg::Operation> nested_;
};


}

#endif /* SIMVIS_GL3UTILS_H */
