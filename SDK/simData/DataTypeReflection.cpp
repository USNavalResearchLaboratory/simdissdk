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

#include <array>
#include <cassert>
#include "CommonPreferences.h"
#include "DataTypeProperties.h"
#include "EntityCommands.h"
#include "EntityPreferences.h"
#include "EnumerationText.h"
#include "DataTypeReflection.h"

namespace simData
{

ReflectionValue::ReflectionValue(bool value)
  : data_(value)
{
}

ReflectionValue::ReflectionValue(int32_t value)
  : data_(value)
{
}

ReflectionValue::ReflectionValue(uint32_t value)
  : data_(value)
{
}

ReflectionValue::ReflectionValue(uint64_t value)
  : data_(value)
{
}

ReflectionValue::ReflectionValue(float value)
  : data_(value)
{
}

ReflectionValue::ReflectionValue(double value)
  : data_(value)
{
}

ReflectionValue::ReflectionValue(const std::string& value)
  : data_(std::make_unique<std::string>(value))
{
}

ReflectionValue::ReflectionValue(const char* value)
  : data_(std::make_unique<std::string>(value))
{
}

ReflectionValue::ReflectionValue(const std::vector<std::string>& values)
  : data_(std::make_unique<std::vector<std::string>>(values))
{
}

ReflectionValue::ReflectionValue(const std::vector<uint64_t>& values)
  : data_(std::make_unique<std::vector<uint64_t>>(values))
{
}

ReflectionValue::ReflectionValue(const ReflectionValue& other)
{
  switch (other.type())
  {
  case ReflectionDataType::Unknown:
    break;

  case ReflectionDataType::Boolean:
    data_ = other.getBoolean();
    break;

  case ReflectionDataType::Int32:
  case ReflectionDataType::Enumeration:
    data_ = other.getInt32();
    break;

  case ReflectionDataType::Uint32:
    data_ = other.getUint32();
    break;

  case ReflectionDataType::Uint64:
    data_ = other.getUint64();
    break;

  case ReflectionDataType::Float:
    data_ = other.getFloat();
    break;

  case ReflectionDataType::Double:
    data_ = other.getDouble();
    break;

  case ReflectionDataType::String:
    data_ = std::make_unique<std::string>(other.getString());
    break;

  case ReflectionDataType::StringVector:
    data_ = std::make_unique<std::vector<std::string>>(other.getStrings());
    break;

  case ReflectionDataType::IdVector:
    data_ = std::make_unique<std::vector<uint64_t>>(other.getIds());
    break;
  }

  enumerationText_ = other.enumerationText_;
}

// Copy Assignment Operator (using copy-and-swap idiom)
ReflectionValue& ReflectionValue::operator=(const ReflectionValue& other)
{
  ReflectionValue temp(other);
  std::swap(data_, temp.data_);
  std::swap(enumerationText_, temp.enumerationText_);
  return *this;
}

ReflectionValue::~ReflectionValue()
{
}

bool ReflectionValue::operator==(const ReflectionValue& rhs) const
{
  if (type() != rhs.type())
    return false;

  if (std::holds_alternative<StringPtr>(data_))
    return getString() == rhs.getString();

  if (std::holds_alternative<StringVectorPtr>(data_))
    return getStrings() == rhs.getStrings();

  if (std::holds_alternative<IdVectorPtr>(data_))
    return getIds() == rhs.getIds();

  return data_ == rhs.data_;
}

bool ReflectionValue::operator!=(const ReflectionValue& rhs) const
{
  return !operator==(rhs);
}

bool ReflectionValue::getBoolean() const
{
  try
  {
    return std::get<bool>(data_);
  }
  catch (const std::bad_variant_access&)
  {
    // Asking for the wrong field type
    assert(false);
  }

  return false;
}

void ReflectionValue::setBoolean(bool value)
{
  if (std::holds_alternative<bool>(data_))
  {
    data_ = value;
  }
  else
  {
    // Setting the wrong field type
    assert(false);
  }
}

int32_t ReflectionValue::getInt32() const
{
  try
  {
    return std::get<int32_t>(data_);
  }
  catch (const std::bad_variant_access&)
  {
    // Asking for the wrong field type
    assert(false);
  }

  return 0;
}

void ReflectionValue::setInt32(int32_t value)
{
  if (std::holds_alternative<int32_t>(data_))
  {
    data_ = value;
  }
  else
  {
    // Setting the wrong field type
    assert(false);
  }
}

uint32_t ReflectionValue::getUint32() const
{
  try
  {
    return std::get<uint32_t>(data_);
  }
  catch (const std::bad_variant_access&)
  {
    // Asking for the wrong field type
    assert(false);
  }

  return 0;
}

void ReflectionValue::setUint32(uint32_t value)
{
  if (std::holds_alternative<uint32_t>(data_))
  {
    data_ = value;
  }
  else
  {
    // Setting the wrong field type
    assert(false);
  }
}

uint64_t ReflectionValue::getUint64() const
{
  try
  {
    return std::get<uint64_t>(data_);
  }
  catch (const std::bad_variant_access&)
  {
    // Asking for the wrong field type
    assert(false);
  }

  return 0;
}

void ReflectionValue::setUint64(uint64_t value)
{
  if (std::holds_alternative<uint64_t>(data_))
  {
    data_ = value;
  }
  else
  {
    // Setting the wrong field type
    assert(false);
  }
}

float ReflectionValue::getFloat() const
{
  try
  {
    return std::get<float>(data_);
  }
  catch (const std::bad_variant_access&)
  {
    // Asking for the wrong field type
    assert(false);
  }

  return 0.0f;
}

void ReflectionValue::setFloat(float value)
{
  if (std::holds_alternative<float>(data_))
  {
    data_ = value;
  }
  else
  {
    // Setting the wrong field type
    assert(false);
  }
}

double ReflectionValue::getDouble() const
{
  try
  {
    return std::get<double>(data_);
  }
  catch (const std::bad_variant_access&)
  {
    // Asking for the wrong field type
    assert(false);
  }

  return 0.0;
}

void ReflectionValue::setDouble(double value)
{
  if (std::holds_alternative<double>(data_))
  {
    data_ = value;
  }
  else
  {
    // Setting the wrong field type
    assert(false);
  }
}

std::string ReflectionValue::getString() const
{
  try
  {
    return *(std::get<StringPtr>(data_));
  }
  catch (const std::bad_variant_access&)
  {
    // Asking for the wrong field type
    assert(false);
  }

  return std::string();
}

void ReflectionValue::setString(const std::string& value)
{
  if (std::holds_alternative<StringPtr>(data_))
  {
    data_ = std::make_unique<std::string>(value);
  }
  else
  {
    // Setting the wrong field type
    assert(false);
  }
}

std::vector<std::string> ReflectionValue::getStrings() const
{
  try
  {
    return *(std::get<StringVectorPtr>(data_));
  }
  catch (const std::bad_variant_access&)
  {
    // Asking for the wrong field type
    assert(false);
  }

  return std::vector<std::string>();
}

void ReflectionValue::setStrings(const std::vector<std::string>& value)
{
  if (std::holds_alternative<StringVectorPtr>(data_))
  {
    data_ = std::make_unique<std::vector<std::string>>(value);
  }
  else
  {
    // Setting the wrong field type
    assert(false);
  }
}

std::vector<uint64_t> ReflectionValue::getIds() const
{
  try
  {
    return *(std::get<IdVectorPtr>(data_));
  }
  catch (const std::bad_variant_access&)
  {
    // Asking for the wrong field type
    assert(false);
  }

  return std::vector<uint64_t>();
}

void ReflectionValue::setIds(const std::vector<uint64_t>& value)
{
  if (std::holds_alternative<IdVectorPtr>(data_))
  {
    data_ = std::make_unique<std::vector<uint64_t>>(value);
  }
  else
  {
    // Setting the wrong field type
    assert(false);
  }
}

void ReflectionValue::setEnumerationText(std::shared_ptr<EnumerationText> text)
{
  enumerationText_ = text;
}

const EnumerationText* ReflectionValue::getEnumerationText() const
{
  return enumerationText_.get();
}

std::string ReflectionValue::getEnumeration() const
{
  // Not an enumeration
  assert(std::holds_alternative<int32_t>(data_));

  return enumerationText_ ? enumerationText_->text(getInt32()) : std::string();
}

simData::ReflectionDataType ReflectionValue::type() const
{
  static std::array<simData::ReflectionDataType, 10> types = {
    ReflectionDataType::Unknown,
    ReflectionDataType::Boolean,
    ReflectionDataType::Int32,
    ReflectionDataType::Uint32,
    ReflectionDataType::Uint64,
    ReflectionDataType::Float,
    ReflectionDataType::Double,
    ReflectionDataType::String,
    ReflectionDataType::StringVector,
    ReflectionDataType::IdVector
  };

  auto idx = data_.index();
  if (idx < types.size())
  {
    auto entityType = types[idx];
    if (entityType != ReflectionDataType::Int32)
      return entityType;

    return enumerationText_ ? ReflectionDataType::Enumeration : entityType;
  }

  // A type was added and the array was not updated
  assert(false);

  return ReflectionDataType::Unknown;
}

//-------------------------------------------------------------------------------------------------------------------------

Reflection::Reflection()
{
}

Reflection::~Reflection()
{
}

std::optional<ReflectionValue> Reflection::getValue(const FieldList* fields, const std::string& path) const
{
  const auto& [key, remainder] = split_(path);

  auto it = reflections_.find(key);
  if (it == reflections_.end())
  {
    // Unit test checks for invalid path so no assert
    return {};
  }

  return it->second.getValue(fields, remainder);
}

std::optional<ReflectionValue> Reflection::getDefaultValue(const FieldList* fields, const std::string& path) const
{
  const auto& [key, remainder] = split_(path);

  auto it = reflections_.find(key);
  if (it == reflections_.end())
  {
    // Unit test checks for invalid path so no assert
    return {};
  }

  return it->second.getDefaultValue(fields, remainder);
}

int Reflection::setValue(FieldList* fields, const ReflectionValue& value, const std::string& path) const
{
  const auto& [key, remainder] = split_(path);

  auto it = reflections_.find(key);
  if (it == reflections_.end())
  {
    // Invalid path
    assert(false);
    return 1;
  }

  return it->second.setValue(fields, value, remainder);
}

int Reflection::clearValue(FieldList* fields, const std::string& path) const
{
  const auto& [key, remainder] = split_(path);

  auto it = reflections_.find(key);
  if (it == reflections_.end())
  {
    // Invalid path
    assert(false);
    return 1;
  }

  return it->second.clearValue(fields, remainder);
}

void Reflection::reflection(const std::string& path, ReflectorVisitorFn reflector) const
{
  const std::string sep = path.empty() ? "" : ".";
  for (const auto& key : order_)
  {
    auto it = reflections_.find(key);
    if (it == reflections_.end())
    {
      // The map and the vector should match
      assert(false);
      continue;
    }

    it->second.reflector(path + sep + it->first, reflector);
  }
}

std::optional<ReflectionValue> Reflection::getValue(const FieldList* fields, const TagStack& tagStack) const
{
  if (tagStack.empty())
  {
    // Should not pass in an empty stack
    assert(false);
    return {};
  }

  auto index = tagStack[0];
  if (index >= static_cast<int>(order_.size()))
  {
    // invalid index
    assert(false);
    return {};
  }

  auto it = reflections_.find(order_[index]);
  if (it == reflections_.end())
  {
    // The map and the vector should match
    assert(false);
    return {};
  }

  if (tagStack.size() == 1)
    return it->second.getValueByTag(fields, {});

  auto remainder = tagStack;
  remainder.pop_front();
  return it->second.getValueByTag(fields, remainder);
}

int Reflection::setValue(FieldList* fields, const ReflectionValue& value, const TagStack& tagStack) const
{
  if (tagStack.empty())
  {
    // Should not pass in an empty stack
    assert(false);
    return {};
  }

  auto index = tagStack[0];
  if (index >= static_cast<int>(order_.size()))
  {
    // invalid index
    assert(false);
    return {};
  }

  auto it = reflections_.find(order_[index]);
  if (it == reflections_.end())
  {
    // The map and the vector should match
    assert(false);
    return {};
  }

  if (tagStack.size() == 1)
    return it->second.setValueByTag(fields, value, {});

  auto remainder = tagStack;
  remainder.pop_front();
  return it->second.setValueByTag(fields, value, remainder);
}

void Reflection::reflection(const TagStack& tagStack, const std::string& path, TagReflectorVisitorFn reflector) const
{
  const std::string sep = path.empty() ? "" : ".";
  int index = 0;
  for (const auto& key : order_)
  {
    auto it = reflections_.find(key);
    if (it == reflections_.end())
    {
      // The map and the vector should match
      assert(false);
      continue;
    }

    auto newStack = tagStack;
    newStack.push_back(index);
    if (it->second.tagReflector)
      it->second.tagReflector(newStack, path + sep + it->first, reflector);

    ++index;
  }
}

void Reflection::addReflection(const std::string& key, const ReflectionMetaData& fn)
{
  // No duplicates
  assert(reflections_.find(key) == reflections_.end());

  order_.push_back(key);
  reflections_[key] = fn;
}

void Reflection::addListReflection(const std::string& key, const ReflectionMetaData& fn, const ListReflectionGetFn& listFn)
{
  addReflection(key, fn);

  // No duplicates
  assert(listReflections_.find(key) == listReflections_.end());

  listReflections_[key] = listFn;
}

FieldList* Reflection::getMutableFieldList(FieldList* fields, const std::string& path) const
{
  if (path.empty())
    return fields;

  const auto& [key, remainder] = split_(path);

  auto it = listReflections_.find(key);
  if (it == listReflections_.end())
  {
    // Unit test checks for invalid path so no assert
    return {};
  }

  return it->second(fields, remainder);
}

std::pair<std::string, std::string> Reflection::split_(const std::string& path) const
{
  size_t pos = path.find('.');

  if (pos == std::string::npos)
    return { path, "" };

  return { path.substr(0, pos), path.substr(pos + 1) };
}

//----------------------------------------------------------------------------------------------------------------------------------

std::unique_ptr<Reflection> Reflection::makeBeamProperty()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(id, id, BeamProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(hostId, hostid, BeamProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(originalId, originalid, BeamProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(source, source, BeamProperties, getString, ReflectionDataType::String);
  std::shared_ptr<EnumerationText> typeEnum = EnumerationText::makeBeamTypeName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(type, type, BeamProperties, BeamProperties::Type, typeEnum);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeClassificationProperty()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(label, label, ClassificationProperties, getString, ReflectionDataType::String);
  rv->SIMDATA_REFLECTION_ADD_FIELD(fontColor, fontcolor, ClassificationProperties, getUint32, ReflectionDataType::Uint32);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeCoordinateFrameProperty()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  std::shared_ptr<EnumerationText> coordinateEnum = EnumerationText::makeCoordinateSystemName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(coordinateSystem, coordinatesystem, CoordinateFrameProperties, CoordinateSystem, coordinateEnum);

  // shared_ptr for lambda capture
  std::shared_ptr<Reflection> ref = makeReferenceProperty();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(referenceLla, referencella, CoordinateFrameProperties, ref);

  std::shared_ptr<EnumerationText> magneticEnum = EnumerationText::makeMagneticVarianceName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(magneticVariance, magneticvariance, CoordinateFrameProperties, MagneticVariance, magneticEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(magneticVarianceUserValue, magneticvarianceuservalue, CoordinateFrameProperties, getDouble, ReflectionDataType::Double);
  std::shared_ptr<EnumerationText> verticalEnum = EnumerationText::makeVerticalDatumName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(verticalDatum, verticaldatum, CoordinateFrameProperties, VerticalDatum, verticalEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(verticalDatumUserValue, verticaldatumuservalue, CoordinateFrameProperties, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(eciReferenceTime, ecireferencetime, CoordinateFrameProperties, getDouble, ReflectionDataType::Double);

  // shared_ptr for lambda capture
  std::shared_ptr<Reflection> tan = makeTangentPlaneOffsetsProperty();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(tangentPlaneOffset, tangentplaneoffset, CoordinateFrameProperties, tan);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeCustomRenderingProperty()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(id, id, CustomRenderingProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(hostId, hostid, CustomRenderingProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(originalId, originalid, CustomRenderingProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(source, source, CustomRenderingProperties, getString, ReflectionDataType::String);
  rv->SIMDATA_REFLECTION_ADD_FIELD(renderer, renderer, CustomRenderingProperties, getString, ReflectionDataType::String);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeDisplayFieldsPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(xLat, xlat, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(yLon, ylon, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(zAlt, zalt, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(genericData, genericdata, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(categoryData, categorydata, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(yaw, yaw, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(pitch, pitch, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(roll, roll, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(course, course, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(flightPathElevation, flightpathelevation, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(displayVX, displayvx, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(displayVY, displayvy, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(displayVZ, displayvz, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(speed, speed, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(mach, mach, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(angleOfAttack, angleofattack, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(sideSlip, sideslip, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(totalAngleOfAttack, totalangleofattack, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(solarAzimuth, solarazimuth, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(solarElevation, solarelevation, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(solarIlluminance, solarilluminance, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(lunarAzimuth, lunarazimuth, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(lunarElevation, lunarelevation, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(lunarIlluminance, lunarilluminance, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(late, late, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(useLabelCode, uselabelcode, DisplayFields, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(labelCode, labelcode, DisplayFields, getString, ReflectionDataType::String);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeGateProperty()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(id, id, GateProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(hostId, hostid, GateProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(originalId, originalid, GateProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(source, source, GateProperties, getString, ReflectionDataType::String);
  std::shared_ptr<EnumerationText> typeEnum = EnumerationText::makeGateTypeName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(type, type, GateProperties, GateProperties::Type, typeEnum);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeLaserProperty()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(id, id, LaserProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(hostId, hostid, LaserProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(originalId, originalid, LaserProperties, getUint64, ReflectionDataType::Uint64);
  std::shared_ptr<EnumerationText> coordinateEnum = EnumerationText::makeCoordinateSystemName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(coordinateSystem, coordinatesystem, LaserProperties, CoordinateSystem, coordinateEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(azElRelativeToHostOri, azelrelativetohostori, LaserProperties, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(source, source, LaserProperties, getString, ReflectionDataType::String);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeLobGroupProperty()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(id, id, LobGroupProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(hostId, hostid, LobGroupProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(originalId, originalid, LobGroupProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(source, source, LobGroupProperties, getString, ReflectionDataType::String);
  std::shared_ptr<EnumerationText> coordinateEnum = EnumerationText::makeCoordinateSystemName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(coordinateSystem, coordinatesystem, LobGroupProperties, CoordinateSystem, coordinateEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(azElRelativeToHostOri, azelrelativetohostori, LobGroupProperties, getBoolean, ReflectionDataType::Boolean);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makePlatformProperty()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(id, id, PlatformProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(originalId, originalid, PlatformProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(source, source, PlatformProperties, getString, ReflectionDataType::String);

  // shared_ptr for lambda capture
  std::shared_ptr<Reflection> coord = makeCoordinateFrameProperty();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(coordinateFrame, coordinateframe, PlatformProperties, coord);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeProjectorProperty()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(id, id, ProjectorProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(hostId, hostid, ProjectorProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(originalId, originalid, ProjectorProperties, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(source, source, ProjectorProperties, getString, ReflectionDataType::String);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeReferenceProperty()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(lat, lat, ReferenceProperties, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(lon, lon, ReferenceProperties, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(alt, alt, ReferenceProperties, getDouble, ReflectionDataType::Double);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeScenarioProperty()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(version, version, ScenarioProperties, getUint32, ReflectionDataType::Uint32);

  // shared_ptr for lambda capture
  std::shared_ptr<Reflection> coord = makeCoordinateFrameProperty();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(coordinateFrame, coordinateframe, ScenarioProperties, coord);

  rv->SIMDATA_REFLECTION_ADD_FIELD(referenceYear, referenceyear, ScenarioProperties, getUint32, ReflectionDataType::Uint32);

  // shared_ptr for lambda capture
  std::shared_ptr<Reflection> classify = makeClassificationProperty();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(classification, classification, ScenarioProperties, classify);

  rv->SIMDATA_REFLECTION_ADD_FIELD(degreeAngles, degreeangles, ScenarioProperties, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(description, description, ScenarioProperties, getString, ReflectionDataType::String);
  rv->SIMDATA_REFLECTION_ADD_FIELD(source, source, ScenarioProperties, getString, ReflectionDataType::String);
  rv->SIMDATA_REFLECTION_ADD_FIELD(windAngle, windangle, ScenarioProperties, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(windSpeed, windspeed, ScenarioProperties, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(viewFile, viewfile, ScenarioProperties, getString, ReflectionDataType::String);
  rv->SIMDATA_REFLECTION_ADD_FIELD(ruleFile, rulefile, ScenarioProperties, getString, ReflectionDataType::String);
  rv->SIMDATA_REFLECTION_ADD_FIELD(terrainFile, terrainfile, ScenarioProperties, getString, ReflectionDataType::String);

  // shared_ptr for lambda capture
  std::shared_ptr<Reflection> sound = makeSoundFileProperty();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(soundFile, soundfile, ScenarioProperties, sound);

  rv->SIMDATA_REFLECTION_ADD_VECTOR_FIELD(mediaFile, mediafile, ScenarioProperties, getStrings, ReflectionDataType::StringVector);
  rv->SIMDATA_REFLECTION_ADD_VECTOR_FIELD(dedFile, dedfile, ScenarioProperties, getStrings, ReflectionDataType::StringVector);
  rv->SIMDATA_REFLECTION_ADD_VECTOR_FIELD(wvsFile, wvsfile, ScenarioProperties, getStrings, ReflectionDataType::StringVector);
  rv->SIMDATA_REFLECTION_ADD_VECTOR_FIELD(gogFile, gogfile, ScenarioProperties, getStrings, ReflectionDataType::StringVector);

  rv->SIMDATA_REFLECTION_ADD_FIELD(dataLimitTime, datalimittime, ScenarioProperties, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(dataLimitPoints, datalimitpoints, ScenarioProperties, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(ignoreDuplicateGenericData, ignoreduplicategenericdata, ScenarioProperties, getBoolean, ReflectionDataType::Boolean);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeSoundFileProperty()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(filename, filename, SoundFileProperties, getString, ReflectionDataType::String);
  rv->SIMDATA_REFLECTION_ADD_FIELD(startTime, starttime, SoundFileProperties, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(endTime, endtime, SoundFileProperties, getDouble, ReflectionDataType::Double);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeTangentPlaneOffsetsProperty()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(tx, tx, TangentPlaneOffsetsProperties, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(ty, ty, TangentPlaneOffsetsProperties, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(angle, angle, TangentPlaneOffsetsProperties, getDouble, ReflectionDataType::Double);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeLabelPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(draw, draw, LabelPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(color, color, LabelPrefs, getUint32, ReflectionDataType::Uint32);
  std::shared_ptr<EnumerationText> textOutlineEnum = EnumerationText::makeTextOutlineName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(textOutline, textoutline, LabelPrefs, TextOutline, textOutlineEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(outlineColor, outlinecolor, LabelPrefs, getUint32, ReflectionDataType::Uint32);
  std::shared_ptr<EnumerationText> backdropTypeEnum = EnumerationText::makeBackdropTypeName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(backdropType, backdroptype, LabelPrefs,BackdropType , backdropTypeEnum);
  std::shared_ptr<EnumerationText> backdropImplementationEnum = EnumerationText::makeBackdropImplementationName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(backdropImplementation, backdropimplementation, LabelPrefs, BackdropImplementation, backdropImplementationEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(overlayFontName, overlayfontname, LabelPrefs, getString, ReflectionDataType::String);
  rv->SIMDATA_REFLECTION_ADD_FIELD(overlayFontPointSize, overlayfontpointsize, LabelPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(offsetX, offsetx, LabelPrefs, getInt32, ReflectionDataType::Int32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(offsetY, offsety, LabelPrefs, getInt32, ReflectionDataType::Int32);
  std::shared_ptr<EnumerationText> textAlignmentEnum = EnumerationText::makeTextAlignmentName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(alignment, alignment, LabelPrefs, TextAlignment, textAlignmentEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(priority, priority, LabelPrefs, getDouble, ReflectionDataType::Double);
  std::shared_ptr<Reflection> displayReflection = makeDisplayFieldsPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(displayFields, displayfields, LabelPrefs, displayReflection);
  std::shared_ptr<Reflection> legendReflection = makeDisplayFieldsPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(legendDisplayFields, legenddisplayfields, LabelPrefs, legendReflection);
  std::shared_ptr<Reflection> hoverReflection = makeDisplayFieldsPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(hoverDisplayFields, hoverdisplayfields, LabelPrefs, hoverReflection);
  std::shared_ptr<Reflection> hookReflection = makeDisplayFieldsPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(hookDisplayFields, hookdisplayfields, LabelPrefs, hookReflection);
  rv->SIMDATA_REFLECTION_ADD_FIELD(applyHeightAboveTerrain, applyheightaboveterrain, LabelPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(applyRoll, applyroll, LabelPrefs, getBoolean, ReflectionDataType::Boolean);
  std::shared_ptr<EnumerationText> coordinateSystemEnum = EnumerationText::makeCoordinateSystemName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(coordinateSystem, coordinatesystem, LabelPrefs, CoordinateSystem, coordinateSystemEnum);
  std::shared_ptr<EnumerationText> verticalDatumEnum = EnumerationText::makeVerticalDatumName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(verticalDatum, verticaldatum, LabelPrefs, VerticalDatum , verticalDatumEnum);
  std::shared_ptr<EnumerationText> magneticVarianceEnum = EnumerationText::makeMagneticVarianceName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(magneticVariance, magneticvariance, LabelPrefs, MagneticVariance, magneticVarianceEnum);
  std::shared_ptr<EnumerationText> distanceUnitsEnum = EnumerationText::makeDistanceUnitsName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(distanceUnits, distanceunits, LabelPrefs, DistanceUnits, distanceUnitsEnum);
  std::shared_ptr<EnumerationText> angleUnitsEnum = EnumerationText::makeAngleUnitsName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(angleUnits, angleunits, LabelPrefs, AngleUnits, angleUnitsEnum);
  std::shared_ptr<EnumerationText> speedUnitsEnum = EnumerationText::makeSpeedUnitsName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(speedUnits, speedunits, LabelPrefs, SpeedUnits, speedUnitsEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(precision, precision, LabelPrefs, getInt32, ReflectionDataType::Int32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(nameLength, namelength, LabelPrefs, getInt32, ReflectionDataType::Int32);
  std::shared_ptr<EnumerationText> geodeticUnitsEnum = EnumerationText::makeGeodeticUnitsName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(geodeticUnits, geodeticunits, LabelPrefs, GeodeticUnits, geodeticUnitsEnum);
  rv->SIMDATA_REFLECTION_ADD_ENUM(altitudeUnits, altitudeunits, LabelPrefs, DistanceUnits, distanceUnitsEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(distancePrecision, distanceprecision, LabelPrefs, getInt32, ReflectionDataType::Int32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(anglePrecision, angleprecision, LabelPrefs, getInt32, ReflectionDataType::Int32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(speedPrecision, speedprecision, LabelPrefs, getInt32, ReflectionDataType::Int32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(geodeticPrecision, geodeticprecision, LabelPrefs, getInt32, ReflectionDataType::Int32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(altitudePrecision, altitudeprecision, LabelPrefs, getInt32, ReflectionDataType::Int32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(timePrecision, timeprecision, LabelPrefs, getInt32, ReflectionDataType::Int32);
  std::shared_ptr<EnumerationText> useValuesEnum = EnumerationText::makeUseValueName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(useValues, usevalues, LabelPrefs, LabelPrefs::UseValue, useValuesEnum);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeSpeedRingPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(useFixedTime, usefixedtime, SpeedRing, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(fixedTime, fixedtime, SpeedRing, getString, ReflectionDataType::String);
  std::shared_ptr<EnumerationText> timeFormatEnum = EnumerationText::makeElapsedTimeFormatName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(timeFormat, timeformat, SpeedRing, ElapsedTimeFormat, timeFormatEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(radius, radius, SpeedRing, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(usePlatformSpeed, useplatformspeed, SpeedRing, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(speedToUse, speedtouse, SpeedRing, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(displayTime, displaytime, SpeedRing, getBoolean, ReflectionDataType::Boolean);
  std::shared_ptr<EnumerationText> speedUnitsEnum = EnumerationText::makeSpeedUnitsName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(speedUnits, speedunits, SpeedRing, SpeedUnits, speedUnitsEnum);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeGridSettingsPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(numDivisions, numdivisions, GridSettings, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(numSubDivisions, numsubdivisions, GridSettings, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(sectorAngle, sectorangle, GridSettings, getDouble, ReflectionDataType::Double);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makePositionPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(x, x, Position, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(y, y, Position, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(z, z, Position, getDouble, ReflectionDataType::Double);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeBodyOrientationPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(yaw, yaw, BodyOrientation, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(pitch, pitch, BodyOrientation, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(roll, roll, BodyOrientation, getDouble, ReflectionDataType::Double);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeLocalGridPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  std::shared_ptr<EnumerationText> localGridTypeEnum = EnumerationText::makeLocalGridTypeName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(gridType, gridtype, LocalGridPrefs, LocalGridPrefs::Type, localGridTypeEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(gridLabelDraw, gridlabeldraw, LocalGridPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(gridLabelColor, gridlabelcolor, LocalGridPrefs, getUint32, ReflectionDataType::Uint32);
  std::shared_ptr<EnumerationText> textOutlineEnum = EnumerationText::makeTextOutlineName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(gridLabelTextOutline, gridlabeltextoutline, LocalGridPrefs, simData::TextOutline , textOutlineEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(gridLabelOutlineColor, gridlabeloutlinecolor, LocalGridPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(gridLabelFontName, gridlabelfontname, LocalGridPrefs, getString, ReflectionDataType::String);
  rv->SIMDATA_REFLECTION_ADD_FIELD(gridLabelFontSize, gridlabelfontsize, LocalGridPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(gridLabelPrecision, gridlabelprecision, LocalGridPrefs, getInt32, ReflectionDataType::Int32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(drawGrid, drawgrid, LocalGridPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(gridColor, gridcolor, LocalGridPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(size, size, LocalGridPrefs, getDouble, ReflectionDataType::Double);
  std::shared_ptr<Reflection> speedRing = makeSpeedRingPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(speedRing, speedring, LocalGridPrefs, speedRing);
  std::shared_ptr<Reflection> grid = makeGridSettingsPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(gridSettings, gridsettings, LocalGridPrefs, grid);
  std::shared_ptr<Reflection> gridPosition = makePositionPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(gridPositionOffset, gridpositionoffset, LocalGridPrefs, gridPosition);
  std::shared_ptr<EnumerationText> positionEnum = EnumerationText::makeDistanceUnitsName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(positionOffsetUnits, positionoffsetunits, LocalGridPrefs, DistanceUnits, positionEnum);
  std::shared_ptr<Reflection> body = makeBodyOrientationPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(gridOrientationOffset, gridorientationoffset, LocalGridPrefs, body);
  rv->SIMDATA_REFLECTION_ADD_FIELD(followYaw, followyaw, LocalGridPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(followPitch, followpitch, LocalGridPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(followRoll, followroll, LocalGridPrefs, getBoolean, ReflectionDataType::Boolean);
  std::shared_ptr<EnumerationText> sizeEnum = EnumerationText::makeDistanceUnitsName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(sizeUnits, sizeunits, LocalGridPrefs, DistanceUnits, sizeEnum);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeCommonPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(dataDraw, datadraw, CommonPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(draw, draw, CommonPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(name, name, CommonPrefs, getString, ReflectionDataType::String);
  rv->SIMDATA_REFLECTION_ADD_FIELD(useAlias, usealias, CommonPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(alias, alias, CommonPrefs, getString, ReflectionDataType::String);
  std::shared_ptr<Reflection> label = makeLabelPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(labelPrefs, labelprefs, CommonPrefs, label);
  rv->SIMDATA_REFLECTION_ADD_FIELD(color, color, CommonPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(useOverrideColor, useoverridecolor, CommonPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(overrideColor, overridecolor, CommonPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(dataLimitTime, datalimittime, CommonPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(dataLimitPoints, datalimitpoints, CommonPrefs, getUint32, ReflectionDataType::Uint32);
  std::shared_ptr<Reflection> grid = makeLocalGridPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(localGrid, localgrid, CommonPrefs, grid);
  rv->SIMDATA_REFLECTION_ADD_FIELD(includeInLegend, includeinlegend, CommonPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_VECTOR_FIELD(acceptProjectorIds, acceptprojectorids, CommonPrefs, getIds, ReflectionDataType::IdVector);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeCustomRenderingPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  std::shared_ptr<Reflection> common = makeCommonPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(commonPrefs, commonprefs, CustomRenderingPrefs, common);
  rv->SIMDATA_REFLECTION_ADD_FIELD(persistence, persistence, CustomRenderingPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(secondsHistory, secondshistory, CustomRenderingPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(pointsHistory, pointshistory, CustomRenderingPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(outline, outline, CustomRenderingPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(useHistoryOverrideColor, usehistoryoverridecolor, CustomRenderingPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(historyOverrideColor, historyoverridecolor, CustomRenderingPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(centerAxis, centeraxis, CustomRenderingPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(showLighted, showlighted, CustomRenderingPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(depthTest, depthtest, CustomRenderingPrefs, getBoolean, ReflectionDataType::Boolean);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeProjectorPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  std::shared_ptr<Reflection> common = makeCommonPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(commonPrefs, commonprefs, ProjectorPrefs, common);
  rv->SIMDATA_REFLECTION_ADD_FIELD(rasterFile, rasterfile, ProjectorPrefs, getString, ReflectionDataType::String);
  rv->SIMDATA_REFLECTION_ADD_FIELD(showFrustum, showfrustum, ProjectorPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(projectorAlpha, projectoralpha, ProjectorPrefs, getFloat, ReflectionDataType::Float);
  rv->SIMDATA_REFLECTION_ADD_FIELD(interpolateProjectorFov, interpolateprojectorfov, ProjectorPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(overrideFov, overridefov, ProjectorPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(overrideFovAngle, overridefovangle, ProjectorPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(overrideHFov, overridehfov, ProjectorPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(overrideHFovAngle, overridehfovangle, ProjectorPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(shadowMapping, shadowmapping, ProjectorPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(maxDrawRange, maxdrawrange, ProjectorPrefs, getFloat, ReflectionDataType::Float);
  rv->SIMDATA_REFLECTION_ADD_FIELD(doubleSided, doublesided, ProjectorPrefs, getBoolean, ReflectionDataType::Boolean);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeLaserPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  std::shared_ptr<Reflection> common = makeCommonPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(commonPrefs, commonprefs, LaserPrefs, common);
  std::shared_ptr<Reflection> position = makePositionPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(laserXyzOffset, laserxyzoffset, LaserPrefs, position);
  rv->SIMDATA_REFLECTION_ADD_FIELD(maxRange, maxrange, LaserPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(laserWidth, laserwidth, LaserPrefs, getInt32, ReflectionDataType::Int32);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeGatePreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  std::shared_ptr<Reflection> common = makeCommonPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(commonPrefs, commonprefs, GatePrefs, common);
  rv->SIMDATA_REFLECTION_ADD_FIELD(gateLighting, gatelighting, GatePrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(gateBlending, gateblending, GatePrefs, getBoolean, ReflectionDataType::Boolean);
  std::shared_ptr<EnumerationText> mode = EnumerationText::makeGateDrawModeName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(gateDrawMode, gatedrawmode, GatePrefs, GatePrefs::DrawMode, mode);
  std::shared_ptr<EnumerationText> fill = EnumerationText::makeGateFillPatternName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(fillPattern, fillpattern, GatePrefs, GatePrefs::FillPattern, fill);
  rv->SIMDATA_REFLECTION_ADD_FIELD(drawCentroid, drawcentroid, GatePrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(interpolateGatePos, interpolategatepos, GatePrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(gateAzimuthOffset, gateazimuthoffset, GatePrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(gateElevationOffset, gateelevationoffset, GatePrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(gateRollOffset, gaterolloffset, GatePrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(drawOutline, drawoutline, GatePrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(centroidColor, centroidcolor, GatePrefs, getUint32, ReflectionDataType::Uint32);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makePreferences(simData::ObjectType type)
{
  switch (type)
  {
  case simData::NONE:
    break;
  case simData::PLATFORM:
    return makePlatformPreferences();
  case simData::BEAM:
    return makeBeamPreferences();
  case simData::GATE:
    return makeGatePreferences();
  case simData::LASER:
    return makeLaserPreferences();
  case simData::PROJECTOR:
    return makeProjectorPreferences();
  case simData::LOB_GROUP:
    return makeLobGroupPreferences();
  case simData::CUSTOM_RENDERING:
    return makeCustomRenderingPreferences();
  case simData::ALL:
    return makeCommonPreferences();
  }

  // Invalid type
  assert(false);
  return nullptr;
}

std::unique_ptr<Reflection> Reflection::makeAntennaPatternsPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  std::shared_ptr<EnumerationText> typeEnum = EnumerationText::makeAntennaPatternTypeName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(type, type, AntennaPatterns, AntennaPatterns::Type, typeEnum);
  std::shared_ptr<EnumerationText> formatEnum = EnumerationText::makeAntennaPatternFileFormatName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(fileFormat, fileformat, AntennaPatterns, AntennaPatterns::FileFormat, formatEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(fileName, filename, AntennaPatterns, getString, ReflectionDataType::String);
  std::shared_ptr<EnumerationText> algorithmEnum = EnumerationText::makeAntennaPatternAlgorithmName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(algorithm, algorithm, AntennaPatterns, AntennaPatterns::Algorithm, algorithmEnum);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeBeamPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  std::shared_ptr<Reflection> common = makeCommonPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(commonPrefs, commonprefs, BeamPrefs, common);
  rv->SIMDATA_REFLECTION_ADD_FIELD(shaded, shaded, BeamPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(blended, blended, BeamPrefs, getBoolean, ReflectionDataType::Boolean);
  std::shared_ptr<EnumerationText> modeEnum = EnumerationText::makeBeamDrawModeName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(beamDrawMode, beamdrawmode, BeamPrefs, BeamPrefs::DrawMode, modeEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(beamScale, beamscale, BeamPrefs, getDouble, ReflectionDataType::Double);
  std::shared_ptr<EnumerationText> typeEnum = EnumerationText::makeBeamDrawTypeName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(drawType, drawtype, BeamPrefs, BeamPrefs::DrawType, typeEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(capResolution, capresolution, BeamPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(coneResolution, coneresolution, BeamPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(renderCone, rendercone, BeamPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(sensitivity, sensitivity, BeamPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(gain, gain, BeamPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(fieldOfView, fieldofview, BeamPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(detail, detail, BeamPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(power, power, BeamPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(frequency, frequency, BeamPrefs, getDouble, ReflectionDataType::Double);
  std::shared_ptr<EnumerationText> polarityEnum = EnumerationText::makePolarityName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(polarity, polarity, BeamPrefs, Polarity, polarityEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(colorScale, colorscale, BeamPrefs, getBoolean, ReflectionDataType::Boolean);
  std::shared_ptr<Reflection> antennaReflection = makeAntennaPatternsPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(antennaPattern, antennapattern, BeamPrefs, antennaReflection);
  rv->SIMDATA_REFLECTION_ADD_FIELD(arepsFile, arepsfile, BeamPrefs, getString, ReflectionDataType::String);
  rv->SIMDATA_REFLECTION_ADD_FIELD(channel, channel, BeamPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(weighting, weighting, BeamPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(interpolateBeamPos, interpolatebeampos, BeamPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(useOffsetPlatform, useoffsetplatform, BeamPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(useOffsetIcon, useoffseticon, BeamPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(useOffsetBeam, useoffsetbeam, BeamPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(azimuthOffset, azimuthoffset, BeamPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(elevationOffset, elevationoffset, BeamPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(rollOffset, rolloffset, BeamPrefs, getDouble, ReflectionDataType::Double);
  std::shared_ptr<Reflection> positionReflection = makePositionPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(beamPositionOffset, beampositionoffset, BeamPrefs, positionReflection);
  rv->SIMDATA_REFLECTION_ADD_FIELD(targetId, targetid, BeamPrefs, getUint64, ReflectionDataType::Uint64);
  rv->SIMDATA_REFLECTION_ADD_FIELD(verticalWidth, verticalwidth, BeamPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(horizontalWidth, horizontalwidth, BeamPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(animate, animate, BeamPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(pulseLength, pulselength, BeamPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(pulseRate, pulserate, BeamPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(pulseStipple, pulsestipple, BeamPrefs, getUint32, ReflectionDataType::Uint32);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeTimeTickPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  std::shared_ptr<EnumerationText> styleEnum = EnumerationText::makeTimeTickDrawStyleName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(drawStyle, drawstyle, TimeTickPrefs, TimeTickPrefs::DrawStyle, styleEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(color, color, TimeTickPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(interval, interval, TimeTickPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(largeIntervalFactor, largeintervalfactor, TimeTickPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(labelIntervalFactor, labelintervalfactor, TimeTickPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(labelFontName, labelfontname, TimeTickPrefs, getString, ReflectionDataType::String);
  rv->SIMDATA_REFLECTION_ADD_FIELD(labelFontPointSize, labelfontpointsize, TimeTickPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(labelColor, labelcolor, TimeTickPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(lineLength, linelength, TimeTickPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(largeSizeFactor, largesizefactor, TimeTickPrefs, getUint32, ReflectionDataType::Uint32);
  std::shared_ptr<EnumerationText> formatEnum = EnumerationText::makeElapsedTimeFormatName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(labelTimeFormat, labeltimeformat, TimeTickPrefs, ElapsedTimeFormat, formatEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(lineWidth, linewidth, TimeTickPrefs, getDouble, ReflectionDataType::Double);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeTrackPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(trackColor, trackcolor, TrackPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(multiTrackColor, multitrackcolor, TrackPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(flatMode, flatmode, TrackPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(altMode, altmode, TrackPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(expireMode, expiremode, TrackPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(usePlatformColor, useplatformcolor, TrackPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(useTrackOverrideColor, usetrackoverridecolor, TrackPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(trackOverrideColor, trackoverridecolor, TrackPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(trackLength, tracklength, TrackPrefs, getInt32, ReflectionDataType::Int32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(lineWidth, linewidth, TrackPrefs, getDouble, ReflectionDataType::Double);
  std::shared_ptr<EnumerationText> modeEnum = EnumerationText::makeTrackModeName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(trackDrawMode, trackdrawmode, TrackPrefs,TrackPrefs::Mode, modeEnum);
  std::shared_ptr<Reflection> timeTickReflection = makeTimeTickPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(timeTicks, timeticks, TrackPrefs, timeTickReflection);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makePlatformPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  std::shared_ptr<Reflection> common = makeCommonPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(commonPrefs, commonprefs, PlatformPrefs, common);
  rv->SIMDATA_REFLECTION_ADD_FIELD(icon, icon, PlatformPrefs, getString, ReflectionDataType::String);
  std::shared_ptr<EnumerationText> modeEnum = EnumerationText::makeModelDrawModeName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(drawMode, drawmode, PlatformPrefs, ModelDrawMode, modeEnum);
  std::shared_ptr<EnumerationText> fragmentEnum = EnumerationText::makeFragmentEffectName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(fragmentEffect, fragmenteffect, PlatformPrefs, FragmentEffect, fragmentEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(fragmentEffectColor, fragmenteffectcolor, PlatformPrefs, getUint32, ReflectionDataType::Uint32);
  std::shared_ptr<EnumerationText> rotationEnum = EnumerationText::makeIconRotationName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(rotateIcons, rotateicons, PlatformPrefs, IconRotation , rotationEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(noDepthIcons, nodepthicons, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  std::shared_ptr<EnumerationText> iconAlignmentEnum = EnumerationText::makeTextAlignmentName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(iconAlignment, iconalignment, PlatformPrefs, TextAlignment, iconAlignmentEnum);
  std::shared_ptr<EnumerationText> combineEnum = EnumerationText::makeOverrideColorCombineModeName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(overrideColorCombineMode, overridecolorcombinemode, PlatformPrefs, OverrideColorCombineMode, combineEnum);
  std::shared_ptr<Reflection> trackReflection = makeTrackPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(trackPrefs, trackprefs, PlatformPrefs, trackReflection);
  rv->SIMDATA_REFLECTION_ADD_FIELD(useClampAlt, useclampalt, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(clampValAltMin, clampvalaltmin, PlatformPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(clampValAltMax, clampvalaltmax, PlatformPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(useClampYaw, useclampyaw, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(clampValYaw, clampvalyaw, PlatformPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(useClampPitch, useclamppitch, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(clampValPitch, clampvalpitch, PlatformPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(useClampRoll, useclamproll, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(clampValRoll, clampvalroll, PlatformPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(clampOrientationAtLowVelocity, clamporientationatlowvelocity, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(surfaceClamping, surfaceclamping, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(aboveSurfaceClamping, abovesurfaceclamping, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(lighted, lighted, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(drawBox, drawbox, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(drawBodyAxis, drawbodyaxis, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(drawInertialAxis, drawinertialaxis, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(drawSunVec, drawsunvec, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(drawMoonVec, drawmoonvec, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(axisScale, axisscale, PlatformPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(wireFrame, wireframe, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(drawOpticLos, drawopticlos, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(drawRfLos, drawrflos, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(rcsFile, rcsfile, PlatformPrefs, getString, ReflectionDataType::String);
  rv->SIMDATA_REFLECTION_ADD_FIELD(drawRcs, drawrcs, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(draw3dRcs, draw3drcs, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(rcsColor, rcscolor, PlatformPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(rcsColorScale, rcscolorscale, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  std::shared_ptr<EnumerationText> rcsPolarityEnum = EnumerationText::makePolarityName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(rcsPolarity, rcspolarity, PlatformPrefs, Polarity, rcsPolarityEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(rcsElevation, rcselevation, PlatformPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(rcsFrequency, rcsfrequency, PlatformPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(rcsDetail, rcsdetail, PlatformPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(drawCircleHilight, drawcirclehilight, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(circleHilightColor, circlehilightcolor, PlatformPrefs, getUint32, ReflectionDataType::Uint32);
  std::shared_ptr<EnumerationText> circleHilightEnum = EnumerationText::makeCircleHilightShapeName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(circleHilightShape, circlehilightshape, PlatformPrefs, CircleHilightShape , circleHilightEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(circleHilightSize, circlehilightsize, PlatformPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(hilightFollowYaw, hilightfollowyaw, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(interpolatePos, interpolatepos, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(extrapolatePos, extrapolatepos, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(scale, scale, PlatformPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(brightness, brightness, PlatformPrefs, getInt32, ReflectionDataType::Int32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(dynamicScale, dynamicscale, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(dynamicScaleOffset, dynamicscaleoffset, PlatformPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(dynamicScaleScalar, dynamicscalescalar, PlatformPrefs, getDouble, ReflectionDataType::Double);
  std::shared_ptr<EnumerationText> dynamicScaleEnum = EnumerationText::makeDynamicScaleAlgorithmName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(dynamicScaleAlgorithm, dynamicscalealgorithm, PlatformPrefs, DynamicScaleAlgorithm , dynamicScaleEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(drawVelocityVec, drawvelocityvec, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(velVecColor, velveccolor, PlatformPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(velVecUseStaticLength, velvecusestaticlength, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(velVecStaticLen, velvecstaticlen, PlatformPrefs, getDouble, ReflectionDataType::Double);
  std::shared_ptr<EnumerationText> velVecStaticLenEnum = EnumerationText::makeDistanceUnitsName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(velVecStaticLenUnits, velvecstaticlenunits, PlatformPrefs, DistanceUnits , velVecStaticLenEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(velVecTime, velvectime, PlatformPrefs, getDouble, ReflectionDataType::Double);
  std::shared_ptr<EnumerationText> velVecTimeEnum = EnumerationText::makeElapsedTimeFormatName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(velVecTimeUnits, velvectimeunits, PlatformPrefs, ElapsedTimeFormat, velVecTimeEnum);
  std::shared_ptr<Reflection> positionReflection = makePositionPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(platPositionOffset, platpositionoffset, PlatformPrefs, positionReflection);
  std::shared_ptr<Reflection> orientationOffsetReflection = makeBodyOrientationPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(orientationOffset, orientationoffset, PlatformPrefs, orientationOffsetReflection);
  rv->SIMDATA_REFLECTION_ADD_VECTOR_FIELD(gogFile, gogfile, PlatformPrefs, getStrings, ReflectionDataType::StringVector);
  std::shared_ptr<Reflection> scaleXyzReflection = makePositionPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(scaleXYZ, scalexyz, PlatformPrefs, scaleXyzReflection);
  rv->SIMDATA_REFLECTION_ADD_FIELD(alphaVolume, alphavolume, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(useCullFace, usecullface, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  std::shared_ptr<EnumerationText> cullFaceEnum = EnumerationText::makePolygonFaceName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(cullFace, cullface, PlatformPrefs, PolygonFace, cullFaceEnum);
  std::shared_ptr<EnumerationText> polygonModeFaceEnum = EnumerationText::makePolygonFaceName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(polygonModeFace, polygonmodeface, PlatformPrefs, PolygonFace, polygonModeFaceEnum);
  std::shared_ptr<EnumerationText> polygonModeEnum = EnumerationText::makePolygonModeName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(polygonMode, polygonmode, PlatformPrefs, PolygonMode , polygonModeEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(usePolygonStipple, usepolygonstipple, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(polygonStipple, polygonstipple, PlatformPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(visibleLosColor, visibleloscolor, PlatformPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(obstructedLosColor, obstructedloscolor, PlatformPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(losRangeResolution, losrangeresolution, PlatformPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(losAzimuthalResolution, losazimuthalresolution, PlatformPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(losAltitudeOffset, losaltitudeoffset, PlatformPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(animateDofNodes, animatedofnodes, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(eciDataMode, ecidatamode, PlatformPrefs, getBoolean, ReflectionDataType::Boolean);
  std::shared_ptr<EnumerationText> drawOffBehaviorEnum = EnumerationText::makePlatformDrawOffBehaviorName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(drawOffBehavior, drawoffbehavior, PlatformPrefs, PlatformPrefs::DrawOffBehavior, drawOffBehaviorEnum);
  std::shared_ptr<EnumerationText> lifespanModeEnum = EnumerationText::makeLifespanModeName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(lifespanMode, lifespanmode, PlatformPrefs, LifespanMode, lifespanModeEnum);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeLobGroupPreferences()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  std::shared_ptr<Reflection> common = makeCommonPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(commonPrefs, commonprefs, LobGroupPrefs, common);
  std::shared_ptr<Reflection> xyzOffsetReflection = makePositionPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(xyzOffset, xyzoffset, LobGroupPrefs, xyzOffsetReflection);
  rv->SIMDATA_REFLECTION_ADD_FIELD(lobwidth, lobwidth, LobGroupPrefs, getInt32, ReflectionDataType::Int32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(color1, color1, LobGroupPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(color2, color2, LobGroupPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(stipple1, stipple1, LobGroupPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(stipple2, stipple2, LobGroupPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(maxDataSeconds, maxdataseconds, LobGroupPrefs, getDouble, ReflectionDataType::Double);
  rv->SIMDATA_REFLECTION_ADD_FIELD(maxDataPoints, maxdatapoints, LobGroupPrefs, getUint32, ReflectionDataType::Uint32);
  rv->SIMDATA_REFLECTION_ADD_FIELD(lobUseClampAlt, lobuseclampalt, LobGroupPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(useRangeOverride, userangeoverride, LobGroupPrefs, getBoolean, ReflectionDataType::Boolean);
  rv->SIMDATA_REFLECTION_ADD_FIELD(rangeOverrideValue, rangeoverridevalue, LobGroupPrefs, getDouble, ReflectionDataType::Double);
  std::shared_ptr<EnumerationText> bendingEnum = EnumerationText::makeAnimatedLineBendName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(bending, bending, LobGroupPrefs, AnimatedLineBend, bendingEnum);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeBeamCommands()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(time, time, BeamCommand, getDouble, ReflectionDataType::Double);
  std::shared_ptr<Reflection> preferenceReflection = makeBeamPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(updatePrefs, updateprefs, BeamCommand, preferenceReflection);
  rv->SIMDATA_REFLECTION_ADD_FIELD(isClearCommand, isclearcommand, BeamCommand, getBoolean, ReflectionDataType::Boolean);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeCustomRenderingCommands()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(time, time, CustomRenderingCommand, getDouble, ReflectionDataType::Double);
  std::shared_ptr<Reflection> preferenceReflection = makeCustomRenderingPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(updatePrefs, updateprefs, CustomRenderingCommand, preferenceReflection);
  rv->SIMDATA_REFLECTION_ADD_FIELD(isClearCommand, isclearcommand, CustomRenderingCommand, getBoolean, ReflectionDataType::Boolean);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeGateCommands()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(time, time, GateCommand, getDouble, ReflectionDataType::Double);
  std::shared_ptr<Reflection> preferenceReflection = makeGatePreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(updatePrefs, updateprefs, GateCommand, preferenceReflection);
  rv->SIMDATA_REFLECTION_ADD_FIELD(isClearCommand, isclearcommand, GateCommand, getBoolean, ReflectionDataType::Boolean);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeLaserCommands()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(time, time, LaserCommand, getDouble, ReflectionDataType::Double);
  std::shared_ptr<Reflection> preferenceReflection = makeLaserPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(updatePrefs, updateprefs, LaserCommand, preferenceReflection);
  rv->SIMDATA_REFLECTION_ADD_FIELD(isClearCommand, isclearcommand, LaserCommand, getBoolean, ReflectionDataType::Boolean);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeLobGroupCommands()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(time, time, LobGroupCommand, getDouble, ReflectionDataType::Double);
  std::shared_ptr<Reflection> preferenceReflection = makeLobGroupPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(updatePrefs, updateprefs, LobGroupCommand, preferenceReflection);
  rv->SIMDATA_REFLECTION_ADD_FIELD(isClearCommand, isclearcommand, LobGroupCommand, getBoolean, ReflectionDataType::Boolean);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makePlatformCommands()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(time, time, PlatformCommand, getDouble, ReflectionDataType::Double);
  std::shared_ptr<Reflection> preferenceReflection = makePlatformPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(updatePrefs, updateprefs, PlatformCommand, preferenceReflection);
  rv->SIMDATA_REFLECTION_ADD_FIELD(isClearCommand, isclearcommand, PlatformCommand, getBoolean, ReflectionDataType::Boolean);

  return rv;
}

std::unique_ptr<Reflection> Reflection::makeProjectorCommands()
{
  auto rv = std::make_unique<Reflection>();

  // Order needs to match legacy protobuf order

  rv->SIMDATA_REFLECTION_ADD_FIELD(time, time, ProjectorCommand, getDouble, ReflectionDataType::Double);
  std::shared_ptr<Reflection> preferenceReflection = makeProjectorPreferences();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(updatePrefs, updateprefs, ProjectorCommand, preferenceReflection);
  rv->SIMDATA_REFLECTION_ADD_FIELD(isClearCommand, isclearcommand, ProjectorCommand, getBoolean, ReflectionDataType::Boolean);

  return rv;
}


Reflection::TagStackMap Reflection::makeTagStackMap(const Reflection& reflection)
{
  Reflection::TagStackMap rv;

  reflection.reflection({}, "", [&rv](const TagStack& tagStack, const std::string& path, ReflectionDataType type) {
    rv[path] = tagStack;
    });

  return rv;
}

Reflection::TagStackMap Reflection::makePreferencesTagStackMap(simData::ObjectType type)
{
  auto preferences = makePreferences(type);
  if (!preferences)
    return Reflection::TagStackMap();

  return makeTagStackMap(*preferences);
}

Reflection::TagStack Reflection::getPreferencesTagStack(const std::string& path, const TagStackMap& tags)
{
  auto it = tags.find(path);
  if (it == tags.end())
  {
    // Might be asking for FieldList
    for (const auto& [key, tagStack] : tags)
    {
      if (key.substr(0, path.size()) == path)
      {
        auto rv = tagStack;
        rv.pop_back();
        return rv;
      }
    }

    // Invalid path
    assert(false);
    return Reflection::TagStack();
  }

  return it->second;
}

Reflection::TagStack Reflection::getPreferencesTagStack(const std::string& path, simData::ObjectType type)
{
  static std::map<simData::ObjectType, Reflection::TagStackMap> tagStackMap;

  auto it = tagStackMap.find(type);
  if (it == tagStackMap.end())
  {
    const auto tags = makePreferencesTagStackMap(type);
    if (tags.empty())
    {
      // Invalid type
      assert(false);
      return Reflection::TagStack();
    }

    tagStackMap[type] = tags;
    it = tagStackMap.find(type);
  }

  return getPreferencesTagStack(path, it->second);
}

}
