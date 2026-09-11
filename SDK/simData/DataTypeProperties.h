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

/**
 * CommonPreferences.h, EntityPreferences.h, and DataTypeProperties.h define the underlying data format for SIMDIS SDK's simData module.
 *
 * Entity types include platforms, beams, gates, lasers, projectors, line of bearing (LOB) groups, and Custom Rendering
 * Entities are generally broken up into 3 components:
 *
 *   Properties - Settings for an entity, such as ID that are mostly unchanging
 *   Preferences - Changeable settings for an entity, including PrefsTool-like settings;
 *                 can be time stamped by using Commands
 *   Commands - A time stamp and a (likely) sparse list of preferences to apply at the time
 *
 * Unless otherwise specified, all units are in metric: radians for angles, meters for distance,
 * and meters per second for speed values.
 *
 * Notes on Draw / dataDraw flags: Draw is a user preference. This is equivalent to the Draw setting
 * in PrefsTool. The dataDraw flag is a data setting that is managed through the data provider and
 * is similar to the On/Off settings in SIMDIS 9. The dataDraw flag is hierarchical -- a gate is not
 * drawn if its beam's dataDraw is off, and even if the gate's dataDraw and draw flags are true. The draw flag is not
 * hierarchical.  An entity is only drawn if its dataDraw is on, its host is active, and its draw flag is on.
 * The dataDraw flag is not set for, and is completely ignored by Platforms.
 * Beams and gates still require that their platform host be active, but this is signaled by other means than the dataDraw flag.
 */

/** Beam Properties */
class SDKDATA_EXPORT BeamProperties : public FieldList
{
public:
  /** Types of beams */
  enum class Type {
    ABSOLUTE_POSITION = 1, ///< Beam pointing is defined based on data
    BODY_RELATIVE,         ///< Beam pointing is relative to host's body orientation
    TARGET                 ///< Beam pointing is towards specified target platform
  };

  SIMDATA_DECLARE_DEFAULT_METHODS(BeamProperties);
  /// ID assigned to the entity from the data store; unchangeable; might change during file conversion
  SIMDATA_DECLARE_FIELD(id_, id, uint64_t);
  /// ID of the beam's host platform
  SIMDATA_DECLARE_FIELD(hostId_, hostid, uint64_t);
  /// User-defined ID value, will not change during file conversion
  SIMDATA_DECLARE_FIELD(originalId_, originalid, uint64_t);
  /// Beam type affects how BeamUpdate values are interpreted
  SIMDATA_DECLARE_FIELD(type_, type, simData::BeamProperties::Type);
  /// User-defined source string
  SIMDATA_DECLARE_FIELD_CONST_REF(source_, source, std::string);
};

/** Custom Rendering Properties */
class SDKDATA_EXPORT CustomRenderingProperties : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(CustomRenderingProperties);
  /// ID assigned to the entity from the data store; unchangeable; might change during file conversion
  SIMDATA_DECLARE_FIELD(id_, id, uint64_t);
  /// ID of the Custom's host, if any
  SIMDATA_DECLARE_FIELD(hostId_, hostid, uint64_t);
  /// User-defined ID value, will not change during file conversion
  SIMDATA_DECLARE_FIELD(originalId_, originalid, uint64_t);
  /// The name of the renderer that should process this entity
  SIMDATA_DECLARE_FIELD_CONST_REF(renderer_, renderer, std::string);
  /// User-defined source string
  SIMDATA_DECLARE_FIELD_CONST_REF(source_, source, std::string);
};

/** Gate Properties */
class SDKDATA_EXPORT GateProperties : public FieldList
{
public:
  /** Types of gates */
  enum class Type {
    ABSOLUTE_POSITION = 1, ///< Gate pointing is defined based on data
    BODY_RELATIVE,         ///< Gate pointing is relative to host's body orientation
    TARGET                 ///< Gate pointing is towards specified target platform
  };

  SIMDATA_DECLARE_DEFAULT_METHODS(GateProperties);
  /// ID assigned to the entity from the data store; unchangeable; might change during file conversion
  SIMDATA_DECLARE_FIELD(id_, id, uint64_t);
  /// ID of the gate's host beam
  SIMDATA_DECLARE_FIELD(hostId_, hostid, uint64_t);
  /// User-defined ID value, will not change during file conversion
  SIMDATA_DECLARE_FIELD(originalId_, originalid, uint64_t);
  /// Gate type affects how GateUpdate values are interpreted
  SIMDATA_DECLARE_FIELD(type_, type, simData::GateProperties::Type);
  /// User-defined source string
  SIMDATA_DECLARE_FIELD_CONST_REF(source_, source, std::string);
};

/** Laser Properties */
class SDKDATA_EXPORT LaserProperties : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(LaserProperties);
  /// ID assigned to the entity from the data store; unchangeable; might change during file conversion
  SIMDATA_DECLARE_FIELD(id_, id, uint64_t);
  /// ID of the laser's host platform
  SIMDATA_DECLARE_FIELD(hostId_, hostid, uint64_t);
  /// User-defined ID value, will not change during file conversion
  SIMDATA_DECLARE_FIELD(originalId_, originalid, uint64_t);
  /// Coordinate system for orientation angle interpretation; only used in loading/saving, all angles stored internally are normalized
  SIMDATA_DECLARE_FIELD(coordinateSystem_, coordinatesystem, simData::CoordinateSystem);
  /// Indicates that the orientation values are in body angles relative to platform (true), or true angles (false)
  SIMDATA_DECLARE_FIELD(azElRelativeToHostOri_, azelrelativetohostori, bool);
  /// User-defined source string
  SIMDATA_DECLARE_FIELD_CONST_REF(source_, source, std::string);
};

/** LobGroup Properties */
class SDKDATA_EXPORT LobGroupProperties : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(LobGroupProperties);
  /// ID assigned to the entity from the data store; unchangeable; might change during file conversion
  SIMDATA_DECLARE_FIELD(id_, id, uint64_t);
  /// ID of the LOB's host platform
  SIMDATA_DECLARE_FIELD(hostId_, hostid, uint64_t);
  /// User-defined ID value, will not change during file conversion
  SIMDATA_DECLARE_FIELD(originalId_, originalid, uint64_t);
  /// Coordinate system for orientation angle interpretation; only used in loading/saving, all angles stored internally are normalized
  SIMDATA_DECLARE_FIELD(coordinateSystem_, coordinatesystem, simData::CoordinateSystem);
  /// Indicates that the orientation values are in body angles relative to platform (true), or true angles (false)
  SIMDATA_DECLARE_FIELD(azElRelativeToHostOri_, azelrelativetohostori, bool);
  /// User-defined source string
  SIMDATA_DECLARE_FIELD_CONST_REF(source_, source, std::string);
};

/** Projector Properties */
class SDKDATA_EXPORT ProjectorProperties : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(ProjectorProperties);
  /// ID assigned to the entity from the data store; unchangeable; might change during file conversion
  SIMDATA_DECLARE_FIELD(id_, id, uint64_t);
  /// ID of the projector's host platform
  SIMDATA_DECLARE_FIELD(hostId_, hostid, uint64_t);
  /// User-defined ID value, will not change during file conversion
  SIMDATA_DECLARE_FIELD(originalId_, originalid, uint64_t);
  /// User-defined source string
  SIMDATA_DECLARE_FIELD_CONST_REF(source_, source, std::string);
};

/** Reference Properties */
class SDKDATA_EXPORT ReferenceProperties : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(ReferenceProperties);
  SIMDATA_DECLARE_FIELD(lat_, lat, double); ///< Latitude value, in radians
  SIMDATA_DECLARE_FIELD(lon_, lon, double); ///< Longitude value, in radians
  SIMDATA_DECLARE_FIELD(alt_, alt, double); ///< Altitude value, in meters
};

/** Tangent Plane Offsets Properties */
class SDKDATA_EXPORT TangentPlaneOffsetsProperties : public FieldList
{
public:
  SIMDATA_DECLARE_DEFAULT_METHODS(TangentPlaneOffsetsProperties);
  /// Distance along true east from origin
  SIMDATA_DECLARE_FIELD(tx_, tx, double);
  /// Distance along true north from origin
  SIMDATA_DECLARE_FIELD(ty_, ty, double);
  /// Clockwise Rotation angle, applied after translation
  SIMDATA_DECLARE_FIELD(angle_, angle, double);
};

/** Coordinate Frame Properties */
class SDKDATA_EXPORT CoordinateFrameProperties : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(CoordinateFrameProperties);
  /// Reference origin of the coordinate system; required for tangent plane systems
  SIMDATA_DECLARE_SUBFIELD_LIST(referenceLla_, referencella, simData::ReferenceProperties);
  /// Generic Tangent Plane offset from the origin; Defined as X offset, Y offset, and tangent plane angle in radians
  SIMDATA_DECLARE_SUBFIELD_LIST(tangentPlaneOffset_, tangentplaneoffset, simData::TangentPlaneOffsetsProperties);
  /// Coordinate system flag.  SIMDIS SDK stores all data internally as ECEF; this is only descriptive for input/output
  SIMDATA_DECLARE_FIELD(coordinateSystem_, coordinatesystem, simData::CoordinateSystem);
  /// Magnetic variance, defaulting to True angles; data normalized in SDK to TRUE angles, this is only descriptive for input/output
  SIMDATA_DECLARE_FIELD(magneticVariance_, magneticvariance, simData::MagneticVariance);
  /// Vertical datum to apply to altitudes, defaulting to WGS-84 ellipsoid; data normalized in SDK to WGS84, this is only descriptive for input/output
  SIMDATA_DECLARE_FIELD(verticalDatum_, verticaldatum, simData::VerticalDatum);
  /// Offset to apply if magneticVariance is set to user defined
  SIMDATA_DECLARE_FIELD(magneticVarianceUserValue_, magneticvarianceuservalue, double);
  /// Offset to apply if verticalDatum is set to user defined
  SIMDATA_DECLARE_FIELD(verticalDatumUserValue_, verticaldatumuservalue, double);
  /// Reference for ECI coordinate frame, in seconds since reference year
  SIMDATA_DECLARE_FIELD(eciReferenceTime_, ecireferencetime, double);
};

/** Platform Properties */
class SDKDATA_EXPORT PlatformProperties : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(PlatformProperties);
  /// Coordinate reference frame, used for defining input/output formats; data still stored internally as ECEF
  SIMDATA_DECLARE_SUBFIELD_LIST(coordinateFrame_, coordinateframe, simData::CoordinateFrameProperties);
  /// ID assigned to the entity from the data store; unchangeable; might change during file conversion
  SIMDATA_DECLARE_FIELD(id_, id, uint64_t);
  /// User-defined ID value, will not change during file conversion
  SIMDATA_DECLARE_FIELD(originalId_, originalid, uint64_t);
  /// User-defined source string
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

/** Global settings for a scene */
class SDKDATA_EXPORT ScenarioProperties : public FieldList
{
public:
  SIMDATA_DECLARE_METHODS(ScenarioProperties);
  /// Note that all values stored internally as ECEF; this is a hint to import/export code
  SIMDATA_DECLARE_SUBFIELD_LIST(coordinateFrame_, coordinateframe, CoordinateFrameProperties);
  SIMDATA_DECLARE_SUBFIELD_LIST(classification_, classification, ClassificationProperties);
  SIMDATA_DECLARE_SUBFIELD_LIST(soundFile_, soundfile, SoundFileProperties);

  /// User defined version field
  SIMDATA_DECLARE_FIELD(version_, version, uint32_t);
  /// Reference year that time values are referenced to
  SIMDATA_DECLARE_FIELD(referenceYear_, referenceyear, uint32_t);
  /// Note that degreeAngles only affects input and output.  All angles are stored internally as radians
  SIMDATA_DECLARE_FIELD(degreeAngles_, degreeangles, bool);
  /// Textual scenario description, e.g. as set by PIData::setDescription()
  SIMDATA_DECLARE_FIELD_CONST_REF(description_, description, std::string);
  /**
   * Source location for the data store, typically a file path.  For
   * live data, this may be set to a generic description or empty.  For
   * example, SIMDIS 10 will place the filename hint from
   * PIData::initializeScenario() in this field.
   */
  SIMDATA_DECLARE_FIELD_CONST_REF(source_, source, std::string);
  /// Wind direction in radians
  SIMDATA_DECLARE_FIELD(windAngle_, windangle, double);
  /// Wind speed in meters per second
  SIMDATA_DECLARE_FIELD(windSpeed_, windspeed, double);
  SIMDATA_DECLARE_FIELD_CONST_REF(viewFile_, viewfile, std::string);
  SIMDATA_DECLARE_FIELD_CONST_REF(ruleFile_, rulefile, std::string);
  SIMDATA_DECLARE_FIELD_CONST_REF(terrainFile_, terrainfile, std::string);
  /// Scenario level data table and generic data limiting value in seconds of history; <= 0 indicates no limiting applied
  SIMDATA_DECLARE_FIELD(dataLimitTime_, datalimittime, double);
  /// Scenario level data table and generic data limiting value in number of data points; == 0 indicates no limiting applied
  SIMDATA_DECLARE_FIELD(dataLimitPoints_, datalimitpoints, uint32_t);
  /// Flags whether Generic Data duplicate entries should be filtered (live mode only)
  SIMDATA_DECLARE_FIELD(ignoreDuplicateGenericData_, ignoreduplicategenericdata, bool);
  SIMDATA_DECLARE_VECTOR_FIELD(mediaFile_, mediafile, std::string);
  SIMDATA_DECLARE_VECTOR_FIELD(dedFile_, dedfile, std::string);
  SIMDATA_DECLARE_VECTOR_FIELD(wvsFile_, wvsfile, std::string);
  SIMDATA_DECLARE_VECTOR_FIELD(gogFile_, gogfile, std::string);
};

}

#endif

