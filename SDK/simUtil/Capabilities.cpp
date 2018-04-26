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
#include "osgEarth/Registry"
#include "osgEarth/Capabilities"
#include "osgEarth/StringUtils"
#include "simCore/String/Format.h"
#include "simCore/String/Utils.h"
#include "simVis/osgEarthVersion.h"
#include "simUtil/Capabilities.h"

namespace simUtil {

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
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,9,0)
  caps_.push_back(std::make_pair("Core Profile", toString_(caps.isCoreProfile())));
#endif

  // List of software drivers is from testing and https://www.opengl.org/wiki/Get_Context_Info
  if (caps.getVendor().find("Mesa") != std::string::npos ||
    caps.getVersion().find("Mesa") != std::string::npos ||
    caps.getVendor().find("Microsoft") != std::string::npos ||
    caps.getRenderer().find("Direct3D") != std::string::npos ||
    caps.getRenderer().find("Gallium") != std::string::npos)
  {
    recordUsabilityConcern_(USABLE_WITH_ARTIFACTS, "Software renderer detected; no 3D acceleration; performance concerns");
  }

  // OpenGL version must be usable.  1.5 is plausibly usable if the GLSL implementation
  // is of appropriate version.  We could add a usable-with-artifacts warning on versions
  // between 1.5 and 3.3, but it's unclear that we actually require features in anything
  // in that area or if it's just a requirement on GLSL version.
  if (glVersion_ < 1.5) // Note release date of 2003
  {
    recordUsabilityConcern_(UNUSABLE, osgEarth::Stringify() << "OpenGL version below 3.3 (detected " << glVersion_ << ")");
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
      recordUsabilityConcern_(USABLE_WITH_ARTIFACTS, "GLSL version reported is under 3.30");
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
