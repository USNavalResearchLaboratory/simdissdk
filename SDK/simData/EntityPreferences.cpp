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

#include "EntityPreferences.h"

namespace simData
{

SIMDATA_DEFINE_METHODS(CustomRenderingPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(CustomRenderingPrefs, commonPrefs_, commonprefs, CommonPrefs);
SIMDATA_DEFINE_FIELD(CustomRenderingPrefs, persistence_, persistence, double, 5.0);
SIMDATA_DEFINE_FIELD(CustomRenderingPrefs, secondsHistory_, secondshistory, double, 5.0);
SIMDATA_DEFINE_FIELD(CustomRenderingPrefs, pointsHistory_, pointshistory, uint32_t, 0);
SIMDATA_DEFINE_FIELD(CustomRenderingPrefs, outline_, outline, bool, false);
SIMDATA_DEFINE_FIELD(CustomRenderingPrefs, useHistoryOverrideColor_, usehistoryoverridecolor, bool, false);
SIMDATA_DEFINE_FIELD(CustomRenderingPrefs, historyOverrideColor_, historyoverridecolor, uint32_t, 0x19E500FF); // Green
SIMDATA_DEFINE_FIELD(CustomRenderingPrefs, centerAxis_, centeraxis, bool, false);
SIMDATA_DEFINE_FIELD(CustomRenderingPrefs, showLighted_, showlighted, bool, false);
SIMDATA_DEFINE_FIELD(CustomRenderingPrefs, depthTest_, depthtest, bool, true);

void CustomRenderingPrefs::MergeFrom(const CustomRenderingPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(commonPrefs_, CommonPrefs, commonprefs);

  if (from.has_persistence())
    persistence_ = from.persistence_;

  if (from.has_secondshistory())
    secondsHistory_ = from.secondsHistory_;

  if (from.has_pointshistory())
    pointsHistory_ = from.pointsHistory_;

  if (from.has_outline())
    outline_ = from.outline_;

  if (from.has_usehistoryoverridecolor())
    useHistoryOverrideColor_ = from.useHistoryOverrideColor_;

  if (from.has_historyoverridecolor())
    historyOverrideColor_ = from.historyOverrideColor_;

  if (from.has_centeraxis())
    centerAxis_ = from.centerAxis_;

  if (from.has_showlighted())
    showLighted_ = from.showLighted_;

  if (from.has_depthtest())
    depthTest_ = from.depthTest_;
}

void CustomRenderingPrefs::CopyFrom(const CustomRenderingPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(commonPrefs_, CommonPrefs, commonprefs);

  persistence_ = from.persistence_;
  secondsHistory_ = from.secondsHistory_;
  pointsHistory_ = from.pointsHistory_;
  outline_ = from.outline_;
  useHistoryOverrideColor_ = from.useHistoryOverrideColor_;
  historyOverrideColor_ = from.historyOverrideColor_;
  centerAxis_ = from.centerAxis_;
  showLighted_ = from.showLighted_;
  depthTest_ = from.depthTest_;
}

bool CustomRenderingPrefs::operator==(const CustomRenderingPrefs& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(commonPrefs_, rhs);

  return ((persistence_ == rhs.persistence_) &&
    (secondsHistory_ == rhs.secondsHistory_) &&
    (pointsHistory_ == rhs.pointsHistory_) &&
    (outline_ == rhs.outline_) &&
    (useHistoryOverrideColor_ == rhs.useHistoryOverrideColor_) &&
    (historyOverrideColor_ == rhs.historyOverrideColor_) &&
    (centerAxis_ == rhs.centerAxis_) &&
    (showLighted_ == rhs.showLighted_) &&
    (depthTest_ == rhs.depthTest_));
}

void CustomRenderingPrefs::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(commonPrefs_);
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(ProjectorPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(ProjectorPrefs, commonPrefs_, commonprefs, CommonPrefs);
SIMDATA_DEFINE_FIELD_CONST_REF(ProjectorPrefs, rasterFile_, rasterfile, std::string, "");
SIMDATA_DEFINE_FIELD(ProjectorPrefs, showFrustum_, showfrustum, bool, false);
SIMDATA_DEFINE_FIELD(ProjectorPrefs, projectorAlpha_, projectoralpha, float, 1.0);
SIMDATA_DEFINE_FIELD(ProjectorPrefs, interpolateProjectorFov_, interpolateprojectorfov, bool, true);
SIMDATA_DEFINE_FIELD(ProjectorPrefs, overrideFov_, overridefov, bool, false);
SIMDATA_DEFINE_FIELD(ProjectorPrefs, overrideFovAngle_, overridefovangle, double, 0.174533);
SIMDATA_DEFINE_FIELD(ProjectorPrefs, overrideHFov_, overridehfov, bool, false);
SIMDATA_DEFINE_FIELD(ProjectorPrefs, overrideHFovAngle_, overridehfovangle, double, 0.174533);
SIMDATA_DEFINE_FIELD(ProjectorPrefs, shadowMapping_, shadowmapping, bool, false);
SIMDATA_DEFINE_FIELD(ProjectorPrefs, maxDrawRange_, maxdrawrange, float, 0.0);
SIMDATA_DEFINE_FIELD(ProjectorPrefs, doubleSided_, doublesided, bool, false);

void ProjectorPrefs::MergeFrom(const ProjectorPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(commonPrefs_, CommonPrefs, commonprefs);

  if (from.has_rasterfile())
    rasterFile_ = from.rasterFile_;

  if (from.has_showfrustum())
    showFrustum_ = from.showFrustum_;

  if (from.has_projectoralpha())
    projectorAlpha_ = from.projectorAlpha_;

  if (from.has_interpolateprojectorfov())
    interpolateProjectorFov_ = from.interpolateProjectorFov_;

  if (from.has_overridefov())
    overrideFov_ = from.overrideFov_;

  if (from.has_overridefovangle())
    overrideFovAngle_ = from.overrideFovAngle_;

  if (from.has_overridehfov())
    overrideHFov_ = from.overrideHFov_;

  if (from.has_overridehfovangle())
    overrideHFovAngle_ = from.overrideHFovAngle_;

  if (from.has_shadowmapping())
    shadowMapping_ = from.shadowMapping_;

  if (from.has_maxdrawrange())
    maxDrawRange_ = from.maxDrawRange_;

  if (from.has_doublesided())
    doubleSided_ = from.doubleSided_;
}

void ProjectorPrefs::CopyFrom(const ProjectorPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(commonPrefs_, CommonPrefs, commonprefs);

  rasterFile_ = from.rasterFile_;
  showFrustum_ = from.showFrustum_;
  projectorAlpha_ = from.projectorAlpha_;
  interpolateProjectorFov_ = from.interpolateProjectorFov_;
  overrideFov_ = from.overrideFov_;
  overrideFovAngle_ = from.overrideFovAngle_;
  overrideHFov_ = from.overrideHFov_;
  overrideHFovAngle_ = from.overrideHFovAngle_;
  shadowMapping_ = from.shadowMapping_;
  maxDrawRange_ = from.maxDrawRange_;
  doubleSided_ = from.doubleSided_;
}

bool ProjectorPrefs::operator==(const ProjectorPrefs& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(commonPrefs_, rhs);

  return ((rasterFile_ == rhs.rasterFile_) &&
    (showFrustum_ == rhs.showFrustum_) &&
    (projectorAlpha_ == rhs.projectorAlpha_) &&
    (interpolateProjectorFov_ == rhs.interpolateProjectorFov_) &&
    (overrideFov_ == rhs.overrideFov_) &&
    (overrideFovAngle_ == rhs.overrideFovAngle_) &&
    (overrideHFov_ == rhs.overrideHFov_) &&
    (overrideHFovAngle_ == rhs.overrideHFovAngle_) &&
    (shadowMapping_ == rhs.shadowMapping_) &&
    (maxDrawRange_ == rhs.maxDrawRange_) &&
    (doubleSided_ == rhs.doubleSided_));
}

void ProjectorPrefs::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(commonPrefs_);
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(LaserPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(LaserPrefs, commonPrefs_, commonprefs, CommonPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(LaserPrefs, laserXyzOffset_, laserxyzoffset, Position);
SIMDATA_DEFINE_FIELD(LaserPrefs, maxRange_, maxrange, double, 1000000.0);
SIMDATA_DEFINE_FIELD(LaserPrefs, laserWidth_, laserwidth, int32_t, 1);

void LaserPrefs::MergeFrom(const LaserPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(commonPrefs_, CommonPrefs, commonprefs);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(laserXyzOffset_, Position, laserxyzoffset);

  if (from.has_maxrange())
    maxRange_ = from.maxRange_;

  if (from.has_laserwidth())
    laserWidth_ = from.laserWidth_;

}

void LaserPrefs::CopyFrom(const LaserPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(commonPrefs_, CommonPrefs, commonprefs);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(laserXyzOffset_, Position, laserxyzoffset);

  maxRange_ = from.maxRange_;
  laserWidth_ = from.laserWidth_;
}

bool LaserPrefs::operator==(const LaserPrefs& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(commonPrefs_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(laserXyzOffset_, rhs);

  return ((maxRange_ == rhs.maxRange_) &&
    (laserWidth_ == rhs.laserWidth_));
}

void LaserPrefs::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(commonPrefs_);
  SIMDATA_SUBFIELD_LIST_PRUNE(laserXyzOffset_);
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(LobGroupPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(LobGroupPrefs, commonPrefs_, commonprefs, CommonPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(LobGroupPrefs, xyzOffset_, xyzoffset, Position); ///< Positional offset from the host platform
SIMDATA_DEFINE_FIELD(LobGroupPrefs, lobwidth_, lobwidth, int32_t, 2); ///< Width of the LOBs in pixels on the screen
SIMDATA_DEFINE_FIELD(LobGroupPrefs, color1_, color1, uint32_t, 0x00FF00FF); ///< Color of the first part of the stipple pattern; RRGGBBA, green
SIMDATA_DEFINE_FIELD(LobGroupPrefs, color2_, color2, uint32_t, 0xFF0000FF); ///< Color of the second part of the stipple pattern; RRGGBBA, red
SIMDATA_DEFINE_FIELD(LobGroupPrefs, stipple1_, stipple1, uint32_t, 0xFF00); ///< GL stipple pattern of color1; 0xffff for a solid line
SIMDATA_DEFINE_FIELD(LobGroupPrefs, stipple2_, stipple2, uint32_t, 0x00FF); ///< GL stipple pattern of color2; 0xffff for a solid line
SIMDATA_DEFINE_FIELD(LobGroupPrefs, maxDataSeconds_, maxdataseconds, double, 5.0); ///< the maximum number of seconds of history data to display
SIMDATA_DEFINE_FIELD(LobGroupPrefs, maxDataPoints_, maxdatapoints, uint32_t, 10); ///< the maximum number of data points to display
SIMDATA_DEFINE_FIELD(LobGroupPrefs, lobUseClampAlt_, lobuseclampalt, bool, false); ///< Clamp to ground if true, otherwise no clamping
SIMDATA_DEFINE_FIELD(LobGroupPrefs, useRangeOverride_, userangeoverride, bool, false); ///< Override the range indicated by data
SIMDATA_DEFINE_FIELD(LobGroupPrefs, rangeOverrideValue_, rangeoverridevalue, double, 1000.0); ///< Value to use for range (m) when overriding
SIMDATA_DEFINE_FIELD(LobGroupPrefs, bending_, bending, AnimatedLineBend, AnimatedLineBend::ALB_AUTO); ///< Line bending preference

void LobGroupPrefs::MergeFrom(const LobGroupPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(commonPrefs_, CommonPrefs, commonprefs);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(xyzOffset_, Position, xyzoffset);

  if (from.has_lobwidth())
    lobwidth_ = from.lobwidth_;

  if (from.has_color1())
    color1_ = from.color1_;

  if (from.has_color2())
    color2_ = from.color2_;

  if (from.has_stipple1())
    stipple1_ = from.stipple1_;

  if (from.has_stipple2())
    stipple2_ = from.stipple2_;

  if (from.has_maxdataseconds())
    maxDataSeconds_ = from.maxDataSeconds_;

  if (from.has_maxdatapoints())
    maxDataPoints_ = from.maxDataPoints_;

  if (from.has_lobuseclampalt())
    lobUseClampAlt_ = from.lobUseClampAlt_;

  if (from.has_userangeoverride())
    useRangeOverride_ = from.useRangeOverride_;

  if (from.has_rangeoverridevalue())
    rangeOverrideValue_ = from.rangeOverrideValue_;

  if (from.has_bending())
    bending_ = from.bending_;
}

void LobGroupPrefs::CopyFrom(const LobGroupPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(commonPrefs_, CommonPrefs, commonprefs);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(xyzOffset_, Position, xyzoffset);

  lobwidth_ = from.lobwidth_;
  color1_ = from.color1_;
  color2_ = from.color2_;
  stipple1_ = from.stipple1_;
  stipple2_ = from.stipple2_;
  maxDataSeconds_ = from.maxDataSeconds_;
  maxDataPoints_ = from.maxDataPoints_;
  lobUseClampAlt_ = from.lobUseClampAlt_;
  useRangeOverride_ = from.useRangeOverride_;
  rangeOverrideValue_ = from.rangeOverrideValue_;
  bending_ = from.bending_;
}

bool LobGroupPrefs::operator==(const LobGroupPrefs& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(commonPrefs_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(xyzOffset_, rhs);

  return ((lobwidth_ == rhs.lobwidth_) &&
    (color1_ == rhs.color1_) &&
    (color2_ == rhs.color2_) &&
    (stipple1_ == rhs.stipple1_) &&
    (stipple2_ == rhs.stipple2_) &&
    (maxDataSeconds_ == rhs.maxDataSeconds_) &&
    (maxDataPoints_ == rhs.maxDataPoints_) &&
    (lobUseClampAlt_ == rhs.lobUseClampAlt_) &&
    (useRangeOverride_ == rhs.useRangeOverride_) &&
    (rangeOverrideValue_ == rhs.rangeOverrideValue_) &&
    (bending_ == rhs.bending_));
}

void LobGroupPrefs::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(commonPrefs_);
  SIMDATA_SUBFIELD_LIST_PRUNE(xyzOffset_);
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(GatePrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(GatePrefs, commonPrefs_, commonprefs, CommonPrefs);
SIMDATA_DEFINE_FIELD(GatePrefs, gateLighting_, gatelighting, bool, false);
SIMDATA_DEFINE_FIELD(GatePrefs, gateBlending_, gateblending, bool, true);
SIMDATA_DEFINE_FIELD(GatePrefs, gateDrawMode_, gatedrawmode, GatePrefs::DrawMode, GatePrefs::DrawMode::UNKNOWN);
SIMDATA_DEFINE_FIELD(GatePrefs, fillPattern_, fillpattern, GatePrefs::FillPattern, GatePrefs::FillPattern::STIPPLE);
SIMDATA_DEFINE_FIELD(GatePrefs, drawCentroid_, drawcentroid, bool, true);
SIMDATA_DEFINE_FIELD(GatePrefs, interpolateGatePos_, interpolategatepos, bool, true);
SIMDATA_DEFINE_FIELD(GatePrefs, gateAzimuthOffset_, gateazimuthoffset, double, 0.0);
SIMDATA_DEFINE_FIELD(GatePrefs, gateElevationOffset_, gateelevationoffset, double, 0.0);
SIMDATA_DEFINE_FIELD(GatePrefs, gateRollOffset_, gaterolloffset, double, 0.0);
SIMDATA_DEFINE_FIELD(GatePrefs, drawOutline_, drawoutline, bool, true);
SIMDATA_DEFINE_FIELD(GatePrefs, centroidColor_, centroidcolor, uint32_t, 0xFFFFFFFF); // White

void GatePrefs::MergeFrom(const GatePrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(commonPrefs_, CommonPrefs, commonprefs);

  if (from.has_gatelighting())
    gateLighting_ = from.gateLighting_;

  if (from.has_gateblending())
    gateBlending_ = from.gateBlending_;

  if (from.has_gatedrawmode())
    gateDrawMode_ = from.gateDrawMode_;

  if (from.has_fillpattern())
    fillPattern_ = from.fillPattern_;

  if (from.has_drawcentroid())
    drawCentroid_ = from.drawCentroid_;

  if (from.has_interpolategatepos())
    interpolateGatePos_ = from.interpolateGatePos_;

  if (from.has_gateazimuthoffset())
    gateAzimuthOffset_ = from.gateAzimuthOffset_;

  if (from.has_gateelevationoffset())
    gateElevationOffset_ = from.gateElevationOffset_;

  if (from.has_gaterolloffset())
    gateRollOffset_ = from.gateRollOffset_;

  if (from.has_drawoutline())
    drawOutline_ = from.drawOutline_;

  if (from.has_centroidcolor())
    centroidColor_ = from.centroidColor_;
}

void GatePrefs::CopyFrom(const GatePrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(commonPrefs_, CommonPrefs, commonprefs);

  gateLighting_ = from.gateLighting_;
  gateBlending_ = from.gateBlending_;
  gateDrawMode_ = from.gateDrawMode_;
  fillPattern_ = from.fillPattern_;
  drawCentroid_ = from.drawCentroid_;
  interpolateGatePos_ = from.interpolateGatePos_;
  gateAzimuthOffset_ = from.gateAzimuthOffset_;
  gateElevationOffset_ = from.gateElevationOffset_;
  gateRollOffset_ = from.gateRollOffset_;
  drawOutline_ = from.drawOutline_;
  centroidColor_ = from.centroidColor_;
}

bool GatePrefs::operator==(const GatePrefs& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(commonPrefs_, rhs);

  return ((gateLighting_ == rhs.gateLighting_) &&
    (gateBlending_ == rhs.gateBlending_) &&
    (gateDrawMode_ == rhs.gateDrawMode_) &&
    (fillPattern_ == rhs.fillPattern_) &&
    (drawCentroid_ == rhs.drawCentroid_) &&
    (interpolateGatePos_ == rhs.interpolateGatePos_) &&
    (gateAzimuthOffset_ == rhs.gateAzimuthOffset_) &&
    (gateElevationOffset_ == rhs.gateElevationOffset_) &&
    (gateRollOffset_ == rhs.gateRollOffset_) &&
    (drawOutline_ == rhs.drawOutline_) &&
    (centroidColor_ == rhs.centroidColor_));
}

void GatePrefs::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(commonPrefs_);
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_DEFAULT_METHODS(AntennaPatterns);
SIMDATA_DEFINE_FIELD(AntennaPatterns, type_, type, AntennaPatterns::Type, AntennaPatterns::Type::ALGORITHM);
SIMDATA_DEFINE_FIELD(AntennaPatterns, fileFormat_, fileformat, AntennaPatterns::FileFormat, AntennaPatterns::FileFormat::TABLE);
SIMDATA_DEFINE_FIELD_CONST_REF(AntennaPatterns, fileName_, filename, std::string, "");
SIMDATA_DEFINE_FIELD(AntennaPatterns, algorithm_, algorithm, AntennaPatterns::Algorithm, AntennaPatterns::Algorithm::PEDESTAL);

void AntennaPatterns::MergeFrom(const AntennaPatterns& from)
{
  if (&from == this)
    return;

  if (from.has_type())
    type_ = from.type_;

  if (from.has_fileformat())
    fileFormat_ = from.fileFormat_;

  if (from.has_filename())
    fileName_ = from.fileName_;

  if (from.has_algorithm())
    algorithm_ = from.algorithm_;
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(BeamPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(BeamPrefs, commonPrefs_, commonprefs, CommonPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(BeamPrefs, antennaPattern_, antennapattern, AntennaPatterns);
SIMDATA_DEFINE_SUBFIELD_LIST(BeamPrefs, beamPositionOffset_, beampositionoffset, Position);
SIMDATA_DEFINE_FIELD(BeamPrefs, shaded_, shaded, bool, false);
SIMDATA_DEFINE_FIELD(BeamPrefs, blended_, blended, bool, true);
SIMDATA_DEFINE_FIELD(BeamPrefs, beamDrawMode_, beamdrawmode, BeamPrefs::DrawMode, BeamPrefs::DrawMode::SOLID);
SIMDATA_DEFINE_FIELD(BeamPrefs, beamScale_, beamscale, double, 1.0);
SIMDATA_DEFINE_FIELD(BeamPrefs, drawType_, drawtype, BeamPrefs::DrawType, BeamPrefs::DrawType::BEAM_3DB);
SIMDATA_DEFINE_FIELD(BeamPrefs, capResolution_, capresolution, uint32_t, 5);
SIMDATA_DEFINE_FIELD(BeamPrefs, coneResolution_, coneresolution, uint32_t, 30);
SIMDATA_DEFINE_FIELD(BeamPrefs, renderCone_, rendercone, bool, true);
SIMDATA_DEFINE_FIELD(BeamPrefs, sensitivity_, sensitivity, double, -50.0);
SIMDATA_DEFINE_FIELD(BeamPrefs, gain_, gain, double, 20.0);
SIMDATA_DEFINE_FIELD(BeamPrefs, fieldOfView_, fieldofview, double, 1.5707963267);
SIMDATA_DEFINE_FIELD(BeamPrefs, detail_, detail, double, 1.0);
SIMDATA_DEFINE_FIELD(BeamPrefs, power_, power, double, 0.0);
SIMDATA_DEFINE_FIELD(BeamPrefs, frequency_, frequency, double, 0.0);
SIMDATA_DEFINE_FIELD(BeamPrefs, polarity_, polarity, Polarity, Polarity::POL_UNKNOWN);
SIMDATA_DEFINE_FIELD(BeamPrefs, colorScale_, colorscale, bool, false);
SIMDATA_DEFINE_FIELD_CONST_REF(BeamPrefs, arepsFile_, arepsfile, std::string, "");
SIMDATA_DEFINE_FIELD(BeamPrefs, channel_, channel, bool, false);
SIMDATA_DEFINE_FIELD(BeamPrefs, weighting_, weighting, bool, true);
SIMDATA_DEFINE_FIELD(BeamPrefs, interpolateBeamPos_, interpolatebeampos, bool, true);
SIMDATA_DEFINE_FIELD(BeamPrefs, useOffsetPlatform_, useoffsetplatform, bool, true);
SIMDATA_DEFINE_FIELD(BeamPrefs, useOffsetIcon_, useoffseticon, bool, false);
SIMDATA_DEFINE_FIELD(BeamPrefs, useOffsetBeam_, useoffsetbeam, bool, false);
SIMDATA_DEFINE_FIELD(BeamPrefs, azimuthOffset_, azimuthoffset, double, 0.0);
SIMDATA_DEFINE_FIELD(BeamPrefs, elevationOffset_, elevationoffset, double, 0.0);
SIMDATA_DEFINE_FIELD(BeamPrefs, rollOffset_, rolloffset, double, 0.0);
SIMDATA_DEFINE_FIELD(BeamPrefs, targetId_, targetid, uint64_t, 0);
SIMDATA_DEFINE_FIELD(BeamPrefs, verticalWidth_, verticalwidth, double, 0.0);
SIMDATA_DEFINE_FIELD(BeamPrefs, horizontalWidth_, horizontalwidth, double, 0.0);
SIMDATA_DEFINE_FIELD(BeamPrefs, animate_, animate, bool, false);
SIMDATA_DEFINE_FIELD(BeamPrefs, pulseLength_, pulselength, double, 100.0);
SIMDATA_DEFINE_FIELD(BeamPrefs, pulseRate_, pulserate, double, 1.0);
SIMDATA_DEFINE_FIELD(BeamPrefs, pulseStipple_, pulsestipple, uint32_t, 0x0F0F);

void BeamPrefs::MergeFrom(const BeamPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(commonPrefs_, CommonPrefs, commonprefs);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(antennaPattern_, AntennaPatterns, antennapattern);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(beamPositionOffset_, Position, beampositionoffset);

  if (from.has_shaded())
    shaded_ = from.shaded_;

  if (from.has_blended())
    blended_ = from.blended_;

  if (from.has_beamdrawmode())
    beamDrawMode_ = from.beamDrawMode_;

  if (from.has_beamscale())
    beamScale_ = from.beamScale_;

  if (from.has_drawtype())
    drawType_ = from.drawType_;

  if (from.has_capresolution())
    capResolution_ = from.capResolution_;

  if (from.has_coneresolution())
    coneResolution_ = from.coneResolution_;

  if (from.has_rendercone())
    renderCone_ = from.renderCone_;

  if (from.has_sensitivity())
    sensitivity_ = from.sensitivity_;

  if (from.has_gain())
    gain_ = from.gain_;

  if (from.has_fieldofview())
    fieldOfView_ = from.fieldOfView_;

  if (from.has_detail())
    detail_ = from.detail_;

  if (from.has_power())
    power_ = from.power_;

  if (from.has_frequency())
    frequency_ = from.frequency_;

  if (from.has_polarity())
    polarity_ = from.polarity_;

  if (from.has_colorscale())
    colorScale_ = from.colorScale_;

  if (from.has_arepsfile())
    arepsFile_ = from.arepsFile_;

  if (from.has_channel())
    channel_ = from.channel_;

  if (from.has_weighting())
    weighting_ = from.weighting_;

  if (from.has_interpolatebeampos())
    interpolateBeamPos_ = from.interpolateBeamPos_;

  if (from.has_useoffsetplatform())
    useOffsetPlatform_ = from.useOffsetPlatform_;

  if (from.has_useoffseticon())
    useOffsetIcon_ = from.useOffsetIcon_;

  if (from.has_useoffsetbeam())
    useOffsetBeam_ = from.useOffsetBeam_;

  if (from.has_azimuthoffset())
    azimuthOffset_ = from.azimuthOffset_;

  if (from.has_elevationoffset())
    elevationOffset_ = from.elevationOffset_;

  if (from.has_rolloffset())
    rollOffset_ = from.rollOffset_;

  if (from.has_targetid())
    targetId_ = from.targetId_;

  if (from.has_verticalwidth())
    verticalWidth_ = from.verticalWidth_;

  if (from.has_horizontalwidth())
    horizontalWidth_ = from.horizontalWidth_;

  if (from.has_animate())
    animate_ = from.animate_;

  if (from.has_pulselength())
    pulseLength_ = from.pulseLength_;

  if (from.has_pulserate())
    pulseRate_ = from.pulseRate_;

  if (from.has_pulsestipple())
    pulseStipple_ = from.pulseStipple_;
}

void BeamPrefs::CopyFrom(const BeamPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(commonPrefs_, CommonPrefs, commonprefs);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(antennaPattern_, AntennaPatterns, antennapattern);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(beamPositionOffset_, Position, beampositionoffset);

  shaded_ = from.shaded_;
  blended_ = from.blended_;
  beamDrawMode_ = from.beamDrawMode_;
  beamScale_ = from.beamScale_;
  drawType_ = from.drawType_;
  capResolution_ = from.capResolution_;
  coneResolution_ = from.coneResolution_;
  renderCone_ = from.renderCone_;
  sensitivity_ = from.sensitivity_;
  gain_ = from.gain_;
  fieldOfView_ = from.fieldOfView_;
  detail_ = from.detail_;
  power_ = from.power_;
  frequency_ = from.frequency_;
  polarity_ = from.polarity_;
  colorScale_ = from.colorScale_;
  arepsFile_ = from.arepsFile_;
  channel_ = from.channel_;
  weighting_ = from.weighting_;
  interpolateBeamPos_ = from.interpolateBeamPos_;
  useOffsetPlatform_ = from.useOffsetPlatform_;
  useOffsetIcon_ = from.useOffsetIcon_;
  useOffsetBeam_ = from.useOffsetBeam_;
  azimuthOffset_ = from.azimuthOffset_;
  elevationOffset_ = from.elevationOffset_;
  rollOffset_ = from.rollOffset_;
  targetId_ = from.targetId_;
  verticalWidth_ = from.verticalWidth_;
  horizontalWidth_ = from.horizontalWidth_;
  animate_ = from.animate_;
  pulseLength_ = from.pulseLength_;
  pulseRate_ = from.pulseRate_;
  pulseStipple_ = from.pulseStipple_;
}

bool BeamPrefs::operator==(const BeamPrefs& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(commonPrefs_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(antennaPattern_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(beamPositionOffset_, rhs);

  return ((shaded_ == rhs.shaded_) &&
    (blended_ == rhs.blended_) &&
    (beamDrawMode_ == rhs.beamDrawMode_) &&
    (beamScale_ == rhs.beamScale_) &&
    (drawType_ == rhs.drawType_) &&
    (capResolution_ == rhs.capResolution_) &&
    (coneResolution_ == rhs.coneResolution_) &&
    (renderCone_ == rhs.renderCone_) &&
    (sensitivity_ == rhs.sensitivity_) &&
    (gain_ == rhs.gain_) &&
    (fieldOfView_ == rhs.fieldOfView_) &&
    (detail_ == rhs.detail_) &&
    (power_ == rhs.power_) &&
    (frequency_ == rhs.frequency_) &&
    (polarity_ == rhs.polarity_) &&
    (colorScale_ == rhs.colorScale_) &&
    (arepsFile_ == rhs.arepsFile_) &&
    (channel_ == rhs.channel_) &&
    (weighting_ == rhs.weighting_) &&
    (interpolateBeamPos_ == rhs.interpolateBeamPos_) &&
    (useOffsetPlatform_ == rhs.useOffsetPlatform_) &&
    (useOffsetIcon_ == rhs.useOffsetIcon_) &&
    (useOffsetBeam_ == rhs.useOffsetBeam_) &&
    (azimuthOffset_ == rhs.azimuthOffset_) &&
    (elevationOffset_ == rhs.elevationOffset_) &&
    (rollOffset_ == rhs.rollOffset_) &&
    (targetId_ == rhs.targetId_) &&
    (verticalWidth_ == rhs.verticalWidth_) &&
    (horizontalWidth_ == rhs.horizontalWidth_) &&
    (animate_ == rhs.animate_) &&
    (pulseLength_ == rhs.pulseLength_) &&
    (pulseRate_ == rhs.pulseRate_) &&
    (pulseStipple_ == rhs.pulseStipple_));
}

void BeamPrefs::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(commonPrefs_);
  SIMDATA_SUBFIELD_LIST_PRUNE(antennaPattern_);
  SIMDATA_SUBFIELD_LIST_PRUNE(beamPositionOffset_);
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_DEFAULT_METHODS(TimeTickPrefs);
SIMDATA_DEFINE_FIELD(TimeTickPrefs, drawStyle_, drawstyle, TimeTickPrefs::DrawStyle, TimeTickPrefs::DrawStyle::NONE);
SIMDATA_DEFINE_FIELD(TimeTickPrefs, color_, color, uint32_t, 0xFFFFFF99); ///< translucent white
SIMDATA_DEFINE_FIELD(TimeTickPrefs, interval_, interval, double, 10.0);
SIMDATA_DEFINE_FIELD(TimeTickPrefs, largeIntervalFactor_, largeintervalfactor, uint32_t, 6);
SIMDATA_DEFINE_FIELD(TimeTickPrefs, labelIntervalFactor_, labelintervalfactor, uint32_t, 6);
SIMDATA_DEFINE_FIELD_CONST_REF(TimeTickPrefs, labelFontName_, labelfontname, std::string, "arial.ttf");
SIMDATA_DEFINE_FIELD(TimeTickPrefs, labelFontPointSize_, labelfontpointsize, uint32_t, 12);
SIMDATA_DEFINE_FIELD(TimeTickPrefs, labelColor_, labelcolor, uint32_t, 0);
SIMDATA_DEFINE_FIELD(TimeTickPrefs, lineLength_, linelength, double, 40.0);
SIMDATA_DEFINE_FIELD(TimeTickPrefs, largeSizeFactor_, largesizefactor, uint32_t, 2);
SIMDATA_DEFINE_FIELD(TimeTickPrefs, labelTimeFormat_, labeltimeformat, ElapsedTimeFormat, ElapsedTimeFormat::ELAPSED_HOURS);
SIMDATA_DEFINE_FIELD(TimeTickPrefs, lineWidth_, linewidth, double, 2.0);

void TimeTickPrefs::MergeFrom(const TimeTickPrefs& from)
{
  if (&from == this)
    return;

  if (from.has_drawstyle())
    drawStyle_ = from.drawStyle_;

  if (from.has_color())
    color_ = from.color_;

  if (from.has_interval())
    interval_ = from.interval_;

  if (from.has_largeintervalfactor())
    largeIntervalFactor_ = from.largeIntervalFactor_;

  if (from.has_labelintervalfactor())
    labelIntervalFactor_ = from.labelIntervalFactor_;

  if (from.has_labelfontname())
    labelFontName_ = from.labelFontName_;

  if (from.has_labelfontpointsize())
    labelFontPointSize_ = from.labelFontPointSize_;

  if (from.has_labelcolor())
    labelColor_ = from.labelColor_;

  if (from.has_linelength())
    lineLength_ = from.lineLength_;

  if (from.has_largesizefactor())
    largeSizeFactor_ = from.largeSizeFactor_;

  if (from.has_labeltimeformat())
    labelTimeFormat_ = from.labelTimeFormat_;

  if (from.has_linewidth())
    lineWidth_ = from.lineWidth_;
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(TrackPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(TrackPrefs, timeTicks_, timeticks, TimeTickPrefs);
SIMDATA_DEFINE_FIELD(TrackPrefs, trackColor_, trackcolor, uint32_t, 0xFBFBFBFF); // Off-white
SIMDATA_DEFINE_FIELD(TrackPrefs, multiTrackColor_, multitrackcolor, bool, true);
SIMDATA_DEFINE_FIELD(TrackPrefs, flatMode_, flatmode, bool, false);
SIMDATA_DEFINE_FIELD(TrackPrefs, altMode_, altmode, bool, false);
SIMDATA_DEFINE_FIELD(TrackPrefs, expireMode_, expiremode, bool, false);
SIMDATA_DEFINE_FIELD(TrackPrefs, usePlatformColor_, useplatformcolor, bool, false);
SIMDATA_DEFINE_FIELD(TrackPrefs, useTrackOverrideColor_, usetrackoverridecolor, bool, false);
SIMDATA_DEFINE_FIELD(TrackPrefs, trackOverrideColor_, trackoverridecolor, uint32_t, 0x19E500FF); // Green
SIMDATA_DEFINE_FIELD(TrackPrefs, trackLength_, tracklength, int32_t, 60);
SIMDATA_DEFINE_FIELD(TrackPrefs, lineWidth_, linewidth, double, 1.0);
SIMDATA_DEFINE_FIELD(TrackPrefs, trackDrawMode_, trackdrawmode, TrackPrefs::Mode, TrackPrefs::Mode::OFF);

void TrackPrefs::MergeFrom(const TrackPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(timeTicks_, TimeTickPrefs, timeticks);

  if (from.has_trackcolor())
    trackColor_ = from.trackColor_;

  if (from.has_multitrackcolor())
    multiTrackColor_ = from.multiTrackColor_;

  if (from.has_flatmode())
    flatMode_ = from.flatMode_;

  if (from.has_altmode())
    altMode_ = from.altMode_;

  if (from.has_expiremode())
    expireMode_ = from.expireMode_;

  if (from.has_useplatformcolor())
    usePlatformColor_ = from.usePlatformColor_;

  if (from.has_usetrackoverridecolor())
    useTrackOverrideColor_ = from.useTrackOverrideColor_;

  if (from.has_trackoverridecolor())
    trackOverrideColor_ = from.trackOverrideColor_;

  if (from.has_tracklength())
    trackLength_ = from.trackLength_;

  if (from.has_linewidth())
    lineWidth_ = from.lineWidth_;

  if (from.has_trackdrawmode())
    trackDrawMode_ = from.trackDrawMode_;
}

void TrackPrefs::CopyFrom(const TrackPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(timeTicks_, TimeTickPrefs, timeticks);

  trackColor_ = from.trackColor_;
  multiTrackColor_ = from.multiTrackColor_;
  flatMode_ = from.flatMode_;
  altMode_ = from.altMode_;
  expireMode_ = from.expireMode_;
  usePlatformColor_ = from.usePlatformColor_;
  useTrackOverrideColor_ = from.useTrackOverrideColor_;
  trackOverrideColor_ = from.trackOverrideColor_;
  trackLength_ = from.trackLength_;
  lineWidth_ = from.lineWidth_;
  trackDrawMode_ = from.trackDrawMode_;
}

bool TrackPrefs::operator==(const TrackPrefs& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(timeTicks_, rhs);

  return ((trackColor_ == rhs.trackColor_) &&
    (multiTrackColor_ == rhs.multiTrackColor_) &&
    (flatMode_ == rhs.flatMode_) &&
    (altMode_ == rhs.altMode_) &&
    (expireMode_ == rhs.expireMode_) &&
    (usePlatformColor_ == rhs.usePlatformColor_) &&
    (useTrackOverrideColor_ == rhs.useTrackOverrideColor_) &&
    (trackOverrideColor_ == rhs.trackOverrideColor_) &&
    (trackLength_ == rhs.trackLength_) &&
    (lineWidth_ == rhs.lineWidth_) &&
    (trackDrawMode_ == rhs.trackDrawMode_));
}


void TrackPrefs::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(timeTicks_);
}

//----------------------------------------------------------------------------------------------------------

SIMDATA_DEFINE_METHODS(PlatformPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(PlatformPrefs, commonPrefs_, commonprefs, CommonPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(PlatformPrefs, trackPrefs_, trackprefs, TrackPrefs);
SIMDATA_DEFINE_SUBFIELD_LIST(PlatformPrefs, platPositionOffset_, platpositionoffset, Position);
SIMDATA_DEFINE_SUBFIELD_LIST(PlatformPrefs, orientationOffset_, orientationoffset, BodyOrientation);
SIMDATA_DEFINE_SUBFIELD_LIST(PlatformPrefs, scaleXYZ_, scalexyz, Position);
SIMDATA_DEFINE_FIELD_CONST_REF(PlatformPrefs, icon_, icon, std::string, "");
SIMDATA_DEFINE_FIELD(PlatformPrefs, drawMode_, drawmode, ModelDrawMode, ModelDrawMode::MDM_SOLID);
SIMDATA_DEFINE_FIELD(PlatformPrefs, fragmentEffect_, fragmenteffect, FragmentEffect, FragmentEffect::FE_NONE);
SIMDATA_DEFINE_FIELD(PlatformPrefs, fragmentEffectColor_, fragmenteffectcolor, uint32_t, 0xFFFFFFFF); // White
SIMDATA_DEFINE_FIELD(PlatformPrefs, rotateIcons_, rotateicons, IconRotation, IconRotation::IR_2D_YAW);
SIMDATA_DEFINE_FIELD(PlatformPrefs, noDepthIcons_, nodepthicons, bool, true);
SIMDATA_DEFINE_FIELD(PlatformPrefs, iconAlignment_, iconalignment, TextAlignment, TextAlignment::ALIGN_CENTER_CENTER);
SIMDATA_DEFINE_FIELD(PlatformPrefs, overrideColorCombineMode_, overridecolorcombinemode, OverrideColorCombineMode, OverrideColorCombineMode::MULTIPLY_COLOR);
SIMDATA_DEFINE_FIELD(PlatformPrefs, useClampAlt_, useclampalt, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, clampValAltMin_, clampvalaltmin, double, -100000.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, clampValAltMax_, clampvalaltmax, double, 1000000000.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, useClampYaw_, useclampyaw, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, clampValYaw_, clampvalyaw, double, 0.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, useClampPitch_, useclamppitch, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, clampValPitch_, clampvalpitch, double, 0.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, useClampRoll_, useclamproll, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, clampValRoll_, clampvalroll, double, 0.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, clampOrientationAtLowVelocity_, clamporientationatlowvelocity, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, surfaceClamping_, surfaceclamping, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, aboveSurfaceClamping_, abovesurfaceclamping, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, lighted_, lighted, bool, true);
SIMDATA_DEFINE_FIELD(PlatformPrefs, drawBox_, drawbox, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, drawBodyAxis_, drawbodyaxis, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, drawInertialAxis_, drawinertialaxis, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, drawSunVec_, drawsunvec, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, drawMoonVec_, drawmoonvec, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, axisScale_, axisscale, double, 1.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, wireFrame_, wireframe, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, drawOpticLos_, drawopticlos, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, drawRfLos_, drawrflos, bool, false);
SIMDATA_DEFINE_FIELD_CONST_REF(PlatformPrefs, rcsFile_, rcsfile, std::string, "");
SIMDATA_DEFINE_FIELD(PlatformPrefs, drawRcs_, drawrcs, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, draw3dRcs_, draw3drcs, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, rcsColor_, rcscolor, uint32_t, 0xFFFFFF80); // White
SIMDATA_DEFINE_FIELD(PlatformPrefs, rcsColorScale_, rcscolorscale, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, rcsPolarity_, rcspolarity, Polarity, Polarity::POL_UNKNOWN);
SIMDATA_DEFINE_FIELD(PlatformPrefs, rcsElevation_, rcselevation, double, 0.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, rcsFrequency_, rcsfrequency, double, 7000.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, rcsDetail_, rcsdetail, double, 1.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, drawCircleHilight_, drawcirclehilight, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, circleHilightColor_, circlehilightcolor, uint32_t, 0xFFFFFFFF); // White
SIMDATA_DEFINE_FIELD(PlatformPrefs, circleHilightShape_, circlehilightshape, CircleHilightShape, CircleHilightShape::CH_PULSING_CIRCLE);
SIMDATA_DEFINE_FIELD(PlatformPrefs, circleHilightSize_, circlehilightsize, double, 0.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, hilightFollowYaw_, hilightfollowyaw, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, interpolatePos_, interpolatepos, bool, true);
SIMDATA_DEFINE_FIELD(PlatformPrefs, extrapolatePos_, extrapolatepos, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, scale_, scale, double, 1.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, brightness_, brightness, int32_t, 36);
SIMDATA_DEFINE_FIELD(PlatformPrefs, dynamicScale_, dynamicscale, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, dynamicScaleOffset_, dynamicscaleoffset, double, 0.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, dynamicScaleScalar_, dynamicscalescalar, double, 1.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, dynamicScaleAlgorithm_, dynamicscalealgorithm, DynamicScaleAlgorithm, DynamicScaleAlgorithm::DSA_METERS_TO_PIXELS);
SIMDATA_DEFINE_FIELD(PlatformPrefs, drawVelocityVec_, drawvelocityvec, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, velVecColor_, velveccolor, uint32_t, 0xFF8000FF);  // Orange
SIMDATA_DEFINE_FIELD(PlatformPrefs, velVecUseStaticLength_, velvecusestaticlength, bool, true);
SIMDATA_DEFINE_FIELD(PlatformPrefs, velVecStaticLen_, velvecstaticlen, double, 0.5);
SIMDATA_DEFINE_FIELD(PlatformPrefs, velVecStaticLenUnits_, velvecstaticlenunits, DistanceUnits, DistanceUnits::UNITS_NAUTICAL_MILES);
SIMDATA_DEFINE_FIELD(PlatformPrefs, velVecTime_, velvectime, double, 1.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, velVecTimeUnits_, velvectimeunits, ElapsedTimeFormat, ElapsedTimeFormat::ELAPSED_SECONDS);
SIMDATA_DEFINE_VECTOR_FIELD(PlatformPrefs, gogFile_, gogfile, std::string);
SIMDATA_DEFINE_FIELD(PlatformPrefs, alphaVolume_, alphavolume, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, useCullFace_, usecullface, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, cullFace_, cullface, PolygonFace, PolygonFace::FRONT_AND_BACK);
SIMDATA_DEFINE_FIELD(PlatformPrefs, polygonModeFace_, polygonmodeface, PolygonFace, PolygonFace::FRONT_AND_BACK);
SIMDATA_DEFINE_FIELD(PlatformPrefs, polygonMode_, polygonmode, PolygonMode, PolygonMode::FILL);
SIMDATA_DEFINE_FIELD(PlatformPrefs, usePolygonStipple_, usepolygonstipple, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, polygonStipple_, polygonstipple, uint32_t, 0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, visibleLosColor_, visibleloscolor, uint32_t, 0x00FF0080);  // Green
SIMDATA_DEFINE_FIELD(PlatformPrefs, obstructedLosColor_, obstructedloscolor, uint32_t, 0xFF000080);  // Red
SIMDATA_DEFINE_FIELD(PlatformPrefs, losRangeResolution_, losrangeresolution, double, 1000.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, losAzimuthalResolution_, losazimuthalresolution, double, 15.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, losAltitudeOffset_, losaltitudeoffset, double, 0.0);
SIMDATA_DEFINE_FIELD(PlatformPrefs, animateDofNodes_, animatedofnodes, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, eciDataMode_, ecidatamode, bool, false);
SIMDATA_DEFINE_FIELD(PlatformPrefs, drawOffBehavior_, drawoffbehavior, PlatformPrefs::DrawOffBehavior, PlatformPrefs::DrawOffBehavior::DEFAULT_BEHAVIOR);
SIMDATA_DEFINE_FIELD(PlatformPrefs, lifespanMode_, lifespanmode, LifespanMode, LifespanMode::LIFE_EXTEND_SINGLE_POINT);

void PlatformPrefs::MergeFrom(const PlatformPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(commonPrefs_, CommonPrefs, commonprefs);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(trackPrefs_, TrackPrefs, trackprefs);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(platPositionOffset_, Position, platpositionoffset);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(orientationOffset_, BodyOrientation, orientationoffset);
  SIMDATA_SUB_FIELD_LIST_MERGE_FROM(scaleXYZ_, Position, scalexyz);

  if (from.has_icon())
    icon_ = from.icon_;

  if (from.has_drawmode())
    drawMode_ = from.drawMode_;

  if (from.has_fragmenteffect())
    fragmentEffect_ = from.fragmentEffect_;

  if (from.has_fragmenteffectcolor())
    fragmentEffectColor_ = from.fragmentEffectColor_;

  if (from.has_rotateicons())
    rotateIcons_ = from.rotateIcons_;

  if (from.has_nodepthicons())
    noDepthIcons_ = from.noDepthIcons_;

  if (from.has_iconalignment())
    iconAlignment_ = from.iconAlignment_;

  if (from.has_overridecolorcombinemode())
    overrideColorCombineMode_ = from.overrideColorCombineMode_;

  if (from.has_useclampalt())
    useClampAlt_ = from.useClampAlt_;

  if (from.has_clampvalaltmin())
    clampValAltMin_ = from.clampValAltMin_;

  if (from.has_clampvalaltmax())
    clampValAltMax_ = from.clampValAltMax_;

  if (from.has_useclampyaw())
    useClampYaw_ = from.useClampYaw_;

  if (from.has_clampvalyaw())
    clampValYaw_ = from.clampValYaw_;

  if (from.has_useclamppitch())
    useClampPitch_ = from.useClampPitch_;

  if (from.has_clampvalpitch())
    clampValPitch_ = from.clampValPitch_;

  if (from.has_useclamproll())
    useClampRoll_ = from.useClampRoll_;

  if (from.has_clampvalroll())
    clampValRoll_ = from.clampValRoll_;

  if (from.has_clamporientationatlowvelocity())
    clampOrientationAtLowVelocity_ = from.clampOrientationAtLowVelocity_;

  if (from.has_surfaceclamping())
    surfaceClamping_ = from.surfaceClamping_;

  if (from.has_abovesurfaceclamping())
    aboveSurfaceClamping_ = from.aboveSurfaceClamping_;

  if (from.has_lighted())
    lighted_ = from.lighted_;

  if (from.has_drawbox())
    drawBox_ = from.drawBox_;

  if (from.has_drawbodyaxis())
    drawBodyAxis_ = from.drawBodyAxis_;

  if (from.has_drawinertialaxis())
    drawInertialAxis_ = from.drawInertialAxis_;

  if (from.has_drawsunvec())
    drawSunVec_ = from.drawSunVec_;

  if (from.has_drawmoonvec())
    drawMoonVec_ = from.drawMoonVec_;

  if (from.has_axisscale())
    axisScale_ = from.axisScale_;

  if (from.has_wireframe())
    wireFrame_ = from.wireFrame_;

  if (from.has_drawopticlos())
    drawOpticLos_ = from.drawOpticLos_;

  if (from.has_drawrflos())
    drawRfLos_ = from.drawRfLos_;

  if (from.has_rcsfile())
    rcsFile_ = from.rcsFile_;

  if (from.has_drawrcs())
    drawRcs_ = from.drawRcs_;

  if (from.has_draw3drcs())
    draw3dRcs_ = from.draw3dRcs_;

  if (from.has_rcscolor())
    rcsColor_ = from.rcsColor_;

  if (from.has_rcscolorscale())
    rcsColorScale_ = from.rcsColorScale_;

  if (from.has_rcspolarity())
    rcsPolarity_ = from.rcsPolarity_;

  if (from.has_rcselevation())
    rcsElevation_ = from.rcsElevation_;

  if (from.has_rcsfrequency())
    rcsFrequency_ = from.rcsFrequency_;

  if (from.has_rcsdetail())
    rcsDetail_ = from.rcsDetail_;

  if (from.has_drawcirclehilight())
    drawCircleHilight_ = from.drawCircleHilight_;

  if (from.has_circlehilightcolor())
    circleHilightColor_ = from.circleHilightColor_;

  if (from.has_circlehilightshape())
    circleHilightShape_ = from.circleHilightShape_;

  if (from.has_circlehilightsize())
    circleHilightSize_ = from.circleHilightSize_;

  if (from.has_hilightfollowyaw())
    hilightFollowYaw_ = from.hilightFollowYaw_;

  if (from.has_interpolatepos())
    interpolatePos_ = from.interpolatePos_;

  if (from.has_extrapolatepos())
    extrapolatePos_ = from.extrapolatePos_;

  if (from.has_scale())
    scale_ = from.scale_;

  if (from.has_brightness())
    brightness_ = from.brightness_;

  if (from.has_dynamicscale())
    dynamicScale_ = from.dynamicScale_;

  if (from.has_dynamicscaleoffset())
    dynamicScaleOffset_ = from.dynamicScaleOffset_;

  if (from.has_dynamicscalescalar())
    dynamicScaleScalar_ = from.dynamicScaleScalar_;

  if (from.has_dynamicscalealgorithm())
    dynamicScaleAlgorithm_ = from.dynamicScaleAlgorithm_;

  if (from.has_drawvelocityvec())
    drawVelocityVec_ = from.drawVelocityVec_;

  if (from.has_velveccolor())
    velVecColor_ = from.velVecColor_;

  if (from.has_velvecusestaticlength())
    velVecUseStaticLength_ = from.velVecUseStaticLength_;

  if (from.has_velvecstaticlen())
    velVecStaticLen_ = from.velVecStaticLen_;

  if (from.has_velvecstaticlenunits())
    velVecStaticLenUnits_ = from.velVecStaticLenUnits_;

  if (from.has_velvectime())
    velVecTime_ = from.velVecTime_;

  if (from.has_velvectimeunits())
    velVecTimeUnits_ = from.velVecTimeUnits_;

  gogFile_.insert(gogFile_.end(), from.gogFile_.begin(), from.gogFile_.end());

  if (from.has_alphavolume())
    alphaVolume_ = from.alphaVolume_;

  if (from.has_usecullface())
    useCullFace_ = from.useCullFace_;

  if (from.has_cullface())
    cullFace_ = from.cullFace_;

  if (from.has_polygonmodeface())
    polygonModeFace_ = from.polygonModeFace_;

  if (from.has_polygonmode())
    polygonMode_ = from.polygonMode_;

  if (from.has_usepolygonstipple())
    usePolygonStipple_ = from.usePolygonStipple_;

  if (from.has_polygonstipple())
    polygonStipple_ = from.polygonStipple_;

  if (from.has_visibleloscolor())
    visibleLosColor_ = from.visibleLosColor_;

  if (from.has_obstructedloscolor())
    obstructedLosColor_ = from.obstructedLosColor_;

  if (from.has_losrangeresolution())
    losRangeResolution_ = from.losRangeResolution_;

  if (from.has_losazimuthalresolution())
    losAzimuthalResolution_ = from.losAzimuthalResolution_;

  if (from.has_losaltitudeoffset())
    losAltitudeOffset_ = from.losAltitudeOffset_;

  if (from.has_animatedofnodes())
    animateDofNodes_ = from.animateDofNodes_;

  if (from.has_ecidatamode())
    eciDataMode_ = from.eciDataMode_;

  if (from.has_drawoffbehavior())
    drawOffBehavior_ = from.drawOffBehavior_;

  if (from.has_lifespanmode())
    lifespanMode_ = from.lifespanMode_;

}

void PlatformPrefs::CopyFrom(const PlatformPrefs& from)
{
  if (&from == this)
    return;

  SIMDATA_SUBFIELD_LIST_COPY_FROM(commonPrefs_, CommonPrefs, commonprefs);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(trackPrefs_, TrackPrefs, trackprefs);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(platPositionOffset_, Position, platpositionoffset);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(orientationOffset_, BodyOrientation, orientationoffset);
  SIMDATA_SUBFIELD_LIST_COPY_FROM(scaleXYZ_, Position, scalexyz);

  icon_ = from.icon_;
  drawMode_ = from.drawMode_;
  fragmentEffect_ = from.fragmentEffect_;
  fragmentEffectColor_ = from.fragmentEffectColor_;
  rotateIcons_ = from.rotateIcons_;
  noDepthIcons_ = from.noDepthIcons_;
  iconAlignment_ = from.iconAlignment_;
  overrideColorCombineMode_ = from.overrideColorCombineMode_;
  useClampAlt_ = from.useClampAlt_;
  clampValAltMin_ = from.clampValAltMin_;
  clampValAltMax_ = from.clampValAltMax_;
  useClampYaw_ = from.useClampYaw_;
  clampValYaw_ = from.clampValYaw_;
  useClampPitch_ = from.useClampPitch_;
  clampValPitch_ = from.clampValPitch_;
  useClampRoll_ = from.useClampRoll_;
  clampValRoll_ = from.clampValRoll_;
  clampOrientationAtLowVelocity_ = from.clampOrientationAtLowVelocity_;
  surfaceClamping_ = from.surfaceClamping_;
  aboveSurfaceClamping_ = from.aboveSurfaceClamping_;
  lighted_ = from.lighted_;
  drawBox_ = from.drawBox_;
  drawBodyAxis_ = from.drawBodyAxis_;
  drawInertialAxis_ = from.drawInertialAxis_;
  drawSunVec_ = from.drawSunVec_;
  drawMoonVec_ = from.drawMoonVec_;
  axisScale_ = from.axisScale_;
  wireFrame_ = from.wireFrame_;
  drawOpticLos_ = from.drawOpticLos_;
  drawRfLos_ = from.drawRfLos_;
  rcsFile_ = from.rcsFile_;
  drawRcs_ = from.drawRcs_;
  draw3dRcs_ = from.draw3dRcs_;
  rcsColor_ = from.rcsColor_;
  rcsColorScale_ = from.rcsColorScale_;
  rcsPolarity_ = from.rcsPolarity_;
  rcsElevation_ = from.rcsElevation_;
  rcsFrequency_ = from.rcsFrequency_;
  rcsDetail_ = from.rcsDetail_;
  drawCircleHilight_ = from.drawCircleHilight_;
  circleHilightColor_ = from.circleHilightColor_;
  circleHilightShape_ = from.circleHilightShape_;
  circleHilightSize_ = from.circleHilightSize_;
  hilightFollowYaw_ = from.hilightFollowYaw_;
  interpolatePos_ = from.interpolatePos_;
  extrapolatePos_ = from.extrapolatePos_;
  scale_ = from.scale_;
  brightness_ = from.brightness_;
  dynamicScale_ = from.dynamicScale_;
  dynamicScaleOffset_ = from.dynamicScaleOffset_;
  dynamicScaleScalar_ = from.dynamicScaleScalar_;
  dynamicScaleAlgorithm_ = from.dynamicScaleAlgorithm_;
  drawVelocityVec_ = from.drawVelocityVec_;
  velVecColor_ = from.velVecColor_;
  velVecUseStaticLength_ = from.velVecUseStaticLength_;
  velVecStaticLen_ = from.velVecStaticLen_;
  velVecStaticLenUnits_ = from.velVecStaticLenUnits_;
  velVecTime_ = from.velVecTime_;
  velVecTimeUnits_ = from.velVecTimeUnits_;
  gogFile_ = from.gogFile_;
  alphaVolume_ = from.alphaVolume_;
  useCullFace_ = from.useCullFace_;
  cullFace_ = from.cullFace_;
  polygonModeFace_ = from.polygonModeFace_;
  polygonMode_ = from.polygonMode_;
  usePolygonStipple_ = from.usePolygonStipple_;
  polygonStipple_ = from.polygonStipple_;
  visibleLosColor_ = from.visibleLosColor_;
  obstructedLosColor_ = from.obstructedLosColor_;
  losRangeResolution_ = from.losRangeResolution_;
  losAzimuthalResolution_ = from.losAzimuthalResolution_;
  losAltitudeOffset_ = from.losAltitudeOffset_;
  animateDofNodes_ = from.animateDofNodes_;
  eciDataMode_ = from.eciDataMode_;
  drawOffBehavior_ = from.drawOffBehavior_;
  lifespanMode_ = from.lifespanMode_;
}

bool PlatformPrefs::operator==(const PlatformPrefs& rhs) const
{
  if (&rhs == this)
    return true;

  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(commonPrefs_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(trackPrefs_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(platPositionOffset_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(orientationOffset_, rhs);
  SIMDATA_FIELD_LIST_RETURN_IF_NOT_EQUAL(scaleXYZ_, rhs);

  return ((icon_ == rhs.icon_) &&
    (drawMode_ == rhs.drawMode_) &&
    (fragmentEffect_ == rhs.fragmentEffect_) &&
    (fragmentEffectColor_ == rhs.fragmentEffectColor_) &&
    (rotateIcons_ == rhs.rotateIcons_) &&
    (noDepthIcons_ == rhs.noDepthIcons_) &&
    (iconAlignment_ == rhs.iconAlignment_) &&
    (overrideColorCombineMode_ == rhs.overrideColorCombineMode_) &&
    (useClampAlt_ == rhs.useClampAlt_) &&
    (clampValAltMin_ == rhs.clampValAltMin_) &&
    (clampValAltMax_ == rhs.clampValAltMax_) &&
    (useClampYaw_ == rhs.useClampYaw_) &&
    (clampValYaw_ == rhs.clampValYaw_) &&
    (useClampPitch_ == rhs.useClampPitch_) &&
    (clampValPitch_ == rhs.clampValPitch_) &&
    (useClampRoll_ == rhs.useClampRoll_) &&
    (clampValRoll_ == rhs.clampValRoll_) &&
    (clampOrientationAtLowVelocity_ == rhs.clampOrientationAtLowVelocity_) &&
    (surfaceClamping_ == rhs.surfaceClamping_) &&
    (aboveSurfaceClamping_ == rhs.aboveSurfaceClamping_) &&
    (lighted_ == rhs.lighted_) &&
    (drawBox_ == rhs.drawBox_) &&
    (drawBodyAxis_ == rhs.drawBodyAxis_) &&
    (drawInertialAxis_ == rhs.drawInertialAxis_) &&
    (drawSunVec_ == rhs.drawSunVec_) &&
    (drawMoonVec_ == rhs.drawMoonVec_) &&
    (axisScale_ == rhs.axisScale_) &&
    (wireFrame_ == rhs.wireFrame_) &&
    (drawOpticLos_ == rhs.drawOpticLos_) &&
    (drawRfLos_ == rhs.drawRfLos_) &&
    (rcsFile_ == rhs.rcsFile_) &&
    (drawRcs_ == rhs.drawRcs_) &&
    (draw3dRcs_ == rhs.draw3dRcs_) &&
    (rcsColor_ == rhs.rcsColor_) &&
    (rcsColorScale_ == rhs.rcsColorScale_) &&
    (rcsPolarity_ == rhs.rcsPolarity_) &&
    (rcsElevation_ == rhs.rcsElevation_) &&
    (rcsFrequency_ == rhs.rcsFrequency_) &&
    (rcsDetail_ == rhs.rcsDetail_) &&
    (drawCircleHilight_ == rhs.drawCircleHilight_) &&
    (circleHilightColor_ == rhs.circleHilightColor_) &&
    (circleHilightShape_ == rhs.circleHilightShape_) &&
    (circleHilightSize_ == rhs.circleHilightSize_) &&
    (hilightFollowYaw_ == rhs.hilightFollowYaw_) &&
    (interpolatePos_ == rhs.interpolatePos_) &&
    (extrapolatePos_ == rhs.extrapolatePos_) &&
    (scale_ == rhs.scale_) &&
    (brightness_ == rhs.brightness_) &&
    (dynamicScale_ == rhs.dynamicScale_) &&
    (dynamicScaleOffset_ == rhs.dynamicScaleOffset_) &&
    (dynamicScaleScalar_ == rhs.dynamicScaleScalar_) &&
    (dynamicScaleAlgorithm_ == rhs.dynamicScaleAlgorithm_) &&
    (drawVelocityVec_ == rhs.drawVelocityVec_) &&
    (velVecColor_ == rhs.velVecColor_) &&
    (velVecUseStaticLength_ == rhs.velVecUseStaticLength_) &&
    (velVecStaticLen_ == rhs.velVecStaticLen_) &&
    (velVecStaticLenUnits_ == rhs.velVecStaticLenUnits_) &&
    (velVecTime_ == rhs.velVecTime_) &&
    (velVecTimeUnits_ == rhs.velVecTimeUnits_) &&
    (gogFile_ == rhs.gogFile_) &&
    (alphaVolume_ == rhs.alphaVolume_) &&
    (useCullFace_ == rhs.useCullFace_) &&
    (cullFace_ == rhs.cullFace_) &&
    (polygonModeFace_ == rhs.polygonModeFace_) &&
    (polygonMode_ == rhs.polygonMode_) &&
    (usePolygonStipple_ == rhs.usePolygonStipple_) &&
    (polygonStipple_ == rhs.polygonStipple_) &&
    (visibleLosColor_ == rhs.visibleLosColor_) &&
    (obstructedLosColor_ == rhs.obstructedLosColor_) &&
    (losRangeResolution_ == rhs.losRangeResolution_) &&
    (losAzimuthalResolution_ == rhs.losAzimuthalResolution_) &&
    (losAltitudeOffset_ == rhs.losAltitudeOffset_) &&
    (animateDofNodes_ == rhs.animateDofNodes_) &&
    (eciDataMode_ == rhs.eciDataMode_) &&
    (drawOffBehavior_ == rhs.drawOffBehavior_) &&
    (lifespanMode_ == rhs.lifespanMode_));
}

void PlatformPrefs::Prune()
{
  SIMDATA_SUBFIELD_LIST_PRUNE(commonPrefs_);
  SIMDATA_SUBFIELD_LIST_PRUNE(trackPrefs_);
  SIMDATA_SUBFIELD_LIST_PRUNE(platPositionOffset_);
  SIMDATA_SUBFIELD_LIST_PRUNE(orientationOffset_);
  SIMDATA_SUBFIELD_LIST_PRUNE(scaleXYZ_);
}


}
