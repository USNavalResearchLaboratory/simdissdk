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

#include "simCore/Common/SDKAssert.h"
#include "simData/DataTypeProperties.h"
#include "simData/DataTypeReflection.h"

namespace {

int testReflectionValue()
{
  int rv = 0;

  // Verify uint64_t
  simData::ReflectionValue valueUint64(static_cast<uint64_t>(1));
  rv += SDK_ASSERT(valueUint64.type() == simData::ReflectionDataType::Uint64);
  rv += SDK_ASSERT(valueUint64.getUint64() == 1ul);
  valueUint64.setUint64(2);
  rv += SDK_ASSERT(valueUint64.type() == simData::ReflectionDataType::Uint64);
  rv += SDK_ASSERT(valueUint64.getUint64() == 2ul);
  rv += SDK_ASSERT(valueUint64 != simData::ReflectionValue(static_cast<uint64_t>(1)));
  rv += SDK_ASSERT(valueUint64 == simData::ReflectionValue(static_cast<uint64_t>(2)));
  rv += SDK_ASSERT(valueUint64 != simData::ReflectionValue("ShouldNotMatch"));

  // Verify std::string
  simData::ReflectionValue valueString("Test");
  rv += SDK_ASSERT(valueString.type() == simData::ReflectionDataType::String);
  rv += SDK_ASSERT(valueString.getString() == "Test");
  valueString.setString("Test2");
  rv += SDK_ASSERT(valueString.type() == simData::ReflectionDataType::String);
  rv += SDK_ASSERT(valueString.getString() == "Test2");
  rv += SDK_ASSERT(valueString != simData::ReflectionValue("Test"));
  rv += SDK_ASSERT(valueString == simData::ReflectionValue("Test2"));
  rv += SDK_ASSERT(valueString != simData::ReflectionValue(static_cast<uint64_t>(1)));

  return rv;
}

int testOneField(const std::map<std::string, simData::ReflectionDataType>& fields, const std::string& key, simData::ReflectionDataType type)
{
  int rv = 0;

  auto it = fields.find(key);
  rv += SDK_ASSERT(it != fields.end());
  if (it == fields.end())
    return 1;

  rv += SDK_ASSERT(it->second == type);
  return rv;
}

int testBeamPropertyReflection()
{
  int rv = 0;

  simData::BeamProperties prop;
  auto reflection = simData::Reflection::makeBeamProperty();

  // Verify get/set for each field

  rv += SDK_ASSERT(!reflection->getValue(&prop, "id").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(1)), "id") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "id").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "id").value().getUint64() == 1ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "hostId").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(2)), "hostId") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "hostId").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "hostId").value().getUint64() == 2ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "originalId").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(3)), "originalId") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "originalId").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "originalId").value().getUint64() == 3ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "type").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<int32_t>(1)), "type") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "type").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "type").value().getInt32() == 1);

  // Verify reflection
  std::map<std::string, simData::ReflectionDataType> fields;
  reflection->reflection("", [&fields](const std::string& path, simData::ReflectionDataType type) {
    fields[path] = type;
    });

  rv += SDK_ASSERT(fields.size() == 5);
  rv += SDK_ASSERT(testOneField(fields, "id", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "hostId", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "originalId", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "type", simData::ReflectionDataType::Enumeration) == 0);
  rv += SDK_ASSERT(testOneField(fields, "source", simData::ReflectionDataType::String) == 0);

  return rv;
}

int testClassificationPropertyReflection()
{
  int rv = 0;

  auto reflection = simData::Reflection::makeClassificationProperty();

  // Verify reflection
  std::map<std::string, simData::ReflectionDataType> fields;
  reflection->reflection("", [&fields](const std::string& path, simData::ReflectionDataType type) {
    fields[path] = type;
    });

  rv += SDK_ASSERT(fields.size() == 2);
  rv += SDK_ASSERT(testOneField(fields, "label", simData::ReflectionDataType::String) == 0);
  rv += SDK_ASSERT(testOneField(fields, "fontColor", simData::ReflectionDataType::Uint32) == 0);

  return rv;
}

int testCoordinateFramePropertyReflection()
{
  int rv = 0;

  auto reflection = simData::Reflection::makeCoordinateFrameProperty();

  // Verify reflection
  std::map<std::string, simData::ReflectionDataType> fields;
  reflection->reflection("", [&fields](const std::string& path, simData::ReflectionDataType type) {
    fields[path] = type;
    });

  rv += SDK_ASSERT(fields.size() == 12);
  rv += SDK_ASSERT(testOneField(fields, "coordinateSystem", simData::ReflectionDataType::Enumeration) == 0);
  rv += SDK_ASSERT(testOneField(fields, "eciReferenceTime", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "magneticVariance", simData::ReflectionDataType::Enumeration) == 0);
  rv += SDK_ASSERT(testOneField(fields, "magneticVarianceUserValue", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "referenceLla.lat", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "referenceLla.lon", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "referenceLla.alt", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "tangentPlaneOffset.angle", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "tangentPlaneOffset.tx", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "tangentPlaneOffset.ty", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "verticalDatumUserValue", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "verticalDatum", simData::ReflectionDataType::Enumeration) == 0);

  // Verify Coordinate System Enumeration
  simData::CoordinateFrameProperties prop;
  rv += SDK_ASSERT(!reflection->getValue(&prop, "coordinateSystem").has_value());
  prop.set_coordinatesystem(simData::CoordinateSystemProperties::LLA);
  rv += SDK_ASSERT(reflection->getValue(&prop, "coordinateSystem").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "coordinateSystem")->getInt32() == static_cast<int>(simData::CoordinateSystemProperties::LLA));
  rv += SDK_ASSERT(reflection->getValue(&prop, "coordinateSystem")->getEnumerationText() == "LLA");

  return rv;
}

int testCustomRenderingPropertyReflection()
{
  int rv = 0;

  simData::CustomRenderingProperties prop;
  auto crpr = simData::Reflection::makeCustomRenderingProperty();

  // Verify get/set for each field

  rv += SDK_ASSERT(!crpr->getValue(&prop, "id").has_value());
  rv += SDK_ASSERT(crpr->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(1)), "id") == 0);
  rv += SDK_ASSERT(crpr->getValue(&prop, "id").has_value());
  rv += SDK_ASSERT(crpr->getValue(&prop, "id").value().getUint64() == 1ul);

  rv += SDK_ASSERT(!crpr->getValue(&prop, "hostId").has_value());
  rv += SDK_ASSERT(crpr->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(2)), "hostId") == 0);
  rv += SDK_ASSERT(crpr->getValue(&prop, "hostId").has_value());
  rv += SDK_ASSERT(crpr->getValue(&prop, "hostId").value().getUint64() == 2ul);

  rv += SDK_ASSERT(!crpr->getValue(&prop, "originalId").has_value());
  rv += SDK_ASSERT(crpr->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(3)), "originalId") == 0);
  rv += SDK_ASSERT(crpr->getValue(&prop, "originalId").has_value());
  rv += SDK_ASSERT(crpr->getValue(&prop, "originalId").value().getUint64() == 3ul);

  rv += SDK_ASSERT(!crpr->getValue(&prop, "renderer").has_value());
  rv += SDK_ASSERT(crpr->setValue(&prop, simData::ReflectionValue("Test"), "renderer") == 0);
  rv += SDK_ASSERT(crpr->getValue(&prop, "renderer").has_value());
  rv += SDK_ASSERT(crpr->getValue(&prop, "renderer").value().getString() == "Test");

  // Verify reflection
  std::map<std::string, simData::ReflectionDataType> fields;
  crpr->reflection("", [&fields](const std::string& path, simData::ReflectionDataType type) {
    fields[path] = type;
    });

  rv += SDK_ASSERT(fields.size() == 5);
  rv += SDK_ASSERT(testOneField(fields, "id", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "hostId", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "originalId", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "renderer", simData::ReflectionDataType::String) == 0);
  rv += SDK_ASSERT(testOneField(fields, "source", simData::ReflectionDataType::String) == 0);

  return rv;
}

int testGatePropertyReflection()
{
  int rv = 0;

  simData::GateProperties prop;
  auto reflection = simData::Reflection::makeGateProperty();

  // Verify get/set for each field

  rv += SDK_ASSERT(!reflection->getValue(&prop, "id").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(1)), "id") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "id").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "id").value().getUint64() == 1ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "hostId").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(2)), "hostId") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "hostId").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "hostId").value().getUint64() == 2ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "originalId").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(3)), "originalId") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "originalId").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "originalId").value().getUint64() == 3ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "type").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<int32_t>(1)), "type") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "type").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "type").value().getInt32() == 1);

  // Verify reflection
  std::map<std::string, simData::ReflectionDataType> fields;
  reflection->reflection("", [&fields](const std::string& path, simData::ReflectionDataType type) {
    fields[path] = type;
    });

  rv += SDK_ASSERT(fields.size() == 5);
  rv += SDK_ASSERT(testOneField(fields, "id", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "hostId", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "originalId", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "type", simData::ReflectionDataType::Enumeration) == 0);
  rv += SDK_ASSERT(testOneField(fields, "source", simData::ReflectionDataType::String) == 0);

  return rv;
}

int testLaserPropertyReflection()
{
  int rv = 0;

  simData::LaserProperties prop;
  auto reflection = simData::Reflection::makeLaserProperty();

  // Verify get/set for each field

  rv += SDK_ASSERT(!reflection->getValue(&prop, "id").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(1)), "id") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "id").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "id").value().getUint64() == 1ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "hostId").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(2)), "hostId") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "hostId").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "hostId").value().getUint64() == 2ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "originalId").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(3)), "originalId") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "originalId").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "originalId").value().getUint64() == 3ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "coordinateSystem").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<int32_t>(3)), "coordinateSystem") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "coordinateSystem").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "coordinateSystem").value().getInt32() == 3);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "azElRelativeToHostOri").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(true), "azElRelativeToHostOri") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "azElRelativeToHostOri").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "azElRelativeToHostOri").value().getBoolean() == true);

  // Verify reflection
  std::map<std::string, simData::ReflectionDataType> fields;
  reflection->reflection("", [&fields](const std::string& path, simData::ReflectionDataType type) {
    fields[path] = type;
    });

  rv += SDK_ASSERT(fields.size() == 6);
  rv += SDK_ASSERT(testOneField(fields, "id", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "hostId", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "originalId", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateSystem", simData::ReflectionDataType::Enumeration) == 0);
  rv += SDK_ASSERT(testOneField(fields, "azElRelativeToHostOri", simData::ReflectionDataType::Boolean) == 0);
  rv += SDK_ASSERT(testOneField(fields, "source", simData::ReflectionDataType::String) == 0);

  return rv;
}

int testLobGroupPropertyReflection()
{
  int rv = 0;

  simData::LobGroupProperties prop;
  auto reflection = simData::Reflection::makeLobGroupProperty();

  // Verify get/set for each field

  rv += SDK_ASSERT(!reflection->getValue(&prop, "id").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(1)), "id") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "id").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "id").value().getUint64() == 1ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "hostId").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(2)), "hostId") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "hostId").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "hostId").value().getUint64() == 2ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "originalId").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(3)), "originalId") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "originalId").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "originalId").value().getUint64() == 3ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "coordinateSystem").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<int32_t>(3)), "coordinateSystem") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "coordinateSystem").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "coordinateSystem").value().getInt32() == 3);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "azElRelativeToHostOri").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(true), "azElRelativeToHostOri") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "azElRelativeToHostOri").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "azElRelativeToHostOri").value().getBoolean() == true);

  // Verify reflection
  std::map<std::string, simData::ReflectionDataType> fields;
  reflection->reflection("", [&fields](const std::string& path, simData::ReflectionDataType type) {
    fields[path] = type;
    });

  rv += SDK_ASSERT(fields.size() == 6);
  rv += SDK_ASSERT(testOneField(fields, "id", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "hostId", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "originalId", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateSystem", simData::ReflectionDataType::Enumeration) == 0);
  rv += SDK_ASSERT(testOneField(fields, "azElRelativeToHostOri", simData::ReflectionDataType::Boolean) == 0);
  rv += SDK_ASSERT(testOneField(fields, "source", simData::ReflectionDataType::String) == 0);

  return rv;
}

int testPlatformPropertyReflection()
{
  int rv = 0;

  simData::PlatformProperties prop;
  auto reflection = simData::Reflection::makePlatformProperty();

  // Verify get/set for each top level field and one at each level below

  rv += SDK_ASSERT(!reflection->getValue(&prop, "id").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(1)), "id") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "id").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "id").value().getUint64() == 1ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "originalId").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(3)), "originalId") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "originalId").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "originalId").value().getUint64() == 3ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "coordinateFrame.eciReferenceTime").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(1.0), "coordinateFrame.eciReferenceTime") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "coordinateFrame.eciReferenceTime").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "coordinateFrame.eciReferenceTime").value().getDouble() == 1.0);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "coordinateFrame.tangentPlaneOffset.angle").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(2.0), "coordinateFrame.tangentPlaneOffset.angle") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "coordinateFrame.tangentPlaneOffset.angle").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "coordinateFrame.tangentPlaneOffset.angle").value().getDouble() == 2.0);

  // Verify reflection
  std::map<std::string, simData::ReflectionDataType> fields;
  reflection->reflection("", [&fields](const std::string& path, simData::ReflectionDataType type) {
    fields[path] = type;
    });

  rv += SDK_ASSERT(fields.size() == 15);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.coordinateSystem", simData::ReflectionDataType::Enumeration) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.eciReferenceTime", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.magneticVariance", simData::ReflectionDataType::Enumeration) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.magneticVarianceUserValue", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.referenceLla.lat", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.referenceLla.lon", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.referenceLla.alt", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.tangentPlaneOffset.angle", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.tangentPlaneOffset.tx", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.tangentPlaneOffset.ty", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.verticalDatumUserValue", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.verticalDatum", simData::ReflectionDataType::Enumeration) == 0);
  rv += SDK_ASSERT(testOneField(fields, "id", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "originalId", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "source", simData::ReflectionDataType::String) == 0);

  return rv;
}

int testProjectorPropertyReflection()
{
  int rv = 0;

  simData::ProjectorProperties prop;
  auto reflection = simData::Reflection::makeProjectorProperty();

  // Verify get/set for each field

  rv += SDK_ASSERT(!reflection->getValue(&prop, "id").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(1)), "id") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "id").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "id").value().getUint64() == 1ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "hostId").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(2)), "hostId") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "hostId").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "hostId").value().getUint64() == 2ul);

  rv += SDK_ASSERT(!reflection->getValue(&prop, "originalId").has_value());
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(static_cast<uint64_t>(3)), "originalId") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "originalId").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "originalId").value().getUint64() == 3ul);

  // Verify reflection
  std::map<std::string, simData::ReflectionDataType> fields;
  reflection->reflection("", [&fields](const std::string& path, simData::ReflectionDataType type) {
    fields[path] = type;
    });

  rv += SDK_ASSERT(fields.size() == 4);
  rv += SDK_ASSERT(testOneField(fields, "id", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "hostId", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "originalId", simData::ReflectionDataType::Uint64) == 0);
  rv += SDK_ASSERT(testOneField(fields, "source", simData::ReflectionDataType::String) == 0);

  return rv;
}

int testReferencePropertyReflection()
{
  int rv = 0;

  auto reflection = simData::Reflection::makeReferenceProperty();

  // Verify reflection
  std::map<std::string, simData::ReflectionDataType> fields;
  reflection->reflection("", [&fields](const std::string& path, simData::ReflectionDataType type) {
    fields[path] = type;
    });

  rv += SDK_ASSERT(fields.size() == 3);
  rv += SDK_ASSERT(testOneField(fields, "lat", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "lon", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "alt", simData::ReflectionDataType::Double) == 0);

  return rv;
}

int testScenarioPropertyReflection()
{
  int rv = 0;

  // Validate that the std::vector<std::string> work
  simData::ScenarioProperties prop;
  auto reflection = simData::Reflection::makeScenarioProperty();

  // Vectors are different, they always have a value, but it might be an empty vector
  rv += SDK_ASSERT(reflection->getValue(&prop, "gogFile").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "gogFile")->getStrings().empty() == true);
  std::vector<std::string> files = { "Test", "Test2"};
  rv += SDK_ASSERT(reflection->setValue(&prop, simData::ReflectionValue(files), "gogFile") == 0);
  rv += SDK_ASSERT(reflection->getValue(&prop, "gogFile")->getStrings().empty() == false);
  rv += SDK_ASSERT(reflection->getValue(&prop, "gogFile")->getStrings().size() == 2);
  rv += SDK_ASSERT(reflection->getValue(&prop, "gogFile")->getStrings() == files);
  rv += SDK_ASSERT(prop.gogfile_size() == 2);
  rv += SDK_ASSERT(prop.gogfile() == files);

  // Verify reflection
  std::map<std::string, simData::ReflectionDataType> fields;
  reflection->reflection("", [&fields](const std::string& path, simData::ReflectionDataType type) {
    fields[path] = type;
    });

  rv += SDK_ASSERT(fields.size() == 34);
  rv += SDK_ASSERT(testOneField(fields, "classification.fontColor", simData::ReflectionDataType::Uint32) == 0);
  rv += SDK_ASSERT(testOneField(fields, "classification.label", simData::ReflectionDataType::String) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.coordinateSystem", simData::ReflectionDataType::Enumeration) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.eciReferenceTime", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.magneticVariance", simData::ReflectionDataType::Enumeration) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.magneticVarianceUserValue", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.referenceLla.alt", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.referenceLla.lat", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.referenceLla.lon", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.tangentPlaneOffset.angle", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.tangentPlaneOffset.tx", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.tangentPlaneOffset.ty", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.verticalDatum", simData::ReflectionDataType::Enumeration) == 0);
  rv += SDK_ASSERT(testOneField(fields, "coordinateFrame.verticalDatumUserValue", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "dataLimitPoints", simData::ReflectionDataType::Uint32) == 0);
  rv += SDK_ASSERT(testOneField(fields, "dataLimitTime", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "dedFile", simData::ReflectionDataType::StringVector) == 0);
  rv += SDK_ASSERT(testOneField(fields, "degreeAngles", simData::ReflectionDataType::Boolean) == 0);
  rv += SDK_ASSERT(testOneField(fields, "description", simData::ReflectionDataType::String) == 0);
  rv += SDK_ASSERT(testOneField(fields, "gogFile", simData::ReflectionDataType::StringVector) == 0);
  rv += SDK_ASSERT(testOneField(fields, "ignoreDuplicateGenericData", simData::ReflectionDataType::Boolean) == 0);
  rv += SDK_ASSERT(testOneField(fields, "mediaFile", simData::ReflectionDataType::StringVector) == 0);
  rv += SDK_ASSERT(testOneField(fields, "referenceYear", simData::ReflectionDataType::Uint32) == 0);
  rv += SDK_ASSERT(testOneField(fields, "ruleFile", simData::ReflectionDataType::String) == 0);
  rv += SDK_ASSERT(testOneField(fields, "soundFile.endTime", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "soundFile.filename", simData::ReflectionDataType::String) == 0);
  rv += SDK_ASSERT(testOneField(fields, "soundFile.startTime", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "source", simData::ReflectionDataType::String) == 0);
  rv += SDK_ASSERT(testOneField(fields, "terrainFile", simData::ReflectionDataType::String) == 0);
  rv += SDK_ASSERT(testOneField(fields, "version", simData::ReflectionDataType::Uint32) == 0);
  rv += SDK_ASSERT(testOneField(fields, "viewFile", simData::ReflectionDataType::String) == 0);
  rv += SDK_ASSERT(testOneField(fields, "windAngle", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "windSpeed", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "wvsFile", simData::ReflectionDataType::StringVector) == 0);

  return rv;
}

int testSoundFilePropertyReflection()
{
  int rv = 0;

  auto reflection = simData::Reflection::makeSoundFileProperty();

  // Verify reflection
  std::map<std::string, simData::ReflectionDataType> fields;
  reflection->reflection("", [&fields](const std::string& path, simData::ReflectionDataType type) {
    fields[path] = type;
    });

  rv += SDK_ASSERT(fields.size() == 3);
  rv += SDK_ASSERT(testOneField(fields, "filename", simData::ReflectionDataType::String) == 0);
  rv += SDK_ASSERT(testOneField(fields, "startTime", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "endTime", simData::ReflectionDataType::Double) == 0);

  return rv;
}

int testTangentPlaneOffsetsPropertyReflection()
{
  int rv = 0;

  auto reflection = simData::Reflection::makeTangentPlaneOffsetsProperty();

  // Verify reflection
  std::map<std::string, simData::ReflectionDataType> fields;
  reflection->reflection("", [&fields](const std::string& path, simData::ReflectionDataType type) {
    fields[path] = type;
    });

  rv += SDK_ASSERT(fields.size() == 3);
  rv += SDK_ASSERT(testOneField(fields, "tx", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "ty", simData::ReflectionDataType::Double) == 0);
  rv += SDK_ASSERT(testOneField(fields, "angle", simData::ReflectionDataType::Double) == 0);

  return rv;
}

}

int TestReflection(int argc, char* argv[])
{
  int rv = 0;

  rv += testReflectionValue();

  rv += testBeamPropertyReflection();
  rv += testClassificationPropertyReflection();
  rv += testCoordinateFramePropertyReflection();
  rv += testCustomRenderingPropertyReflection();
  rv += testGatePropertyReflection();
  rv += testLaserPropertyReflection();
  rv += testLobGroupPropertyReflection();
  rv += testPlatformPropertyReflection();
  rv += testProjectorPropertyReflection();
  rv += testReferencePropertyReflection();
  rv += testScenarioPropertyReflection();
  rv += testSoundFilePropertyReflection();
  rv += testTangentPlaneOffsetsPropertyReflection();

  return rv;
}
