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
#ifndef SIMUTIL_DEFAULTDATASTOREVALUES_H
#define SIMUTIL_DEFAULTDATASTOREVALUES_H

namespace simData {
  class DataStore;
  class PlatformPrefs;
  class ScenarioProperties;
}

namespace simUtil {

/**
* Class that provides support for DataStore by defining the default values for the different pref types, as well as a convenience
* method to set the default pref values in the DataStore
*/
class SDKUTIL_EXPORT DefaultEntityPrefs
{
public:
  /** Initialize the default pref values for a DataStore  */
  static void initializeDataStorePrefs(simData::DataStore& dataStore);

  /** Set the default values for platform preferences */
  static void initializeDefaultPlatformPrefs(simData::PlatformPrefs& prefs);
};

}

#endif

