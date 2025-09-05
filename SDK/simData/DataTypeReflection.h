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
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "simCore/Common/Common.h"
#include "simCore/Common/Export.h"
#include "simData/ObjectId.h"
#include "DataTypeBasics.h"

namespace simData
{

class EnumerationText;

/** Use a pointer instead of a std::string in the std::variant to save memory space */
using StringPtr = std::unique_ptr<std::string>;

/** Use a pointer instead of a std::vector<std::string> in the std::variant to save memory space */
using StringVectorPtr = std::unique_ptr<std::vector<std::string>>;

/** Use a pointer instead of a std::vector<uint64_t> in the std::variant to save memory space */
using IdVectorPtr = std::unique_ptr<std::vector<uint64_t>>;

/** Enumeration for all the data types */
enum class ReflectionDataType
{
  Unknown,
  Boolean,
  Int32,
  Uint32,
  Uint64,
  Float,
  Double,
  String,
  StringVector,
  IdVector,
  Enumeration
};

/** Use a variant to hold all the different data types */
using ReflectionData = std::variant<std::monostate, bool, int32_t, uint32_t, uint64_t, float, double, StringPtr, StringVectorPtr, IdVectorPtr>;

/** Wrapper around ReflectionData to hide the std::variant */
class SDKDATA_EXPORT ReflectionValue
{
public:
  explicit ReflectionValue(bool value);
  explicit ReflectionValue(int32_t value);
  explicit ReflectionValue(uint32_t value);
  explicit ReflectionValue(uint64_t value);
  explicit ReflectionValue(float value);
  explicit ReflectionValue(double value);
  explicit ReflectionValue(const std::string& value);
  explicit ReflectionValue(const char* value);
  explicit ReflectionValue(const std::vector<std::string>& values);
  explicit ReflectionValue(const std::vector<uint64_t>& values);

  ReflectionValue(const ReflectionValue& other);
  ReflectionValue& operator=(const ReflectionValue& other);
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

  /** Get/Set float value; get returns 0.0 on error */
  float getFloat() const;
  void setFloat(float value);

  /** Get/Set double value; get returns 0.0 on error */
  double getDouble() const;
  void setDouble(double value);

  /** Get/Set string value; get returns empty string on error */
  std::string getString() const;
  void setString(const std::string& value);

  /** Get/Set string vector values; get returns empty vector on error */
  std::vector<std::string> getStrings() const;
  void setStrings(const std::vector<std::string>& value);

  /** Get/Set string vector values; get returns empty vector on error */
  std::vector<uint64_t> getIds() const;
  void setIds(const std::vector<std::uint64_t>& value);

  /** Set the text provider for an enumeration; base type should be Int32 */
  void setEnumerationText(std::shared_ptr<EnumerationText> text);
  const EnumerationText* getEnumerationText() const;

  /** Get the text for an enumeration; base type should be Int32 */
  std::string getEnumeration() const;

  /** Returns the type for the field */
  simData::ReflectionDataType type() const;

private:
  ReflectionData data_;
  std::shared_ptr<EnumerationText> enumerationText_;
};

/**
 * Base reflection
 * Provides an interface to access fields of a field list. Access is either via a field name or via
 * a tag stack of integers. The field names match the field names from the original protobuf implementation.
 * The tag stack is a same concept from the protobuf implementation but the integer values are not the same.
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

  /** Returns the default value for the given path from the given field list; return empty on error */
  std::optional<ReflectionValue> getDefaultValue(const FieldList* fields, const std::string& path) const;

  /** Sets the value for the given field list to the given value; return 0 on success and 1 on error */
  int setValue(FieldList* fields, const ReflectionValue& value, const std::string& path) const;

  /** Clear the value for the given field list; return 0 on success and 1 on error */
  int clearValue(FieldList* fields, const std::string& path) const;

  /** Visitor that returns the path and type of the fields in a field list */
  using ReflectorVisitorFn = std::function<void(const std::string&, ReflectionDataType)>;

  /** Calls the reflector for each field */
  void reflection(const std::string& path, ReflectorVisitorFn reflector) const;

  /** Identifies a field in a FieldList via a list of integers. */
  using TagStack = std::deque<int>;
  /** A map of TagStack key by field name */
  using TagStackMap = std::map<std::string, TagStack>;

  /** Returns the value for the given path from the given field list; return empty on error */
  std::optional<ReflectionValue> getValue(const FieldList* fields, const TagStack& tagStack) const;

  /** Sets the value for the given field list to the given value; return 0 on success and 1 on error */
  int setValue(FieldList* fields, const ReflectionValue& value, const TagStack& tagStack) const;

  /** Visitor that returns the path and type of the fields in a field list */
  using TagReflectorVisitorFn = std::function<void(const TagStack&, const std::string&, ReflectionDataType)>;

  /** Calls the reflector for each field */
  void reflection(const TagStack& tagStack, const std::string& path, TagReflectorVisitorFn reflector) const;

  /** Function to get a field value */
  using ReflectionGetFn = std::function<std::optional<ReflectionValue>(const FieldList*, const std::string&)>;
  /** Function to set a field value; return 0 on success or 1 on failure */
  using ReflectionSetFn = std::function<int(FieldList*, const ReflectionValue&, const std::string&)>;
  /** Function to clear a field value; return 0 on success or 1 on failure */
  using ReflectionClearFn = std::function<int(FieldList*, const std::string&)>;
  /** Function to get the path and type of a field */
  using ReflectionReflectorFn = std::function<void(const std::string&, const ReflectorVisitorFn&)>;

  /** Function to get a field value */
  using TagReflectionGetFn = std::function<std::optional<ReflectionValue>(const FieldList*, const TagStack&)>;
  /** Function to set a field value; return 0 on success or 1 on failure */
  using TagReflectionSetFn = std::function<int(FieldList*, const ReflectionValue&, const TagStack&)>;
  /** Function to get the path and type of a field */
  using TagReflectionReflectorFn = std::function<void(const TagStack&, const std::string&, const TagReflectorVisitorFn&)>;

  /** Struct for getting/setting field values */
  struct ReflectionMetaData
  {
    // String based reflection
    ReflectionGetFn getValue;
    ReflectionGetFn getDefaultValue;
    ReflectionSetFn setValue;
    ReflectionClearFn clearValue;
    ReflectionReflectorFn reflector;

    // Tag based reflection
    TagReflectionGetFn getValueByTag;
    TagReflectionSetFn setValueByTag;
    TagReflectionReflectorFn tagReflector;
  };

  /** Add a reflection for the given key */
  void addReflection(const std::string& key, const ReflectionMetaData& fn);


  /** Function to get a field list */
  using ListReflectionGetFn = std::function<FieldList*(FieldList*, const std::string&)>;

  /** Add a reflection for getting a field list */
  void addListReflection(const std::string& key, const ReflectionMetaData& fn, const ListReflectionGetFn& listFn);

  /** Get a field list; returns nullptr on error */
  FieldList* getMutableFieldList(FieldList* fields, const std::string& path) const;

  //-----------------------------------------------------------------------------------------------------------

  /** Returns the preferences reflection for the given type; return nullptr on error */
  static std::unique_ptr<Reflection> makePreferences(simData::ObjectType type);

  /** Returns a reflection for antenna patterns preferences */
  static std::unique_ptr<Reflection> makeAntennaPatternsPreferences();

  /** Returns a reflection for beam commands */
  static std::unique_ptr<Reflection> makeBeamCommands();

  /** Returns a reflection for beam preferences */
  static std::unique_ptr<Reflection> makeBeamPreferences();

  /** Returns a reflection for beam property */
  static std::unique_ptr<Reflection> makeBeamProperty();

  /** Returns a reflection for body orientation preferences */
  static std::unique_ptr<Reflection> makeBodyOrientationPreferences();

  /** Returns a reflection for classification property */
  static std::unique_ptr<Reflection> makeClassificationProperty();

  /** Returns a reflection for common preferences */
  static std::unique_ptr<Reflection> makeCommonPreferences();

  /** Returns a reflection for coordinate frame property */
  static std::unique_ptr<Reflection> makeCoordinateFrameProperty();

  /** Returns a reflection for custom rendering commands */
  static std::unique_ptr<Reflection> makeCustomRenderingCommands();

  /** Returns a reflection for custom rendering preferences */
  static std::unique_ptr<Reflection> makeCustomRenderingPreferences();

  /** Returns a reflection for custom rendering property */
  static std::unique_ptr<Reflection> makeCustomRenderingProperty();

  /** Returns a reflection for Display Fields */
  static std::unique_ptr<Reflection> makeDisplayFieldsPreferences();

  /** Returns a reflection for gate commands */
  static std::unique_ptr<Reflection> makeGateCommands();

  /** Returns a reflection for gate preferences */
  static std::unique_ptr<Reflection> makeGatePreferences();

  /** Returns a reflection for gate property */
  static std::unique_ptr<Reflection> makeGateProperty();

  /** Returns a reflection for grid settings preferences */
  static std::unique_ptr<Reflection> makeGridSettingsPreferences();

  /** Returns a reflection for label preferences */
  static std::unique_ptr<Reflection> makeLabelPreferences();

  /** Returns a reflection for laser commands */
  static std::unique_ptr<Reflection> makeLaserCommands();

  /** Returns a reflection for laser preferences */
  static std::unique_ptr<Reflection> makeLaserPreferences();

  /** Returns a reflection for laser property */
  static std::unique_ptr<Reflection> makeLaserProperty();

  /** Returns a reflection for LOB Group commands */
  static std::unique_ptr<Reflection> makeLobGroupCommands();

  /** Returns a reflection for LOB group preferences */
  static std::unique_ptr<Reflection> makeLobGroupPreferences();

  /** Returns a reflection for LOB group property */
  static std::unique_ptr<Reflection> makeLobGroupProperty();

  /** Returns a reflection for local grid preferences */
  static std::unique_ptr<Reflection> makeLocalGridPreferences();

  /** Returns a reflection for platform commands */
  static std::unique_ptr<Reflection> makePlatformCommands();

  /** Returns a reflection for platform preferences */
  static std::unique_ptr<Reflection> makePlatformPreferences();

  /** Returns a reflection for platform property */
  static std::unique_ptr<Reflection> makePlatformProperty();

  /** Returns a reflection for position preferences */
  static std::unique_ptr<Reflection> makePositionPreferences();

  /** Returns a reflection for projector commands */
  static std::unique_ptr<Reflection> makeProjectorCommands();

  /** Returns a reflection for projector preferences */
  static std::unique_ptr<Reflection> makeProjectorPreferences();

  /** Returns a reflection for projector property */
  static std::unique_ptr<Reflection> makeProjectorProperty();

  /** Returns a reflection for reference property */
  static std::unique_ptr<Reflection> makeReferenceProperty();

  /** Returns a reflection for scenario property */
  static std::unique_ptr<Reflection> makeScenarioProperty();

  /** Returns a reflection for sound file property */
  static std::unique_ptr<Reflection> makeSoundFileProperty();

  /** Returns a reflection for speed ring preferences */
  static std::unique_ptr<Reflection> makeSpeedRingPreferences();

  /** Returns a reflection for tangent plane offsets property */
  static std::unique_ptr<Reflection> makeTangentPlaneOffsetsProperty();

  /** Returns a reflection for time tick preferences */
  static std::unique_ptr<Reflection> makeTimeTickPreferences();

  /** Returns a reflection for track preferences */
  static std::unique_ptr<Reflection> makeTrackPreferences();

  /** Returns a tag stack map for the given reflection*/
  static TagStackMap makeTagStackMap(const Reflection& reflection);

  /** Returns a tag stack map for the given preference type; returns empty tag stack map on error */
  static TagStackMap makePreferencesTagStackMap(simData::ObjectType type);

  /**
   * Returns a preference tag stack for the given path and tag stack map; returns empty tag stack on error
   * The path can either be to a field or to a field list
   */
  static TagStack getPreferencesTagStack(const std::string& path, const TagStackMap& tags);

  /** Returns a preference tag stack for the given path and entity type; returns empty tag stack on error */
  static TagStack getPreferencesTagStack(const std::string& path, simData::ObjectType type);

private:
  /** Return the text before the first / plus any remaining text */
  std::pair<std::string, std::string> split_(const std::string& path) const;

  /** Keyed by the full protobuf path, i.e commonprefs.draw */
  std::map<std::string, ReflectionMetaData> reflections_;

  /** Need the defined order to match legacy protobuf behavior */
  std::vector<std::string> order_;

  /** Reflection for returning a field list */
  std::map<std::string, ListReflectionGetFn> listReflections_;
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
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        if (static_cast<const fieldListType*>(fields)->has_##fieldName()) \
          return std::optional<simData::ReflectionValue>(static_cast<const fieldListType*>(fields)->fieldName()); \
        return std::optional<simData::ReflectionValue>(); \
    }, \
    [](const simData::FieldList* fields, const std::string& key) { \
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        return std::optional<simData::ReflectionValue>(static_cast<const fieldListType*>(fields)->fieldName()); \
    }, \
    [](simData::FieldList* fields, const simData::ReflectionValue& value, const std::string& key) { \
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        static_cast<fieldListType*>(fields)->set_##fieldName(value.accessor()); \
        return 0; \
    }, \
    [](simData::FieldList* fields, const std::string& key) { \
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        static_cast<fieldListType*>(fields)->clear_##fieldName(); \
        return 0; \
    }, \
    [](const std::string& key, const simData::Reflection::ReflectorVisitorFn& reflector) { \
        reflector(key, reflectionType); \
    }, \
    [](const simData::FieldList* fields, const TagStack& tagStack) { \
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        return std::optional<simData::ReflectionValue>(static_cast<const fieldListType*>(fields)->fieldName()); \
    }, \
    [](simData::FieldList* fields, const simData::ReflectionValue& value, const TagStack& tagStack) { \
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        static_cast<fieldListType*>(fields)->set_##fieldName(value.accessor()); \
        return 0; \
    }, \
    [](const TagStack& tagStack, const std::string& key, const simData::Reflection::TagReflectorVisitorFn& reflector) { \
        reflector(tagStack, key, reflectionType); \
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
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        if (static_cast<const fieldListType*>(fields)->has_##fieldName()) \
        {\
          auto rv = std::optional<simData::ReflectionValue>(static_cast<int32_t>(static_cast<const fieldListType*>(fields)->fieldName())); \
          rv->setEnumerationText(enumeration); \
          return rv; \
        }\
        return std::optional<simData::ReflectionValue>(); \
    }, \
    [enumeration](const simData::FieldList* fields, const std::string& key) { \
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        auto rv = std::optional<simData::ReflectionValue>(static_cast<int32_t>(static_cast<const fieldListType*>(fields)->fieldName())); \
        rv->setEnumerationText(enumeration); \
        return rv; \
    }, \
    [](simData::FieldList* fields, const simData::ReflectionValue& value, const std::string& key) { \
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        static_cast<fieldListType*>(fields)->set_##fieldName(static_cast<enumType>(value.getInt32())); \
        return 0; \
    }, \
    [](simData::FieldList* fields, const std::string& key) { \
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        static_cast<fieldListType*>(fields)->clear_##fieldName(); \
        return 0; \
    }, \
    [](const std::string& key, const simData::Reflection::ReflectorVisitorFn& reflector) { \
        reflector(key, ReflectionDataType::Enumeration); \
    }, \
    [enumeration](const simData::FieldList* fields, const TagStack& tagStack) { \
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        auto rv = std::optional<simData::ReflectionValue>(static_cast<int32_t>(static_cast<const fieldListType*>(fields)->fieldName())); \
        rv->setEnumerationText(enumeration); \
        return rv; \
    }, \
    [](simData::FieldList* fields, const simData::ReflectionValue& value, const TagStack& tagStack) { \
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        static_cast<fieldListType*>(fields)->set_##fieldName(static_cast<enumType>(value.getInt32())); \
        return 0; \
    }, \
    [](const TagStack& tagStack, const std::string& key, const simData::Reflection::TagReflectorVisitorFn& reflector) { \
        reflector(tagStack, key, ReflectionDataType::Enumeration); \
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
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        if (static_cast<const fieldListType*>(fields)->fieldName##_size() > 0) \
          return std::optional<simData::ReflectionValue>(static_cast<const fieldListType*>(fields)->fieldName()); \
        return std::optional<simData::ReflectionValue>(); \
    }, \
    [](const simData::FieldList* fields, const std::string& key) { \
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        return std::optional<simData::ReflectionValue>(static_cast<const fieldListType*>(fields)->fieldName()); \
    }, \
    [](simData::FieldList* fields, const simData::ReflectionValue& value, const std::string& key) { \
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        *(static_cast<fieldListType*>(fields)->mutable_##fieldName()) = value.accessor(); \
        return 0; \
    }, \
    [](simData::FieldList* fields, const std::string& key) { \
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        static_cast<fieldListType*>(fields)->clear_##fieldName(); \
        return 0; \
    }, \
    [](const std::string& key, const simData::Reflection::ReflectorVisitorFn& reflector) { \
        reflector(key, reflectionType); \
    }, \
    [](const simData::FieldList* fields, const TagStack& tagStack) { \
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        return std::optional<simData::ReflectionValue>(static_cast<const fieldListType*>(fields)->fieldName()); \
    }, \
    [](simData::FieldList* fields, const simData::ReflectionValue& value, const TagStack& tagStack) { \
        /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
        assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
        *(static_cast<fieldListType*>(fields)->mutable_##fieldName()) = value.accessor(); \
        return 0; \
    }, \
    [](const TagStack& tagStack, const std::string& key, const simData::Reflection::TagReflectorVisitorFn& reflector) { \
        reflector(tagStack, key, reflectionType); \
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
  addListReflection(#path, simData::Reflection::ReflectionMetaData( \
    [subfield](const simData::FieldList* fields, const std::string& key) { \
      /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
      assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
      if (!static_cast<const fieldListType*>(fields)->has_##fieldName()) \
        return std::optional<simData::ReflectionValue>(); \
      return subfield->getValue(&static_cast<const fieldListType*>(fields)->fieldName(), key); \
    }, \
    [subfield](const simData::FieldList* fields, const std::string& key) { \
      /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
      assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
      return subfield->getDefaultValue(&static_cast<const fieldListType*>(fields)->fieldName(), key); \
    }, \
    [subfield](simData::FieldList* fields, const simData::ReflectionValue& value, const std::string& key) { \
      /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
      assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
      return subfield->setValue(static_cast<fieldListType*>(fields)->mutable_##fieldName(), value, key); \
    }, \
    [subfield](simData::FieldList* fields, const std::string& key) { \
      /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
      assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
      return subfield->clearValue(static_cast<fieldListType*>(fields)->mutable_##fieldName(), key); \
    }, \
    [subfield](const std::string& key, const simData::Reflection::ReflectorVisitorFn& reflector) { \
      subfield->reflection(key, reflector); \
    }, \
    [subfield](const simData::FieldList* fields, const TagStack& tagStack) { \
      /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
      assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
      return subfield->getValue(&static_cast<const fieldListType*>(fields)->fieldName(), tagStack); \
    }, \
    [subfield](simData::FieldList* fields, const simData::ReflectionValue& value, const TagStack& tagStacky) { \
      /** Reflection not built correctly, wrong fieldListType passed in during construction */ \
      assert(dynamic_cast<const fieldListType*>(fields) != nullptr); \
      return subfield->setValue(static_cast<fieldListType*>(fields)->mutable_##fieldName(), value, tagStacky); \
    }, \
    [subfield](const TagStack& tagStack,const std::string& key, const simData::Reflection::TagReflectorVisitorFn& reflector) { \
      subfield->reflection(tagStack, key, reflector); \
    }), \
    [subfield] (FieldList* fields, const std::string& key) { \
       return subfield->getMutableFieldList(static_cast<fieldListType*>(fields)->mutable_##fieldName(), key); \
    } \
  );
}

#endif
