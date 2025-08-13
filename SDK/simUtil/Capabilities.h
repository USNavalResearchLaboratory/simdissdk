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
#ifndef SIMUTIL_CAPABILITIES_H
#define SIMUTIL_CAPABILITIES_H

#include <utility>
#include <vector>
#include <string>
#include <iostream>
#include "osg/Texture"
#include "simCore/Common/Common.h"

namespace osg { class GraphicsContext; }
namespace osgEarth { class Capabilities; }

namespace simUtil {

/**
 * Manages name/value pairs for graphics capabilities, useful for Help>About-like settings.
 *
 * This class can either read values from the global osgEarth Registry's Capabilities data
 * structure, or from an arbitrary provided graphics context.  The Capabilities data structure
 * provides a mostly accurate answer when using OSG to create the context, but has the potential
 * to have incorrect values if the context is created outside of OSG, such as using osgQt.
 * Because of the potential for a difference, we recommend using the GraphicsContext-supplied
 * constructor at all times for most accurate information.
 */
class SDKUTIL_EXPORT Capabilities
{
public:
  /** Develop capabilities for the provided graphics context. */
  explicit Capabilities(osg::GraphicsContext& gc);
  /** Develop capabilities using osgEarth Registry's global Capabilities structure. */
  Capabilities();

  /** Enumeration of usability criteria upon inspection of system capabilities */
  enum Usability
  {
    UNUSABLE,
    USABLE_WITH_ARTIFACTS,
    USABLE
  };

  /** Retrieve the usability test results */
  Usability isUsable() const;
  /** Retrieves a vector of complaints on usability */
  std::vector<std::string> usabilityConcerns() const;

  /** Retrieve the current capabilities in a map of name/value pairs */
  void getCapabilities(std::vector<std::pair<std::string, std::string> >& data) const;
  /** Print the capabilities to the given ostream */
  void print(std::ostream& os, size_t indent=0) const;

private:
  /** Adds osgEarth, OSG, and GDAL version data to the capabilities vector. */
  void recordThirdPartyVersions_();
  /** Adds OpenGL limits detected by osgEarth capabilities */
  void recordGlLimits_(const osgEarth::Capabilities& caps);

  /** Adds Graphics Context info (vendor, renderer, version, core profile), records version, from Capabilities */
  void recordContextInfoFromCaps_(const osgEarth::Capabilities& caps);
  /** Adds Graphics Context info (vendor, renderer, version, core profile), records version, from OpenGL direct calls; returns non-zero on critical error */
  int recordContextInfoFromContext_(osg::GraphicsContext& gc);

  /** Initialize the capabilities vector from osgEarth::Registry's global Capabilities. */
  void init_();
  /** Initialize the capabilities vector from the provided graphics context. */
  void init_(osg::GraphicsContext& gc);

  /** Records a concern about usability */
  void recordUsabilityConcern_(Usability severity, const std::string& concern);
  /** Extracts the OpenGL version from the GL_VERSION string */
  double extractGlVersion_(const std::string& glVersionString) const;
  /** Checks for invalid GL version, recording a usability concern if needed */
  void checkInvalidOpenGlVersion_();
  /** Checks for usability concerns wrt vendor-specific OpenGL support */
  void checkVendorOpenGlSupport_(const std::string& vendor, const std::string& glVersionString);

  /** Converts boolean to string */
  std::string toString_(bool val) const;
  /** Converts integer to string */
  std::string toString_(int val) const;
  /** Converts float to string */
  std::string toString_(float val) const;

  std::vector<std::pair<std::string, std::string>> caps_;
  double glVersion_ = 0.0;
  std::string vendorString_;
  std::string glVersionString_;
  Usability isUsable_ = USABLE;
  std::vector<std::string> usabilityConcerns_;
};

}

#endif /* SIMUTIL_CAPABILITIES_H */
