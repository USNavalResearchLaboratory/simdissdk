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

#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "simCore/Common/Export.h"
#include "simCore/String/Tokenizer.h"

namespace simCore {

/**
* A class for improving performance of DIS model map wildcard searches. Caches found DIS IDs in a basic map so subsequent
* searches for that ID don't require wildcard matching.
*/
template<typename T>
class DisModelCache
{
public:
  DisModelCache() {}

  /**
   * Get the model using wildcard searching. If model is found, stored in the model cache so next search on the same disId is faster.
   * @param disId DIS ID in format of "kind.domain.country.category.subcat.specific.extra"
   * @param wildcardLevel Starting from the right and working to the left replace parts of the
   *                      DIS ID with "0" until a match is achieved or specified level is reached.
   * @param modelToSearch Map from DIS ID to template type to search through
   * @return Returns mapped template value if the key is found in the map, or the default constructor value of the type if it's not
   */
  T getModel(const std::string& disId, unsigned int wildcardLevel, const std::map<std::string, T>& modelToSearch)
  {
    auto iter = foundModels_.find(disId);
    if (iter != foundModels_.end())
      return iter->second;

    auto rv = getFromGenericMap(disId, wildcardLevel, modelToSearch);
    foundModels_[disId] = rv;
    return rv;
  }

  /** Clear out the found model cache */
  void clear()
  {
    foundModels_.clear();
  }

  /**
   * Helper function to assemble an entity type string from the given parts, replacing wildcarded components with 0
   * @param parts 7 part DIS enum split by '.'
   * @param wildcardLevel Number of components to replace with 0
   * @return Constructed entity type string
   */
  static std::string entityTypeString(const std::vector<std::string>& parts, unsigned int wildcardLevel)
  {
    // must specify all 7 parts
    if (parts.size() != 7)
      return "";

    std::ostringstream buff;
    // The order of entity type is kind.domain.country.category.subcat.specific.extra;
    // but order of the default-model processing here is kind/domain/category/country/...
    // which allows assigning one default model for kind/domain/category that can apply to all countries,
    // while still allowing specific country overrides.
    buff << parts[0];
    buff << "." << ((wildcardLevel >= 6) ? "0" : parts[1]);
    buff << "." << ((wildcardLevel >= 4) ? "0" : parts[2]);
    buff << "." << ((wildcardLevel >= 5) ? "0" : parts[3]);
    buff << "." << ((wildcardLevel >= 3) ? "0" : parts[4]);
    buff << "." << ((wildcardLevel >= 2) ? "0" : parts[5]);
    buff << "." << ((wildcardLevel >= 1) ? "0" : parts[6]);
    return buff.str();
  }

  /**
   * Template function to expose DIS ID matching logic to external maps
   * @param disId DIS ID in format of "kind.domain.country.category.subcat.specific.extra"
   * @param wildcardLevel Starting from the right and working to the left replace parts of the
   *                      DIS ID with "0" until a match is achieved or specified level is reached.
   * @param modelToSearch Map from DIS ID to template type to search through
   * @return Returns mapped template value if the key is found in the map, or the default constructor value of the type if it's not
   */
  static T getFromGenericMap(const std::string& disId, unsigned int wildcardLevel, const std::map<std::string, T>& modelToSearch)
  {
    if (modelToSearch.empty())
      return T();

    // Break the DIS ID down into its component parts, k.d.c.c.s.s.e
    std::vector<std::string> parts;
    simCore::stringTokenizer(parts, disId, ".");
    if (parts.empty())
      return T();

    // A full DIS ID has 7 components. If a partial ID was given, pad it with 0s to get a full ID
    while (parts.size() < 7)
      parts.push_back("0");

    // iterative search for best match to the entityType
    for (unsigned int ii = 0; ii <= wildcardLevel; ii++)
    {
      const std::string& etString = entityTypeString(parts, ii);
      const auto iter = modelToSearch.find(etString);
      if (iter != modelToSearch.end())
        return iter->second;
    }

    return T();
  }
private:
  /// Cache found models to avoid duplicate wild card searches
  std::map<std::string, T> foundModels_;
};


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

  /**
   * Template function to expose DIS ID matching logic to external maps
   * @param disId DIS ID in format of "kind.domain.country.category.subcat.specific.extra"
   * @param wildcardLevel Starting from the right and working to the left replace parts of the
   *                      DIS ID with "0" until a match is achieved or specified level is reached.
   * @param modelToSearch Map from DIS ID to template type to search through
   * @return Returns mapped template value if the key is found in the map, or the default constructor value of the type if it's not
   */
  template<typename T>
  static T getFromGenericMap(const std::string& disId, unsigned int wildcardLevel, const std::map<std::string, T>& modelToSearch)
  {
    return DisModelCache<T>::getFromGenericMap(disId, wildcardLevel, modelToSearch);
  }

  /**
  * Helper function to assemble an entity type string from the given parts, replacing wildcarded components with 0
  * @param parts 7 part DIS enum split by '.'
  * @param wildcardLevel Number of components to replace with 0
  * @return Constructed entity type string
  */
  static std::string entityTypeString(const std::vector<std::string>& parts, unsigned int wildcardLevel);

private:
  std::map<std::string, std::string> disModels_; ///< Map of DIS ID to SIMDIS models
  mutable DisModelCache<std::string> modelCache_; ///< Cache found models
};

}

#endif
