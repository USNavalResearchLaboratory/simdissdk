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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifdef USE_DEPRECATED_SIMDISSDK_API
// this is deprecated; use simCore::RadarParameters

#ifndef SIMVIS_RFPROP_RADAR_PARAMETERS_H
#define SIMVIS_RFPROP_RADAR_PARAMETERS_H

#include <memory>

namespace simCore { struct RadarParameters; }
namespace simRF
{  
/**
 * RadarParameters contains RF system parameter values used in RF Propagation calculations by One-Way, Two-Way and SNR data providers.
 */
typedef simCore::RadarParameters RadarParameters;

/** Shared pointer of a RadarParameters */
typedef std::shared_ptr<RadarParameters> RadarParametersPtr;

}

#endif /* SIMVIS_RFPROP_RADAR_PARAMETERS_H */
#endif /* USE_DEPRECATED_SIMDISSDK_API */
