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

#include "DataTypeProperties.h"

namespace simData
{

SIMDATA_DEFINE_DEFAULT_METHODS(BeamProperties);
SIMDATA_DEFINE_FIELD(BeamProperties, id_, id, uint64_t, 0);
SIMDATA_DEFINE_FIELD(BeamProperties, hostId_, hostid, uint64_t, 0);
SIMDATA_DEFINE_FIELD(BeamProperties, originalId_, originalid, uint64_t, 0);
SIMDATA_DEFINE_FIELD(BeamProperties, type_, type, simData::BeamProperties::Type, simData::BeamProperties::Type::ABSOLUTE_POSITION);
SIMDATA_DEFINE_FIELD_CONST_REF(BeamProperties, source_, source, std::string, "");

void BeamProperties::MergeFrom(const BeamProperties& from)
{
  if (&from == this)
    return;

  if (from.has_id())
    id_ = from.id();

  if (from.has_hostid())
    hostId_ = from.hostid();

  if (from.has_originalid())
    originalId_ = from.originalid();

  if (from.has_type())
    type_ = from.type();

  if (from.has_source())
    source_ = from.source();
}

//---------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_DEFAULT_METHODS(CustomRenderingProperties);
SIMDATA_DEFINE_FIELD(CustomRenderingProperties, id_, id, uint64_t, 0);
SIMDATA_DEFINE_FIELD(CustomRenderingProperties, hostId_, hostid, uint64_t, 0);
SIMDATA_DEFINE_FIELD(CustomRenderingProperties, originalId_, originalid, uint64_t, 0);
SIMDATA_DEFINE_FIELD_CONST_REF(CustomRenderingProperties, renderer_, renderer, std::string, "");
SIMDATA_DEFINE_FIELD_CONST_REF(CustomRenderingProperties, source_, source, std::string, "");

void CustomRenderingProperties::MergeFrom(const CustomRenderingProperties& from)
{
  if (&from == this)
    return;

  if (from.has_id())
    id_ = from.id();

  if (from.has_hostid())
    hostId_ = from.hostid();

  if (from.has_originalid())
    originalId_ = from.originalid();

  if (from.has_renderer())
    renderer_ = from.renderer();

  if (from.has_source())
    source_ = from.source();
}

//---------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_DEFAULT_METHODS(GateProperties);
SIMDATA_DEFINE_FIELD(GateProperties, id_, id, uint64_t, 0);
SIMDATA_DEFINE_FIELD(GateProperties, hostId_, hostid, uint64_t, 0);
SIMDATA_DEFINE_FIELD(GateProperties, originalId_, originalid, uint64_t, 0);
SIMDATA_DEFINE_FIELD(GateProperties, type_, type, simData::GateProperties::Type, simData::GateProperties::Type::ABSOLUTE_POSITION);
SIMDATA_DEFINE_FIELD_CONST_REF(GateProperties, source_, source, std::string, "");

void GateProperties::MergeFrom(const GateProperties& from)
{
  if (&from == this)
    return;

  if (from.has_id())
    id_ = from.id();

  if (from.has_hostid())
    hostId_ = from.hostid();

  if (from.has_originalid())
    originalId_ = from.originalid();

  if (from.has_type())
    type_ = from.type();

  if (from.has_source())
    source_ = from.source();
}

//---------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_DEFAULT_METHODS(LaserProperties);
SIMDATA_DEFINE_FIELD(LaserProperties, id_, id, uint64_t, 0);
SIMDATA_DEFINE_FIELD(LaserProperties, hostId_, hostid, uint64_t, 0);
SIMDATA_DEFINE_FIELD(LaserProperties, originalId_, originalid, uint64_t, 0);
SIMDATA_DEFINE_FIELD(LaserProperties, coordinateSystem_, coordinatesystem, simData::CoordinateSystemProperties, simData::CoordinateSystemProperties::NED);
SIMDATA_DEFINE_FIELD(LaserProperties, azElRelativeToHostOri_, azelrelativetohostori, bool, false);
SIMDATA_DEFINE_FIELD_CONST_REF(LaserProperties, source_, source, std::string, "");

void LaserProperties::MergeFrom(const LaserProperties& from)
{
  if (&from == this)
    return;

  if (from.has_id())
    id_ = from.id();

  if (from.has_hostid())
    hostId_ = from.hostid();

  if (from.has_originalid())
    originalId_ = from.originalid();

  if (from.has_coordinatesystem())
    coordinateSystem_ = from.coordinatesystem();

  if (from.has_azelrelativetohostori())
    azElRelativeToHostOri_ = from.azelrelativetohostori();

  if (from.has_source())
    source_ = from.source();
}

//---------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_DEFAULT_METHODS(LobGroupProperties);
SIMDATA_DEFINE_FIELD(LobGroupProperties, id_, id, uint64_t, 0);
SIMDATA_DEFINE_FIELD(LobGroupProperties, hostId_, hostid, uint64_t, 0);
SIMDATA_DEFINE_FIELD(LobGroupProperties, originalId_, originalid, uint64_t, 0);
SIMDATA_DEFINE_FIELD(LobGroupProperties, coordinateSystem_, coordinatesystem, simData::CoordinateSystemProperties, simData::CoordinateSystemProperties::NED);
SIMDATA_DEFINE_FIELD(LobGroupProperties, azElRelativeToHostOri_, azelrelativetohostori, bool, false);
SIMDATA_DEFINE_FIELD_CONST_REF(LobGroupProperties, source_, source, std::string, "");

void LobGroupProperties::MergeFrom(const LobGroupProperties& from)
{
  if (&from == this)
    return;

  if (from.has_id())
    id_ = from.id();

  if (from.has_hostid())
    hostId_ = from.hostid();

  if (from.has_originalid())
    originalId_ = from.originalid();

  if (from.has_coordinatesystem())
    coordinateSystem_ = from.coordinatesystem();

  if (from.has_azelrelativetohostori())
    azElRelativeToHostOri_ = from.azelrelativetohostori();

  if (from.has_source())
    source_ = from.source();
}

//---------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_DEFAULT_METHODS(ProjectorProperties);
SIMDATA_DEFINE_FIELD(ProjectorProperties, id_, id, uint64_t, 0);
SIMDATA_DEFINE_FIELD(ProjectorProperties, hostId_, hostid, uint64_t, 0);
SIMDATA_DEFINE_FIELD(ProjectorProperties, originalId_, originalid, uint64_t, 0);
SIMDATA_DEFINE_FIELD_CONST_REF(ProjectorProperties, source_, source, std::string, "");

void ProjectorProperties::MergeFrom(const ProjectorProperties& from)
{
  if (&from == this)
    return;

  if (from.has_id())
    id_ = from.id();

  if (from.has_hostid())
    hostId_ = from.hostid();

  if (from.has_originalid())
    originalId_ = from.originalid();

  if (from.has_source())
    source_ = from.source();
}

//---------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_DEFAULT_METHODS(ReferenceProperties);
SIMDATA_DEFINE_FIELD(ReferenceProperties, lat_, lat, double, 0.0);
SIMDATA_DEFINE_FIELD(ReferenceProperties, lon_, lon, double, 0.0);
SIMDATA_DEFINE_FIELD(ReferenceProperties, alt_, alt, double, 0.0);

void ReferenceProperties::MergeFrom(const ReferenceProperties& from)
{
  if (&from == this)
    return;

  if (from.has_lat())
    lat_ = from.lat_;

  if (from.has_lon())
    lon_ = from.lon_;

  if (from.has_alt())
    alt_ = from.alt_;
}

//---------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_DEFAULT_METHODS(TangentPlaneOffsetsProperties);
SIMDATA_DEFINE_FIELD(TangentPlaneOffsetsProperties, tx_, tx, double, 0.0);
SIMDATA_DEFINE_FIELD(TangentPlaneOffsetsProperties, ty_, ty, double, 0.0);
SIMDATA_DEFINE_FIELD(TangentPlaneOffsetsProperties, angle_, angle, double, 0.0);

void TangentPlaneOffsetsProperties::MergeFrom(const TangentPlaneOffsetsProperties& from)
{
  if (&from == this)
    return;

  if (from.has_tx())
    tx_ = from.tx_;

  if (from.has_ty())
    ty_ = from.ty_;

  if (from.has_angle())
    angle_ = from.angle_;
}

//---------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(CoordinateFrameProperties);
SIMDATA_DEFINE_SUBFIELD_LIST(CoordinateFrameProperties, referenceLla_, referencella, simData::ReferenceProperties);
SIMDATA_DEFINE_SUBFIELD_LIST(CoordinateFrameProperties, tangentPlaneOffset_, tangentplaneoffset, simData::TangentPlaneOffsetsProperties);
SIMDATA_DEFINE_FIELD(CoordinateFrameProperties, coordinateSystem_, coordinatesystem, simData::CoordinateSystemProperties, simData::CoordinateSystemProperties::NED);
SIMDATA_DEFINE_FIELD(CoordinateFrameProperties, magneticVariance_, magneticvariance, simData::MagneticVarianceProperties, simData::MagneticVarianceProperties::MV_WMM);
SIMDATA_DEFINE_FIELD(CoordinateFrameProperties, verticalDatum_, verticaldatum, simData::VerticalDatumProperties, simData::VerticalDatumProperties::VD_WGS84);
SIMDATA_DEFINE_FIELD(CoordinateFrameProperties, magneticVarianceUserValue_, magneticvarianceuservalue, double, 0.0);
SIMDATA_DEFINE_FIELD(CoordinateFrameProperties, verticalDatumUserValue_, verticaldatumuservalue, double, 0.0);
SIMDATA_DEFINE_FIELD(CoordinateFrameProperties, eciReferenceTime_, ecireferencetime, double, 0.0);

void CoordinateFrameProperties::CopyFrom(const CoordinateFrameProperties& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(referenceLla_, simData::ReferenceProperties, referencella);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(tangentPlaneOffset_, simData::TangentPlaneOffsetsProperties, tangentplaneoffset);

  coordinateSystem_ = from.coordinateSystem_;
  magneticVariance_ = from.magneticVariance_;
  verticalDatum_ = from.verticalDatum_;
  magneticVarianceUserValue_ = from.magneticVarianceUserValue_;
  verticalDatumUserValue_ = from.verticalDatumUserValue_;
  eciReferenceTime_ = from.eciReferenceTime_;
}

void CoordinateFrameProperties::MergeFrom(const CoordinateFrameProperties& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(referenceLla_, simData::ReferenceProperties, referencella);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(tangentPlaneOffset_, simData::TangentPlaneOffsetsProperties, tangentplaneoffset);

  if (from.has_coordinatesystem())
    coordinateSystem_ = from.coordinateSystem_;
  if (from.has_magneticvariance())
    magneticVariance_ = from.magneticVariance_;
  if (from.has_verticaldatum())
    verticalDatum_ = from.verticalDatum_;
  if (from.has_magneticvarianceuservalue())
    magneticVarianceUserValue_ = from.magneticVarianceUserValue_;
  if (from.has_verticaldatumuservalue())
    verticalDatumUserValue_ = from.verticalDatumUserValue_;
  if (from.has_ecireferencetime())
    eciReferenceTime_ = from.eciReferenceTime_;
}

bool CoordinateFrameProperties::operator==(const CoordinateFrameProperties& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(referenceLla_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(tangentPlaneOffset_, rhs);

  return ((coordinateSystem_ == rhs.coordinateSystem_) &&
    (magneticVariance_ == rhs.magneticVariance_) &&
    (verticalDatum_ == rhs.verticalDatum_) &&
    (magneticVarianceUserValue_ == rhs.magneticVarianceUserValue_) &&
    (verticalDatumUserValue_ == rhs.verticalDatumUserValue_) &&
    (eciReferenceTime_ == rhs.eciReferenceTime_));
}

//---------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(PlatformProperties);
SIMDATA_DEFINE_SUBFIELD_LIST(PlatformProperties, coordinateFrame_, coordinateframe, simData::CoordinateFrameProperties);
SIMDATA_DEFINE_FIELD(PlatformProperties, id_, id, uint64_t, 0);
SIMDATA_DEFINE_FIELD(PlatformProperties, originalId_, originalid, uint64_t, 0);
SIMDATA_DEFINE_FIELD_CONST_REF(PlatformProperties, source_, source, std::string, "");

void PlatformProperties::CopyFrom(const PlatformProperties& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(coordinateFrame_, simData::CoordinateFrameProperties, coordinateframe);

  id_ = from.id_;
  originalId_ = from.originalId_;
  source_ = from.source_;
}

void PlatformProperties::MergeFrom(const PlatformProperties& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(coordinateFrame_, simData::CoordinateFrameProperties, coordinateframe);

  if (from.has_id())
    id_ = from.id();

  if (from.has_originalid())
    originalId_ = from.originalid();

  if (from.has_source())
    source_ = from.source();
}

bool PlatformProperties::operator==(const PlatformProperties& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(coordinateFrame_, rhs);

  return ((id_ == rhs.id_) &&
    (originalId_ == rhs.originalId_) &&
    (source_ == rhs.source_));
}

//---------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_DEFAULT_METHODS(ClassificationProperties);
SIMDATA_DEFINE_FIELD_CONST_REF(ClassificationProperties, label_, label, std::string, "");
SIMDATA_DEFINE_FIELD(ClassificationProperties, fontColor_, fontcolor, uint32_t, 0x00FF00FF);

void ClassificationProperties::MergeFrom(const ClassificationProperties& from)
{
  if (&from == this)
    return;

  if (from.has_label())
    label_ = from.label_;

  if (from.has_fontcolor())
    fontColor_ = from.fontColor_;
}

//---------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_DEFAULT_METHODS(SoundFileProperties);
SIMDATA_DEFINE_FIELD_CONST_REF(SoundFileProperties, filename_, filename, std::string, "");
SIMDATA_DEFINE_FIELD(SoundFileProperties, startTime_, starttime, double, 0.0);
SIMDATA_DEFINE_FIELD(SoundFileProperties, endTime_, endtime, double, 0.0);

void SoundFileProperties::MergeFrom(const SoundFileProperties& from)
{
  if (&from == this)
    return;

  if (from.has_filename())
    filename_ = from.filename_;

  if (from.has_starttime())
    startTime_ = from.startTime_;

  if (from.has_endtime())
    endTime_ = from.endTime_;
}

//---------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(ScenarioProperties);
SIMDATA_DEFINE_SUBFIELD_LIST(ScenarioProperties, coordinateFrame_, coordinateframe, CoordinateFrameProperties);
SIMDATA_DEFINE_SUBFIELD_LIST(ScenarioProperties, classification_, classification, ClassificationProperties);
SIMDATA_DEFINE_SUBFIELD_LIST(ScenarioProperties, soundFile_, soundfile, SoundFileProperties);

SIMDATA_DEFINE_FIELD(ScenarioProperties, version_, version, uint32_t, 20);
SIMDATA_DEFINE_FIELD(ScenarioProperties, referenceYear_, referenceyear, uint32_t, 1970);
SIMDATA_DEFINE_FIELD(ScenarioProperties, degreeAngles_, degreeangles, bool, true);
SIMDATA_DEFINE_FIELD_CONST_REF(ScenarioProperties, description_, description, std::string, "");
SIMDATA_DEFINE_FIELD_CONST_REF(ScenarioProperties, source_, source, std::string, "");
SIMDATA_DEFINE_FIELD(ScenarioProperties, windAngle_, windangle, double, 0.0);
SIMDATA_DEFINE_FIELD(ScenarioProperties, windSpeed_, windspeed, double, 0.0);
SIMDATA_DEFINE_FIELD_CONST_REF(ScenarioProperties, viewFile_, viewfile, std::string, "");
SIMDATA_DEFINE_FIELD_CONST_REF(ScenarioProperties, ruleFile_, rulefile, std::string, "");
SIMDATA_DEFINE_FIELD_CONST_REF(ScenarioProperties, terrainFile_, terrainfile, std::string, "");
SIMDATA_DEFINE_FIELD(ScenarioProperties, dataLimitTime_, datalimittime, double, 600.0);
SIMDATA_DEFINE_FIELD(ScenarioProperties, dataLimitPoints_, datalimitpoints, uint32_t, 1000);
SIMDATA_DEFINE_FIELD(ScenarioProperties, ignoreDuplicateGenericData_, ignoreduplicategenericdata, bool, true);
SIMDATA_DEFINE_VECTOR_FIELD(ScenarioProperties, mediaFile_, mediafile, std::string);
SIMDATA_DEFINE_VECTOR_FIELD(ScenarioProperties, dedFile_, dedfile, std::string);
SIMDATA_DEFINE_VECTOR_FIELD(ScenarioProperties, wvsFile_, wvsfile, std::string);
SIMDATA_DEFINE_VECTOR_FIELD(ScenarioProperties, gogFile_, gogfile, std::string);

void ScenarioProperties::CopyFrom(const ScenarioProperties& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(coordinateFrame_, CoordinateFrameProperties, coordinateframe);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(classification_, ClassificationProperties, classification);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(soundFile_, SoundFileProperties, soundfile);

  version_ = from.version_;
  referenceYear_ = from.referenceYear_;
  degreeAngles_ = from.degreeAngles_;
  description_ = from.description_;
  source_ = from.source_;
  windAngle_ = from.windAngle_;
  windSpeed_ = from.windSpeed_;
  viewFile_ = from.viewFile_;
  ruleFile_ = from.ruleFile_;
  terrainFile_ = from.terrainFile_;
  dataLimitTime_ = from.dataLimitTime_;
  dataLimitPoints_ = from.dataLimitPoints_;
  ignoreDuplicateGenericData_ = from.ignoreDuplicateGenericData_;
  mediaFile_ = from.mediaFile_;
  dedFile_ = from.dedFile_;
  wvsFile_ = from.wvsFile_;
  gogFile_ = from.gogFile_;
}

void ScenarioProperties::MergeFrom(const ScenarioProperties& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(coordinateFrame_, CoordinateFrameProperties, coordinateframe);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(classification_, ClassificationProperties, classification);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(soundFile_, SoundFileProperties, soundfile);

  if (from.has_version())
    version_ = from.version_;
  if (from.has_referenceyear())
    referenceYear_ = from.referenceYear_;
  if (from.has_degreeangles())
    degreeAngles_ = from.degreeAngles_;
  if (from.has_description())
    description_ = from.description_;
  if (from.has_source())
    source_ = from.source_;
  if (from.has_windangle())
    windAngle_ = from.windAngle_;
  if (from.has_windspeed())
    windSpeed_ = from.windSpeed_;
  if (from.has_viewfile())
    viewFile_ = from.viewFile_;
  if (from.has_rulefile())
    ruleFile_ = from.ruleFile_;
  if (from.has_terrainfile())
    terrainFile_ = from.terrainFile_;
  if (from.has_datalimittime())
    dataLimitTime_ = from.dataLimitTime_;
  if (from.has_datalimitpoints())
    dataLimitPoints_ = from.dataLimitPoints_;
  if (from.has_ignoreduplicategenericdata())
    ignoreDuplicateGenericData_ = from.ignoreDuplicateGenericData_;

  mediaFile_.insert(mediaFile_.end(), from.mediaFile_.begin(), from.mediaFile_.end());
  dedFile_.insert(dedFile_.end(), from.dedFile_.begin(), from.dedFile_.end());
  wvsFile_.insert(wvsFile_.end(), from.wvsFile_.begin(), from.wvsFile_.end());
  gogFile_.insert(gogFile_.end(), from.gogFile_.begin(), from.gogFile_.end());
}

bool ScenarioProperties::operator==(const ScenarioProperties& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(coordinateFrame_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(classification_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(soundFile_, rhs);

  if (version_ != rhs.version_)
    return false;

  if (referenceYear_ != rhs.referenceYear_)
    return false;

  if (degreeAngles_ != rhs.degreeAngles_)
    return false;

  if (description_ != rhs.description_)
    return false;

  if (source_ != rhs.source_)
    return false;

  if (windAngle_ != rhs.windAngle_)
    return false;

  if (windSpeed_ != rhs.windSpeed_)
    return false;

  if (viewFile_ != rhs.viewFile_)
    return false;

  if (ruleFile_ != rhs.ruleFile_)
    return false;

  if (terrainFile_ != rhs.terrainFile_)
    return false;

  if (dataLimitTime_ != rhs.dataLimitTime_)
    return false;

  if (dataLimitPoints_ != rhs.dataLimitPoints_)
    return false;

  if (ignoreDuplicateGenericData_ != rhs.ignoreDuplicateGenericData_)
    return false;

  if (mediaFile_ != rhs.mediaFile_)
    return false;

  if (dedFile_ != rhs.dedFile_)
    return false;

  if (wvsFile_ != rhs.wvsFile_)
    return false;

  if (gogFile_ != rhs.gogFile_)
    return false;

  return true;
}

}
