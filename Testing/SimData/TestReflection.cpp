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

#include <limits>
#include "simCore/Common/SDKAssert.h"
#include "simData/DataTypeProperties.h"
#include "simData/DataTypeReflection.h"
#include "simData/EntityCommands.h"
#include "simData/EnumerationText.h"

namespace {

int testReflectionValue()
{
  int rv = 0;

  // Verify Boolean
  simData::ReflectionValue valueBoolean(true);
  rv += SDK_ASSERT(valueBoolean.type() == simData::ReflectionDataType::Boolean);
  rv += SDK_ASSERT(valueBoolean.getBoolean() == true);
  valueBoolean.setBoolean(false);
  rv += SDK_ASSERT(valueBoolean.type() == simData::ReflectionDataType::Boolean);
  rv += SDK_ASSERT(valueBoolean.getBoolean() == false);
  rv += SDK_ASSERT(valueBoolean != simData::ReflectionValue(true));
  rv += SDK_ASSERT(valueBoolean == simData::ReflectionValue(false));
  rv += SDK_ASSERT(valueBoolean != simData::ReflectionValue("ShouldNotMatch"));
  // Test copy assignment, which also tests the copy constructor
  simData::ReflectionValue copyValueBoolean = valueBoolean;
  rv += SDK_ASSERT(valueBoolean == copyValueBoolean);
  rv += SDK_ASSERT(valueBoolean == simData::ReflectionValue(false));
  rv += SDK_ASSERT(copyValueBoolean == simData::ReflectionValue(false));

  // Verify int32_t
  simData::ReflectionValue valueInt32(static_cast<int32_t>(1));
  rv += SDK_ASSERT(valueInt32.type() == simData::ReflectionDataType::Int32);
  rv += SDK_ASSERT(valueInt32.getInt32() == 1);
  valueInt32.setInt32(2);
  rv += SDK_ASSERT(valueInt32.type() == simData::ReflectionDataType::Int32);
  rv += SDK_ASSERT(valueInt32.getInt32() == 2);
  rv += SDK_ASSERT(valueInt32 != simData::ReflectionValue(static_cast<int32_t>(1)));
  rv += SDK_ASSERT(valueInt32 == simData::ReflectionValue(static_cast<int32_t>(2)));
  rv += SDK_ASSERT(valueInt32 != simData::ReflectionValue("ShouldNotMatch"));
  // Test copy assignment, which also tests the copy constructor
  simData::ReflectionValue copyValueInt32 = valueInt32;
  rv += SDK_ASSERT(valueInt32 == copyValueInt32);
  rv += SDK_ASSERT(valueInt32 == simData::ReflectionValue(static_cast<int32_t>(2)));
  rv += SDK_ASSERT(copyValueInt32 == simData::ReflectionValue(static_cast<int32_t>(2)));

  // Verify uint32_t
  simData::ReflectionValue valueUint32(static_cast<uint32_t>(1));
  rv += SDK_ASSERT(valueUint32.type() == simData::ReflectionDataType::Uint32);
  rv += SDK_ASSERT(valueUint32.getUint32() == 1);
  valueUint32.setUint32(2);
  rv += SDK_ASSERT(valueUint32.type() == simData::ReflectionDataType::Uint32);
  rv += SDK_ASSERT(valueUint32.getUint32() == 2);
  rv += SDK_ASSERT(valueUint32 != simData::ReflectionValue(static_cast<uint32_t>(1)));
  rv += SDK_ASSERT(valueUint32 == simData::ReflectionValue(static_cast<uint32_t>(2)));
  rv += SDK_ASSERT(valueUint32 != simData::ReflectionValue("ShouldNotMatch"));
  // Test copy assignment, which also tests the copy constructor
  simData::ReflectionValue copyValueUint32 = valueUint32;
  rv += SDK_ASSERT(valueUint32 == copyValueUint32);
  rv += SDK_ASSERT(valueUint32 == simData::ReflectionValue(static_cast<uint32_t>(2)));
  rv += SDK_ASSERT(copyValueUint32 == simData::ReflectionValue(static_cast<uint32_t>(2)));

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
  // Test copy assignment, which also tests the copy constructor
  simData::ReflectionValue copyValueUint64 = valueUint64;
  rv += SDK_ASSERT(valueUint64 == copyValueUint64);
  rv += SDK_ASSERT(valueUint64 == simData::ReflectionValue(static_cast<uint64_t>(2)));
  rv += SDK_ASSERT(copyValueUint64 == simData::ReflectionValue(static_cast<uint64_t>(2)));

  // Verify double
  simData::ReflectionValue valueDouble(1.0);
  rv += SDK_ASSERT(valueDouble.type() == simData::ReflectionDataType::Double);
  rv += SDK_ASSERT(valueDouble.getDouble() == 1.0);
  valueDouble.setDouble(2.0);
  rv += SDK_ASSERT(valueDouble.type() == simData::ReflectionDataType::Double);
  rv += SDK_ASSERT(valueDouble.getDouble() == 2.0);
  rv += SDK_ASSERT(valueDouble != simData::ReflectionValue(1.0));
  rv += SDK_ASSERT(valueDouble == simData::ReflectionValue(2.0));
  rv += SDK_ASSERT(valueDouble != simData::ReflectionValue("ShouldNotMatch"));
  // Test copy assignment, which also tests the copy constructor
  simData::ReflectionValue copyValueDouble = valueDouble;
  rv += SDK_ASSERT(valueDouble == copyValueDouble);
  rv += SDK_ASSERT(valueDouble == simData::ReflectionValue(2.0));
  rv += SDK_ASSERT(copyValueDouble == simData::ReflectionValue(2.0));

  // Verify float
  simData::ReflectionValue valueFloat(1.0f);
  rv += SDK_ASSERT(valueFloat.type() == simData::ReflectionDataType::Float);
  rv += SDK_ASSERT(valueFloat.getFloat() == 1.0f);
  valueFloat.setFloat(2.0f);
  rv += SDK_ASSERT(valueFloat.type() == simData::ReflectionDataType::Float);
  rv += SDK_ASSERT(valueFloat.getFloat() == 2.0f);
  rv += SDK_ASSERT(valueFloat != simData::ReflectionValue(1.0f));
  rv += SDK_ASSERT(valueFloat == simData::ReflectionValue(2.0f));
  rv += SDK_ASSERT(valueFloat != simData::ReflectionValue("ShouldNotMatch"));
  // Test copy assignment, which also tests the copy constructor
  simData::ReflectionValue copyValueFloat = valueFloat;
  rv += SDK_ASSERT(valueFloat == copyValueFloat);
  rv += SDK_ASSERT(valueFloat == simData::ReflectionValue(2.0f));
  rv += SDK_ASSERT(copyValueFloat == simData::ReflectionValue(2.0f));

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
  // Test copy assignment, which also tests the copy constructor
  simData::ReflectionValue copyValueString = valueString;
  rv += SDK_ASSERT(valueString == copyValueString);
  rv += SDK_ASSERT(valueString == simData::ReflectionValue("Test2"));
  rv += SDK_ASSERT(copyValueString == simData::ReflectionValue("Test2"));

  {
    // Verify std::vector of std::string
    std::vector<std::string> strings = { "Test", "Test2" };
    simData::ReflectionValue valueStringVector(strings);
    rv += SDK_ASSERT(valueStringVector.type() == simData::ReflectionDataType::StringVector);
    rv += SDK_ASSERT(valueStringVector.getStrings() == strings);
    std::vector<std::string> strings2 = { "Test3", "Test4" };
    valueStringVector.setStrings(strings2);
    rv += SDK_ASSERT(valueStringVector.type() == simData::ReflectionDataType::StringVector);
    rv += SDK_ASSERT(valueStringVector.getStrings() == strings2);
    rv += SDK_ASSERT(valueStringVector != simData::ReflectionValue(strings));
    rv += SDK_ASSERT(valueStringVector == simData::ReflectionValue(strings2));
    rv += SDK_ASSERT(valueStringVector != simData::ReflectionValue(static_cast<uint64_t>(1)));
    // Test copy assignment, which also tests the copy constructor
    simData::ReflectionValue copyValueStringVector = valueStringVector;
    rv += SDK_ASSERT(valueStringVector == copyValueStringVector);
    rv += SDK_ASSERT(valueStringVector == simData::ReflectionValue(strings2));
    rv += SDK_ASSERT(copyValueStringVector == simData::ReflectionValue(strings2));
  }

  // Verify std::vector of uint64_t
  std::vector<uint64_t> ids = { 1, 2 };
  simData::ReflectionValue valueIdVector(ids);
  rv += SDK_ASSERT(valueIdVector.type() == simData::ReflectionDataType::IdVector);
  rv += SDK_ASSERT(valueIdVector.getIds() == ids);
  std::vector<uint64_t> ids2 = { 3, 4 };
  valueIdVector.setIds(ids2);
  rv += SDK_ASSERT(valueIdVector.type() == simData::ReflectionDataType::IdVector);
  rv += SDK_ASSERT(valueIdVector.getIds() == ids2);
  rv += SDK_ASSERT(valueIdVector != simData::ReflectionValue(ids));
  rv += SDK_ASSERT(valueIdVector == simData::ReflectionValue(ids2));
  rv += SDK_ASSERT(valueIdVector != simData::ReflectionValue(static_cast<uint64_t>(1)));
  // Test copy assignment, which also tests the copy constructor
  simData::ReflectionValue copyValueIdVector = valueIdVector;
  rv += SDK_ASSERT(valueIdVector == copyValueIdVector);
  rv += SDK_ASSERT(valueIdVector == simData::ReflectionValue(ids2));
  rv += SDK_ASSERT(copyValueIdVector == simData::ReflectionValue(ids2));

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
  prop.set_coordinatesystem(simData::CoordinateSystem::LLA);
  rv += SDK_ASSERT(reflection->getValue(&prop, "coordinateSystem").has_value());
  rv += SDK_ASSERT(reflection->getValue(&prop, "coordinateSystem")->getInt32() == static_cast<int>(simData::CoordinateSystem::LLA));
  rv += SDK_ASSERT(reflection->getValue(&prop, "coordinateSystem")->getEnumeration() == "LLA");

  std::map<size_t, std::string> actualValues;
  reflection->getValue(&prop, "coordinateSystem")->getEnumerationText()->visit([&actualValues](size_t index, const std::string& title) { actualValues[index] = title; });

  rv += SDK_ASSERT(actualValues.find(1)->second == "NED");
  rv += SDK_ASSERT(actualValues.find(2)->second == "NWU");
  rv += SDK_ASSERT(actualValues.find(3)->second == "ENU");
  rv += SDK_ASSERT(actualValues.find(4)->second == "LLA");
  rv += SDK_ASSERT(actualValues.find(5)->second == "ECEF");
  rv += SDK_ASSERT(actualValues.find(6)->second == "ECI");
  rv += SDK_ASSERT(actualValues.find(7)->second == "XEAST");
  rv += SDK_ASSERT(actualValues.find(8)->second == "GTP");

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

  rv += SDK_ASSERT(!reflection->getValue(&prop, "gogFile").has_value());
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

int testDefaults()
{
  int rv = 0;

  auto reflection = simData::Reflection::makeScenarioProperty();
  simData::ScenarioProperties properties;

  // The 3 sets of tests cover all 4 reflection macros

  // Test top level
  rv += SDK_ASSERT(reflection->getDefaultValue(&properties, "dataLimitPoints").has_value());
  rv += SDK_ASSERT(reflection->getDefaultValue(&properties, "dataLimitPoints")->type() == simData::ReflectionDataType::Uint32);
  rv += SDK_ASSERT(reflection->getDefaultValue(&properties, "dataLimitPoints")->getUint32() == 1000);

  // Test one level down
  rv += SDK_ASSERT(reflection->getDefaultValue(&properties, "coordinateFrame.coordinateSystem").has_value());
  rv += SDK_ASSERT(reflection->getDefaultValue(&properties, "coordinateFrame.coordinateSystem")->type() == simData::ReflectionDataType::Enumeration);
  rv += SDK_ASSERT(reflection->getDefaultValue(&properties, "coordinateFrame.coordinateSystem")->getInt32() == 1);
  rv += SDK_ASSERT(reflection->getDefaultValue(&properties, "coordinateFrame.coordinateSystem")->getEnumeration() == "NED");

  // Test vector
  rv += SDK_ASSERT(reflection->getDefaultValue(&properties, "gogFile").has_value());
  rv += SDK_ASSERT(reflection->getDefaultValue(&properties, "gogFile")->type() == simData::ReflectionDataType::StringVector);
  rv += SDK_ASSERT(reflection->getDefaultValue(&properties, "gogFile")->getStrings().empty());

  return rv;
}

int testClear()
{
  int rv = 0;

  auto reflection = simData::Reflection::makeScenarioProperty();
  simData::ScenarioProperties properties;

  // Test top level string
  properties.set_description("Test");
  rv += SDK_ASSERT(reflection->getValue(&properties, "description").has_value());
  rv += SDK_ASSERT(reflection->getValue(&properties, "description")->getString() == "Test");
  rv += SDK_ASSERT(reflection->clearValue(&properties, "description") == 0);
  rv += SDK_ASSERT(!reflection->getValue(&properties, "description").has_value());

  // Test top level unsigned integer
  properties.set_datalimitpoints(3);
  rv += SDK_ASSERT(reflection->getValue(&properties, "dataLimitPoints").has_value());
  rv += SDK_ASSERT(reflection->getValue(&properties, "dataLimitPoints")->getUint32() == 3);
  rv += SDK_ASSERT(reflection->clearValue(&properties, "dataLimitPoints") == 0);
  rv += SDK_ASSERT(!reflection->getValue(&properties, "dataLimitPoints").has_value());

  // Test top level vector of strings
  properties.add_gogfile("Test1");
  properties.add_gogfile("Test2");
  rv += SDK_ASSERT(reflection->getValue(&properties, "gogFile").has_value());
  const std::vector<std::string> expected = { "Test1", "Test2" };
  rv += SDK_ASSERT(reflection->getValue(&properties, "gogFile")->getStrings() == expected);
  rv += SDK_ASSERT(reflection->clearValue(&properties, "gogFile") == 0);
  rv += SDK_ASSERT(!reflection->getValue(&properties, "gogFile").has_value());

  // Test one level down
  properties.mutable_coordinateframe()->set_coordinatesystem(simData::CoordinateSystem::LLA);
  rv += SDK_ASSERT(reflection->getValue(&properties, "coordinateFrame.coordinateSystem").has_value());
  rv += SDK_ASSERT(reflection->getValue(&properties, "coordinateFrame.coordinateSystem")->type() == simData::ReflectionDataType::Enumeration);
  rv += SDK_ASSERT(reflection->getValue(&properties, "coordinateFrame.coordinateSystem")->getInt32() == 4);
  rv += SDK_ASSERT(reflection->getValue(&properties, "coordinateFrame.coordinateSystem")->getEnumeration() == "LLA");
  rv += SDK_ASSERT(reflection->clearValue(&properties, "coordinateFrame.coordinateSystem") == 0);
  rv += SDK_ASSERT(!reflection->getValue(&properties, "coordinateFrame.coordinateSystem").has_value());

  return rv;
}

int testPruneAndIsEmpty()
{
  int rv = 0;

  auto reflection = simData::Reflection::makePlatformCommands();
  simData::PlatformCommand command;

  // Should start off empty
  rv += SDK_ASSERT(command.isEmpty());

  // Set time, should not be empty
  command.set_time(1.0);
  rv += SDK_ASSERT(!command.isEmpty());

  // Clear time, should be empty
  command.clear_time();
  rv += SDK_ASSERT(command.isEmpty());

  // Set clamping, should not be empty
  command.mutable_updateprefs()->set_abovesurfaceclamping(true);
  rv += SDK_ASSERT(!command.isEmpty());

  // Clear clamping, should be empty after pruning
  command.mutable_updateprefs()->clear_abovesurfaceclamping();
  command.Prune();
  rv += SDK_ASSERT(command.isEmpty());

  // Set alias, should not be empty
  command.mutable_updateprefs()->mutable_commonprefs()->set_alias("Test");
  rv += SDK_ASSERT(!command.isEmpty());

  // Clear alias, should be empty
  command.mutable_updateprefs()->mutable_commonprefs()->clear_alias();
  command.Prune();
  rv += SDK_ASSERT(command.isEmpty());

  return rv;
}

int  testTagStack()
{
  int rv = 0;

  auto reflection = simData::Reflection::makePlatformPreferences();
  simData::PlatformPrefs preferences;

  auto tagStackMap = simData::Reflection::makeTagStackMap(*reflection);

  auto it = tagStackMap.find("commonPrefs.draw");
  rv += SDK_ASSERT(it != tagStackMap.end());
  if (it == tagStackMap.end())
    return rv;

  // Test getValue for Boolean

  // Defaults to true
  auto value = reflection->getValue(&preferences, it->second);
  rv += SDK_ASSERT(value.has_value());
  rv += SDK_ASSERT(value->getBoolean());

  // Set to false
  preferences.mutable_commonprefs()->set_draw(false);
  value = reflection->getValue(&preferences, it->second);
  rv += SDK_ASSERT(value.has_value());
  rv += SDK_ASSERT(!value->getBoolean());

  // Set to true
  preferences.mutable_commonprefs()->set_draw(true);
  value = reflection->getValue(&preferences, it->second);
  rv += SDK_ASSERT(value.has_value());
  rv += SDK_ASSERT(value->getBoolean());

  // Test setValue for Boolean

  // Set to false
  value->setBoolean(false);
  rv += SDK_ASSERT(reflection->setValue(&preferences, *value, it->second) == 0);
  rv += SDK_ASSERT(preferences.commonprefs().has_draw());
  rv += SDK_ASSERT(!preferences.commonprefs().draw());

  // Set to true
  value->setBoolean(true);
  rv += SDK_ASSERT(reflection->setValue(&preferences, *value, it->second) == 0);
  rv += SDK_ASSERT(preferences.commonprefs().has_draw());
  rv += SDK_ASSERT(preferences.commonprefs().draw());

  // Test vector of strings

  it = tagStackMap.find("gogFile");
  rv += SDK_ASSERT(it != tagStackMap.end());
  if (it == tagStackMap.end())
    return rv;

  std::vector<std::string> files = { "Test1", "Test2" };

  // Defaults to empty
  value = reflection->getValue(&preferences, it->second);
  rv += SDK_ASSERT(value.has_value());
  rv += SDK_ASSERT(value->getStrings().empty());

  // Test getValue
  *preferences.mutable_gogfile() = files;
  value = reflection->getValue(&preferences, it->second);
  rv += SDK_ASSERT(value.has_value());
  rv += SDK_ASSERT(value->getStrings() == files);

  // Clear before the next test
  preferences.clear_gogfile();
  rv += SDK_ASSERT(preferences.gogfile_size() == 0);

  // Test setValue
  rv += SDK_ASSERT(reflection->setValue(&preferences, *value, it->second) == 0);
  rv += SDK_ASSERT(preferences.gogfile_size() == 2);
  rv += SDK_ASSERT(preferences.gogfile() == files);

  // Test enumeration

  it = tagStackMap.find("cullFace");
  rv += SDK_ASSERT(it != tagStackMap.end());
  if (it == tagStackMap.end())
    return rv;

  // Defaults to FRONT_AND_BACK
  value = reflection->getValue(&preferences, it->second);
  rv += SDK_ASSERT(value.has_value());
  rv += SDK_ASSERT(value->getInt32() == static_cast<int>(simData::PolygonFace::FRONT_AND_BACK));

  preferences.set_cullface(simData::PolygonFace::FRONT);
  value = reflection->getValue(&preferences, it->second);
  rv += SDK_ASSERT(value.has_value());
  rv += SDK_ASSERT(value->getInt32() == static_cast<int>(simData::PolygonFace::FRONT));

  value->setInt32(static_cast<int>(simData::PolygonFace::BACK));
  rv += SDK_ASSERT(reflection->setValue(&preferences, *value, it->second) == 0);
  rv += SDK_ASSERT(preferences.cullface() == simData::PolygonFace::BACK);

  return rv;
}

int testGetFieldList()
{
  int rv = 0;

  auto reflection = simData::Reflection::makePlatformPreferences();
  simData::PlatformPrefs preferences;

  // Should return itself
  auto reflectedField = reflection->getMutableFieldList(&preferences, "");
  rv += SDK_ASSERT(reflectedField == &preferences);

  // Go down one level
  reflectedField = reflection->getMutableFieldList(&preferences, "commonPrefs");
  rv += SDK_ASSERT(reflectedField == preferences.mutable_commonprefs());

  // Go down two levels
  reflectedField = reflection->getMutableFieldList(&preferences, "commonPrefs.labelPrefs");
  rv += SDK_ASSERT(reflectedField == preferences.mutable_commonprefs()->mutable_labelprefs());

  // Go down three levels
  reflectedField = reflection->getMutableFieldList(&preferences, "commonPrefs.labelPrefs.displayFields");
  rv += SDK_ASSERT(reflectedField == preferences.mutable_commonprefs()->mutable_labelprefs()->mutable_displayfields());

  // Should handle invalid path
  reflectedField = reflection->getMutableFieldList(&preferences, "ShouldNotExist");
  rv += SDK_ASSERT(reflectedField == nullptr);

  return rv;
}

int testHelperMethods()
{
  int rv = 0;

  // This one test covers all 3 helper methods

  auto tags = simData::Reflection::getPreferencesTagStack("commonPrefs.draw", simData::PLATFORM);
  simData::Reflection::TagStack expected = { 0, 1 };
  rv += SDK_ASSERT(tags == expected);

  return rv;
}

int testCommonPrefs()
{
  int rv = 0;

  // Verify that all the entity types have the same tags for commonPrefs

  auto expected = simData::Reflection::getPreferencesTagStack("commonPrefs", simData::PLATFORM);
  // Should be one tag
  rv += SDK_ASSERT(expected.size() == 1);

  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs", simData::BEAM));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs", simData::GATE));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs", simData::LASER));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs", simData::LOB_GROUP));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs", simData::PROJECTOR));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs", simData::CUSTOM_RENDERING));

  // Test field
  expected = simData::Reflection::getPreferencesTagStack("commonPrefs.draw", simData::PLATFORM);
  // Should be two tags
  rv += SDK_ASSERT(expected.size() == 2);

  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.draw", simData::BEAM));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.draw", simData::GATE));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.draw", simData::LASER));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.draw", simData::LOB_GROUP));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.draw", simData::PROJECTOR));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.draw", simData::CUSTOM_RENDERING));

  // Test field list
  expected = simData::Reflection::getPreferencesTagStack("commonPrefs.labelPrefs", simData::PLATFORM);
  // Should be two tags
  rv += SDK_ASSERT(expected.size() == 2);

  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.labelPrefs", simData::BEAM));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.labelPrefs", simData::GATE));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.labelPrefs", simData::LASER));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.labelPrefs", simData::LOB_GROUP));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.labelPrefs", simData::PROJECTOR));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.labelPrefs", simData::CUSTOM_RENDERING));

  // Test field list another level down
  expected = simData::Reflection::getPreferencesTagStack("commonPrefs.labelPrefs.displayFields", simData::PLATFORM);
  // Should be two tags
  rv += SDK_ASSERT(expected.size() == 3);

  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.labelPrefs.displayFields", simData::BEAM));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.labelPrefs.displayFields", simData::GATE));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.labelPrefs.displayFields", simData::LASER));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.labelPrefs.displayFields", simData::LOB_GROUP));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.labelPrefs.displayFields", simData::PROJECTOR));
  rv += SDK_ASSERT(expected == simData::Reflection::getPreferencesTagStack("commonPrefs.labelPrefs.displayFields", simData::CUSTOM_RENDERING));

  return rv;
}

int testEnumerationText()
{
  int rv = 0;

  auto enumeration = simData::EnumerationText::makeDistanceUnitsName();

  // Meter is hard-code to 20 and is the first index
  rv += SDK_ASSERT(enumeration->valueToIndex(20) == 0);
  rv += SDK_ASSERT(enumeration->indexToValue(0) == 20);
  rv += SDK_ASSERT(enumeration->text(20) == "UNITS_METERS");

  // kps is hard-coded to 46 and is index 5
  enumeration = simData::EnumerationText::makeSpeedUnitsName();
  rv += SDK_ASSERT(enumeration->valueToIndex(46) == 5);
  rv += SDK_ASSERT(enumeration->indexToValue(5) == 46);
  rv += SDK_ASSERT(enumeration->text(46) == "UNITS_KILOMETERS_PER_SECOND");

  // Test failure
  rv += SDK_ASSERT(enumeration->valueToIndex(9999) == std::numeric_limits<size_t>::max());
  rv += SDK_ASSERT(enumeration->indexToValue(9999) == std::numeric_limits<size_t>::max());

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
  rv += testDefaults();
  rv += testClear();
  rv += testPruneAndIsEmpty();
  rv += testTagStack();
  rv += testGetFieldList();
  rv += testHelperMethods();
  rv += testCommonPrefs();
  rv += testEnumerationText();

  return rv;
}
