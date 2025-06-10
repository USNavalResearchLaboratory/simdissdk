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
#ifndef SIMDATA_DATATYPE_REFLECTION_H
#define SIMDATA_DATATYPE_REFLECTION_H

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "simCore/Common/Common.h"
#include "simCore/Common/Export.h"
#include "DataTypeBasics.h"

namespace simData
{

class EnumerationText;

/** Use a pointer instead of a std::string in the std::variant to save memory space */
using StringPtr = std::unique_ptr<std::string>;

/** Use a pointer instead of a std::vector<std::string> in the std::variant to save memory space */
using StringVectorPtr = std::unique_ptr<std::vector<std::string>>;

/** Enumeration for all the data types */
enum class ReflectionDataType
{
  Unknown,
  Boolean,
  Int32,
  Uint32,
  Uint64,
  Double,
  String,
  StringVector,
  Enumeration
};

/** Use a variant to hold all the different data types */
using ReflectionData = std::variant<std::monostate, bool, int32_t, uint32_t, uint64_t, double, StringPtr, StringVectorPtr>;

/** Wrapper around ReflectionData to hide the std::variant */
class SDKDATA_EXPORT ReflectionValue
{
public:
  explicit ReflectionValue(bool value);
  explicit ReflectionValue(int32_t value);
  explicit ReflectionValue(uint32_t value);
  explicit ReflectionValue(uint64_t value);
  explicit ReflectionValue(double value);
  explicit ReflectionValue(const std::string& value);
  explicit ReflectionValue(const char* value);
  explicit ReflectionValue(const std::vector<std::string>& values);

  ReflectionValue(const ReflectionValue& other) = default;
  ReflectionValue& operator=(const ReflectionValue& other) = default;
  ReflectionValue(ReflectionValue&&) = default;
  ReflectionValue& operator=(ReflectionValue&&) = default;

  // Intentionally not virtual
  ~ReflectionValue();

  bool operator==(const ReflectionValue& rhs) const;
  bool operator!=(const ReflectionValue& rhs) const;

  /** Get/Set Boolean value; get returns false on error */
  bool getBoolean() const;
  void setBoolean(bool value);

  /** Get/Set int32_t value; get returns 0 on error */
  int32_t getInt32() const;
  void setInt32(int32_t value);

  /** Get/Set int32_t value; get returns 0 on error */
  uint32_t getUint32() const;
  void setUint32(uint32_t value);

  /** Get/Set uint64_t value; get returns 0 on error */
  uint64_t getUint64() const;
  void setUint64(uint64_t value);

  /** Get/Set double value; get returns 0.0 on error */
  double getDouble() const;
  void setDouble(double value);

  /** Get/Set string value; get returns empty string on error */
  std::string getString() const;
  void setString(const std::string& value);

  /** Get/Set string vector values; get returns empty vector on error */
  std::vector<std::string> getStrings() const;
  void setStrings(const std::vector<std::string>& value);

  /** Set the text provider for an enumeration; base type should be Int32 */
  void setEnumerationText(std::shared_ptr<EnumerationText> text);
  /** Get the text for an enumeration; base type should be Int32 */
  std::string getEnumerationText() const;

  /** Returns the type for the field */
  simData::ReflectionDataType type() const;

private:
  ReflectionData data_;
  std::shared_ptr<EnumerationText> enumerationText_;
};

/**
 * Base reflection
 * Provides an interface to access fields of a field list. Also provide a method to retrieve all
 * the fields of the field list. The field names match the field names from the original protobuf
 * implementation.
 */
class SDKDATA_EXPORT Reflection
{
public:
  Reflection();
  ~Reflection();

  // Contains unique pointer so no copy or move
  SDK_DISABLE_COPY_MOVE(Reflection);

  /** Returns the value for the given path from the given field list; return empty on error */
  std::optional<ReflectionValue> getValue(const FieldList* fields, const std::string& path) const;

  /** Sets the value for the given field list to the given value; return 0 on success and 1 on error */
  int setValue(FieldList* fields, const ReflectionValue& value, const std::string& path);

  /** Visitor that returns the path and type of the fields in a field list */
  using ReflectorVisitorFn = std::function<void(const std::string&, ReflectionDataType)>;

  /** Calls the reflector for each field */
  void reflection(const std::string& path, ReflectorVisitorFn reflector);

  /** Function to get a field value */
  using ReflectionGetFn = std::function<std::optional<ReflectionValue>(const FieldList*, const std::string&)>;
  /** Function to set a field value; return 0 on success or 1 on failure */
  using ReflectionSetFn = std::function<int(FieldList*, const ReflectionValue&, const std::string&)>;
  /** Function to get the path and type of a field */
  using ReflectionReflectorFn = std::function<void(const std::string&, const ReflectorVisitorFn&)>;

  /** Struct for getting/setting field values */
  struct ReflectionMetaData
  {
    ReflectionGetFn getValue;
    ReflectionSetFn setValue;
    ReflectionReflectorFn reflector;
  };

  /** Add a reflection for the given key */
  void addReflection(const std::string& key, const ReflectionMetaData& fn);

  /** Returns a reflection for beam property */
  static std::unique_ptr<Reflection> makeBeamProperty();

  /** Returns a reflection for classification property */
  static std::unique_ptr<Reflection> makeClassificationProperty();

  /** Returns a reflection for coordinate frame property */
  static std::unique_ptr<Reflection> makeCoordinateFrameProperty();

  /** Returns a reflection for custom rendering property */
  static std::unique_ptr<Reflection> makeCustomRenderingProperty();

  /** Returns a reflection for gate property */
  static std::unique_ptr<Reflection> makeGateProperty();

  /** Returns a reflection for laser property */
  static std::unique_ptr<Reflection> makeLaserProperty();

  /** Returns a reflection for LOB group property */
  static std::unique_ptr<Reflection> makeLobGroupProperty();

  /** Returns a reflection for platform property */
  static std::unique_ptr<Reflection> makePlatformProperty();

  /** Returns a reflection for projector property */
  static std::unique_ptr<Reflection> makeProjectorProperty();

  /** Returns a reflection for reference property */
  static std::unique_ptr<Reflection> makeReferenceProperty();

  /** Returns a reflection for scenario property */
  static std::unique_ptr<Reflection> makeScenarioProperty();

  /** Returns a reflection for sound file property */
  static std::unique_ptr<Reflection> makeSoundFileProperty();

  /** Returns a reflection for tangent plane offsets property */
  static std::unique_ptr<Reflection> makeTangentPlaneOffsetsProperty();

private:
  /** Return the text before the first / plus any remaining text */
  std::pair<std::string, std::string> split_(const std::string& path) const;

  /** Keyed by the full protobuf path, i.e commonprefs.draw */
  std::map<std::string, ReflectionMetaData> reflections_;

  /** Need the defined order to match legacy protobuf behavior */
  std::vector<std::string> order_;
};

/**
  * Adds a POD field to reflection.
  * @param path A string that matches the protobuf path which is mixed case field name
  * @param fieldName A string that matches the protobuf field name but all lowercase
  * @param fieldListType The field list type
  * @param accessor The ReflectionValue accessor for the field
  * @param reflectionType The ReflectionDataType for the field
  *
  * Example: SIMDATA_REFLECTION_ADD_FIELD(originalId, originalId, PlatformProperty,  getUint64, simData::ReflectionDataType::UInt64);
  */
#define SIMDATA_REFLECTION_ADD_FIELD(path, fieldName, fieldListType, accessor, reflectionType) \
  addReflection(#path, simData::Reflection::ReflectionMetaData( \
    [](const simData::FieldList* fields, const std::string& key) { \
        if (static_cast<const fieldListType*>(fields)->has_##fieldName()) \
          return std::optional<simData::ReflectionValue>(static_cast<const fieldListType*>(fields)->fieldName()); \
        return std::optional<simData::ReflectionValue>(); \
    }, \
    [](simData::FieldList* fields, const simData::ReflectionValue& value, const std::string& key) { \
        static_cast<fieldListType*>(fields)->set_##fieldName(value.accessor()); \
        return 0; \
    }, \
    [](const std::string& key, const simData::Reflection::ReflectorVisitorFn& reflector) { \
        reflector(key, reflectionType); \
    } \
  ));

/**
  * Adds an enumeration field to reflection.
  * @param path A string that matches the protobuf path which is mixed case field name
  * @param fieldName A string that matches the protobuf field name but all lowercase
  * @param fieldListType The field list type
  * @param enumType The enumeration class type
  * @param enumeration A shared pointer variable of type EnumerationText. Must be a variable since it is captured.
  *
  * Example: SIMDATA_REFLECTION_ADD_ENUM(type, type, BeamProperty, BeamProperty::Type, shared_ptr);
  */
#define SIMDATA_REFLECTION_ADD_ENUM(path, fieldName, fieldListType, enumType, enumeration) \
  addReflection(#path, simData::Reflection::ReflectionMetaData( \
    [enumeration](const simData::FieldList* fields, const std::string& key) { \
        if (static_cast<const fieldListType*>(fields)->has_##fieldName()) \
        {\
          auto rv = std::optional<simData::ReflectionValue>(static_cast<int32_t>(static_cast<const fieldListType*>(fields)->fieldName())); \
          rv->setEnumerationText(enumeration); \
          return rv; \
        }\
        return std::optional<simData::ReflectionValue>(); \
    }, \
    [](simData::FieldList* fields, const simData::ReflectionValue& value, const std::string& key) { \
        static_cast<fieldListType*>(fields)->set_##fieldName(static_cast<enumType>(value.getInt32())); \
        return 0; \
    }, \
    [](const std::string& key, const simData::Reflection::ReflectorVisitorFn& reflector) { \
        reflector(key, ReflectionDataType::Enumeration); \
    } \
  ));

/**
  * Adds a vector field to reflection.
  * @param path A string that matches the protobuf path which is mixed case field name
  * @param fieldName A string that matches the protobuf field name but all lowercase
  * @param fieldListType The field list type
  * @param accessor The ReflectionValue accessor for the field
  * @param reflectionType The ReflectionDataType for the field
  *
  * Example: SIMDATA_REFLECTION_ADD_VECTOR_FIELD(originalId, originalId, PlatformProperty,  getUint64, simData::ReflectionDataType::UInt64);
  */
#define SIMDATA_REFLECTION_ADD_VECTOR_FIELD(path, fieldName, fieldListType, accessor, reflectionType) \
  addReflection(#path, simData::Reflection::ReflectionMetaData( \
    [](const simData::FieldList* fields, const std::string& key) { \
        return std::optional<simData::ReflectionValue>(static_cast<const fieldListType*>(fields)->fieldName()); \
    }, \
    [](simData::FieldList* fields, const simData::ReflectionValue& value, const std::string& key) { \
        *(static_cast<fieldListType*>(fields)->mutable_##fieldName()) = value.accessor(); \
        return 0; \
    }, \
    [](const std::string& key, const simData::Reflection::ReflectorVisitorFn& reflector) { \
        reflector(key, reflectionType); \
    } \
  ));

 /**
  * Adds a subfield lists to reflection.
  * @param path A string that matches the protobuf path which is mixed case field name
  * @param fieldName A string that matches the protobuf field name but all lowercase
  * @param fieldListType The field list type
  * @param subfield The reflection object of the subfield lists
  *
  * Example: SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(commonPrefs, commonprefs, PlatformPreferences, commonReflection_);
  */
#define SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(path, fieldName, fieldListType, subfield)  \
  addReflection(#path, simData::Reflection::ReflectionMetaData( \
    [subfield](const simData::FieldList* fields, const std::string& key) { \
      if (!static_cast<const fieldListType*>(fields)->has_##fieldName()) \
        return std::optional<simData::ReflectionValue>(); \
      return subfield->getValue(&static_cast<const fieldListType*>(fields)->fieldName(), key); }, \
    [subfield](simData::FieldList* fields, const simData::ReflectionValue& value, const std::string& key) { \
      return subfield->setValue(static_cast<fieldListType*>(fields)->mutable_##fieldName(), value, key); }, \
    [subfield](const std::string& key, const simData::Reflection::ReflectorVisitorFn& reflector) { subfield->reflection(key, reflector); }) \
  );

}

#endif

