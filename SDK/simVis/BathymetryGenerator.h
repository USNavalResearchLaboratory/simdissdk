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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_BATHYMETRY_GENERATOR_H
#define SIMVIS_BATHYMETRY_GENERATOR_H

/**@file
* Utilities generate or modify the bathymetry (underwater terrain) of the Earth
* for integration with an ocean surface.
*/
#include "simCore/Common/Common.h"

#include "osgEarth/TerrainEngineNode"
#include "osgEarth/ShaderLoader"
#include "osg/Uniform"

namespace simVis
{

/**
 * Sinks the bathymetry to allow for an ocean surface (a.g., Triton) whereever
 * the native terrain is at MSL 0.
 */
class SDKVIS_EXPORT BathymetryGenerator : public osgEarth::TerrainEffect
{
public:
  /**
   * Constructs a new bathymetry terrain modifier
   */
  BathymetryGenerator();

  /**
   * Maximum elevation in meters at which to apply the vertical offset.
   * Anything equal to or less than this value will be offset by the
   * value specified in setOffset().
   * Defaults to 0.01
   */
  void setSeaLevelElevation(float value);
  /** Retrieves maximum elevation (m) at which to apply the vertical offset. */
  float getSeaLevelElevation() const;

  /** Set meters by which to offset the terrain below the Sea Level elevation. */
  void setOffset(float offset);
  /** Retrieve meters by which to offset the terrain below the Sea Level elevation. */
  float getOffset() const;

public: // osgEarth::TerrainEffect
  /** Called by the terrain engine when you install the effect */
  void onInstall(osgEarth::TerrainEngineNode* engine);

  /** Called by the terrain engine when you uninstall the effect */
  void onUninstall(osgEarth::TerrainEngineNode* engine);

public: // osg::Object
  /** Return the proper library name */
  const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  const char* className() const { return "BathymetryGenerator"; }

protected:
  /// osg::Referenced-derived
  virtual ~BathymetryGenerator();

private:
  osg::ref_ptr<osg::Uniform> seaLevelUniform_;
  osg::ref_ptr<osg::Uniform> offsetUniform_;

  /// callback to ensure a proper tile bounding box on tiles that are dropped in altitude
  class AlterTileBBoxCB;
  osg::ref_ptr<AlterTileBBoxCB> alterTileBBoxCB_;
};

} // namespace simVis

#endif // SIMVIS_BATHYMETRY_GENERATOR_H

