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

#include "simData/CommonPreferences.h"

namespace simData
{

SIMDATA_DEFINE_DEFAULT_METHODS(DisplayFields);
SIMDATA_DEFINE_FIELD(DisplayFields, xLat_, xlat, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, yLon_, ylon, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, zAlt_, zalt, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, genericData_, genericdata, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, categoryData_, categorydata, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, yaw_, yaw, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, pitch_, pitch, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, roll_, roll, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, course_, course, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, flightPathElevation_, flightpathelevation, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, displayVX_, displayvx, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, displayVY_, displayvy, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, displayVZ_, displayvz, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, speed_, speed, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, mach_, mach, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, angleOfAttack_, angleofattack, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, sideSlip_, sideslip, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, totalAngleOfAttack_, totalangleofattack, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, solarAzimuth_, solarazimuth, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, solarElevation_, solarelevation, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, solarIlluminance_, solarilluminance, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, lunarAzimuth_, lunarazimuth, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, lunarElevation_, lunarelevation, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, lunarIlluminance_, lunarilluminance, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, late_, late, bool, false);
SIMDATA_DEFINE_FIELD(DisplayFields, useLabelCode_, uselabelcode, bool, false);
SIMDATA_DEFINE_FIELD_CONST_REF(DisplayFields, labelCode_, labelcode, std::string, "");

 void DisplayFields::MergeFrom(const DisplayFields& from)
{
   if (&from == this)
     return;

  if (from.has_xlat())
    xLat_ = from.xLat_;

  if (from.has_ylon())
    yLon_ = from.yLon_;

  if (from.has_zalt())
    zAlt_ = from.zAlt_;

  if (from.has_genericdata())
    genericData_ = from.genericData_;

  if (from.has_categorydata())
    categoryData_ = from.categoryData_;

  if (from.has_yaw())
    yaw_ = from.yaw_;

  if (from.has_pitch())
    pitch_ = from.pitch_;

  if (from.has_roll())
    roll_ = from.roll_;

  if (from.has_course())
    course_ = from.course_;

  if (from.has_flightpathelevation())
    flightPathElevation_ = from.flightPathElevation_;

  if (from.has_displayvx())
    displayVX_ = from.displayVX_;

  if (from.has_displayvy())
    displayVY_ = from.displayVY_;

  if (from.has_displayvz())
    displayVZ_ = from.displayVZ_;

  if (from.has_speed())
    speed_ = from.speed_;

  if (from.has_mach())
    mach_ = from.mach_;

  if (from.has_angleofattack())
    angleOfAttack_ = from.angleOfAttack_;

  if (from.has_sideslip())
    sideSlip_ = from.sideSlip_;

  if (from.has_totalangleofattack())
    totalAngleOfAttack_ = from.totalAngleOfAttack_;

  if (from.has_solarazimuth())
    solarAzimuth_ = from.solarAzimuth_;

  if (from.has_solarelevation())
    solarElevation_ = from.solarElevation_;

  if (from.has_solarilluminance())
    solarIlluminance_ = from.solarIlluminance_;

  if (from.has_lunarazimuth())
    lunarAzimuth_ = from.lunarAzimuth_;

  if (from.has_lunarelevation())
    lunarElevation_ = from.lunarElevation_;

  if (from.has_lunarilluminance())
    lunarIlluminance_ = from.lunarIlluminance_;

  if (from.has_late())
    late_ = from.late_;

  if (from.has_uselabelcode())
    useLabelCode_ = from.useLabelCode_;

  if (from.has_labelcode())
    labelCode_ = from.labelCode_;
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(LabelPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(LabelPrefs, displayFields_, displayfields, simData::DisplayFields);
SIMDATA_DEFINE_SUBFIELD_LIST(LabelPrefs, legendDisplayFields_, legenddisplayfields, simData::DisplayFields);
SIMDATA_DEFINE_SUBFIELD_LIST(LabelPrefs, hoverDisplayFields_, hoverdisplayfields, simData::DisplayFields);
SIMDATA_DEFINE_SUBFIELD_LIST(LabelPrefs, hookDisplayFields_, hookdisplayfields, simData::DisplayFields);
SIMDATA_DEFINE_FIELD(LabelPrefs, draw_, draw, bool, false);
SIMDATA_DEFINE_FIELD(LabelPrefs, color_, color, uint32_t, 0xFBFBFBFF); // off-white
SIMDATA_DEFINE_FIELD(LabelPrefs, textOutline_, textoutline, simData::TextOutline, simData::TextOutline::TO_THIN);
SIMDATA_DEFINE_FIELD(LabelPrefs, outlineColor_, outlinecolor, uint32_t, 255); // black (full alpha)
SIMDATA_DEFINE_FIELD(LabelPrefs, backdropType_, backdroptype, simData::BackdropType, simData::BackdropType::BDT_OUTLINE);
SIMDATA_DEFINE_FIELD(LabelPrefs, backdropImplementation_, backdropimplementation, simData::BackdropImplementation, simData::BackdropImplementation::BDI_POLYGON_OFFSET);
SIMDATA_DEFINE_FIELD_CONST_REF(LabelPrefs, overlayFontName_, overlayfontname, std::string, "arial.ttf");
SIMDATA_DEFINE_FIELD(LabelPrefs, overlayFontPointSize_, overlayfontpointsize, uint32_t, 14);
SIMDATA_DEFINE_FIELD(LabelPrefs, offsetX_, offsetx, int32_t, 0);
SIMDATA_DEFINE_FIELD(LabelPrefs, offsetY_, offsety, int32_t, 0);
SIMDATA_DEFINE_FIELD(LabelPrefs, alignment_, alignment, simData::TextAlignment, simData::TextAlignment::ALIGN_CENTER_TOP);
SIMDATA_DEFINE_FIELD(LabelPrefs, priority_, priority, double, 100.0);
SIMDATA_DEFINE_FIELD(LabelPrefs, applyHeightAboveTerrain_, applyheightaboveterrain, bool, false);
SIMDATA_DEFINE_FIELD(LabelPrefs, applyRoll_, applyroll, bool, false);
SIMDATA_DEFINE_FIELD(LabelPrefs, coordinateSystem_, coordinatesystem, simData::CoordinateSystem, simData::CoordinateSystem::LLA);
SIMDATA_DEFINE_FIELD(LabelPrefs, verticalDatum_, verticaldatum, simData::VerticalDatum, simData::VerticalDatum::VD_WGS84);
SIMDATA_DEFINE_FIELD(LabelPrefs, magneticVariance_, magneticvariance, simData::MagneticVariance, simData::MagneticVariance::MV_TRUE);
SIMDATA_DEFINE_FIELD(LabelPrefs, distanceUnits_, distanceunits, simData::DistanceUnits, simData::DistanceUnits::UNITS_METERS);
SIMDATA_DEFINE_FIELD(LabelPrefs, angleUnits_, angleunits, simData::AngleUnits, simData::AngleUnits::UNITS_DEGREES);
SIMDATA_DEFINE_FIELD(LabelPrefs, speedUnits_, speedunits, simData::SpeedUnits, simData::SpeedUnits::UNITS_KNOTS);
SIMDATA_DEFINE_FIELD(LabelPrefs, precision_, precision, int32_t, 2);
SIMDATA_DEFINE_FIELD(LabelPrefs, nameLength_, namelength, int32_t, 0);
SIMDATA_DEFINE_FIELD(LabelPrefs, geodeticUnits_, geodeticunits, simData::GeodeticUnits, simData::GeodeticUnits::GEODETIC_DEGREES);
SIMDATA_DEFINE_FIELD(LabelPrefs, altitudeUnits_, altitudeunits, simData::DistanceUnits, simData::DistanceUnits::UNITS_METERS);
SIMDATA_DEFINE_FIELD(LabelPrefs, distancePrecision_, distanceprecision, int32_t, 1);
SIMDATA_DEFINE_FIELD(LabelPrefs, anglePrecision_, angleprecision, int32_t, 1);
SIMDATA_DEFINE_FIELD(LabelPrefs, speedPrecision_, speedprecision, int32_t, 1);
SIMDATA_DEFINE_FIELD(LabelPrefs, geodeticPrecision_, geodeticprecision, int32_t, 6);
SIMDATA_DEFINE_FIELD(LabelPrefs, altitudePrecision_, altitudeprecision, int32_t, 1);
SIMDATA_DEFINE_FIELD(LabelPrefs, timePrecision_, timeprecision, int32_t, 0);
SIMDATA_DEFINE_FIELD(LabelPrefs, useValues_, usevalues, LabelPrefs::UseValue, LabelPrefs::UseValue::DISPLAY_VALUE);

void LabelPrefs::MergeFrom(const LabelPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(displayFields_, simData::DisplayFields, displayfields);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(legendDisplayFields_, simData::DisplayFields, legenddisplayfields);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(hoverDisplayFields_, simData::DisplayFields, hoverdisplayfields);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(hookDisplayFields_, simData::DisplayFields, hookdisplayfields);

  if (from.has_draw())
    draw_ = from.draw_;

  if (from.has_color())
    color_ = from.color_;

  if (from.has_textoutline())
    textOutline_ = from.textOutline_;

  if (from.has_outlinecolor())
    outlineColor_ = from.outlineColor_;

  if (from.has_backdroptype())
    backdropType_ = from.backdropType_;

  if (from.has_backdropimplementation())
    backdropImplementation_ = from.backdropImplementation_;

  if (from.has_overlayfontname())
    overlayFontName_ = from.overlayFontName_;

  if (from.has_overlayfontpointsize())
    overlayFontPointSize_ = from.overlayFontPointSize_;

  if (from.has_offsetx())
    offsetX_ = from.offsetX_;

  if (from.has_offsety())
    offsetY_ = from.offsetY_;

  if (from.has_alignment())
    alignment_ = from.alignment_;

  if (from.has_priority())
    priority_ = from.priority_;

  if (from.has_applyheightaboveterrain())
    applyHeightAboveTerrain_ = from.applyHeightAboveTerrain_;

  if (from.has_applyroll())
    applyRoll_ = from.applyRoll_;

  if (from.has_coordinatesystem())
    coordinateSystem_ = from.coordinateSystem_;

  if (from.has_verticaldatum())
    verticalDatum_ = from.verticalDatum_;

  if (from.has_magneticvariance())
    magneticVariance_ = from.magneticVariance_;

  if (from.has_distanceunits())
    distanceUnits_ = from.distanceUnits_;

  if (from.has_angleunits())
    angleUnits_ = from.angleUnits_;

  if (from.has_speedunits())
    speedUnits_ = from.speedUnits_;

  if (from.has_precision())
    precision_ = from.precision_;

  if (from.has_namelength())
    nameLength_ = from.nameLength_;

  if (from.has_geodeticunits())
    geodeticUnits_ = from.geodeticUnits_;

  if (from.has_altitudeunits())
    altitudeUnits_ = from.altitudeUnits_;

  if (from.has_distanceprecision())
    distancePrecision_ = from.distancePrecision_;

  if (from.has_angleprecision())
    anglePrecision_ = from.anglePrecision_;

  if (from.has_speedprecision())
    speedPrecision_ = from.speedPrecision_;

  if (from.has_geodeticprecision())
    geodeticPrecision_ = from.geodeticPrecision_;

  if (from.has_altitudeprecision())
    altitudePrecision_ = from.altitudePrecision_;

  if (from.has_timeprecision())
    timePrecision_ = from.timePrecision_;

  if (from.has_usevalues())
    useValues_ = from.useValues_;
}

void LabelPrefs::CopyFrom(const LabelPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(displayFields_, simData::DisplayFields, displayfields);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(legendDisplayFields_, simData::DisplayFields, legenddisplayfields);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(hoverDisplayFields_, simData::DisplayFields, hoverdisplayfields);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(hookDisplayFields_, simData::DisplayFields, hookdisplayfields);

  draw_ = from.draw_;
  color_ = from.color_;
  textOutline_ = from.textOutline_;
  outlineColor_ = from.outlineColor_;
  backdropType_ = from.backdropType_;
  backdropImplementation_ = from.backdropImplementation_;
  overlayFontName_ = from.overlayFontName_;
  overlayFontPointSize_ = from.overlayFontPointSize_;
  offsetX_ = from.offsetX_;
  offsetY_ = from.offsetY_;
  alignment_ = from.alignment_;
  priority_ = from.priority_;
  applyHeightAboveTerrain_ = from.applyHeightAboveTerrain_;
  applyRoll_ = from.applyRoll_;
  coordinateSystem_ = from.coordinateSystem_;
  verticalDatum_ = from.verticalDatum_;
  magneticVariance_ = from.magneticVariance_;
  distanceUnits_ = from.distanceUnits_;
  angleUnits_ = from.angleUnits_;
  speedUnits_ = from.speedUnits_;
  precision_ = from.precision_;
  nameLength_ = from.nameLength_;
  geodeticUnits_ = from.geodeticUnits_;
  altitudeUnits_ = from.altitudeUnits_;
  distancePrecision_ = from.distancePrecision_;
  anglePrecision_ = from.anglePrecision_;
  speedPrecision_ = from.speedPrecision_;
  geodeticPrecision_ = from.geodeticPrecision_;
  altitudePrecision_ = from.altitudePrecision_;
  timePrecision_ = from.timePrecision_;
  useValues_ = from.useValues_;
}

bool LabelPrefs::operator==(const LabelPrefs& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(displayFields_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(legendDisplayFields_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(hoverDisplayFields_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(hookDisplayFields_, rhs);

  return ((draw_ == rhs.draw_) &&
    (color_ == rhs.color_) &&
    (textOutline_ == rhs.textOutline_) &&
    (outlineColor_ == rhs.outlineColor_) &&
    (backdropType_ == rhs.backdropType_) &&
    (backdropImplementation_ == rhs.backdropImplementation_) &&
    (overlayFontName_ == rhs.overlayFontName_) &&
    (overlayFontPointSize_ == rhs.overlayFontPointSize_) &&
    (offsetX_ == rhs.offsetX_) &&
    (offsetY_ == rhs.offsetY_) &&
    (alignment_ == rhs.alignment_) &&
    (priority_ == rhs.priority_) &&
    (applyHeightAboveTerrain_ == rhs.applyHeightAboveTerrain_) &&
    (applyRoll_ == rhs.applyRoll_) &&
    (coordinateSystem_ == rhs.coordinateSystem_) &&
    (verticalDatum_ == rhs.verticalDatum_) &&
    (magneticVariance_ == rhs.magneticVariance_) &&
    (distanceUnits_ == rhs.distanceUnits_) &&
    (angleUnits_ == rhs.angleUnits_) &&
    (speedUnits_ == rhs.speedUnits_) &&
    (precision_ == rhs.precision_) &&
    (nameLength_ == rhs.nameLength_) &&
    (geodeticUnits_ == rhs.geodeticUnits_) &&
    (altitudeUnits_ == rhs.altitudeUnits_) &&
    (distancePrecision_ == rhs.distancePrecision_) &&
    (anglePrecision_ == rhs.anglePrecision_) &&
    (speedPrecision_ == rhs.speedPrecision_) &&
    (geodeticPrecision_ == rhs.geodeticPrecision_) &&
    (altitudePrecision_ == rhs.altitudePrecision_) &&
    (timePrecision_ == rhs.timePrecision_) &&
    (useValues_ == rhs.useValues_));
}

void LabelPrefs::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(displayFields_);
  SIMDATA_SUBFIELD_LIST_PRUNE(legendDisplayFields_);
  SIMDATA_SUBFIELD_LIST_PRUNE(hoverDisplayFields_);
  SIMDATA_SUBFIELD_LIST_PRUNE(hookDisplayFields_);
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_DEFAULT_METHODS(SpeedRing);
SIMDATA_DEFINE_FIELD(SpeedRing, useFixedTime_, usefixedtime, bool, false);
SIMDATA_DEFINE_FIELD_CONST_REF(SpeedRing, fixedTime_, fixedtime, std::string, "");
SIMDATA_DEFINE_FIELD(SpeedRing, timeFormat_, timeformat, ElapsedTimeFormat, ElapsedTimeFormat::ELAPSED_HOURS);
SIMDATA_DEFINE_FIELD(SpeedRing, radius_, radius, double, 1.0);
SIMDATA_DEFINE_FIELD(SpeedRing, usePlatformSpeed_, useplatformspeed, bool, true);
SIMDATA_DEFINE_FIELD(SpeedRing, speedToUse_, speedtouse, double, 10.0);
SIMDATA_DEFINE_FIELD(SpeedRing, displayTime_, displaytime, bool, true);
SIMDATA_DEFINE_FIELD(SpeedRing, speedUnits_, speedunits, SpeedUnits, SpeedUnits::UNITS_KNOTS);

void SpeedRing::MergeFrom(const SpeedRing& from)
{
  if (&from == this)
    return;

  if (from.has_usefixedtime())
    useFixedTime_ = from.useFixedTime_;

  if (from.has_fixedtime())
    fixedTime_ = from.fixedTime_;

  if (from.has_timeformat())
    timeFormat_ = from.timeFormat_;

  if (from.has_radius())
    radius_ = from.radius_;

  if (from.has_useplatformspeed())
    usePlatformSpeed_ = from.usePlatformSpeed_;

  if (from.has_speedtouse())
    speedToUse_ = from.speedToUse_;

  if (from.has_displaytime())
    displayTime_ = from.displayTime_;

  if (from.has_speedunits())
    speedUnits_ = from.speedUnits_;
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_DEFAULT_METHODS(GridSettings);
SIMDATA_DEFINE_FIELD(GridSettings, numDivisions_, numdivisions, uint32_t, 1);
SIMDATA_DEFINE_FIELD(GridSettings, numSubDivisions_, numsubdivisions, uint32_t, 1);
SIMDATA_DEFINE_FIELD(GridSettings, sectorAngle_, sectorangle, double, 30.0);

void GridSettings::MergeFrom(const GridSettings& from)
{
  if (&from == this)
    return;

  if (from.has_numdivisions())
    numDivisions_ = from.numDivisions_;

  if (from.has_numsubdivisions())
    numSubDivisions_ = from.numSubDivisions_;

  if (from.has_sectorangle())
    sectorAngle_ = from.sectorAngle_;
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_DEFAULT_METHODS(Position);
SIMDATA_DEFINE_FIELD(Position, x_, x, double, 0.0);
SIMDATA_DEFINE_FIELD(Position, y_, y, double, 0.0);
SIMDATA_DEFINE_FIELD(Position, z_, z, double, 0.0);

void Position::MergeFrom(const Position& from)
{
  if (&from == this)
    return;

  if (from.has_x())
    x_ = from.x_;

  if (from.has_y())
    y_ = from.y_;

  if (from.has_z())
    z_ = from.z_;
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_DEFAULT_METHODS(BodyOrientation);
SIMDATA_DEFINE_FIELD(BodyOrientation, yaw_, yaw, double, 0.0);
SIMDATA_DEFINE_FIELD(BodyOrientation, pitch_, pitch, double, 0.0);
SIMDATA_DEFINE_FIELD(BodyOrientation, roll_, roll, double, 0.0);

void BodyOrientation::MergeFrom(const BodyOrientation& from)
{
  if (&from == this)
    return;

  if (from.has_yaw())
    yaw_ = from.yaw_;

  if (from.has_pitch())
    pitch_ = from.pitch_;

  if (from.has_roll())
    roll_ = from.roll_;
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(LocalGridPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(LocalGridPrefs, speedRing_, speedring, SpeedRing);
SIMDATA_DEFINE_SUBFIELD_LIST(LocalGridPrefs, gridSettings_, gridsettings, GridSettings);
SIMDATA_DEFINE_SUBFIELD_LIST(LocalGridPrefs, gridPositionOffset_, gridpositionoffset, Position);
SIMDATA_DEFINE_SUBFIELD_LIST(LocalGridPrefs, gridOrientationOffset_, gridorientationoffset, BodyOrientation);
SIMDATA_DEFINE_FIELD(LocalGridPrefs, gridType_, gridtype, LocalGridPrefs::Type, LocalGridPrefs::Type::POLAR);
SIMDATA_DEFINE_FIELD(LocalGridPrefs, gridLabelDraw_, gridlabeldraw, bool, true);
SIMDATA_DEFINE_FIELD(LocalGridPrefs, gridLabelColor_, gridlabelcolor, uint32_t, 0xFFFF00FF);
SIMDATA_DEFINE_FIELD(LocalGridPrefs, gridLabelTextOutline_, gridlabeltextoutline, TextOutline, TextOutline::TO_THIN);
SIMDATA_DEFINE_FIELD(LocalGridPrefs, gridLabelOutlineColor_, gridlabeloutlinecolor, uint32_t, 255);
SIMDATA_DEFINE_FIELD_CONST_REF(LocalGridPrefs, gridLabelFontName_, gridlabelfontname, std::string, "arialbd.ttf");
SIMDATA_DEFINE_FIELD(LocalGridPrefs, gridLabelFontSize_, gridlabelfontsize, uint32_t, 14);
SIMDATA_DEFINE_FIELD(LocalGridPrefs, gridLabelPrecision_, gridlabelprecision, int32_t, 1);
SIMDATA_DEFINE_FIELD(LocalGridPrefs, drawGrid_, drawgrid, bool, false);
SIMDATA_DEFINE_FIELD(LocalGridPrefs, gridColor_, gridcolor, uint32_t, 0xFFFF00FF);
SIMDATA_DEFINE_FIELD(LocalGridPrefs, size_, size, double, 20.0);
SIMDATA_DEFINE_FIELD(LocalGridPrefs, positionOffsetUnits_, positionoffsetunits, DistanceUnits, DistanceUnits::UNITS_METERS);
SIMDATA_DEFINE_FIELD(LocalGridPrefs, followYaw_, followyaw, bool, true);
SIMDATA_DEFINE_FIELD(LocalGridPrefs, followPitch_, followpitch, bool, false);
SIMDATA_DEFINE_FIELD(LocalGridPrefs, followRoll_, followroll, bool, false);
SIMDATA_DEFINE_FIELD(LocalGridPrefs, sizeUnits_, sizeunits, DistanceUnits, DistanceUnits::UNITS_NAUTICAL_MILES);

void LocalGridPrefs::MergeFrom(const LocalGridPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(speedRing_, SpeedRing, speedring);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(gridSettings_, GridSettings, gridsettings);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(gridPositionOffset_, Position, gridpositionoffset);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(gridOrientationOffset_, BodyOrientation, gridorientationoffset);

  if (from.has_gridtype())
    gridType_ = from.gridType_;

  if (from.has_gridlabeldraw())
    gridLabelDraw_ = from.gridLabelDraw_;

  if (from.has_gridlabelcolor())
    gridLabelColor_ = from.gridLabelColor_;

  if (from.has_gridlabeltextoutline())
    gridLabelTextOutline_ = from.gridLabelTextOutline_;

  if (from.has_gridlabeloutlinecolor())
    gridLabelOutlineColor_ = from.gridLabelOutlineColor_;

  if (from.has_gridlabelfontname())
    gridLabelFontName_ = from.gridLabelFontName_;

  if (from.has_gridlabelfontsize())
    gridLabelFontSize_ = from.gridLabelFontSize_;

  if (from.has_gridlabelprecision())
    gridLabelPrecision_ = from.gridLabelPrecision_;

  if (from.has_drawgrid())
    drawGrid_ = from.drawGrid_;

  if (from.has_gridcolor())
    gridColor_ = from.gridColor_;

  if (from.has_size())
    size_ = from.size_;

  if (from.has_positionoffsetunits())
    positionOffsetUnits_ = from.positionOffsetUnits_;

  if (from.has_followyaw())
    followYaw_ = from.followYaw_;

  if (from.has_followpitch())
    followPitch_ = from.followPitch_;

  if (from.has_followroll())
    followRoll_ = from.followRoll_;

  if (from.has_sizeunits())
    sizeUnits_ = from.sizeUnits_;
}

void LocalGridPrefs::CopyFrom(const LocalGridPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(speedRing_, SpeedRing, speedring);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(gridSettings_, GridSettings, gridsettings);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(gridPositionOffset_, Position, gridpositionoffset);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(gridOrientationOffset_, BodyOrientation, gridorientationoffset);

  gridType_ = from.gridType_;
  gridLabelDraw_ = from.gridLabelDraw_;
  gridLabelColor_ = from.gridLabelColor_;
  gridLabelTextOutline_ = from.gridLabelTextOutline_;
  gridLabelOutlineColor_ = from.gridLabelOutlineColor_;
  gridLabelFontName_ = from.gridLabelFontName_;
  gridLabelFontSize_ = from.gridLabelFontSize_;
  gridLabelPrecision_ = from.gridLabelPrecision_;
  drawGrid_ = from.drawGrid_;
  gridColor_ = from.gridColor_;
  size_ = from.size_;
  positionOffsetUnits_ = from.positionOffsetUnits_;
  followYaw_ = from.followYaw_;
  followPitch_ = from.followPitch_;
  followRoll_ = from.followRoll_;
  sizeUnits_ = from.sizeUnits_;
}

bool LocalGridPrefs::operator==(const LocalGridPrefs& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(speedRing_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(gridSettings_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(gridPositionOffset_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(gridOrientationOffset_, rhs);

  return ((gridType_ == rhs.gridType_) &&
    (gridLabelDraw_ == rhs.gridLabelDraw_) &&
    (gridLabelColor_ == rhs.gridLabelColor_) &&
    (gridLabelTextOutline_ == rhs.gridLabelTextOutline_) &&
    (gridLabelOutlineColor_ == rhs.gridLabelOutlineColor_) &&
    (gridLabelFontName_ == rhs.gridLabelFontName_) &&
    (gridLabelFontSize_ == rhs.gridLabelFontSize_) &&
    (gridLabelPrecision_ == rhs.gridLabelPrecision_) &&
    (drawGrid_ == rhs.drawGrid_) &&
    (gridColor_ == rhs.gridColor_) &&
    (size_ == rhs.size_) &&
    (positionOffsetUnits_ == rhs.positionOffsetUnits_) &&
    (followYaw_ == rhs.followYaw_) &&
    (followPitch_ == rhs.followPitch_) &&
    (followRoll_ == rhs.followRoll_) &&
    (sizeUnits_ == rhs.sizeUnits_));
}

void LocalGridPrefs::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(speedRing_);
  SIMDATA_SUBFIELD_LIST_PRUNE(gridSettings_);
  SIMDATA_SUBFIELD_LIST_PRUNE(gridPositionOffset_);
  SIMDATA_SUBFIELD_LIST_PRUNE(gridOrientationOffset_);
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(CommonPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(CommonPrefs, labelPrefs_, labelprefs, LabelPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(CommonPrefs, localGrid_, localgrid, LocalGridPrefs);
SIMDATA_DEFINE_FIELD(CommonPrefs, dataDraw_, datadraw, bool, true);
SIMDATA_DEFINE_FIELD(CommonPrefs, draw_, draw, bool, true);
SIMDATA_DEFINE_FIELD_CONST_REF(CommonPrefs, name_, name, std::string, "entity");
SIMDATA_DEFINE_FIELD(CommonPrefs, useAlias_, usealias, bool, false);
SIMDATA_DEFINE_FIELD_CONST_REF(CommonPrefs, alias_, alias, std::string, "");
SIMDATA_DEFINE_FIELD(CommonPrefs, color_, color, uint32_t, 0xFFFF00FF);
SIMDATA_DEFINE_FIELD(CommonPrefs, useOverrideColor_, useoverridecolor, bool, false);
SIMDATA_DEFINE_FIELD(CommonPrefs, overrideColor_, overridecolor, uint32_t, 0xFF0000FF);
SIMDATA_DEFINE_FIELD(CommonPrefs, dataLimitTime_, datalimittime, double, -1.0);
SIMDATA_DEFINE_FIELD(CommonPrefs, dataLimitPoints_, datalimitpoints, uint32_t, 1000);
SIMDATA_DEFINE_FIELD(CommonPrefs, includeInLegend_, includeinlegend, bool, false);
SIMDATA_DEFINE_VECTOR_FIELD(CommonPrefs, acceptProjectorIds_, acceptprojectorids, uint64_t);

void CommonPrefs::MergeFrom(const CommonPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(labelPrefs_, LabelPrefs, labelprefs);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(localGrid_, LocalGridPrefs, localgrid);

  if (from.has_datadraw())
    dataDraw_ = from.dataDraw_;

  if (from.has_draw())
    draw_ = from.draw_;

  if (from.has_name())
    name_ = from.name_;

  if (from.has_usealias())
    useAlias_ = from.useAlias_;

  if (from.has_alias())
    alias_ = from.alias_;

  if (from.has_color())
    color_ = from.color_;

  if (from.has_useoverridecolor())
    useOverrideColor_ = from.useOverrideColor_;

  if (from.has_overridecolor())
    overrideColor_ = from.overrideColor_;

  if (from.has_datalimittime())
    dataLimitTime_ = from.dataLimitTime_;

  if (from.has_datalimitpoints())
    dataLimitPoints_ = from.dataLimitPoints_;

  if (from.has_includeinlegend())
    includeInLegend_ = from.includeInLegend_;

  acceptProjectorIds_.insert(acceptProjectorIds_.end(), from.acceptProjectorIds_.begin(), from.acceptProjectorIds_.end());
}

void CommonPrefs::CopyFrom(const CommonPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(labelPrefs_, LabelPrefs, labelprefs);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(localGrid_, LocalGridPrefs, localgrid);

  dataDraw_ = from.dataDraw_;
  draw_ = from.draw_;
  name_ = from.name_;
  useAlias_ = from.useAlias_;
  alias_ = from.alias_;
  color_ = from.color_;
  useOverrideColor_ = from.useOverrideColor_;
  overrideColor_ = from.overrideColor_;
  dataLimitTime_ = from.dataLimitTime_;
  dataLimitPoints_ = from.dataLimitPoints_;
  includeInLegend_ = from.includeInLegend_;
  acceptProjectorIds_ = from.acceptProjectorIds_;
}
bool CommonPrefs::operator==(const CommonPrefs& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(labelPrefs_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(localGrid_, rhs);

  return ((dataDraw_ == rhs.dataDraw_) &&
    (draw_ == rhs.draw_) &&
    (name_ == rhs.name_) &&
    (useAlias_ == rhs.useAlias_) &&
    (alias_ == rhs.alias_) &&
    (color_ == rhs.color_) &&
    (useOverrideColor_ == rhs.useOverrideColor_) &&
    (overrideColor_ == rhs.overrideColor_) &&
    (dataLimitTime_ == rhs.dataLimitTime_) &&
    (dataLimitPoints_ == rhs.dataLimitPoints_) &&
    (includeInLegend_ == rhs.includeInLegend_) &&
    (acceptProjectorIds_ == rhs.acceptProjectorIds_));
}

void CommonPrefs::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(labelPrefs_);
  SIMDATA_SUBFIELD_LIST_PRUNE(localGrid_);
}

}
