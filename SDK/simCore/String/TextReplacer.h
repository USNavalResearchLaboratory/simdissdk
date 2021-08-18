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
#ifndef SIMCORE_STRING_TEXT_REPLACER_H
#define SIMCORE_STRING_TEXT_REPLACER_H

#include <memory>
#include <map>
#include <string>
#include "simCore/Common/Common.h"

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
     * Returns the variable string that will be replaced.  This should not change.
     * @return String that will be replaced, e.g., %TIME%.  Must have % marks on either side.
     */
    virtual std::string getVariableName() const = 0;
  };

  /** Handles cases when the variable is not defined. */
  class UndefinedVariableHandler
  {
  public:
    virtual ~UndefinedVariableHandler() {}
    /** Returns the string to use for the undefined variable string, such as "%TIME%".  Guaranteed to have surrounding % marks */
    virtual std::string getText(const std::string& varName) const = 0;
  };
  typedef std::shared_ptr<UndefinedVariableHandler> UndefinedVariableHandlerPtr;

  /** Construct a TextReplacer */
  TextReplacer();
  virtual ~TextReplacer();

  /**
   * Process the format string using all replaceables in this TextReplacer registry
   * @param formatString String that will be processed for replacement
   * @return the processed format string
   */
  std::string format(const std::string& formatString) const;

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
  /** String version of deleting a replaceable, deleting by its variable name. */
  int deleteReplaceable(const std::string& variableName);

  /**
   * Changes the handler to use for undefined variables.  By default, undefined
   * variables are replaced with an empty string.  If a UndefinedVariableHandler
   * is defined, then it is responsible for supplying the text to replace.  This
   * can be useful for a system environment variable replacer.  Only one
   * UndefinedVariableHandler can be active at a time.
   */
  void setUndefinedVariableHandler(UndefinedVariableHandlerPtr hander);

private:
  /** Implementation of format(), which helps with recursive variable resolving */
  std::string formatImpl_(const std::string& formatString, int depth) const;

  /** Given the variable name, return the string to replace */
  std::string evaluate_(const std::string& varNameWithPct, int depth) const;
  /** Returns the validated variable name.  Empty string returned if name invalid. */
  std::string validateName_(const std::string& inputName) const;

  /// vector of registered replaceables
  std::map<std::string, TextReplacer::Replaceable*> replaceables_;
  /// Handles undefined variables
  UndefinedVariableHandlerPtr undefinedHandler_;
};

/**
 * Undefined variables handler that simply ignores variables that are undefined,
 * not even replacing with empty string.  This is the default handler.
 */
class IgnoreUndefinedVariables : public TextReplacer::UndefinedVariableHandler
{
public:
  virtual std::string getText(const std::string& varName) const { return varName; }
};

/// Shared pointer to a TextReplacer
typedef std::shared_ptr<TextReplacer> TextReplacerPtr;

} // namespace simCore

#endif /* SIMCORE_STRING_TEXT_REPLACER_H */
