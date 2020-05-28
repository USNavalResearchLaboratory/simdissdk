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
#ifndef SIMQT_FILEUTILITIES_H
#define SIMQT_FILEUTILITIES_H

#include <QString>
#include "simCore/Common/Export.h"

namespace simQt {

/** Helper class for common file operations */
class SDKQT_EXPORT FileUtilities
{
public:
  /**
  * Tests the write permissions of fully specified file path
  * and creates the path if it is writable
  * @param absoluteFilePath the file path to test/create
  * @return true if file path is writable, false if file path not writable
  */
  static bool isPathWritable(const QString& absoluteFilePath);

  /**
  * Tests the write permissions of the specified relative path within an organization folder within
  * the standard OS-specific %APPDATA%/$HOME user folder and creates the path if it is writable.
  * @param[in] relativeFilePath Desired relative file path to create under the home path.
  * @param[in] roaming If true, attempt to use the roaming path (e.g. APPDATA); if false, attempt
  *         to use the local path (e.g. LOCALAPPDATA).  Roaming path data might transfer with a
  *         user's profile on an enterprise Windows environment.  Ignored for Linux.  When the
  *         roaming path is not writable, this function will fall back to the local path.
  * @param[out] absolutePath Output for the writable path, only if return value is 0 (success)
  * @return 0 if successful;
            1 if the relative file path was not writable;
            2 if standard OS path is not usable;
            3 if specified path was not a relative path;
  */
  static int createHomePath(const QString& relativeFilePath, bool roaming, QString& absolutePath);
};

}
#endif /* SIMQT_FILEUTILITIES_H */
