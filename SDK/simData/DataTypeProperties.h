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
#ifndef SIMDATA_DATATYPE_PROPERTIES_H
#define SIMDATA_DATATYPE_PROPERTIES_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "simCore/Common/Export.h"
#include "DataTypeBasics.h"

namespace simData
{

/** Beam Properties */
class SDKDATA_EXPORT BeamProperties : public FieldList
{
public:
  /** Types of beams */
  enum class Type {
    ABSOLUTE_POSITION = 1,  // Start with 1 to match protobuf
    BODY_RELATIVE,
    TARGET
  };

  SIMDATA_DECLARE_DEFAULT_METHODS(BeamProperties);
  SIMDATA_DECLARE_FIELD(id_, id, uint64_t);
  SIMDATA_DECLARE_FIELD(hostId_, hostid, uint64_t);
  SIMDATA_DECLARE_FIELD(originalId_, originalid, uint64_t);
  SIMDATA_DECLARE_FIELD(type_, type, simData::BeamProperties::Type);
  SIMDATA_DECLARE_FIELD_CONST_REF(source_, source, std::string);
};

/** Custom Rendering Properties */
class SDKDATA_EXPORT CustomRenderingProperties : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(CustomRenderingProperties);
  SIMDATA_DECLARE_FIELD(id_, id, uint64_t);
  SIMDATA_DECLARE_FIELD(hostId_, hostid, uint64_t);
  SIMDATA_DECLARE_FIELD(originalId_, originalid, uint64_t);
  SIMDATA_DECLARE_FIELD_CONST_REF(renderer_, renderer, std::string);
  SIMDATA_DECLARE_FIELD_CONST_REF(source_, source, std::string);
};

/** Gate Properties */
class SDKDATA_EXPORT GateProperties : public FieldList
{
public:
  /** Types of gates */
  enum class Type {
    ABSOLUTE_POSITION = 1,  // Start with 1 to match protobuf
    BODY_RELATIVE,
    TARGET
  };

  SIMDATA_DECLARE_DEFAULT_METHODS(GateProperties);
  SIMDATA_DECLARE_FIELD(id_, id, uint64_t);
  SIMDATA_DECLARE_FIELD(hostId_, hostid, uint64_t);
  SIMDATA_DECLARE_FIELD(originalId_, originalid, uint64_t);
  SIMDATA_DECLARE_FIELD(type_, type, simData::GateProperties::Type);
  SIMDATA_DECLARE_FIELD_CONST_REF(source_, source, std::string);
};

/** Laser Properties */
class SDKDATA_EXPORT LaserProperties : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(LaserProperties);
  SIMDATA_DECLARE_FIELD(id_, id, uint64_t);
  SIMDATA_DECLARE_FIELD(hostId_, hostid, uint64_t);
  SIMDATA_DECLARE_FIELD(originalId_, originalid, uint64_t);
  SIMDATA_DECLARE_FIELD(coordinateSystem_, coordinatesystem, simData::CoordinateSystem);
  SIMDATA_DECLARE_FIELD(azElRelativeToHostOri_, azelrelativetohostori, bool);
  SIMDATA_DECLARE_FIELD_CONST_REF(source_, source, std::string);
};

/** LobGroup Properties */
class SDKDATA_EXPORT LobGroupProperties : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(LobGroupProperties);
  SIMDATA_DECLARE_FIELD(id_, id, uint64_t);
  SIMDATA_DECLARE_FIELD(hostId_, hostid, uint64_t);
  SIMDATA_DECLARE_FIELD(originalId_, originalid, uint64_t);
  SIMDATA_DECLARE_FIELD(coordinateSystem_, coordinatesystem, simData::CoordinateSystem);
  SIMDATA_DECLARE_FIELD(azElRelativeToHostOri_, azelrelativetohostori, bool);
  SIMDATA_DECLARE_FIELD_CONST_REF(source_, source, std::string);
};

/** Projector Properties */
class SDKDATA_EXPORT ProjectorProperties : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(ProjectorProperties);
  SIMDATA_DECLARE_FIELD(id_, id, uint64_t);
  SIMDATA_DECLARE_FIELD(hostId_, hostid, uint64_t);
  SIMDATA_DECLARE_FIELD(originalId_, originalid, uint64_t);
  SIMDATA_DECLARE_FIELD_CONST_REF(source_, source, std::string);
};

/** Reference Properties */
class SDKDATA_EXPORT ReferenceProperties : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(ReferenceProperties);
  SIMDATA_DECLARE_FIELD(lat_, lat, double);
  SIMDATA_DECLARE_FIELD(lon_, lon, double);
  SIMDATA_DECLARE_FIELD(alt_, alt, double);
};

/** Tangent Plane Offsets Properties */
class SDKDATA_EXPORT TangentPlaneOffsetsProperties : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(TangentPlaneOffsetsProperties);
  SIMDATA_DECLARE_FIELD(tx_, tx, double);
  SIMDATA_DECLARE_FIELD(ty_, ty, double);
  SIMDATA_DECLARE_FIELD(angle_, angle, double);
};

/** Coordinate Frame Properties */
class SDKDATA_EXPORT CoordinateFrameProperties : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(CoordinateFrameProperties);
  SIMDATA_DECLARE_SUBFIELD_LIST(referenceLla_, referencella, simData::ReferenceProperties);
  SIMDATA_DECLARE_SUBFIELD_LIST(tangentPlaneOffset_, tangentplaneoffset, simData::TangentPlaneOffsetsProperties);
  SIMDATA_DECLARE_FIELD(coordinateSystem_, coordinatesystem, simData::CoordinateSystem);
  SIMDATA_DECLARE_FIELD(magneticVariance_, magneticvariance, simData::MagneticVariance);
  SIMDATA_DECLARE_FIELD(verticalDatum_, verticaldatum, simData::VerticalDatum);
  SIMDATA_DECLARE_FIELD(magneticVarianceUserValue_, magneticvarianceuservalue, double);
  SIMDATA_DECLARE_FIELD(verticalDatumUserValue_, verticaldatumuservalue, double);
  SIMDATA_DECLARE_FIELD(eciReferenceTime_, ecireferencetime, double);
};

/** Platform Properties */
class SDKDATA_EXPORT PlatformProperties : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(PlatformProperties);
  SIMDATA_DECLARE_SUBFIELD_LIST(coordinateFrame_, coordinateframe, simData::CoordinateFrameProperties);
  SIMDATA_DECLARE_FIELD(id_, id, uint64_t);
  SIMDATA_DECLARE_FIELD(originalId_, originalid, uint64_t);
  SIMDATA_DECLARE_FIELD_CONST_REF(source_, source, std::string);
};

/** Classification Properties */
class SDKDATA_EXPORT ClassificationProperties : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(ClassificationProperties);
  SIMDATA_DECLARE_FIELD_CONST_REF(label_, label, std::string);
  SIMDATA_DECLARE_FIELD(fontColor_, fontcolor, uint32_t);
};

/** Sound File Properties */
class SDKDATA_EXPORT SoundFileProperties : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(SoundFileProperties);
  SIMDATA_DECLARE_FIELD_CONST_REF(filename_, filename, std::string);
  SIMDATA_DECLARE_FIELD(startTime_, starttime, double);
  SIMDATA_DECLARE_FIELD(endTime_, endtime, double);
};

/** Scenario Properties */
class SDKDATA_EXPORT ScenarioProperties : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(ScenarioProperties);
  SIMDATA_DECLARE_SUBFIELD_LIST(coordinateFrame_, coordinateframe, CoordinateFrameProperties);
  SIMDATA_DECLARE_SUBFIELD_LIST(classification_, classification, ClassificationProperties);
  SIMDATA_DECLARE_SUBFIELD_LIST(soundFile_, soundfile, SoundFileProperties);

  SIMDATA_DECLARE_FIELD(version_, version, uint32_t);
  SIMDATA_DECLARE_FIELD(referenceYear_, referenceyear, uint32_t);
  SIMDATA_DECLARE_FIELD(degreeAngles_, degreeangles, bool);
  SIMDATA_DECLARE_FIELD_CONST_REF(description_, description, std::string);
  SIMDATA_DECLARE_FIELD_CONST_REF(source_, source, std::string);
  SIMDATA_DECLARE_FIELD(windAngle_, windangle, double);
  SIMDATA_DECLARE_FIELD(windSpeed_, windspeed, double);
  SIMDATA_DECLARE_FIELD_CONST_REF(viewFile_, viewfile, std::string);
  SIMDATA_DECLARE_FIELD_CONST_REF(ruleFile_, rulefile, std::string);
  SIMDATA_DECLARE_FIELD_CONST_REF(terrainFile_, terrainfile, std::string);
  SIMDATA_DECLARE_FIELD(dataLimitTime_, datalimittime, double);
  SIMDATA_DECLARE_FIELD(dataLimitPoints_, datalimitpoints, uint32_t);
  SIMDATA_DECLARE_FIELD(ignoreDuplicateGenericData_, ignoreduplicategenericdata, bool);
  SIMDATA_DECLARE_VECTOR_FIELD(mediaFile_, mediafile, std::string);
  SIMDATA_DECLARE_VECTOR_FIELD(dedFile_, dedfile, std::string);
  SIMDATA_DECLARE_VECTOR_FIELD(wvsFile_, wvsfile, std::string);
  SIMDATA_DECLARE_VECTOR_FIELD(gogFile_, gogfile, std::string);
};

}

#endif

