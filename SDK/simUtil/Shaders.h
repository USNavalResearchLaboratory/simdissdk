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
#ifndef SIMUTIL_SHADERS_H
#define SIMUTIL_SHADERS_H

#include "osgEarth/ShaderLoader"

namespace simUtil {

/** Note: Not exported. */
class Shaders : public osgEarth::Util::ShaderPackage
{
public:
  Shaders();

  /** Name of combined shader for map scale flat coloring */
  std::string mapScale() const;

  /** Name of the vertex shader for terrain toggle effect */
  std::string terrainToggleEffectVertex() const;
  /** Name of the fragment shader for terrain toggle effect */
  std::string terrainToggleEffectFragment() const;

  /** Name of particle-based vertex shader for particles in the velocity particle layer. */
  std::string velocityParticleLayerParticleVertex() const;
  /** Name of the fragment shader for particles in the velocity particle layer. */
  std::string velocityParticleLayerParticleFragment() const;

  /** Name of the velocity particle layer's compute shader's vertex code. */
  std::string velocityParticleLayerComputeVertex() const;
  /** Name of the velocity particle layer's compute shader's position-generating fragment code. */
  std::string velocityParticleLayerComputePositionFragment() const;
  /** Name of the velocity particle layer's compute shader's direction-generating fragment code. */
  std::string velocityParticleLayerComputeDirectionFragment() const;
};

}

#endif /* SIMUTIL_SHADERS_H */
