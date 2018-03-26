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
#ifndef SIMVIS_SHADERS_H
#define SIMVIS_SHADERS_H

#include "osgEarth/ShaderLoader"

namespace simVis {

/** Note: Not exported. */
class Shaders : public osgEarth::ShaderPackage
{
public:
  Shaders();

  /** Name of the fragment shader for Alpha Color Filter */
  std::string alphaColorFilterFragment() const;

  /** Name of the fragment shader for the Alpha Test shader */
  std::string alphaTestFragment() const;

  /** Name of the vertex shader for area highlight */
  std::string areaHighlightVertex() const;
  /** Name of the fragment shader for area highlight */
  std::string areaHighlightFragment() const;

  /** Vertex shader for beam pulses */
  std::string beamPulseVertex() const;

  /** Fragment shader for beam pulses */
  std::string beamPulseFragment() const;

  /** Name of vertex shader for bathymetry generator */
  std::string bathymetryGeneratorVertex() const;

  /** Name of fragment shader for glowing highlight */
  std::string glowHighlightFragment() const;

  /** Name of vertex shader for overhead mode */
  std::string overheadModeVertex() const;

  /** Name of fragment shader for override color */
  std::string overrideColorFragment() const;

  /** Name of Platform Azimuth/Elevation warping shader */
  std::string platformAzimElevWarpVertex() const;

  /** Name of vertex shader for picker */
  std::string pickerVertex() const;
  /** Name of fragment shader for picker */
  std::string pickerFragment() const;

  /** Name of vertex shader for point size */
  std::string pointSizeVertex() const;

  /** Name of vertex shader for projector manager */
  std::string projectorManagerVertex() const;
  /** Name of fragment shader for projector manager */
  std::string projectorManagerFragment() const;

  /** Name of RF Propagation vertex based main shader */
  std::string rfPropVertexBasedVertex() const;
  /** Name of RF Propagation vertex based main shader */
  std::string rfPropVertexBasedFragment() const;

  /** Name of RF Propagation texture based main shader */
  std::string rfPropTextureBasedVertex() const;
  /** Name of RF Propagation texture based main shader */
  std::string rfPropTextureBasedFragment() const;

  /** Name of RF Propagation default loss-to-color shader (used in vertex and fragment) */
  std::string rfPropLossToColorDefault() const;
  /** Name of RF Propagation threshold loss-to-color shader (used in vertex and fragment) */
  std::string rfPropLossToColorThreshold() const;

  /** Name of vertex shader that sets the gl_ClipVertex appropriately */
  std::string setClipVertex() const;

  /** Name of vertex shader for track history (flattening) */
  std::string trackHistoryVertex() const;
  /** Name of fragment shader for track history (track override color) */
  std::string trackHistoryFragment() const;

  /** Name of fragment shader for flashing LOB Group */
  std::string flashingFragment() const;

  /** Polygon stipple fragment shader */
  std::string polygonStippleFragment() const;
};

}

#endif /* SIMVIS_SHADERS_H */
