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
#ifndef SIMDIS_UTIL_EXAMPLE_CONTROLS_H
#define SIMDIS_UTIL_EXAMPLE_CONTROLS_H

#include "simVis/View.h"
#include "osgEarth/Controls"

namespace simData { class DataStore; }

/**
* UI Controls used in the SIMDIS SDK Examples
*/
namespace simExamples
{
  /** Creates a map with a one image and one elevation layer. */
  extern SDKUTIL_EXPORT osgEarth::Util::Controls::Control* createPlatformListControl(
    simVis::View* view,
    simData::DataStore*  dataStore);

  /** Creates a VCR control for playing back a datastore. */
  extern SDKUTIL_EXPORT osgEarth::Util::Controls::Control* createVCRControl(
    simVis::View* view,
    simData::DataStore*  dataStore);

  ///** Creates a list of beams with a toggle for the draw type */
  extern SDKUTIL_EXPORT osgEarth::Util::Controls::Control* createBeamListControl(
    simVis::View* view,
    simData::DataStore*  dataStore,
    const std::string& antennaPattern="SINXX");

} // end namespace simExamples

#endif /* SIMDIS_UTIL_EXAMPLE_CONTROLS_H */
