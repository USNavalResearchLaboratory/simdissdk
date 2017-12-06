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
#include "simCore/String/Tokenizer.h"
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
  // Does nothing without a datastore
  assert(dataStore != NULL);
  if (dataStore_ == NULL)
    return;

  if (autoUpdate_)
  {
    buildCategoryFilter_(true, true, true, true);

    // create observers/listeners
    listenerPtr_ = simData::CategoryNameManager::ListenerPtr(new CategoryFilterListener(this));
    dataStore_->categoryNameManager().addListener(listenerPtr_);
  }
}

CategoryFilter::CategoryFilter(const CategoryFilter& other)
  : dataStore_(other.dataStore_),
    regExpFactory_(other.regExpFactory_),
    autoUpdate_(other.autoUpdate_),
    categoryCheck_(other.categoryCheck_),
    categoryRegExp_(other.categoryRegExp_)
{
  if (dataStore_ == NULL)
    return;

  if (autoUpdate_)
  {
    // create observers/listeners
    listenerPtr_ = simData::CategoryNameManager::ListenerPtr(new CategoryFilterListener(this));
    dataStore_->categoryNameManager().addListener(listenerPtr_);
  }
}

CategoryFilter::~CategoryFilter()
{
  if ((dataStore_ != NULL) && (listenerPtr_ != NULL))
    dataStore_->categoryNameManager().removeListener(listenerPtr_);
}

CategoryFilter& CategoryFilter::operator=(const CategoryFilter& other)
{
  if (&other == this)
    return *this;

  if ((dataStore_ != NULL) && (listenerPtr_ != NULL))
  {
    dataStore_->categoryNameManager().removeListener(listenerPtr_);
    listenerPtr_.reset();
  }
  dataStore_ = other.dataStore_;
  regExpFactory_ = other.regExpFactory_;
  autoUpdate_ = other.autoUpdate_;
  categoryCheck_ = other.categoryCheck_;
  categoryRegExp_ = other.categoryRegExp_;

  if (dataStore_ != NULL && autoUpdate_)
  {
    // re-add observers/listeners
    assert(listenerPtr_ == NULL);
    listenerPtr_.reset(new CategoryFilterListener(this));
    dataStore_->categoryNameManager().addListener(listenerPtr_);
  }

  return *this;
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

  // The initial value should match the parent's current value
  valuesCheck[valueIndex] = it->second.first;
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

void CategoryFilter::getCurrentCategoryValues(simData::DataStore& dataStore, uint64_t entityId, CurrentCategoryValues& curVals)
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

void CategoryFilter::updateCategoryFilterRegExp(int nameInt, const std::string& regExp)
{
  // nothing to do if no reg exp factory
  if (!regExpFactory_)
    return;

  CategoryRegExp::iterator nameIter = categoryRegExp_.find(nameInt);

  // if an invalid ptr was returned, reg exp was invalid
  simData::RegExpFilterPtr regExpObject = regExpFactory_->createRegExpFilter(regExp);

  if (!regExpObject)
    return;

  // new entry, add to the map if this is a non-empty string
  if (nameIter == categoryRegExp_.end())
  {
    if (!regExp.empty())
      categoryRegExp_[nameInt] = regExpObject;
    return;
  }

  // update the expression if non-empty, remove if empty
  if (!regExp.empty())
    nameIter->second = regExpObject;
  else
    categoryRegExp_.erase(nameIter);
}

bool CategoryFilter::match(uint64_t entityId) const
{
  if (dataStore_ == NULL)
    return true;
  CurrentCategoryValues curCategoryData;
  CategoryFilter::getCurrentCategoryValues(*dataStore_, entityId, curCategoryData);
  return matchData(curCategoryData);
}

bool CategoryFilter::matchData(const CurrentCategoryValues& curCategoryData) const
{
  if (categoryCheck_.empty())
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
    const CategoryValues& catValues = checksIter->second;
    // category is unchecked if and only if all children are unchecked
    const bool categoryIsChecked = catValues.first;

    // no category values, move on
    if (catValues.second.size() == 0 ||
      checksIter->first == simData::CategoryNameManager::NO_CATEGORY_NAME ||
      !categoryIsChecked)
    {
      continue;
    }

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
  if (categoryRegExp_.empty() || dataStore_ == NULL)
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
  if (dataStore_ == NULL)
    return " ";

  simData::CategoryNameManager& catNameMgr = dataStore_->categoryNameManager();
  std::string rv;

  // Make a copy of the category checks
  CategoryCheck categoryCheckCopy = categoryCheck_;
  if (simplify)
  {
    simplify_(categoryCheckCopy);

    if (categoryCheckCopy.empty())
      return " "; // SIMDIS 9 expects this if no category filter
  }

  for (CategoryCheck::const_iterator categoryIter = categoryCheckCopy.begin();
    categoryIter != categoryCheckCopy.end();
    ++categoryIter)
  {
    int categoryName = categoryIter->first;
    const ValuesCheck& values = categoryIter->second.second;
    std::string regExp;

    CategoryRegExp::const_iterator regExpIter = categoryRegExp_.find(categoryName);
    if (regExpIter != categoryRegExp_.end())
      regExp = regExpIter->second->pattern();

    if ((values.empty() && regExp.empty()) || (categoryName == simData::CategoryNameManager::NO_CATEGORY_NAME))
      continue; // ignore if there are no values and no regExp, or the category name int value is not valid

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
bool CategoryFilter::deserialize(const std::string &checksString, bool skipEmptyCategories)
{
  if (dataStore_ == NULL)
    return false;

  categoryCheck_.clear();

  // Empty string means no values, meaning clear vector; valid state
  if (checksString.empty())
    return true;

  // categories are separated by back tick, break out the vector of categories
  std::vector<std::string> catStrVec;
  simCore::stringTokenizer(catStrVec, checksString, SIM_PREF_RULE_CAT_SEP);
  if (catStrVec.empty())
    return false; // something wrong

  simData::CategoryNameManager& categoryManager = dataStore_->categoryNameManager();

  // avoid loop overhead
  std::vector<std::string> valueStrVec;
  std::string tmpString;
  std::string categoryNameString;
  std::string categoryValueString;

  for (std::vector<std::string>::const_iterator catStrIter = catStrVec.begin();
    catStrIter != catStrVec.end();
    ++catStrIter)
  {
    valueStrVec.clear();

    // within a category, constraints are separated by tilde
    simCore::stringTokenizer(valueStrVec, *catStrIter, SIM_PREF_RULE_VAL_SEP);
    std::string regExpStr;

    if (valueStrVec.empty())
      continue;

    // NOTE: structure of the category filter serialization is <name>^<regExp>~<val>~<val>`<name>^<regExp>~<val>~<val>...

    // first token has the category name
    tmpString = valueStrVec[0];
    // make sure to remove the regExp if it exists
    size_t regExpStart = tmpString.find_first_of(SIM_PREF_RULE_REGEXP_SEP);
    if (regExpStart > 0)
    {
      // store off the regExp string first
      if (regExpStart < tmpString.size())
        regExpStr = tmpString.substr(regExpStart + 1);
      tmpString = tmpString.substr(0, regExpStart);
    }

    // minimum size of 4, includes at least 1 char for name and 3 for the '(0)' or '(1)' state
    if (tmpString.size() < 4)
      continue;

    categoryNameString.assign(tmpString, 0, (int(tmpString.size()) - 3));
    if (categoryNameString.empty())
      return false;

    const char checkString = tmpString[(int(tmpString.size()) - 2)];
    const bool categoryChecked = (checkString == '1');

    // skip unchecked categories if optimizing
    if (skipEmptyCategories && !categoryChecked)
      continue;

    const int categoryName = categoryManager.addCategoryName(categoryNameString);

    // process regular expression if it exists
    if (!regExpStr.empty())
    {
      // add the reg exp
      updateCategoryFilterRegExp(categoryName, regExpStr);
    }

    // retrieve the values map
    CategoryCheck::iterator catIter = categoryCheck_.find(categoryName);
    if (catIter == categoryCheck_.end())
    {
      catIter = categoryCheck_.insert(std::make_pair(categoryName, CategoryValues())).first;
    }

    catIter->second.first = categoryChecked;
    ValuesCheck *const values = &catIter->second.second;

    // extract the category values
    std::vector<std::string>::const_iterator valueStrIter = valueStrVec.begin();
    // note that iter pre-increments to skip over the first item, which is the category name (and possibly regexp)
    for (++valueStrIter; valueStrIter != valueStrVec.end(); ++valueStrIter)
    {
      tmpString = *valueStrIter;

      categoryValueString.assign(tmpString, 0, (int(tmpString.size()) - 3));
      if (categoryValueString.empty())
      {
        return false;
      }

      const char checkString = tmpString[(int(tmpString.size()) - 2)];
      const bool checkValue = (checkString == '1');

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

  return true;
}

void CategoryFilter::setRegExpFilterFactory(RegExpFilterFactoryPtr factory)
{
  regExpFactory_ = factory;
}

void CategoryFilter::simplifyValues_(CategoryFilter::CategoryCheck& checks) const
{
  for (CategoryCheck::iterator checksIter = checks.begin(); checksIter != checks.end(); ++checksIter)
  {
    ValuesCheck& values = checksIter->second.second;
    // Get the value of the "Unlisted Value" entry
    ValuesCheck::const_iterator valuesIter = values.find(simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE);
    if (valuesIter != values.end())
    {
      const bool isListed = valuesIter->second;

      // Create a new values vector
      ValuesCheck newValues;
      for (valuesIter = values.begin(); valuesIter != values.end(); ++valuesIter)
      {
        // Only include items that are different than Unlisted (and also include unlisted)
        // also include "No Value" if it is true (defaults to false)
        if ((valuesIter->first == simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE && isListed) ||
            (valuesIter->first == simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME && valuesIter->second) ||
            valuesIter->second != isListed)
        {
          newValues[valuesIter->first] = valuesIter->second;
        }
      }

      // Replace the contents
      if (newValues != values)
        checksIter->second.second = newValues;
    }
  }
}

void CategoryFilter::simplifyCategories_(CategoryFilter::CategoryCheck& checks) const
{
  CategoryCheck newChecks;
  for (CategoryCheck::const_iterator checksIter = checks.begin(); checksIter != checks.end(); ++checksIter)
  {
    // if a reg exp exists for this category name, keep it, no matter if it has other values
    if (categoryRegExp_.find(checksIter->first) != categoryRegExp_.end())
    {
      newChecks[checksIter->first] = checksIter->second;
      continue;
    }

    // No checks are on, skip this category
    if (checksIter->second.first == false)
      continue;

    const ValuesCheck& values = checksIter->second.second;

    // Get the value of the "Unlisted Value" entry
    ValuesCheck::const_iterator unlistedIter = values.find(simData::CategoryNameManager::UNLISTED_CATEGORY_VALUE);
    const bool unlistedValue = (unlistedIter != values.end()) ? unlistedIter->second : false;

    // if "Unlisted Value" is not set, and something else is
    if (!unlistedValue && !values.empty())
    {
      // need this check
      newChecks[checksIter->first] = checksIter->second;
      continue;
    }

    // Get the value of the "No Value" entry
    ValuesCheck::const_iterator novalueIter = values.find(simData::CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME);
    const bool novalueValue = (novalueIter != values.end()) ? novalueIter->second : false;

    // if "No Value" is not set, and something else is
    if (!novalueValue && !values.empty())
    {
      // need this check
      newChecks[checksIter->first] = checksIter->second;
      continue;
    }

    // Investigate each value
    for (ValuesCheck::const_iterator valuesIter = values.begin(); valuesIter != values.end(); ++valuesIter)
    {
      // If any single value is off, add the item to the new checks system and break out
      if (valuesIter->second == false)
      {
        newChecks[checksIter->first] = checksIter->second;
        break;
      }
    }
  }

  // Assign the value if there's been any changes
  if (newChecks != checks)
    checks = newChecks;
}

void CategoryFilter::simplify_(CategoryFilter::CategoryCheck& checks) const
{
  simplifyValues_(checks);
  simplifyCategories_(checks);
}

}
