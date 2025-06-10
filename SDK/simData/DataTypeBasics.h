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
#ifndef DATATYPE_BASICS_H
#define DATATYPE_BASICS_H

#include <limits>
#include <optional>

namespace simData
{
constexpr double INVALID_UPDATE_DOUBLE = std::numeric_limits<double>::max();
constexpr float INVALID_UPDATE_FLOAT = std::numeric_limits<float>::max();

/** A replacement for std::optional<double> that uses less memory */
class OptionalDouble
{
public:
  inline bool has_value() const { return value_ != INVALID_UPDATE_DOUBLE; }
  inline double value() const { return value_;  }
  inline double value_or(double defaultValue) const { return has_value() ? value_ : defaultValue; }
  inline void reset() { value_ = INVALID_UPDATE_DOUBLE; }
  inline OptionalDouble& operator=(double value) { value_ = value; return *this; }

private:
  double value_ = INVALID_UPDATE_DOUBLE;
};

/** A replacement for std::optional<float> that uses less memory */
class OptionalFloat
{
public:
  inline bool has_value() const { return value_ != INVALID_UPDATE_FLOAT; }
  inline float value() const { return value_; }
  inline float value_or(double defaultValue) const { return has_value() ? value_ : defaultValue; }
  inline void reset() { value_ = INVALID_UPDATE_FLOAT; }
  inline OptionalFloat& operator=(float value) { value_ = value; return *this; }

private:
  float value_ = INVALID_UPDATE_FLOAT;
};

/** Base class to allow polymorphic behavior between the preference types and property types */
class FieldList
{
public:
  virtual ~FieldList() {}

  bool operator==(const FieldList& rhs) const = default;
  bool operator!=(const FieldList& rhs) const = default;
};

// Macros to help with defining preference messages and property messages

/**
 * Declares the standard methods
 * @param className Class name
 *
 * Example: SIMDATA_DECLARE_METHODS(PlatformProperties);
 */
#define SIMDATA_DECLARE_METHODS(className) \
public: \
  className(); \
  virtual ~className(); \
  className(const className& other); \
  className& operator=(const className& other); \
  className(className&&) = default; \
  className& operator=(className&&) = default; \
  bool operator==(const className& rhs) const; \
  bool operator!=(const className& rhs) const = default; \
  void CopyFrom(const className& from); \
  void MergeFrom(const className& from); \
  void Clear(); \
  className* New();

/**
 * Declares the standard methods with default for the operators
 * @param className Class name
 *
 * Example: SIMDATA_DECLARE_DEFAULT_METHODS(BeamProperties);
 */
#define SIMDATA_DECLARE_DEFAULT_METHODS(className) \
public: \
  className(); \
  virtual ~className(); \
  className(const className& other) = default; \
  className& operator=(const className& other) = default; \
  className(className&&) = default; \
  className& operator=(className&&) = default; \
  bool operator==(const className& rhs) const = default; \
  bool operator!=(const className& rhs) const = default; \
  void CopyFrom(const className& from); \
  void MergeFrom(const className& from); \
  void Clear(); \
  className* New();

/**
 * Declares a field member variable with corresponding accessors
 * @param variableName The name of the member variable to add to the class definition
 * @param fieldName A string that matches the protobuf field name
 * @param type The c++ type for the field
 *
 * Example: SIMDATA_DECLARE_FIELD(draw_, draw, bool);
 */
#define SIMDATA_DECLARE_FIELD(variableName, fieldName, type) \
public: \
  bool has_##fieldName() const; \
  void clear_##fieldName(); \
  type fieldName() const; \
  void set_##fieldName(type value); \
\
private: \
  std::optional<type> variableName;

/**
 * Declares a field member variable, where the argument for the set_ method is by const ref
 * @param variableName The name of the member variable to add to the class definition
 * @param fieldName A string that matches the protobuf field name
 * @param type The c++ type for the field
 *
 * Example: SIMDATA_DECLARE_FIELD_CONST_REF(draw_, draw, bool);
 */
#define SIMDATA_DECLARE_FIELD_CONST_REF(variableName, fieldName, type) \
public: \
  bool has_##fieldName() const; \
  void clear_##fieldName(); \
  type fieldName() const; \
  void set_##fieldName(const type& value); \
\
private: \
  std::optional<type> variableName;

/**
 * Declares a vector member variable with corresponding accessors
 * @param variableName The name of the member variable to add to the class definition
 * @param fieldName A string that matches the protobuf field name
 * @param type The C++ type for the elements contained in the std::vector
 *
 * Example: SIMDATA_DECLARE_VECTOR_FIELD(files_, files, std::string);
 */
#define SIMDATA_DECLARE_VECTOR_FIELD(variableName, fieldName, type) \
public: \
  int fieldName##_size(); \
  void clear_##fieldName(); \
  const std::vector<type>& fieldName() const; \
  const type& fieldName(int index) const; \
  std::vector<type>* mutable_##fieldName(); \
  type* add_##fieldName(); \
  void add_##fieldName(const type& value); \
\
private: \
  std::vector<type> variableName;

/**
 * Declares a subfield list member variable with corresponding accessors
 * @param variableName The name of the member variable to add to the class definition
 * @param fieldName A string that matches the protobuf field name
 * @param type The subfield list type
 *
 * Example: SIMDATA_DECLARE_SUBFIELD_LIST(commonPreferences_, commonprefs, CommonPreferences);
 */
#define SIMDATA_DECLARE_SUBFIELD_LIST(variableName, fieldName, type) \
public: \
  bool has_##fieldName() const; \
  void clear_##fieldName(); \
  const type& fieldName() const; \
  type* mutable_##fieldName(); \
\
private: \
  mutable std::unique_ptr<type> variableName; ///< mutable because some code references the field name method without checking to make sure the field exists

//---------------------------------------------------------------------------------------------------------

/**
 * Define the standard methods
 * @param className Class name
 *
 * Example: SIMDATA_DEFINE_METHODS(PlatformProperties);
 */
#define SIMDATA_DEFINE_METHODS(className) \
  className::className() \
  { \
  } \
\
  className::~className() \
  { \
  } \
\
  className::className(const className& other) \
  { \
    Clear(); \
    CopyFrom(other); \
  } \
\
  className& className::operator=(const className& other) \
  { \
    CopyFrom(other); \
    return *this; \
  } \
\
  void className::Clear() \
  { \
    *this = className(); \
  } \
\
  className* className::New() \
  { \
    return new className(); \
  }

/**
 * Defines the standard methods with default for the operators
 * @param className Class name
 *
 * Example: SIMDATA_DEFINE_DEFAULT_METHODS(BeamProperties);
 */
#define SIMDATA_DEFINE_DEFAULT_METHODS(className) \
  className::className() \
  { \
  } \
\
  className::~className() \
  { \
  } \
\
  void className::CopyFrom(const className& from) \
  { \
    if (&from == this) \
      return; \
    *this = from;  \
  } \
\
  void className::Clear() \
  { \
    *this = className(); \
  } \
\
  className* className::New() \
  { \
    return new className(); \
  }

/**
 * Define field member accessors
 * @param variableName The name of the member variable to add to the class definition
 * @param fieldName A string that matches the protobuf field name
 * @param type The c++ type for the field
 * @param defaultValue The default value
 *
 * Example: SIMDATA_DEFINE_FIELD(draw_, draw, bool, false);
 */
#define SIMDATA_DEFINE_FIELD(className, variableName, fieldName, type, defaultValue) \
  bool className::has_##fieldName() const { return variableName.has_value(); } \
  void className::clear_##fieldName() { variableName.reset(); } \
  type className::fieldName() const { return variableName.value_or(defaultValue); } \
  void className::set_##fieldName(type value) { variableName = value; } \

/**
 * Defined a field member, where the argument for the set_ method is by const ref
 * @param variableName The name of the member variable to add to the class definition
 * @param fieldName A string that matches the protobuf field name
 * @param type The c++ type for the field
 * @param defaultValue The default value
 *
 * Example: SIMDATA_DEFINE_FIELD_CONST_REF(ruleFile_, rulefile, std::string, "");
 */
#define SIMDATA_DEFINE_FIELD_CONST_REF(className, variableName, fieldName, type, defaultValue) \
  bool className::has_##fieldName() const { return variableName.has_value(); } \
  void className::clear_##fieldName() { variableName.reset(); } \
  type className::fieldName() const { return variableName.value_or(defaultValue); } \
  void className::set_##fieldName(const type& value) { variableName = value; } \

/**
 * Define vector member accessors
 * @param variableName The name of the member variable to add to the class definition
 * @param fieldName A string that matches the protobuf field name
 * @param type The C++ type for the elements contained in the std::vector
 *
 * Example: SIMDATA_DEFINE_VECTOR_FIELD(files_, files, std::string);
 */
#define SIMDATA_DEFINE_VECTOR_FIELD(className, variableName, fieldName, type) \
  int className::fieldName##_size() { return static_cast<int>(variableName.size()); } \
  void className::clear_##fieldName() { variableName.clear();} \
  const std::vector<type>& className::fieldName() const { return variableName; } \
  const type& className::fieldName(int index) const { return variableName[index]; } \
  std::vector<type>* className::mutable_##fieldName() { return &variableName; } \
  type* className::add_##fieldName() { variableName.push_back(type()); return &variableName.back(); } \
  void className::add_##fieldName(const type& value) { variableName.push_back(value); } \

/**
 * Define subfield list accessors
 * @param variableName The name of the member variable to add to the class definition
 * @param fieldName A string that matches the protobuf field name
 * @param type The subfield list type
 *
 * Example: SIMDATA_DEFINE_SUBFIELD_LIST(commonPreferences_, commonprefs, CommonPreferences);
 */
#define SIMDATA_DEFINE_SUBFIELD_LIST(className, variableName, fieldName, type) \
  bool className::has_##fieldName() const { return variableName != nullptr; } \
  void className::clear_##fieldName() { variableName.reset(); } \
  const type& className::fieldName() const { if (variableName == nullptr) variableName = std::make_unique<type>(); return *variableName; } \
  type* className::mutable_##fieldName() { if (variableName == nullptr) variableName = std::make_unique<type>(); return variableName.get(); } \

//---------------------------------------------------------------------------------------------------------

/**
 * Exits the method if the subfield lists are not equal
 * @param variableName The name of the member variable to compare
 * @param rhs The rhs variable that contains a field matching variableName
 *
 * Example: SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(commonPreferences_, rhs);
 */
#define SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(variableName, rhs) \
  if ((variableName && !rhs.variableName) || \
    (!variableName && rhs.variableName)) \
    return false; \
\
  if (variableName) \
  { \
    if (*variableName != *rhs.variableName) \
      return false; \
  }

 /**
  * Merges a "from" variable (externally defined) into the specified local variable
  * @param variableName The name of the member variable to compare
  * @param variableType The variable type
  * @param fieldName A string that matches the protobuf field name
  *
  * Example: SIMDATA_SUB_FIELD_LIST_MERGE_FROM(commonPreferences_, CommonPreferences, commonprefs);
  */
#define SIMDATA_SUB_FIELD_LIST_MERGE_FROM(variableName, variableType, fieldName)\
  if (from.has_##fieldName()) \
  { \
    if (!variableName) \
      variableName = std::make_unique<variableType>(); \
    variableName->MergeFrom(from.fieldName()); \
  }

 /**
  * Copy a "from" variable (externally defined) into the specified local variable
  * @param variableName The name of the member variable to compare
  * @param variableType The variable type
  * @param fieldName A string that matches the protobuf field name
  *
  * Example: SIMDATA_SUBFIELD_LIST_COPY_FROM(commonPreferences_, CommonPreferences, commonprefs);
  */
#define SIMDATA_SUBFIELD_LIST_COPY_FROM(variableName, variableType, fieldName)\
  if (from.has_##fieldName()) \
  { \
    if (!variableName) \
      variableName = std::make_unique<variableType>(); \
    variableName->CopyFrom(from.fieldName()); \
  } \
  else \
    variableName.reset()

//---------------------------------------------------------------------------------------------------------

/** Supported geographic reference frames */
enum class CoordinateSystemProperties {
  NED = 1,   ///< North/East/Down
  NWU = 2,   ///< North/West/Up
  ENU = 3,   ///< East/North/Up
  LLA = 4,   ///< Lat/Lon/Alt
  ECEF = 5,  ///< Earth-centered, Earth-fixed (stationary frame)
  ECI = 6,   ///< Earth-centered, inertial (rotates in time)
  XEAST = 7, ///< Tangent plane, X-axis pointing East
  GTP = 8,   ///< Generic tangent plane that can be rotated and/or translated
};

/** Magnetic variance to apply to orientation angles */
enum class MagneticVarianceProperties {
  MV_WMM = 1,   ///< Variance based on World Magnetic Model (WMM)
  MV_TRUE = 2,  ///< No variance, also known as True North
  MV_USER = 3,  ///< User defined variance
};

/** Vertical datum to apply to altitude values in certain systems */
enum class VerticalDatumProperties {
  VD_WGS84 = 1,  ///< Referenced to WGS-84 ellipsoid
  VD_MSL = 2,    ///< Referenced to Earth Gravity Model (EGM)
  VD_USER = 3    ///< User defined datum
};

}

#endif

