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
#include "simCore/Common/SDKAssert.h"
#include "simCore/Common/Version.h"
#include "simCore/Calc/Math.h"
#include "simVis/Utils.h"

namespace
{

int testFontSize()
{
  int rv = 0;
  for (int k = 4; k < 15; ++k)
  {
    const float asOsg = simVis::osgFontSize(k);
    const float backToSimdis = simVis::simdisFontSize(asOsg);
    rv += SDK_ASSERT(simCore::areEqual(k, backToSimdis));
  }
  return rv;
}

}

int FontSizeTest(int argc, char* argv[])
{
  int rv = 0;

  // Check the SIMDIS SDK version
  simCore::checkVersionThrow();

  // Run tests
  rv += testFontSize();

  return rv;
}


