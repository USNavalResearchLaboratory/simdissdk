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
#include "DataTypeProperties.h"
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

ReflectionValue::~ReflectionValue()
{
}

bool ReflectionValue::operator==(const ReflectionValue& rhs) const
{
  if (type() != rhs.type())
    return false;

  if (std::holds_alternative<StringPtr>(data_))
    return getString() == rhs.getString();

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

void ReflectionValue::setEnumerationText(std::shared_ptr<EnumerationText> text)
{
  enumerationText_ = text;
}

std::string ReflectionValue::getEnumerationText() const
{
  // Not an enumeration
  assert(std::holds_alternative<int32_t>(data_));

  return enumerationText_ ? enumerationText_->text(getInt32()) : std::string();
}

simData::ReflectionDataType ReflectionValue::type() const
{
  static std::array<simData::ReflectionDataType, 8> types = {
    ReflectionDataType::Unknown,
    ReflectionDataType::Boolean,
    ReflectionDataType::Int32,
    ReflectionDataType::Uint32,
    ReflectionDataType::Uint64,
    ReflectionDataType::Double,
    ReflectionDataType::String,
    ReflectionDataType::StringVector,
  };

  auto idx = data_.index();
  if (idx < types.size())
  {
    auto type = types[idx];
    if (type != ReflectionDataType::Int32)
      return type;

    return enumerationText_ ? ReflectionDataType::Enumeration : type;
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
    // Invalid path
    assert(false);
    return {};
  }

  return it->second.getValue(fields, remainder);
}

int Reflection::setValue(FieldList* fields, const ReflectionValue& value, const std::string& path)
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

void Reflection::reflection(const std::string& path, ReflectorVisitorFn reflector)
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

void Reflection::addReflection(const std::string& key, const ReflectionMetaData& fn)
{
  // No duplicates
  assert(reflections_.find(key) == reflections_.end());

  order_.push_back(key);
  reflections_[key] = fn;
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
  rv->SIMDATA_REFLECTION_ADD_ENUM(coordinateSystem, coordinatesystem, CoordinateFrameProperties, CoordinateSystemProperties, coordinateEnum);

  // shared_ptr for lambda capture
  std::shared_ptr<Reflection> ref = makeReferenceProperty();
  rv->SIMDATA_REFLECTION_ADD_SUBFIELD_LIST(referenceLla, referencella, CoordinateFrameProperties, ref);

  std::shared_ptr<EnumerationText> magneticEnum = EnumerationText::makeMagneticVarianceName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(magneticVariance, magneticvariance, CoordinateFrameProperties, MagneticVarianceProperties, magneticEnum);
  rv->SIMDATA_REFLECTION_ADD_FIELD(magneticVarianceUserValue, magneticvarianceuservalue, CoordinateFrameProperties, getDouble, ReflectionDataType::Double);
  std::shared_ptr<EnumerationText> verticalEnum = EnumerationText::makeVerticalDatumName();
  rv->SIMDATA_REFLECTION_ADD_ENUM(verticalDatum, verticaldatum, CoordinateFrameProperties, VerticalDatumProperties, verticalEnum);
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
  rv->SIMDATA_REFLECTION_ADD_ENUM(coordinateSystem, coordinatesystem, LaserProperties, CoordinateSystemProperties, coordinateEnum);
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
  rv->SIMDATA_REFLECTION_ADD_ENUM(coordinateSystem, coordinatesystem, LobGroupProperties, CoordinateSystemProperties, coordinateEnum);
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

}
