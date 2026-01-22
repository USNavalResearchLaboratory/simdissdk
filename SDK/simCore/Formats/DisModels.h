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
#ifndef SIMCORE_FORMAT_DISMODELS_H
#define SIMCORE_FORMAT_DISMODELS_H

#include <iosfwd>
#include <map>
#include <string>
#include <vector>
#include "simCore/Common/Export.h"

namespace simCore {

/** A class for reading a DIS model file then translating DIS ID into SIMDIS models */
class SDKCORE_EXPORT DisModels
{
public:
  DisModels();

  /** Loads the given file; returns 0 on success */
  int loadFile(const std::string& filename);
  /** Loads the input stream as though it was a model.dis file; returns 0 on success */
  int loadStream(std::istream& is);

  /**
   * Loads a model from either the format 'k.d.c.c.s.s.e modelName' or the legacy format 'DIS k.d.c.c.s.s.e modelName'
   * Returns 0 if a DIS ID and model are successfully found, non-zero otherwise.
   */
  int loadModel(const std::string& modelTokens);
  /**
   * Maps a single model to the given DIS ID.
   * @param disId DIS ID string, of the format 'k.d.c.c.s.s.e'
   * @param modelName Name of the modelfile to apply
   * @return 0 if a mapping is successfully stored, non-zero otherwise
   */
  int loadModel(const std::string& disId, const std::string& modelName);
  /** Returns the count of model mappings enumerated by the loaded file */
  size_t modelCount() const;
  /** Returns true if there are no models loaded */
  bool empty() const;
  /** Clears the disModels_ map, removing all model mappings */
  void clear();

  /**
   * Returns the SIMDIS model for the given DIS ID.
   * @param disId DIS ID in format of "kind.domain.country.category.subcat.specific.extra"
   * @param wildcardLevel Starting from the right and working to the left replace parts of the
   *                      DIS ID with "0" until a match is achieved or specified level is reached.
   * @return Returns mapped DIS Model string on success, or empty string on failure
   */
  std::string getModel(const std::string& disId, unsigned int wildcardLevel) const;

private:
  /** Applies the given wildcard setting to the type string, replacing parts with 0 */
  std::string entityTypeString_(const std::vector<std::string>& parts, unsigned int wildcardLevel) const;

  std::map<std::string, std::string> disModels_; ///< Map of DIS ID to SIMDIS models
};

}

#endif
