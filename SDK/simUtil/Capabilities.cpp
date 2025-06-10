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
#include <cassert>
#include "osg/GL"
#include "osg/GLExtensions"
#include "osg/Version"
#include "gdal_priv.h"
#include "osgEarth/Capabilities"
#include "osgEarth/Registry"
#include "osgEarth/StringUtils"
#include "osgEarth/Version"
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

  // OpenGL version must be usable.  OSG 3.6 with core profile support will not function
  // without support for VAO, which requires OpenGL 3.0, released in 2008.  Although we
  // require interface blocks from GLSL 3.3, we only absolutely require OpenGL features
  // from 3.0, so test against that.
  if (glVersion_ < 3.0f) // Note release date of 2008
  {
    recordUsabilityConcern_(UNUSABLE, osgEarth::Stringify() << "OpenGL version below 3.0 (detected " << glVersion_ << ")");
  }

  checkVendorOpenGlSupport_(caps.getVendor(), caps.getVersion());

  caps_.push_back(std::make_pair("Max GPU texture units", toString_(caps.getMaxGPUTextureUnits())));
  caps_.push_back(std::make_pair("Max texture size", toString_(caps.getMaxTextureSize())));
  caps_.push_back(std::make_pair("GLSL", toString_(caps.supportsGLSL())));
  if (caps.supportsGLSL())
  {
    caps_.push_back(std::make_pair("GLSL Version", toString_(caps.getGLSLVersion())));
    if (!caps.supportsGLSL(3.3f))
      recordUsabilityConcern_(UNUSABLE, "GLSL version reported is under 3.30");
  }
  else
    recordUsabilityConcern_(UNUSABLE, "GLSL is not supported.");
  caps_.push_back(std::make_pair("Depth-packed stencil", toString_(caps.supportsDepthPackedStencilBuffer())));
  caps_.push_back(std::make_pair("Draw instanced", toString_(caps.supportsDrawInstanced())));
  caps_.push_back(std::make_pair("NPOT textures", toString_(caps.supportsNonPowerOfTwoTextures())));
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
  osg::GLExtensions* ext = nullptr;
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

  const std::string& vendorString = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
  caps_.push_back(std::make_pair("osgEarth Version", osgEarthGetVersion()));
  caps_.push_back(std::make_pair("OSG Version", osgGetVersion()));
#ifdef GDAL_RELEASE_NAME
  caps_.push_back(std::make_pair("GDAL Version", GDAL_RELEASE_NAME));
#endif
  caps_.push_back(std::make_pair("Vendor", vendorString));
  const std::string& rendererString = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
  caps_.push_back(std::make_pair("Renderer", rendererString));
  const std::string& glVersionString = reinterpret_cast<const char*>(glGetString(GL_VERSION));
  caps_.push_back(std::make_pair("OpenGL Version", glVersionString));
  glVersion_ = extractGlVersion_(glVersionString);

  // Detect core profile by investigating GL_CONTEXT_PROFILE_MASK
  GLint profileMask = 0;
  glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profileMask);
  const bool isCoreProfile = (glVersion_ >= 3.2f && ((profileMask & GL_CONTEXT_CORE_PROFILE_BIT) != 0));
  caps_.push_back(std::make_pair("Core Profile", toString_(isCoreProfile)));

  // OpenGL version must be usable.  OSG 3.6 with core profile support will not function
  // without support for VAO, which requires OpenGL 3.0, released in 2008.  Although we
  // require interface blocks from GLSL 3.3, we only absolutely require OpenGL features
  // from 3.0, so test against that.
  if (glVersion_ < 3.0f) // Note release date of 2008
  {
    recordUsabilityConcern_(UNUSABLE, osgEarth::Stringify() << "OpenGL version below 3.0 (detected " << glVersion_ << ")");
  }

  checkVendorOpenGlSupport_(vendorString, glVersionString);
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

void Capabilities::checkVendorOpenGlSupport_(const std::string& vendor, const std::string& glVersionString)
{
  // osgEarth 1b9c06725 introduced a Capabilities-breaking headless mode that can be
  // detected by looking at the vendor and renderer strings
  if (vendor == "Unknown")
  {
    glVersion_ = 0.0;
    recordUsabilityConcern_(Capabilities::UNUSABLE, "Unable to detect vendor, OpenGL may not be available.");
    return;
  }

  // Based on recommendation from https://www.khronos.org/opengl/wiki/OpenGL_Context#Context_information_queries
  // Note that Mesa, Gallium, and Direct3D renderers are all potentially backed by a hardware
  // acceleration, and do not necessarily imply software acceleration.
  if (vendor.find("Microsoft") != std::string::npos)
  {
    recordUsabilityConcern_(USABLE_WITH_ARTIFACTS, "Software renderer detected; possibly no 3D acceleration; performance concerns");
    return;
  }

  if (vendor.find("NVIDIA") != std::string::npos)
  {
    // glVersionString is expected to look like: 3.3.0 NVIDIA major.minor
    const std::string& nvidiaVersionStr = simCore::StringUtils::after(glVersionString, "NVIDIA");
    const std::string& nvidiaMajorStr = simCore::StringUtils::before(nvidiaVersionStr, ".");
    const std::string& nvidiaMinorStr = simCore::StringUtils::after(nvidiaVersionStr, ".");
    if (nvidiaVersionStr.empty() || nvidiaMajorStr.empty() || nvidiaMinorStr.empty())
    {
      // nvidia driver that does not return version string as part of opengl version
      // nothing to do
      return;
    }
    const int nVidiaMajor = atoi(nvidiaMajorStr.c_str());
    const int nVidiaMinor = atoi(nvidiaMinorStr.c_str());

    // SIM-18144 details issues with 571. - 573. drivers;
    // as of 2025-06, all known drivers >= 571 have memory leak issues due to threaded optimization; revisit before SR18 to see if newer nVidia are free of issues.
    if (nVidiaMajor > 571)
    {
      recordUsabilityConcern_(USABLE_WITH_ARTIFACTS, osgEarth::Stringify() << "nVidia driver version " << nVidiaMajor << "." << nVidiaMinor << " has issues when threaded optimization is not disabled.");
      recordUsabilityConcern_(USABLE_WITH_ARTIFACTS, "Disable threaded optimization in the NVIDIA control panel's 'Manage 3D settings pane'.");
      return;
    }

    // testing indicates that 304.125 and most drivers > 340 work; SIM-18144 details issues with 571. - 573. drivers
    // as of 2025-06, all known drivers >= 571 have memory leak issues; revisit before SR18 to see if newer nVidia are free of issues.
    const bool usable = (nVidiaMajor == 304 && nVidiaMinor >= 125) || (nVidiaMajor >= 340 && nVidiaMajor < 571);
    if (usable)
      return;
    // testing indicates that: nvidia 331 drivers were not usable, most drivers <= 340 had issues
    const bool unusable = (nVidiaMajor == 331);
    recordUsabilityConcern_((unusable ? UNUSABLE : USABLE_WITH_ARTIFACTS), osgEarth::Stringify() << "nVidia driver version " << nVidiaMajor << "." << nVidiaMinor);
    return;
  }

  if (vendor.find("Intel") != std::string::npos)
  {
    if (glVersionString.find("9.18.10.3186") != std::string::npos)
    {
      // driver 9.18.10.3186 known to have issues
      recordUsabilityConcern_(UNUSABLE, osgEarth::Stringify() << "Intel driver version 9.18.10.3186");
    }
    return;
  }
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
