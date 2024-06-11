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
#ifndef SIMDATA_PREFRULESMANAGER_H
#define SIMDATA_PREFRULESMANAGER_H

#include <deque>
#include <vector>
#include <string>
#include <iostream>

#include "simCore/Common/Common.h"
#include "simData/ObjectId.h"

namespace simData
{
class CategoryFilter;
class DataStore;

/** Empty class stub that represents a preference rule.  Preference rules have an opaque structure */
class PrefRule
{
public:
  virtual ~PrefRule() {}

  /** Serialize this into SIMDIS 9 formatted string */
  virtual std::string serialize() const = 0;

  /**
  * Apply this rule to the specified entity, if all conditions are met.
  * (Gathers all the required data from the DataStore in this call, which can be expensive.)
  * @param[in] entityId id of the entity.  0 is not a valid value and will not match any entities.
  * @param[in] ds handle to the data store
  * @return 0 on successful application, non-zero on error
  */
  virtual int apply(uint64_t entityId, simData::DataStore& ds) = 0;

  /** Get the category filter used to determine if an entity is affected by this */
  virtual const simData::CategoryFilter* categoryFilter() const = 0;
};

/** Manages all preference rules, pure interface */
class PrefRulesManager
{
public:

  virtual ~PrefRulesManager() {}

  /**
  * Load the rules in the specified pref rule file, adding them to the currently loaded rules.  Rules are always
  * force applied when added.
  * @param ruleFile the pref rule file to load
  * @return 0 on success, non-zero on error
  */
  virtual int appendRuleFile(const std::string& ruleFile) = 0;

  /**
  * Enforce the pref value specified by tagStack and entityType on the specified entity. This means that the pref value will
  * not be updated by normal processing. Only components with authority to override the pref value enforcement will do so.
  * This is useful for cases where a pref update from one source should be flagged to take priority over updates from other sources.
  * @param id  the data store entity id. 0 is not a valid value and will not match any entities.
  * @param tagStack  the protobuf message field numbers that identify the pref (fully qualified from PlatformPrefs, BeamPrefs, etc.)
  * @param enforce If true, then turn on enforcing, preventing rules from changing the value.  If false, disables the enforcement,
  *   allowing preference rules to work with the value again.
  */
  virtual void enforcePrefValue(simData::ObjectId id, const std::deque<int>& tagStack, bool enforce=true) = 0;

  /**
   * Returns true if the preference value is set to enforcing.  Enforced prefs cannot be changed by Preference Rules.  For
   * more details, see enforcePrefValue().
   * @param id  the data store entity id. 0 is not a valid value and will not match any entities.
   * @param tagStack  the protobuf message field numbers that identify the pref (fully qualified from PlatformPrefs, BeamPrefs, etc.)
   * @return True if the value is marked for enforcement, false otherwise
   */
  virtual bool isPrefValueEnforced(simData::ObjectId id, const std::deque<int>& tagStack) const = 0;

  /**
  * Load the rules in the specified pref rule files.  Rules are always force applied when added.
  * Note that the last file in the vector is the one saved to the scenario as the current pref rule file.
  * @param ruleFiles list of the pref rule files to load
  * @param removeOldRules  if true, all rules will be removed and replaced with those in the ruleFiles param
  * @return 0 on success, non-zero on error
  */
  virtual int loadRuleFiles(const std::vector<std::string>& ruleFiles, bool removeOldRules) = 0;

  /**
   * Remove all the preference rules
   * @return 0 on success, non-zero on error
   */
  virtual int removeAllRules() = 0;

  /**
   * Serializes the rules into a string
   * @param[in] rules Vector of rules to serialize to string
   * @return String representing the serialized rules
   */
  virtual std::string serializeRules(const std::vector<PrefRule*>& rules) = 0;

  /**
   * Serializes the rules of this preference rules manager into the given output stream
   * @param os the given output stream
   * @return 0 on total success, non-zero if there were any problems serializing the rules
   */
  virtual int serializeRules(std::ostream& os) = 0;

  /**
   * Deserializes the rules passed in the istream.  Rules are always force applied when added.
   * @param[in] rules  rules to deserialize
   * @return 0 on total success, non-zero if there were any problems loading rules. Note that non-zero return can still indicate partial success
   */
  virtual int deserializeRules(std::istream& rules) = 0;

  /**
   * Add a preference rule.  PrefRuleManager will deserialize the string into a pref rule, or multiple
   * rules if the pref rule is compound.  The latest version of the preference rules format is presumed.
   * Rules are always force applied when added.
   * @param[out] rules that resulted from the serialized string (rule could be compound)
   * @param[in] serializedRule  a string representing a serialized pref rule
   * @param[in] fileFormatVersion  an int representing the pref .rul file format version the pref rule is formatted in
   * @return 0 on success, non-zero on error.
   */
  virtual int addSerializedRule(std::vector<PrefRule*>& rules, const std::string& serializedRule, int fileFormatVersion) = 0;

  /**
   * Fills in a vector with all the PrefRule objects.  Note that PrefRuleManager
   * owns this memory
   * @param[out] prefRules  vector of all the current preference rules
   */
  virtual void listRules(std::vector<PrefRule*>& prefRules) = 0;

  /**
   * Remove the specified preference rule.  PrefRuleManager deletes the memory in this method
   * @param[in] prefRule  preference rule to delete
   * @return 0 on success, non-zero on error
   */
  virtual int removeRule(PrefRule* prefRule) = 0;

  /**
   * Applies all the current preference rules to all the current entities in the data store. Will not force apply unless
   * the force parameter is set to true. Otherwise, other pref settings take precedence
   * @param force  if true, forces application of rules, overrides other pref settings
   * @return 0 on success, non-zero on error
   */
  virtual int applyRules(bool force = false) = 0;

  /**
   * Applies all the rules to the specified entity
   * @param[in] id  the specified entity. 0 is not a valid value and will not match any entities.
   * @return 0 on success, non-zero on error
   */
  virtual int applyRules(uint64_t id) = 0;

  /**
   * Sets the Enable Rules state.  If the state is true all rules are processed.
   * If the state is false rules are not processed when entities are added or
   * when entities are changed. Rules are still processing when initiated
   * by Pref Tool GUI or via Plug-in API.
   * @param[in] state the Enable Rules state
   */
  virtual void setRulesEnabled(bool state) = 0;

  /**
   * Returns the Enable Rules state
   * @return the Enable Rules state
   */
  virtual bool rulesEnabled() const = 0;

  /**
  * Set the enforcement state of preferred pref value changes. When pref values are enforced, this means that the pref will
  * not be updated by normal processing of pref rules. This is useful for cases where certain pref changes (e.g. manual edits)
  * should be flagged to take priority over pref rules. If enforcement is disabled, all pref rule value changes are allowed.
  * @param enforce if true, enforces preferred pref values and prevents normal changes by pref rules
  */
  virtual void setEnforcePrefs(bool enforce) = 0;

  /**
  * Returns true if currently enforcing pref values, which prevents changes to pref values by normal processing of pref rules.
  * @param true if pref values are being enforced, false otherwise
  */
  virtual bool isEnforcingPrefs() const = 0;

};

/** Null object implementation for PrefRulesManager */
class NullPrefRulesManager : public simData::PrefRulesManager
{
  virtual int appendRuleFile(const std::string& ) { return 1; }
  virtual int loadRuleFiles(const std::vector<std::string>& , bool ) { return 1; }
  virtual int removeAllRules() { return 1; }
  virtual std::string serializeRules(const std::vector<simData::PrefRule*>& rules) { return ""; }
  virtual int serializeRules(std::ostream& os) { return 1; }
  virtual int deserializeRules(std::istream& rules) { return 1; }
  virtual int addSerializedRule(std::vector<simData::PrefRule*>& rules, const std::string& serializedRule, int fileFormatVersion){ return 1; }
  virtual void listRules(std::vector<simData::PrefRule*>& prefRules) { }
  virtual int removeRule(simData::PrefRule* prefRule) { return 1; }
  virtual int applyRules(bool force) { return 1; }
  virtual int applyRules(uint64_t id) { return 1; }
  virtual void enforcePrefValue(simData::ObjectId id, const std::deque<int>& tagStack, bool enforce) { }
  virtual bool isPrefValueEnforced(simData::ObjectId id, const std::deque<int>& tagStack) const { return false; }
  virtual void setRulesEnabled(bool state) { }
  virtual bool rulesEnabled() const { return true; }
  virtual void setEnforcePrefs(bool enforce) { }
  virtual bool isEnforcingPrefs() const { return true; }
};

}

#endif /* SIMDATA_PREFRULESMANAGER_H */
