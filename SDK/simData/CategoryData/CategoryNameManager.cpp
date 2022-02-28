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
#include <algorithm>
#include <cassert>
#include "simCore/String/Format.h"
#include "simData/CategoryData/CategoryNameManager.h"

namespace simData
{
//----------------------------------------------------------------------------
const std::string CategoryNameManager::NO_CATEGORY_NAME_STR = "No Name";
const std::string CategoryNameManager::NO_CATEGORY_VALUE_STR = "No Value";
const std::string CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME_STR = "No Value";
const std::string CategoryNameManager::UNLISTED_CATEGORY_VALUE_STR = "Unlisted Value";
const int CategoryNameManager::NO_CATEGORY_NAME = -1;
const int CategoryNameManager::NO_CATEGORY_VALUE = -1;
const int CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME = -2;
const int CategoryNameManager::UNLISTED_CATEGORY_VALUE = -3;

//----------------------------------------------------------------------------
CategoryNameManager::CategoryNameManager()
  : caseSensitive_(true),
    nextInt_(1)
{
}

int CategoryNameManager::setCaseSensitive(bool caseSensitive)
{
  if (!map_.empty())
    return 1;

  caseSensitive_ = caseSensitive;
  return 0;
}

void CategoryNameManager::clear()
{
  map_.clear();
  reverseMap_.clear();
  categoryStringInts_.clear();
  for (std::vector<ListenerPtr>::const_iterator i = listeners_.begin(); i != listeners_.end(); ++i)
  {
    (*i)->onClear();
  }

  for (std::vector<ListenerPtr>::const_iterator i = listeners_.begin(); i != listeners_.end(); ++i)
  {
    (*i)->doneClearing();
  }
}

std::string CategoryNameManager::fixString_(const std::string &str) const
{
  if (caseSensitive_)
    return str;

  return simCore::upperCase(str);
}


int CategoryNameManager::getOrCreateStringId_(const std::string &str)
{
  std::string key = fixString_(str);
  std::map<std::string, int>::const_iterator i = map_.find(key);

  if (i != map_.end())
    return i->second;

  // generate id
  int id = nextInt_;
  ++nextInt_;

  map_[key] = id;
  reverseMap_[id] = str;  // Use str so the original case is maintained

  return id;
}

/// add a new category
///@return new id that was assigned
int CategoryNameManager::addCategoryName(const std::string &name)
{
  int catInt = getOrCreateStringId_(name);

  if (categoryStringInts_.find(catInt) == categoryStringInts_.end())
  {
    // add the key "catInt" to the map
    categoryStringInts_[catInt];

    for (std::vector<ListenerPtr>::const_iterator i = listeners_.begin(); i != listeners_.end(); ++i)
    {
      (*i)->onAddCategory(catInt);
    }
  }

  return catInt;
}

/// add a new value in the given category
///@return new id that was assigned
int CategoryNameManager::addCategoryValue(int nameInt, const std::string &value)
{
  // 1. get an id for the value
  int valueInt = getOrCreateStringId_(value);

  // 2. add the value to the category list
  std::map<int, std::vector<int> >::iterator i = categoryStringInts_.find(nameInt);
  if (i != categoryStringInts_.end())
  {
    // check if the category already has the value
    std::vector<int>& vecInt = i->second;
    std::vector<int>::const_iterator valueIter = std::find(vecInt.begin(), vecInt.end(), valueInt);
    if (valueIter != vecInt.end())
      return valueInt; // it does, done

    //else add it
    vecInt.push_back(valueInt);
  }
  else // category has no values
  {
    categoryStringInts_[nameInt].push_back(valueInt);
  }

  for (std::vector<ListenerPtr>::const_iterator i = listeners_.begin(); i != listeners_.end(); ++i)
  {
    (*i)->onAddValue(nameInt, valueInt);
  }

  return valueInt;
}

void CategoryNameManager::removeCategory(int nameInt)
{
  std::map<int, std::vector<int> >::iterator i = categoryStringInts_.find(nameInt);
  if (i != categoryStringInts_.end())
    categoryStringInts_.erase(i);

  // we will leave the mapping, the category might come back, and we don't keep a reference count
}

void CategoryNameManager::removeValue(int nameInt, int valueInt)
{
  std::map<int, std::vector<int> >::iterator i = categoryStringInts_.find(nameInt);
  if (i != categoryStringInts_.end())
  {
    std::vector<int>& vecInt = i->second;
    std::vector<int>::iterator j = std::find(vecInt.begin(), vecInt.end(), valueInt);
    if (j != vecInt.end())
      vecInt.erase(j);
  }
}

// provide one mapping: string to int
int CategoryNameManager::nameToInt(const std::string &name) const
{
  std::map<std::string, int>::const_iterator i = map_.find(fixString_(name));
  if (i == map_.end())
    return CategoryNameManager::NO_CATEGORY_NAME; // category name not found

  return i->second;
}

int CategoryNameManager::valueToInt(const std::string &value) const
{
  std::map<std::string, int>::const_iterator i = map_.find(fixString_(value));
  if (i == map_.end())
    return CategoryNameManager::NO_CATEGORY_VALUE; // category value not found

  return i->second;
}

// provide mapping: int to string
std::string CategoryNameManager::nameIntToString(int nameInt) const
{
  std::map<int, std::string>::const_iterator i = reverseMap_.find(nameInt);
  if (i == reverseMap_.end())
  {
    if (nameInt == CategoryNameManager::NO_CATEGORY_VALUE)
      return CategoryNameManager::NO_CATEGORY_VALUE_STR;
    else if (nameInt == CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME)
      return CategoryNameManager::NO_CATEGORY_VALUE_AT_TIME_STR;
    else if (nameInt == CategoryNameManager::NO_CATEGORY_NAME)
      return CategoryNameManager::NO_CATEGORY_NAME_STR;
    else if (nameInt == CategoryNameManager::UNLISTED_CATEGORY_VALUE)
      return CategoryNameManager::UNLISTED_CATEGORY_VALUE_STR;
    else
      return ""; // not found
  }

  return i->second;
}

std::string CategoryNameManager::valueIntToString(int valueInt) const
{
  return nameIntToString(valueInt); // there is only one map for both sets of strings
}

// retrieve all the category names (as strings or ints)
void CategoryNameManager::allCategoryNames(std::vector<std::string> &nameVec) const
{
  // for each entry in the category string ints
  for (std::map<int, std::vector<int> >::const_iterator i = categoryStringInts_.begin(); i != categoryStringInts_.end(); ++i)
  {
    // add the name (which we get from the reverse map, using the category id)
    std::map<int, std::string>::const_iterator j = reverseMap_.find(i->first);
    if (j != reverseMap_.end())
      nameVec.push_back(j->second);
  }
}

void CategoryNameManager::allCategoryNameInts(std::vector<int> &nameIntVec) const
{
  // for each entry in the category string ints
  for (std::map<int, std::vector<int> >::const_iterator i = categoryStringInts_.begin(); i != categoryStringInts_.end(); ++i)
  {
    // add the name id
    nameIntVec.push_back(i->first);
  }
}

// retrieve all the values in a given category
void CategoryNameManager::allValuesInCategory(int categoryInt, std::vector<std::string> &categoryValueVec) const
{
  // find category in the category string ints
  std::map<int, std::vector<int> >::const_iterator i = categoryStringInts_.find(categoryInt);
  if (i != categoryStringInts_.end())
  {
    //for each value in the category
    for (std::vector<int>::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
    {
      // add the string value (which we get from the reverse map, using the value id)
      std::map<int, std::string>::const_iterator k = reverseMap_.find(*j);
      if (k != reverseMap_.end())
        categoryValueVec.push_back(k->second);
    }
  }
}

void CategoryNameManager::allValueIntsInCategory(int categoryInt, std::vector<int> &categoryValueIntVec) const
{
  // find category in the category string ints
  std::map<int, std::vector<int> >::const_iterator i = categoryStringInts_.find(categoryInt);
  if (i != categoryStringInts_.end())
  {
    //for each value in the category
    for (std::vector<int>::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
    {
      // add the string value (which we get from the reverse map, using the value id)
      categoryValueIntVec.push_back(*j);
    }
  }
}

void CategoryNameManager::addListener(CategoryNameManager::ListenerPtr callback)
{
  // don't add it twice
  assert(std::find(listeners_.begin(), listeners_.end(), callback) == listeners_.end());
  listeners_.push_back(callback);
}

void CategoryNameManager::removeListener(CategoryNameManager::ListenerPtr callback)
{
  // Removing something that does not exist
  assert(std::find(listeners_.begin(), listeners_.end(), callback) != listeners_.end());
  listeners_.erase(std::remove(listeners_.begin(), listeners_.end(), callback), listeners_.end());
}

void CategoryNameManager::getListeners(std::vector<ListenerPtr>& listeners)
{
  listeners = listeners_;
}

}
