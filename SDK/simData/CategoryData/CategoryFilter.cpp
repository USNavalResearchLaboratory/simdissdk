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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include <cassert>
#include "simNotify/Notify.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/String/Utils.h"
#include "simData/DataStore.h"
#include "simData/CategoryData/CategoryNameManager.h"
#include "simData/CategoryData/CategoryFilter.h"

namespace simData {

// NOTE: these need to match the common SIMDIS pref rule file format
/** Sentinel token in a category data tokenization string that separates categories */
static const std::string SIM_PREF_RULE_CAT_SEP = "`";
/** Sentinel token in a category data tokenization string that separates values of a category */
static const std::string SIM_PREF_RULE_VAL_SEP = "~";
/** Sentinel token in a category data tokenization string that separates regexp for values of a category */
static const std::string SIM_PREF_RULE_REGEXP_SEP = "^";

//---------------------------------------------------------

/// Monitors for category data changes
class CategoryFilter::CategoryFilterListener : public simData::CategoryNameManager::Listener
{
public:
  /// Constructor
  explicit CategoryFilterListener(CategoryFilter* parent)
    : parent_(parent)
  {
  }

  virtual ~CategoryFilterListener()
  {
  }

  /// Invoked when a new category is added
  virtual void onAddCategory(int categoryIndex)
  {
    parent_->addCategoryName_(categoryIndex);
  }

  /// Invoked when a new value is added to a category
  virtual void onAddValue(int categoryIndex, int valueIndex)
  {
    parent_->addCategoryValue_(categoryIndex, valueIndex);
  }

  /// Invoked when all data is cleared
  virtual void onClear()
  {
    parent_->clear_();
  }

  /// Invoked when all listeners have received onClear()
  virtual void doneClearing()
  {
    // noop
  }

private:
  CategoryFilter* parent_;
};


//---------------------------------------------------------

CategoryFilter::CategoryFilter(simData::DataStore* dataStore, bool autoUpdate)
  : dataStore_(dataStore),
    autoUpdate_(autoUpdate)
{
  if (autoUpdate_)
  {
    // Does nothing without a datastore
    assert(dataStore != nullptr);
    if (dataStore_ == nullptr)
      return;

    buildCategoryFilter_(true, true, true, true);

    // create observers/listeners
    listenerPtr_.reset(new CategoryFilterListener(this));
    dataStore_->categoryNameManager().addListener(listenerPtr_);
  }
}

CategoryFilter::CategoryFilter(const CategoryFilter& other)
  : dataStore_(other.dataStore_),
    autoUpdate_(other.autoUpdate_),
    categoryCheck_(other.categoryCheck_),
    categoryRegExp_(other.categoryRegExp_)
{
  if (autoUpdate_ && dataStore_)
  {
    // create observers/listeners
    listenerPtr_.reset(new CategoryFilterListener(this));
    dataStore_->categoryNameManager().addListener(listenerPtr_);
  }
}

CategoryFilter::~CategoryFilter()
{
  if ((dataStore_ != nullptr) && (listenerPtr_ != nullptr))
    dataStore_->categoryNameManager().removeListener(listenerPtr_);
}

CategoryFilter& CategoryFilter::operator=(const CategoryFilter& other)
{
  return assign(other, true);
}

CategoryFilter& CategoryFilter::assign(const CategoryFilter& other, bool copyAutoUpdateFlag)
{
  if (&other == this)
    return *this;

  // Clear the listener pointer unconditionally
  if ((dataStore_ != nullptr) && (listenerPtr_ != nullptr))
  {
    dataStore_->categoryNameManager().removeListener(listenerPtr_);
    listenerPtr_.reset();
  }
  dataStore_ = other.dataStore_;
  if (copyAutoUpdateFlag)
    autoUpdate_ = other.autoUpdate_;
  categoryCheck_ = other.categoryCheck_;
  categoryRegExp_ = other.categoryRegExp_;

  if (dataStore_ != nullptr && autoUpdate_)
  {
    // re-add observers/listeners
    assert(listenerPtr_ == nullptr);
    listenerPtr_.reset(new CategoryFilterListener(this));
    dataStore_->categoryNameManager().addListener(listenerPtr_);
  }

  return *this;
}

bool CategoryFilter::isEmpty() const
{
  return categoryCheck_.empty() && categoryRegExp_.empty();
}

bool CategoryFilter::operator==(const CategoryFilter& rhs) const
{
  // Though data stores must match, the auto-update / listener pointers do not need to match
  return (dataStore_ == rhs.dataStore_ &&
    categoryCheck_ == rhs.categoryCheck_ &&
    categoryRegExp_ == rhs.categoryRegExp_);
}

bool CategoryFilter::nameContributesToFilter(int nameInt) const
{
  return categoryCheck_.find(nameInt) != categoryCheck_.end() ||
    categoryRegExp_.find(nameInt) != categoryRegExp_.end();
}

void CategoryFilter::addCategoryName_(int nameIndex)
{
  // prevent duplicates
  if (categoryCheck_.find(nameIndex) != categoryCheck_.end())
    return;

  ValuesCheck values;
  values[simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME] = true;
  values[simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE] = true;
  CategoryValues catNameValPair;
  catNameValPair.first = true;
  catNameValPair.second = values;
  categoryCheck_[nameIndex] = catNameValPair;
}

void CategoryFilter::addCategoryValue_(int nameIndex, int valueIndex)
{
  CategoryCheck::iterator it = categoryCheck_.find(nameIndex);
  if (it == categoryCheck_.end())
  {
    addCategoryName_(nameIndex);
    it = categoryCheck_.find(nameIndex);
  }

  ValuesCheck& valuesCheck = it->second.second;

  // prevent duplicates
  if (valuesCheck.find(valueIndex) != valuesCheck.end())
    return;

  // The initial value should match the parent's Unlisted Value state
  auto i = valuesCheck.find(simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE);
  valuesCheck[valueIndex] = (i != valuesCheck.end() && i->second);
}

void CategoryFilter::clear_()
{
  categoryCheck_.clear();
  categoryRegExp_.clear();
}

void CategoryFilter::buildPrefRulesCategoryFilter()
{
  buildCategoryFilter_(true, true, true, true);
}

void CategoryFilter::buildCategoryFilter_(bool addNoValue, bool noValue, bool addUnlisted, bool unlisted)
{
  if (!dataStore_)
    return;

  CategoryCheck tempCheck = categoryCheck_; // temp holder of original values, if any
  categoryCheck_.clear(); // clean out categoryCheck_, since we are rebuilding it

  // rebuild the category check map, everything new defaults to true
  // find all names from the CategoryNameManager
  std::vector<int> catNameInts;
  simData::CategoryNameManager& catNameMgr = dataStore_->categoryNameManager();
  catNameMgr.allCategoryNameInts(catNameInts);
  for (std::vector<int>::const_iterator iter = catNameInts.begin(); iter != catNameInts.end(); ++iter)
  {
    bool nameChecked = true;
    // see if this name is already in checks, retain the value
    CategoryCheck::iterator catValueIter = tempCheck.find(*iter);
    if (catValueIter != tempCheck.end())
      nameChecked = catValueIter->second.first;
    CategoryValues catNameValPair;
    catNameValPair.first = nameChecked; // set the checked state of the category name
    ValuesCheck::iterator valuesCheckIter; // iterator for finding the value check
    // for each category name, add all values, retaining the old values
    std::vector<int> catValInts;
    catNameMgr.allValueIntsInCategory(*iter, catValInts);
    for (std::vector<int>::const_iterator valIter = catValInts.begin(); valIter != catValInts.end(); ++valIter)
    {
      bool valueChecked = true;
      if (catValueIter != tempCheck.end())
      {
        valuesCheckIter = catValueIter->second.second.find(*valIter);
        if (valuesCheckIter != catValueIter->second.second.end())
          valueChecked = valuesCheckIter->second; // retain old value if extant
      }
      // now add value to name value pair map, setting the checked state of the category value
      catNameValPair.second[*valIter] = valueChecked;
    }
    // handle adding no value and unlisted, retaining old values if they existed
    if (addNoValue)
    {
      bool valueChecked = noValue;
      if (catValueIter != tempCheck.end())
      {
        valuesCheckIter = catValueIter->second.second.find(simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME);
        if (valuesCheckIter != catValueIter->second.second.end())
          valueChecked = valuesCheckIter->second;
      }
      catNameValPair.second[simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME] =  valueChecked;
    }
    if (addUnlisted)
    {
      bool valueChecked = unlisted;
      if (catValueIter != tempCheck.end())
      {
        valuesCheckIter = catValueIter->second.second.find(simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE);
        if (valuesCheckIter != catValueIter->second.second.end())
          valueChecked = valuesCheckIter->second;
      }
      catNameValPair.second[simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE] = valueChecked;
    }
    // now add the structure to the map
    categoryCheck_[*iter] = catNameValPair;
  }
}

void CategoryFilter::getCurrentCategoryValues(const simData::DataStore& dataStore, uint64_t entityId, CurrentCategoryValues& curVals)
{
  // get the current values from the data store
  const simData::CategoryDataSlice* slice = dataStore.categoryDataSlice(entityId);
  if (!slice)
    return;

  slice->allInts(curVals);
}

const CategoryFilter::CategoryCheck& CategoryFilter::getCategoryFilter() const
{
  return categoryCheck_;
}

simData::DataStore* CategoryFilter::getDataStore() const
{
  return dataStore_;
}

void CategoryFilter::updateAll(bool value)
{
  for (CategoryCheck::iterator nameIter = categoryCheck_.begin(); nameIter != categoryCheck_.end(); ++nameIter)
  {
    CategoryValues& catValues = nameIter->second;
    catValues.first = value; // set all name check states
    for (ValuesCheck::iterator valueIter = catValues.second.begin(); valueIter != catValues.second.end(); ++valueIter)
    {
      valueIter->second = value; // set all value check states
    }
  }
}

void CategoryFilter::updateCategoryFilterName(int nameInt, bool nameChecked)
{
  CategoryCheck::iterator nameIter = categoryCheck_.find(nameInt);
  if (nameIter == categoryCheck_.end())
  {
    assert(0);
    return; // did not find name, return
  }
  CategoryValues& catValues = nameIter->second;
  catValues.first = nameChecked; // set the name check state
  for (ValuesCheck::iterator valueIter = catValues.second.begin(); valueIter != catValues.second.end(); ++valueIter)
  {
    valueIter->second = nameChecked; // set all value check states
  }
}

void CategoryFilter::updateCategoryFilterValue(int nameInt, int valueInt, bool valueChecked)
{
  CategoryCheck::iterator nameIter = categoryCheck_.find(nameInt);
  if (nameIter == categoryCheck_.end())
    return; // did not find name, return
  CategoryValues& catValues = nameIter->second;
  ValuesCheck::iterator valueIter = catValues.second.find(valueInt);
  if (valueIter == catValues.second.end())
    return; // did not find value, return
  valueIter->second = valueChecked;

  // now see if name check state needs to be updated
  if (valueChecked) // this is setting check state to true, make sure the name is now set to true
    catValues.first = true;
  else // name may need to be set to false now
  {
    bool setNameIter = false;
    for (ValuesCheck::const_iterator valIter = catValues.second.begin(); valIter != catValues.second.end(); ++valIter)
    {
      if (valIter->second) // if any value is true, set name to true
      {
        setNameIter = true;
        break;
      }
    }
    catValues.first = setNameIter;
  }
}

void CategoryFilter::setCategoryRegExp(int nameInt, const simData::RegExpFilterPtr& regExp)
{
  CategoryRegExp::iterator nameIter = categoryRegExp_.find(nameInt);

  // new entry, add to the map if this is a non-empty string
  if (nameIter == categoryRegExp_.end())
  {
    if (regExp != nullptr && !regExp->pattern().empty())
      categoryRegExp_[nameInt] = regExp;
    return;
  }

  // update the expression if non-empty, remove if empty
  if (regExp != nullptr && !regExp->pattern().empty())
    nameIter->second = regExp;
  else
    removeName(nameInt);
}

bool CategoryFilter::match(const simData::DataStore& dataStore, uint64_t entityId) const
{
  CurrentCategoryValues curCategoryData;
  CategoryFilter::getCurrentCategoryValues(dataStore, entityId, curCategoryData);
  return matchData(curCategoryData);
}

bool CategoryFilter::matchData(const CurrentCategoryValues& curCategoryData) const
{
  if (categoryCheck_.empty() && categoryRegExp_.empty())
    return true;

  CurrentCategoryValues::const_iterator curCategoryDataIter;
  CategoryCheck::const_iterator checksIter;
  ValuesCheck::const_iterator currentChecksValuesIter;
  int valueAtGivenTime;

  // steps through each of the categories in checks
  for (checksIter = categoryCheck_.begin();
    checksIter != categoryCheck_.end();
    ++checksIter)
  {
    // Ignore any category checks that have valid regular expressions
    auto regIter = categoryRegExp_.find(checksIter->first);
    if (regIter != categoryRegExp_.end() && regIter->second && !regIter->second->pattern().empty())
      continue;

    const CategoryValues& catValues = checksIter->second;
    // category is unchecked if and only if all children are unchecked
    const bool categoryIsChecked = catValues.first;

    // Skip testing this category if it's unchecked (does not apply to matching), or if name is special no-name value
    if (checksIter->first == simData::CategoryNameManager::NO_CATEGORY_NAME ||
      !categoryIsChecked)
      continue;

    // grab the pointer for ease of use
    const ValuesCheck* currentChecksValues = &(catValues.second);

    // checks if the curCategoryData has category data (name) for the current category
    curCategoryDataIter = curCategoryData.find(checksIter->first);
    if (curCategoryDataIter == curCategoryData.end())
    {
      // curCategoryData has no category data for the current check category

      // checks if there is a NoValue item in currentChecksValues
      currentChecksValuesIter = currentChecksValues->find(simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME);
      if (currentChecksValuesIter != currentChecksValues->end())
      {
        if (currentChecksValuesIter->second == true)
        {
          // a data value is not required for this category
          continue;
        }
        else
        {
          // a data value is required for this category
          return false;
        }
      }
      else
      {
        // assumes that if there is not a NoValue item in currentChecksValues then a data value is required
        return false;
      }
    }
    else
    {
      // curCategoryData has category data for the current check category
      valueAtGivenTime = curCategoryDataIter->second;

      // checks for a check value that corresponds to the valueAtGivenTime
      currentChecksValuesIter = currentChecksValues->find(valueAtGivenTime);
      if (currentChecksValuesIter == currentChecksValues->end())
      {
        // no check value was found that corresponded to the valueAtGivenTime

        // looks for an "unlisted value" item in currentChecksValues and if that is
        // checked, then pass, otherwise fail
        currentChecksValuesIter = currentChecksValues->find(simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE);
        if ((currentChecksValuesIter != currentChecksValues->end()) &&
          (currentChecksValuesIter->second == true))
          continue;
        else
          return false;
      }
      else
      {
        // checks if the current category's value is checked
        if (currentChecksValuesIter->second == true)
        {
          continue;
        }
        else
        {
          // the current category's value is not checked
          return false;
        }
      }
    }
  }

  // finally check against the RegExpFilters. Only fail if there is a regular expression and no match
  if (!matchRegExpFilter_(curCategoryData))
    return false;

  return true;
}

bool CategoryFilter::matchRegExpFilter_(const CurrentCategoryValues& curCategoryData) const
{
  // no failure if no regular expressions
  if (categoryRegExp_.empty() || dataStore_ == nullptr)
    return true;
  CurrentCategoryValues::const_iterator curCategoryDataIter;
  // first, check the reg exp, since this is likely to be more comprehensive
  simData::CategoryNameManager& catNameMgr = dataStore_->categoryNameManager();
  for (CategoryRegExp::const_iterator iter = categoryRegExp_.begin(); iter != categoryRegExp_.end(); ++iter)
  {
    // pass if the regexp is empty string
    if (iter->second->pattern().empty())
      continue;

    curCategoryDataIter = curCategoryData.find(iter->first);
    if (curCategoryDataIter != curCategoryData.end())
    {
      // convert value int to string for regular expression matching
      std::string valueString = catNameMgr.valueIntToString(curCategoryDataIter->second);
      // if the string doesn't match the regexp, then we fail
      if (!iter->second->match(valueString))
        return false;
    }
    // did not have category data required by regular expression, test empty string
    else if (!iter->second->match(""))
      return false;
  }
  return true;
}

std::string CategoryFilter::serialize(bool simplify) const
{
  if (dataStore_ == nullptr)
    return " ";

  simData::CategoryNameManager& catNameMgr = dataStore_->categoryNameManager();
  std::string rv;

  // Make a copy of the category checks
  CategoryCheck categoryCheckCopy = categoryCheck_;
  CategoryRegExp categoryRegExpCopy = categoryRegExp_;
  if (simplify)
  {
    simplifyRegExp_(categoryRegExpCopy);
    simplify_(categoryCheckCopy);

    if (categoryCheckCopy.empty() && categoryRegExpCopy.empty())
      return " "; // SIMDIS 9 expects this if no category filter
  }

  // Because the writing loop iterates on Category Check Copy and not on RegExp, we have a potential
  // problem where there's a RegExp but not a Category Check.  This is a certainty when we have RegExp
  // with simplification.  There are two solutions.  Either iterate category checks, then detect the
  // set_difference in the two map keys, or populate category checks with dummy maps.  Here we
  // populate the category checks copy with a dummy empty map.
  for (auto regIter = categoryRegExpCopy.begin(); regIter != categoryRegExpCopy.end(); ++regIter)
  {
    // Skip this regexp if it's not valid
    if (regIter->second == nullptr || regIter->second->pattern().empty())
      continue;

    auto catIter = categoryCheckCopy.find(regIter->first);
    // Always mark the value as enabled (true) to avoid ignoring the RegExp on parse
    if (catIter == categoryCheckCopy.end())
      categoryCheckCopy[regIter->first] = CategoryFilter::CategoryValues(true, CategoryFilter::ValuesCheck());
  }

  for (CategoryCheck::const_iterator categoryIter = categoryCheckCopy.begin();
    categoryIter != categoryCheckCopy.end();
    ++categoryIter)
  {
    int categoryName = categoryIter->first;
    const ValuesCheck& values = categoryIter->second.second;
    std::string regExp;

    CategoryRegExp::const_iterator regExpIter = categoryRegExpCopy.find(categoryName);
    if (regExpIter != categoryRegExpCopy.end())
      regExp = regExpIter->second->pattern();

    // Ignore if the category name int value is not valid
    if (categoryName == simData::CategoryNameManager::NO_CATEGORY_NAME)
      continue;

    // Note here that values.empty() and regExp.empty() is a valid state.  It means that the
    // filter is set up so that the category matches, but Unlisted Values is unchecked (default).
    // Therefore, all category matching will eventually fail on this filter.  But it is valid.

    std::string categoryNameString = catNameMgr.nameIntToString(categoryName);
    if (categoryNameString.empty())
      continue; // could not find a valid name for this int, skip it

    if (rv.empty())
      rv = categoryNameString;
    else
    {
      rv.append(SIM_PREF_RULE_CAT_SEP);
      rv.append(categoryNameString);
    }
    rv.push_back('(');
    rv.append(categoryIter->second.first ? "1" : "0");
    rv.push_back(')');

    // add the regular expression filter, if there is one for this category
    if (!regExp.empty())
    {
      rv.append(SIM_PREF_RULE_REGEXP_SEP);
      rv.append(regExp);
    }

    for (ValuesCheck::const_iterator valueIter = values.begin();
      valueIter != values.end();
      ++valueIter)
    {
      int categoryValue = valueIter->first;
      if (categoryValue == simData::CategoryNameManager::NO_CATEGORY_VALUE)
        continue;  // ignore if the category value is not valid

      std::string categoryValueString = catNameMgr.nameIntToString(categoryValue);
      if (categoryValueString.empty())
        continue; // could not find a valid value for this int, skip it

      rv.append(SIM_PREF_RULE_VAL_SEP);
      rv.append(categoryValueString);
      rv.push_back('(');
      rv.append(valueIter->second ? "1" : "0");
      rv.push_back(')');
    }
  }

  if (rv.empty())
    rv = " "; // SIMDIS 9 expects this if no category filter
  return rv;
}

///@return false on fail
bool CategoryFilter::deserialize(const std::string &checksString, bool skipEmptyCategories, RegExpFilterFactory* regExpFactory)
{
  if (dataStore_ == nullptr)
    return false;

  categoryCheck_.clear();
  categoryRegExp_.clear();

  // Empty string means no values, meaning clear vector; valid state
  if (simCore::StringUtils::trim(checksString).empty())
    return true;

  // categories are separated by back tick, break out the vector of categories
  std::vector<std::string> catStrVec;
  simCore::stringTokenizer(catStrVec, checksString, SIM_PREF_RULE_CAT_SEP, true, false);
  if (catStrVec.empty())
    return false; // something wrong

  simData::CategoryNameManager& categoryManager = dataStore_->categoryNameManager();

  // avoid loop overhead
  std::vector<std::string> valueStrVec;
  std::string tmpString;
  std::string categoryNameString;
  std::string categoryValueString;
  std::string checkString;
  bool hasErrors = false;

  for (std::vector<std::string>::const_iterator catStrIter = catStrVec.begin();
    catStrIter != catStrVec.end();
    ++catStrIter)
  {
    valueStrVec.clear();

    // within a category, constraints are separated by tilde
    simCore::stringTokenizer(valueStrVec, *catStrIter, SIM_PREF_RULE_VAL_SEP, true, false);

    // Empty category; ignore it
    if (valueStrVec.empty())
    {
      SIM_DEBUG << "Invalid category detected in filter.\n";
      continue;
    }

    // NOTE: structure of the category filter serialization is <name>^<regExp>~<val>~<val>`<name>^<regExp>~<val>~<val>...

    // first token has the category name
    tmpString = valueStrVec[0];
    // make sure to remove the regExp if it exists
    size_t regExpStart = tmpString.find_first_of(SIM_PREF_RULE_REGEXP_SEP);
    std::string regExpStr;
    if (regExpStart != std::string::npos)
    {
      // store off the regExp string first
      if (regExpStart < tmpString.size())
        regExpStr = tmpString.substr(regExpStart + 1);
      tmpString = tmpString.substr(0, regExpStart);
    }

    // minimum size of 4, includes at least 1 char for name and 3 for the '(0)' or '(1)' state
    if (tmpString.size() < 4)
    {
      SIM_DEBUG << "Invalid value string in filter: too short.\n";
      hasErrors = true;
      continue;
    }

    categoryNameString = tmpString.substr(0, tmpString.size() - 3);
    checkString = tmpString.substr(tmpString.size() - 3);

    // Enforce "(0)" or "(1)"
    const bool categoryChecked = (checkString == "(1)");
    if (!categoryChecked && checkString != "(0)")
    {
      SIM_DEBUG << "Invalid check string '" << checkString << "' for category " << categoryNameString << "\n";
      hasErrors = true;
      continue;
    }

    // skip unchecked categories if optimizing
    if (skipEmptyCategories && !categoryChecked)
      continue;

    const int categoryName = categoryManager.addCategoryName(categoryNameString);

    // process regular expression if it exists
    if (!regExpStr.empty())
    {
      // Assertion failure means caller is deserializing a regular expression without a factory to create them
      assert(regExpFactory);
      // add the reg exp
      if (regExpFactory)
        setCategoryRegExp(categoryName, regExpFactory->createRegExpFilter(regExpStr));
      else
      {
        SIM_DEBUG << "Unable to create regular expression for category '" << categoryNameString << "'\n";
        hasErrors = true;
        continue;
      }
    }

    // retrieve the values map
    CategoryCheck::iterator catIter = categoryCheck_.find(categoryName);
    if (catIter == categoryCheck_.end())
      catIter = categoryCheck_.insert(std::make_pair(categoryName, CategoryValues())).first;

    catIter->second.first = categoryChecked;
    ValuesCheck *const values = &catIter->second.second;

    // extract the category values
    std::vector<std::string>::const_iterator valueStrIter = valueStrVec.begin();
    // note that iter pre-increments to skip over the first item, which is the category name (and possibly regexp)
    for (++valueStrIter; valueStrIter != valueStrVec.end(); ++valueStrIter)
    {
      tmpString = *valueStrIter;

      // minimum size of 4, includes at least 1 char for name and 3 for the '(0)' or '(1)' state
      if (tmpString.size() < 4)
      {
        SIM_DEBUG << "Invalid value string in category '" << categoryNameString << "' in filter: too short.\n";
        hasErrors = true;
        continue;
      }

      categoryValueString = tmpString.substr(0, tmpString.size() - 3);
      checkString = tmpString.substr(tmpString.size() - 3);

      // Enforce "(0)" or "(1)"
      const bool checkValue = (checkString == "(1)");
      if (!checkValue && checkString != "(0)")
      {
        SIM_DEBUG << "Invalid check string '" << checkString << "' for category value " << categoryNameString << "." << categoryValueString << "\n";
        hasErrors = true;
        continue;
      }

      // test for 'unlisted value' or 'no value' here, don't add them to the data store
      if (categoryValueString == simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME_STR)
        (*values)[simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME] = checkValue;
      else if (categoryValueString == simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE_STR)
        (*values)[simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE] = checkValue;
      else
      {
        const int categoryValue = categoryManager.addCategoryValue(categoryName, categoryValueString);
        (*values)[categoryValue] = checkValue;
      }
    }
  }

  // True: success;  False: failure
  return !hasErrors;
}

bool CategoryFilter::deserialize(const std::string &checksString, RegExpFilterFactory& regExpFactory)
{
  return deserialize(checksString, true, &regExpFactory);
}

void CategoryFilter::simplifyValues_(CategoryFilter::CategoryCheck& checks) const
{
  // Iterate through each category
  for (CategoryCheck::iterator checksIter = checks.begin(); checksIter != checks.end(); ++checksIter)
    simplifyValues_(checksIter->second.second);
}

void CategoryFilter::simplifyValues_(CategoryFilter::ValuesCheck& values) const
{
  if (values.empty())
    return;

  // Get the value of the "Unlisted Value" entry
  ValuesCheck::const_iterator unlistedValueIter = values.find(simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE);
  // "Unlisted Value" defaults to OFF
  const bool unlistedValuesCheck = (unlistedValueIter == values.end()) ? false : unlistedValueIter->second;

  // Create a new values vector; we are going to remove items
  ValuesCheck newValues;
  for (ValuesCheck::const_iterator valuesIter = values.begin(); valuesIter != values.end(); ++valuesIter)
  {
    // Only include items that are different than Unlisted (and also include unlisted)
    // also include "No Value" if it is true (defaults to false)
    bool saveValue = false;

    // Unlisted Value is only needed if set to true
    if (valuesIter->first == simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE && unlistedValuesCheck)
      saveValue = true;
    // No Value is only needed if set to true
    else if (valuesIter->first == simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME)
      saveValue = valuesIter->second;
    // Other values are only needed if different from "Unlisted Value"
    else if (valuesIter->second != unlistedValuesCheck)
      saveValue = true;

    // Copy to the "newValues" map
    if (saveValue)
      newValues[valuesIter->first] = valuesIter->second;
  }

  // Replace the contents
  if (newValues != values)
    values = newValues;
}

bool CategoryFilter::doesCategoryAffectFilter_(int nameInt, const CategoryFilter::CategoryValues& nameBoolAndChecks) const
{
  // Precondition: values under this category are already simplified

  // if a reg exp exists for this category name, keep it, no matter if it has other values
  if (categoryRegExp_.find(nameInt) != categoryRegExp_.end())
    return true;

  // No checks are on, skip this category
  if (nameBoolAndChecks.first == false)
    return false;

  const ValuesCheck& values = nameBoolAndChecks.second;

  // if values is empty, then "Unlisted Value" defaults OFF, so nothing should match this filter, but it still is valid
  if (values.empty())
    return true;

  // Get the value of the "Unlisted Value" entry
  ValuesCheck::const_iterator unlistedIter = values.find(simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE);
  const bool unlistedValue = (unlistedIter != values.end()) ? unlistedIter->second : false;

  // if "Unlisted Value" is not set, and something else is
  if (!unlistedValue && !values.empty())
    return true;

  // Get the value of the "No Value" entry
  ValuesCheck::const_iterator novalueIter = values.find(simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME);
  const bool novalueValue = (novalueIter != values.end()) ? novalueIter->second : false;

  // if "No Value" is not set, and something else is
  if (!novalueValue && !values.empty())
    return true;

  // Investigate each value
  for (ValuesCheck::const_iterator valuesIter = values.begin(); valuesIter != values.end(); ++valuesIter)
  {
    // If any single value is off, then the category name does affect filtering, return true
    if (valuesIter->second == false)
      return true;
  }
  return false;
}

void CategoryFilter::simplifyCategories_(CategoryFilter::CategoryCheck& checks) const
{
  // Precondition: values under this category are already simplified
  CategoryCheck newChecks;
  for (CategoryCheck::const_iterator checksIter = checks.begin(); checksIter != checks.end(); ++checksIter)
  {
    if (doesCategoryAffectFilter_(checksIter->first, checksIter->second))
      newChecks[checksIter->first] = checksIter->second;
  }

  // Assign the value if there's been any changes
  if (newChecks != checks)
    checks = newChecks;
}

void CategoryFilter::simplifyRegExp_(CategoryFilter::CategoryRegExp& regExps) const
{
  for (auto i = regExps.begin(); i != regExps.end(); /* no increment */)
  {
    if (i->second == nullptr || i->second->pattern().empty())
      regExps.erase(i++);
    else
      ++i;
  }
}

void CategoryFilter::simplify_(CategoryFilter::CategoryCheck& checks) const
{
  // Remove all categories that have a non-empty regular expression
  for (auto i = categoryRegExp_.begin(); i != categoryRegExp_.end(); ++i)
  {
    if (i->second != nullptr && !i->second->pattern().empty())
      checks.erase(i->first);
  }
  simplifyValues_(checks);
  simplifyCategories_(checks);
}

void CategoryFilter::simplify(int categoryName)
{
  // Search in checks -- we'll need this iterator soon
  auto i = categoryCheck_.find(categoryName);

  // Remove the entire category if there's a valid regex associated
  auto refIter = categoryRegExp_.find(categoryName);
  if (refIter != categoryRegExp_.end())
  {
    // Clean up categoryRegExp_ first
    if (refIter->second == nullptr || refIter->second->pattern().empty())
      categoryRegExp_.erase(refIter);
    else if (i != categoryCheck_.end())
    {
      categoryCheck_.erase(i);
      i = categoryCheck_.end();
    }
  }

  // Exit now if there is no checks state for category
  if (i == categoryCheck_.end())
    return;

  // First simplify the check values
  simplifyValues_(i->second.second);

  // Then remove the category if it doesn't add value
  if (!doesCategoryAffectFilter_(categoryName, i->second))
  {
    categoryCheck_.erase(i);
    // Assertion failure means we get out of sync with the regex, and implies a failure
    // in the method doesCategoryAffectFilter_().
    assert(categoryRegExp_.find(categoryName) == categoryRegExp_.end());
  }
}

void CategoryFilter::simplify()
{
  simplifyRegExp_(categoryRegExp_);
  simplify_(categoryCheck_);
}

void CategoryFilter::clear()
{
  categoryCheck_.clear();
  categoryRegExp_.clear();
}

void CategoryFilter::setValue(int nameInt, int valueInt, bool valueChecked)
{
  // Avoid setting NO_CATEGORY_VALUE.  In this class, we use NO_CATEGORY_VALUE_AT_TIME instead.
  // Rather than failing later, we address the problem here.  This prevents a common bug of
  // trying to set the "No Value" here by using categoryNameMgr.valueToInt("No Value").
  if (valueInt == CategoryNameManager::NO_CATEGORY_VALUE)
    valueInt = CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME;

  // Create the category if it does not exist
  CategoryCheck::iterator catIter = categoryCheck_.find(nameInt);
  if (catIter == categoryCheck_.end())
    catIter = categoryCheck_.insert(std::make_pair(nameInt, CategoryValues())).first;

  // By default the category should do something useful
  auto& categoryChecks = catIter->second;
  categoryChecks.first = true;
  categoryChecks.second[valueInt] = valueChecked;
}

void CategoryFilter::removeName(int nameInt)
{
  categoryCheck_.erase(nameInt);
  categoryRegExp_.erase(nameInt);
}

int CategoryFilter::removeValue(int nameInt, int valueInt)
{
  // Avoid setting NO_CATEGORY_VALUE.  In this class, we use NO_CATEGORY_VALUE_AT_TIME instead.
  // Rather than failing later, we address the problem here.  This prevents a common bug of
  // trying to remove the "No Value" here by using categoryNameMgr.valueToInt("No Value").
  if (valueInt == CategoryNameManager::NO_CATEGORY_VALUE)
    valueInt = CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME;

  // Find the entry for the category name in all of our categories
  auto nameIter = categoryCheck_.find(nameInt);
  if (nameIter != categoryCheck_.end())
  {
    // Find the entry for the value of this category name
    auto& valueMap = nameIter->second.second;
    auto valueIter = valueMap.find(valueInt);
    if (valueIter != valueMap.end())
    {
      valueMap.erase(valueIter);

      // If the value map is empty, then the category name does not contribute to
      // the filtering in any way (either in deserialize(true) or deserialize(false).
      // So we remove the entire key (category) here in that case.
      if (valueMap.empty())
        categoryCheck_.erase(nameIter);
      return 0;
    }
  }

  // Did not remove anything
  return 1;
}

void CategoryFilter::getNames(std::vector<int>& names) const
{
  // Combine the names from category checks and category regexp into a set
  std::set<int> namesSet;
  for (auto i = categoryCheck_.begin(); i != categoryCheck_.end(); ++i)
    namesSet.insert(i->first);
  for (auto i = categoryRegExp_.begin(); i != categoryRegExp_.end(); ++i)
    namesSet.insert(i->first);

  // Convert set into a vector
  names.resize(namesSet.size());
  names.assign(namesSet.begin(), namesSet.end());
}

void CategoryFilter::getValues(int nameInt, ValuesCheck& checks) const
{
  checks.clear();
  auto i = categoryCheck_.find(nameInt);
  if (i != categoryCheck_.end())
    checks = i->second.second;
}

const simData::RegExpFilter* CategoryFilter::getRegExp(int nameInt) const
{
  auto i = categoryRegExp_.find(nameInt);
  return (i == categoryRegExp_.end() ? nullptr : i->second.get());
}

std::string CategoryFilter::getRegExpPattern(int nameInt) const
{
  auto i = categoryRegExp_.find(nameInt);
  if (i == categoryRegExp_.end())
    return "";
  return i->second->pattern();
}

}
