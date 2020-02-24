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
 *               EW Modeling and Simulation, Code 5770
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * For more information please send email to simdis@enews.nrl.navy.mil
 *
 * U.S. Naval Research Laboratory.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 *
 *
 */
#include <cassert>
#include "osg/GL"
#include "osg/GLExtensions"
#include "osg/Version"
#include "osgEarth/Registry"
#include "osgEarth/Capabilities"
#include "osgEarth/StringUtils"
#include "simCore/String/Format.h"
#include "simCore/String/Utils.h"
#include "simUtil/Capabilities.h"

#ifndef GL_CONTEXT_PROFILE_MASK
#define GL_CONTEXT_PROFILE_MASK 0x9126
#endif
#ifndef GL_CONTEXT_CORE_PROFILE_BIT
#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#endif

namespace simUtil {

Capabilities::Capabilities(osg::GraphicsContext& gc)
  : glVersion_(1.0),
    isUsable_(USABLE)
{
  init_(gc);
}

Capabilities::Capabilities()
  : glVersion_(1.0),
  isUsable_(USABLE)
{
  init_();
}

void Capabilities::getCapabilities(std::vector<std::pair<std::string, std::string> >& data) const
{
  data = caps_;
}

void Capabilities::print(std::ostream& os, size_t indent) const
{
  std::string tabs(indent, ' ');
  for (std::vector<std::pair<std::string, std::string> >::const_iterator i = caps_.begin(); i != caps_.end(); ++i)
    os << tabs << (*i).first << " = " << (*i).second << std::endl;
}

std::string Capabilities::toString_(bool val) const
{
  return (val ? "yes" : "no");
}

std::string Capabilities::toString_(int val) const
{
  std::stringstream ss;
  ss << val;
  return ss.str();
}

std::string Capabilities::toString_(float val) const
{
  std::stringstream ss;
  ss << val;
  return ss.str();
}

void Capabilities::init_()
{
  const osgEarth::Capabilities& caps = osgEarth::Registry::instance()->getCapabilities();
  caps_.push_back(std::make_pair("Vendor", caps.getVendor()));
  caps_.push_back(std::make_pair("Renderer", caps.getRenderer()));
  caps_.push_back(std::make_pair("OpenGL Version", caps.getVersion()));
  glVersion_ = extractGlVersion_(caps.getVersion());
  caps_.push_back(std::make_pair("Core Profile", toString_(caps.isCoreProfile())));

  // Based on recommendation from https://www.khronos.org/opengl/wiki/OpenGL_Context#Context_information_queries
  // Note that Mesa, Gallium, and Direct3D renderers are all potentially backed by a hardware
  // acceleration, and do not necessarily imply software acceleration.
  if (caps.getVendor().find("Microsoft") != std::string::npos)
  {
    recordUsabilityConcern_(USABLE_WITH_ARTIFACTS, "Software renderer detected; possibly no 3D acceleration; performance concerns");
  }

  // OpenGL version must be usable.  OSG 3.6 with core profile support will not function
  // without support for VAO, which requires OpenGL 3.0, released in 2008.  Although we
  // require interface blocks from GLSL 3.3, we only absolutely require OpenGL features
  // from 3.0, so test against that.
  if (glVersion_ < 3.0f) // Note release date of 2008
  {
    recordUsabilityConcern_(UNUSABLE, osgEarth::Stringify() << "OpenGL version below 3.0 (detected " << glVersion_ << ")");
  }

  caps_.push_back(std::make_pair("Max FFP texture units", toString_(caps.getMaxFFPTextureUnits())));
  caps_.push_back(std::make_pair("Max GPU texture units", toString_(caps.getMaxGPUTextureUnits())));
  caps_.push_back(std::make_pair("Max GPU texture coordinate sets", toString_(caps.getMaxGPUTextureCoordSets())));
  caps_.push_back(std::make_pair("Max GPU attributes", toString_(caps.getMaxGPUAttribs())));
  caps_.push_back(std::make_pair("Depth buffer bits", toString_(caps.getDepthBufferBits())));
  caps_.push_back(std::make_pair("Max texture size", toString_(caps.getMaxTextureSize())));
  caps_.push_back(std::make_pair("Max lights", toString_(caps.getMaxLights())));
  caps_.push_back(std::make_pair("GLSL", toString_(caps.supportsGLSL())));
  if (caps.supportsGLSL())
  {
    caps_.push_back(std::make_pair("GLSL Version", toString_(caps.getGLSLVersion())));
    if (!caps.supportsGLSL(3.3f))
      recordUsabilityConcern_(UNUSABLE, "GLSL version reported is under 3.30");
  }
  else
    recordUsabilityConcern_(UNUSABLE, "GLSL is not supported.");
  caps_.push_back(std::make_pair("Texture arrays", toString_(caps.supportsTextureArrays())));
  caps_.push_back(std::make_pair("3D textures", toString_(caps.supportsTexture3D())));
  caps_.push_back(std::make_pair("Multitexturing", toString_(caps.supportsMultiTexture())));
  caps_.push_back(std::make_pair("Stencil wrap", toString_(caps.supportsStencilWrap())));
  caps_.push_back(std::make_pair("2-sided stencils", toString_(caps.supportsTwoSidedStencil())));
  caps_.push_back(std::make_pair("Depth-packed stencil", toString_(caps.supportsDepthPackedStencilBuffer())));
  caps_.push_back(std::make_pair("Occlusion query", toString_(caps.supportsOcclusionQuery())));
  caps_.push_back(std::make_pair("Draw instanced", toString_(caps.supportsDrawInstanced())));
  caps_.push_back(std::make_pair("Max uniform block size", toString_(caps.getMaxUniformBlockSize())));
  caps_.push_back(std::make_pair("Uniform buffer objects", toString_(caps.supportsUniformBufferObjects())));
  caps_.push_back(std::make_pair("NPOT textures", toString_(caps.supportsNonPowerOfTwoTextures())));
  caps_.push_back(std::make_pair("Static geometry style", caps.preferDisplayListsForStaticGeometry() ? "Display List" : "VBO"));
  caps_.push_back(std::make_pair("Mipmapped texture updates", toString_(caps.supportsMipmappedTextureUpdates())));
  caps_.push_back(std::make_pair("Max fast texture size", toString_(caps.getMaxFastTextureSize())));

  // Reconstruct the supported compressions string
  std::string compressionSupported;
  if (caps.supportsTextureCompression(osg::Texture::USE_ARB_COMPRESSION))
    compressionSupported += "ARB ";
  if (caps.supportsTextureCompression(osg::Texture::USE_S3TC_DXT1_COMPRESSION))
    compressionSupported += "S3 ";
  if (caps.supportsTextureCompression(osg::Texture::USE_PVRTC_2BPP_COMPRESSION))
    compressionSupported += "PVR ";
  if (caps.supportsTextureCompression(osg::Texture::USE_ETC_COMPRESSION))
    compressionSupported += "ETC1 ";
  if (caps.supportsTextureCompression(osg::Texture::USE_RGTC1_COMPRESSION))
    compressionSupported += "RG ";
  if (compressionSupported.empty())
    compressionSupported = "no";
  else // Remove trailing space
    compressionSupported = compressionSupported.substr(0, compressionSupported.length() - 1);
  caps_.push_back(std::make_pair("Texture compression", compressionSupported));
}

void Capabilities::init_(osg::GraphicsContext& gc)
{
  osg::GLExtensions* ext = NULL;
  if (gc.makeCurrent() && gc.getState())
    ext = osg::GLExtensions::Get(gc.getState()->getContextID(), true);

  // Make sure we have an active context, else we can't initialize.
  if (!ext) // not initialized
  {
    caps_.push_back(std::make_pair("Vendor", "Unknown"));
    caps_.push_back(std::make_pair("Renderer", "Unknown"));
    caps_.push_back(std::make_pair("OpenGL Version", "Unknown"));
    caps_.push_back(std::make_pair("Core Profile", toString_(false)));
    glVersion_ = 0.0;
    recordUsabilityConcern_(Capabilities::UNUSABLE, "Unable to activate context.");
    return;
  }

  const std::string vendorString = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
  caps_.push_back(std::make_pair("Vendor", vendorString));
  caps_.push_back(std::make_pair("Renderer", reinterpret_cast<const char*>(glGetString(GL_RENDERER))));
  const std::string glVersionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
  caps_.push_back(std::make_pair("OpenGL Version", glVersionString));
  glVersion_ = extractGlVersion_(glVersionString);
  const unsigned int contextId = gc.getState()->getContextID();

  // Detect core profile by investigating GL_CONTEXT_PROFILE_MASK
  GLint profileMask = 0;
  glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profileMask);
  const bool isCoreProfile = (glVersion_ >= 3.2f && ((profileMask & GL_CONTEXT_CORE_PROFILE_BIT) != 0));
  caps_.push_back(std::make_pair("Core Profile", toString_(isCoreProfile)));

  // Based on recommendation from https://www.khronos.org/opengl/wiki/OpenGL_Context#Context_information_queries
  // Note that Mesa, Gallium, and Direct3D renderers are all potentially backed by a hardware
  // acceleration, and do not necessarily imply software acceleration.
  if (vendorString.find("Microsoft") != std::string::npos)
  {
    recordUsabilityConcern_(USABLE_WITH_ARTIFACTS, "Software renderer detected; possibly no 3D acceleration; performance concerns");
  }

  // OpenGL version must be usable.  OSG 3.6 with core profile support will not function
  // without support for VAO, which requires OpenGL 3.0, released in 2008.  Although we
  // require interface blocks from GLSL 3.3, we only absolutely require OpenGL features
  // from 3.0, so test against that.
  if (glVersion_ < 3.0f) // Note release date of 2008
  {
    recordUsabilityConcern_(UNUSABLE, osgEarth::Stringify() << "OpenGL version below 3.0 (detected " << glVersion_ << ")");
  }

#if OSG_MIN_VERSION_REQUIRED(3,6,0)
  // OSG 3.6 auto-detects for us
  GLint maxTextureUnits = ext->glMaxTextureUnits;
  GLint maxTextureCoords = ext->glMaxTextureCoords;
#else
  // Follow th OSG 3.6 style of detection for texture units and coords
  GLint maxTextureUnits = 1;
  GLint maxTextureCoords = 1;
  if (glVersion_ >= 2.0f || osg::isGLExtensionSupported(contextId, "GL_ARB_vertex_shader"))
  {
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
    // GL_MAX_TEXTURE_COORDS goes away in newer OpenGL
    glGetIntegerv(GL_MAX_TEXTURE_COORDS, &maxTextureCoords);
#else
    maxTextureCoords = maxTextureUnits;
#endif
  }
  else if (glVersion_ >= 1.3f || osg::isGLExtensionSupported(contextId, "GL_ARB_multitexture"))
  {
    // Fall back to multitexturing units for oldest OpenGL
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits);
    maxTextureCoords = maxTextureUnits;
  }
#endif
  caps_.push_back(std::make_pair("Max GPU texture units", toString_(maxTextureUnits)));
  caps_.push_back(std::make_pair("Max GPU texture coordinate sets", toString_(maxTextureCoords)));

  // Need to query for maximum vertex attributes
  GLint maxAttribs = 0;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
  caps_.push_back(std::make_pair("Max GPU attributes", toString_(maxAttribs)));

  // Continue pulling values out of ext
  caps_.push_back(std::make_pair("Max texture size", toString_(ext->maxTextureSize)));
  caps_.push_back(std::make_pair("GLSL", toString_(ext->isGlslSupported)));
  if (ext->isGlslSupported)
  {
    caps_.push_back(std::make_pair("GLSL Version", toString_(ext->glslLanguageVersion)));
    if (ext->glslLanguageVersion < 3.3f)
      recordUsabilityConcern_(UNUSABLE, "GLSL version reported is under 3.30");
  }
  else
    recordUsabilityConcern_(UNUSABLE, "GLSL is not supported.");
  caps_.push_back(std::make_pair("Texture arrays", toString_(ext->isTexture2DArraySupported)));
  caps_.push_back(std::make_pair("Multitexturing", toString_(ext->isMultiTexSupported)));
  caps_.push_back(std::make_pair("Stencil wrap", toString_(ext->isStencilWrapSupported)));
  caps_.push_back(std::make_pair("2-sided stencils", toString_(ext->isStencilTwoSidedSupported)));
  caps_.push_back(std::make_pair("Depth-packed stencil", toString_(ext->isPackedDepthStencilSupported)));
  caps_.push_back(std::make_pair("Occlusion query", toString_(ext->isOcclusionQuerySupported)));

  // Need to query for uniform block size
  GLint maxUniformBlockSize = 0;
  glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
  caps_.push_back(std::make_pair("Max uniform block size", toString_(maxUniformBlockSize)));

  // Keep pulling values out of ext
  caps_.push_back(std::make_pair("Uniform buffer objects", toString_(ext->isUniformBufferObjectSupported)));
  caps_.push_back(std::make_pair("NPOT textures", toString_(ext->isNonPowerOfTwoTextureMipMappedSupported)));

  // Reconstruct the supported compressions string
  std::string compressionSupported;
  if (ext->isTextureCompressionARBSupported)
    compressionSupported += "ARB ";
  if (ext->isTextureCompressionS3TCSupported)
    compressionSupported += "S3 ";
  if (ext->isTextureCompressionPVRTCSupported)
    compressionSupported += "PVR ";
  if (ext->isTextureCompressionETCSupported)
    compressionSupported += "ETC1 ";
  if (ext->isTextureCompressionRGTCSupported)
    compressionSupported += "RG ";
  if (compressionSupported.empty())
    compressionSupported = "no";
  else // Remove trailing space
    compressionSupported = compressionSupported.substr(0, compressionSupported.length() - 1);
  caps_.push_back(std::make_pair("Texture compression", compressionSupported));
}

void Capabilities::recordUsabilityConcern_(Capabilities::Usability severity, const std::string& concern)
{
  // Method shouldn't be called with USABLE parameter (else it's not a concern...)
  assert(severity != USABLE);

  // Elevate our usability -- take over the severity warning unless we're already at unusable
  if (isUsable_ != UNUSABLE)
    isUsable_ = severity;

  // If this makes it unusable, put it at front of queue
  if (severity == UNUSABLE)
    usabilityConcerns_.insert(usabilityConcerns_.begin(), concern);
  else
    usabilityConcerns_.push_back(concern);
}

double Capabilities::extractGlVersion_(const std::string& glVersionString) const
{
  // From https://www.opengl.org/wiki/Get_Context_Info:
  // The string returned starts with "<major version>.<minor version>". Following the
  // minor version, there can be another '.', then a vendor-specific build number.
  // The string may have more content, which is completely vendor-specific (thus not
  // a part of the OpenGL standard).
  //
  // Note that the use of glGetIntegerv(GL_MAJOR_VERSION) is not usable until after
  // OpenGL version 3.0; in earlier versions the call is unsupported.

  return atof(glVersionString.c_str());
}

Capabilities::Usability Capabilities::isUsable() const
{
  return isUsable_;
}

std::vector<std::string> Capabilities::usabilityConcerns() const
{
  return usabilityConcerns_;
}

}
