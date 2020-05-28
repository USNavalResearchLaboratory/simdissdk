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
#ifndef SIMCORE_CALC_VERTICALDATUM_H
#define SIMCORE_CALC_VERTICALDATUM_H

namespace simCore {

/// Enumeration of Vertical datum type constants
enum VerticalDatum
{
  VERTDATUM_WGS84,    /**< WGS-84 Ellipsoidal Vertical Datum */
  VERTDATUM_MSL,      /**< Mean Sea Level based on EGM-96 Datum */
  VERTDATUM_USER      /**< User Defined Vertical Datum Offset */
};

}

#endif /* SIMCORE_CALC_VERTICALDATUM_H */
