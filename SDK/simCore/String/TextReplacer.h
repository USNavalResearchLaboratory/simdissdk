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
#ifndef SIMCORE_STRING_TEXT_REPLACER_H
#define SIMCORE_STRING_TEXT_REPLACER_H

#include <vector>
#include "simCore/Common/Common.h"
#include "simCore/Common/Memory.h"

namespace simCore
{
/// TextReplacer - a registry for replaceable objects
class SDKCORE_EXPORT TextReplacer
{
public:
  /** Represents a single variable that may be replaced by this class instance. */
  class Replaceable
  {
  public:
    virtual ~Replaceable() {}
    /**
    * Returns the replacement string that matches the variable string
    * @return String that will replace the variable string
    */
    virtual std::string getText() const = 0;
    /**
    * Returns the variable string that will be replaced
    * @return String that will be replaced, e.g., %TIME%
    */
    virtual std::string getVariableName() const = 0;
  };

  /**
  * Construct a TextReplacer
  */
  TextReplacer();
  virtual ~TextReplacer();

  /**
  * Process the format string using all replaceables in this TextReplacer registry
  * @param formatString String that will be processed for replacement
  * @return the processed format string
  */
  std::string format(const std::string& formatString);

  /**
  * Add a replaceable to this registry, TextReplacer assumes ownership of the Replaceable
  * @param r Pointer to the replaceable to be added to registry
  * @return 0 on success, 1 on failure if replaceable is already in registry
  */
  int addReplaceable(TextReplacer::Replaceable* r);

  /**
  * Delete a replaceable from this registry.  Deletes replaceable on success.
  * @param r Pointer to the replaceable to be deleted from registry
  * @return 0 on success; r will be deleted.  Non-zero on error (not found); r will not be deleted.
  */
  int deleteReplaceable(TextReplacer::Replaceable* r);

private:
  std::vector<TextReplacer::Replaceable*> replaceables_;   /// vector of registered replaceables
};

/// Shared pointer to a TextReplacer
typedef std::shared_ptr<TextReplacer> TextReplacerPtr;

} // namespace simCore

#endif /* SIMCORE_STRING_TEXT_REPLACER_H */
