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
#ifndef SIMDATA_CATEGORY_NAME_MANAGER_H
#define SIMDATA_CATEGORY_NAME_MANAGER_H

#include <memory>
#include <vector>
#include "entt/container/dense_map.hpp"
#include "simData/CategoryData/CategoryData.h"

namespace simData {

/** manages category name and value mappings to int
 *
 * There should be one category manager, which is used by the other category
 * data elements to convert between int and string
 */
class SDKDATA_EXPORT CategoryNameManager
{

public:

/// Provides notification when new category names or values are created
class Listener
{
public:
  virtual ~Listener() {}

  /// Invoked when a new category is added
  virtual void onAddCategory(int categoryIndex) = 0;

  /// Invoked when a new value is added to a category
  virtual void onAddValue(int categoryIndex, int valueIndex) = 0;

  /// Invoked when all data is cleared
  virtual void onClear() = 0;

  /// Invoked after all onClear are called so that a Listener can safely add category data
  virtual void doneClearing() = 0;
};

  /// Managed pointer to be used when holding a pointer to a Observer object.
  /// Memory for the Listener object is deleted automatically when the last managed pointer is released.
  typedef std::shared_ptr<Listener> ListenerPtr;

  CategoryNameManager();

  /** Return the case-sensitive state of the manager. */
  bool isCaseSensitive() const;

  /**
   * By default the category manager is case sensitive; call this routine with false to make category manager case insensitive.
   * This routine should only be called once and should be called immediately after the constructor.
   * @param caseSensitive If true category tags and category values are case sensitive.
   * @return 0 on success, non zero on failure because the map_ was not empty
   */
  int setCaseSensitive(bool caseSensitive);

  /// clear all category name and value mappings
  ///@note: this will invalidate any int id's being held elsewhere
  void clear();

  /// add a new category
  ///@return new id that was assigned
  int addCategoryName(const std::string &name);

  /// add a new value in the given category
  ///@return new id that was assigned
  int addCategoryValue(int nameInt, const std::string &value);

  /// remove the given category (and all it's values)
  void removeCategory(int nameInt);
  /// remove just one value from the given category
  void removeValue(int nameInt, int valueInt);

  /// provide one category name mapping: string to int
  int nameToInt(const std::string &name) const;
  /// provide one category value mapping: string to int
  int valueToInt(const std::string &value) const;

  /// provide one category name mapping: int to string
  std::string nameIntToString(int nameInt) const;
  /// provide one category value mapping: int to string
  std::string valueIntToString(int valueInt) const;

  /// retrieve all the category name strings
  void allCategoryNames(std::vector<std::string> &nameVec) const;
  /// retrieve all the category name keys
  void allCategoryNameInts(std::vector<int> &nameIntVec) const;

  /// retrieve all the value strings in a given category
  void allValuesInCategory(int categoryInt, std::vector<std::string> &categoryValueVec) const;
  /// retrieve all the value keys in a given category
  void allValueIntsInCategory(int categoryInt, std::vector<int> &categoryValueIntVec) const;

  /// Add a listener for category messages
  void addListener(ListenerPtr callback);
  /// Remove a listener for category messages
  void removeListener(ListenerPtr callback);
  /// Get listeners
  void getListeners(std::vector<ListenerPtr>& listeners);

  static const int NO_CATEGORY_NAME; ///< used when there is no category name to refer to
  static const int NO_CATEGORY_VALUE; ///< used when there is no category value to refer to
  static const int NO_CATEGORY_VALUE_AT_TIME; ///< used with Pref Rules indicates if there is no category data expected at the specified time
  static const int UNLISTED_CATEGORY_VALUE; ///< used with Pref Rules when there is category data at the specified time, indicates if an equivalent value it not expected

  ///< string versions of the above constants
  static const std::string NO_CATEGORY_NAME_STR; ///< invalid category name string
  static const std::string NO_CATEGORY_VALUE_STR; ///< invalid category value string
  static const std::string NO_CATEGORY_VALUE_AT_TIME_STR; ///< string associated with the Pref Rule index above
  static const std::string UNLISTED_CATEGORY_VALUE_STR; ///< string associated with the Pref Rule index above

private:
  /// Defined but not implemented; if implemented will need to clearout listeners_
  CategoryNameManager(const CategoryNameManager& rhs);

  /// Return the string if caseSensitive_ is true or return an upper case version of the string if caseSensitive_ is false
  std::string fixString_(const std::string &str) const;

  /// Get or create an ID for the given string
  int getOrCreateStringId_(const std::string &str);

  bool caseSensitive_;
  int nextInt_;

  /// all the values for a given category name
  entt::dense_map<int, std::vector<int>> categoryStringInts_;

  entt::dense_map<std::string, int> map_;
  entt::dense_map<int, std::string> reverseMap_;

  std::vector<ListenerPtr> listeners_;
};
}

#endif

