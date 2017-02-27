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
 * License for source code at https://simdis.nrl.navy.mil/License.aspx
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osg/Geode"
#include "osg/Geometry"
#include "osgEarthSymbology/MeshConsolidator"

#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/EM/AntennaPattern.h"

#include "simData/DataTypes.h"

#include "simVis/Constants.h"
#include "simVis/Registry.h"
#include "simVis/Utils.h"
#include "simVis/Antenna.h"

namespace simVis
{

AntennaNode::AntennaNode(const osg::Quat& rot)
  : antennaPattern_(NULL),
    loadedOK_(false),
    beamRange_(1.0f),
    beamScale_(1.0f),
    scaleFactor_(-1.0f),
    rot_(rot),
    min_(HUGE_VAL),
    max_(-HUGE_VAL)
{
  colorUtils_ = new ColorUtils(0.3);
}

AntennaNode::~AntennaNode()
{
  delete colorUtils_;
  delete antennaPattern_;
}

// antennaPattern's scale is a product of update range (in m) and pref beamScale (no units, 1.0 default)
void AntennaNode::setRange(float range)
{
  beamRange_ = range;

  // only applyScale if we have a geode/geometry
  if (getNumChildren() != 0)
    applyScale_();
}

bool AntennaNode::setPrefs(const simData::BeamPrefs& prefs)
{
  const simData::BeamPrefs* oldPrefs = &lastPrefs_.get();
  const simData::BeamPrefs* newPrefs = &prefs;

  bool requiresRebuild =
       !lastPrefs_.isSet() ||
        PB_SUBFIELD_CHANGED(oldPrefs, newPrefs, antennapattern, type) ||
        PB_SUBFIELD_CHANGED(oldPrefs, newPrefs, antennapattern, algorithm) ||
        PB_SUBFIELD_CHANGED(oldPrefs, newPrefs, antennapattern, filename) ||
        PB_FIELD_CHANGED(oldPrefs, newPrefs, polarity) ||
        PB_FIELD_CHANGED(oldPrefs, newPrefs, gain) ||
        PB_FIELD_CHANGED(oldPrefs, newPrefs, frequency) ||
        PB_FIELD_CHANGED(oldPrefs, newPrefs, channel) ||
        PB_FIELD_CHANGED(oldPrefs, newPrefs, weighting) ||
        PB_FIELD_CHANGED(oldPrefs, newPrefs, horizontalwidth) ||
        PB_FIELD_CHANGED(oldPrefs, newPrefs, verticalwidth);

  if (requiresRebuild)
  {
    patternFile_.clear();

    if (prefs.antennapattern().type() == simData::BeamPrefs_AntennaPattern_Type_ALGORITHM)
    {
      switch (prefs.antennapattern().algorithm())
      {
      case simData::BeamPrefs_AntennaPattern_Algorithm_PEDESTAL:
        patternFile_ = simCore::ANTENNA_STRING_ALGORITHM_PEDESTAL;
        break;
      case simData::BeamPrefs_AntennaPattern_Algorithm_GAUSS:
        patternFile_ = simCore::ANTENNA_STRING_ALGORITHM_GAUSS;
        break;
      case simData::BeamPrefs_AntennaPattern_Algorithm_CSCSQ:
        patternFile_ = simCore::ANTENNA_STRING_ALGORITHM_CSCSQ;
        break;
      case simData::BeamPrefs_AntennaPattern_Algorithm_SINXX:
        patternFile_ = simCore::ANTENNA_STRING_ALGORITHM_SINXX;
        break;
      case simData::BeamPrefs_AntennaPattern_Algorithm_OMNI:
        patternFile_ = simCore::ANTENNA_STRING_ALGORITHM_OMNI;
        break;
      }
    }
    else if (prefs.antennapattern().type() == simData::BeamPrefs_AntennaPattern_Type_FILE)
    {
      if (!prefs.antennapattern().filename().empty())
      {
        patternFile_ = prefs.antennapattern().filename();
      }
    }

    // load the new pattern file
    delete(antennaPattern_);
    // Frequency must be > 0, if <= 0 use default value
    double freq = prefs.frequency() > 0 ? prefs.frequency() : simCore::DEFAULT_FREQUENCY;
    antennaPattern_ = simCore::loadPatternFile(patternFile_, freq);
    loadedOK_ = (antennaPattern_ != NULL);
  }

  polarity_ = static_cast<simCore::PolarityType>(prefs.polarity());

  bool drawAntennaPattern = loadedOK_ &&
    (prefs.drawtype() == simData::BeamPrefs_DrawType_ANTENNA_PATTERN);

  bool requiresRedraw = drawAntennaPattern &&
      (requiresRebuild ||
        PB_FIELD_CHANGED(oldPrefs, newPrefs, drawtype)      ||
        PB_FIELD_CHANGED(oldPrefs, newPrefs, colorscale)      ||
        PB_FIELD_CHANGED(oldPrefs, newPrefs, detail) ||
        PB_FIELD_CHANGED(oldPrefs, newPrefs, sensitivity) ||
        PB_FIELD_CHANGED(oldPrefs, newPrefs, fieldofview) ||
        PB_FIELD_CHANGED(oldPrefs, newPrefs, elevationoffset) ||
        PB_SUBFIELD_CHANGED(oldPrefs, newPrefs, commonprefs, useoverridecolor) ||
        PB_SUBFIELD_CHANGED(oldPrefs, newPrefs, commonprefs, overridecolor) ||
        PB_SUBFIELD_CHANGED(oldPrefs, newPrefs, commonprefs, color));

  if (!drawAntennaPattern)
  {
    removeChildren(0, getNumChildren());
  }
  else if (requiresRedraw)
  {
    // this needs to be recalc'd if prefs change. reset to -1 to force recalc
    scaleFactor_ = -1.0f;
    beamScale_ = prefs.beamscale();
    lastPrefs_ = prefs;
    render_();
    setNodeMask(simVis::DISPLAY_MASK_BEAM);
    updateLighting_(prefs.shaded());
    updateBlending_(prefs.blended());
    return true;
  }
  else
  {
    // this is a guard on the use of oldPrefs; if assert fails, check that !lastPrefs_.isSet() forces requiresRebuild to true
    assert(lastPrefs_.isSet());
    if (PB_FIELD_CHANGED(oldPrefs, newPrefs, shaded))
      updateLighting_(prefs.shaded());
    if (PB_FIELD_CHANGED(oldPrefs, newPrefs, blended))
      updateBlending_(prefs.blended());
    if (PB_FIELD_CHANGED(oldPrefs, newPrefs, beamscale))
    {
      beamScale_ = newPrefs->beamscale();
      // this is a slightly roundabout way of applying the pref.
      setRange(beamRange_);
    }
  }
  lastPrefs_ = prefs;
  return false;
}

void AntennaNode::updateLighting_(bool shaded)
{
  osg::StateSet* stateSet = getOrCreateStateSet();
  simVis::setLighting(stateSet,
    (shaded ? osg::StateAttribute::ON : osg::StateAttribute::OFF));
}

void AntennaNode::updateBlending_(bool blending)
{
  osg::StateSet* stateSet = getOrCreateStateSet();
  stateSet->setMode(GL_BLEND, 
    (blending ? osg::StateAttribute::ON : osg::StateAttribute::OFF));
}

float AntennaNode::PatternGain(float azim, float elev, simCore::PolarityType polarity) const
{
  if (!lastPrefs_.isSet())
    return 0.0f;
  // convert freq in MHz to Hz (note that freq is not actually used in any supported gain calcs)
  double freq = lastPrefs_->frequency() * 1e6;
  if (!antennaPattern_)
    return lastPrefs_->gain();
  switch (antennaPattern_->type())
  {
  case simCore::ANTENNA_PATTERN_MONOPULSE:
    return antennaPattern_->gain(simCore::AntennaGainParameters(azim, elev, simCore::POLARITY_UNKNOWN, 0, 0, lastPrefs_->gain(), 0, 0, freq, false, lastPrefs_->channel()));
  case simCore::ANTENNA_PATTERN_CRUISE:
    return antennaPattern_->gain(simCore::AntennaGainParameters(azim, elev, simCore::POLARITY_UNKNOWN, 0, 0, 0, 0, 0, freq));
  case simCore::ANTENNA_PATTERN_NSMA:
  case simCore::ANTENNA_PATTERN_EZNEC:
  case simCore::ANTENNA_PATTERN_XFDTD:
    return antennaPattern_->gain(simCore::AntennaGainParameters(azim, elev, polarity_, 0, 0, lastPrefs_->gain()));
  default:
    return antennaPattern_->gain(simCore::AntennaGainParameters(azim, elev, simCore::POLARITY_UNKNOWN, simCore::angFix2PI(lastPrefs_->horizontalwidth()), osg::absolute(simCore::angFixPI(lastPrefs_->verticalwidth())), lastPrefs_->gain(), -23.2f, -20.0f, freq, lastPrefs_->weighting()));
  }
  // this point should never be reached; if assert fails, logic in this routine has been changed
  assert(0);
  return lastPrefs_->gain();
}

float AntennaNode::ComputeRadius_(float azim, float elev, simCore::PolarityType polarity, osg::Vec3f &p) const
{
  // values returned from PatternGain are in dB
  float gain = PatternGain(azim, elev, polarity);
  float radius;

  if (gain < simCore::SMALL_DB_COMPARE)
    radius = 0.0;

  // prevent multiply by zero error when fMin == fMax (OMNI case)
  else if (min_ == max_)
    radius = ((gain > lastPrefs_->sensitivity()) ? osg::absolute(gain) * scaleFactor_ : 0.0f);
  else
    radius = ((gain > lastPrefs_->sensitivity()) ? osg::absolute(gain - min_) * scaleFactor_ : 0.0f);

  // convert azim & elev to a rectangular coordinate
  p.set(radius * cosf(azim) * cosf(elev),
    radius * sinf(azim) * cosf(elev),
    radius * sinf(elev));

  return gain;
}

// antennaPattern's scale is a product of update range (in m) and beamScale preference (no units, 1.0 default)
void AntennaNode::applyScale_()
{
  const float newScale = beamRange_ * beamScale_;
  setMatrix(osg::Matrixf::scale(newScale, newScale, newScale) * osg::Matrix::rotate(rot_));
  if (newScale != 1.0f)
  {
    getOrCreateStateSet()->setMode(GL_RESCALE_NORMAL, 1);
  }
}


void AntennaNode::render_()
{
  // render should never be called unless a valid pattern is set. if assert fails, check logic in setPrefs
  assert(loadedOK_);
  // lastPrefs_ must be valid before a pattern can be rendered; if assert fails, check for changes in setPrefs
  assert(lastPrefs_.isSet());

  removeChildren(0, getNumChildren());

  osg::Geometry* antGeom = new osg::Geometry();
  antGeom->setDataVariance(osg::Object::DYNAMIC);
  antGeom->setUseVertexBufferObjects(true);

  osg::Geode* geode = new osg::Geode();
  geode->addDrawable(antGeom);

  osg::Vec3Array* verts = new osg::Vec3Array();
  antGeom->setVertexArray(verts);

  osg::Vec3Array* norms = new osg::Vec3Array();
  antGeom->setNormalArray(norms);
  antGeom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

  osg::Vec4Array* colors = new osg::Vec4Array;
  antGeom->setColorArray(colors);
  antGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

  //TODO: Add support for overriding BeamPrefs values?
  double vRange = lastPrefs_->fieldofview();
  double hRange = vRange;

  if (vRange > M_PI)
    vRange = M_PI;

  if (hRange > M_TWOPI)
    hRange = M_TWOPI;

  // determine pattern bounds in order to normalize
  if (scaleFactor_ < 0.0)
  {
    antennaPattern_->minMaxGain(&min_, &max_, simCore::AntennaGainParameters(0, 0, polarity_, simCore::angFix2PI(lastPrefs_->horizontalwidth()), osg::absolute(simCore::angFixPI(lastPrefs_->verticalwidth())), lastPrefs_->gain(), -23.2f, -20.0f, lastPrefs_->frequency() * 1e6, lastPrefs_->weighting()));

    // prevent divide by zero error for OMNI case
    scaleFactor_ = (max_ == min_) ? 1.0f/max_ : 1.0f/(max_ - min_);
  }

  float elev = lastPrefs_->elevationoffset();
  float azim = 0;
  float endelev = static_cast<float>(simCore::RAD2DEG*(vRange * 0.5));
  float startelev = -endelev;
  float endazim = static_cast<float>(simCore::RAD2DEG*(hRange * 0.5));
  float startazim = -endazim;
  float degDetail = lastPrefs_->detail();
  float beamDetail = static_cast<float>(simCore::DEG2RAD*(degDetail));
  int intGain = 0;
  osg::Vec3f zeroPt = osg::Vec3f(0.0, 0.0, 0.0);
  osg::Vec3f zeroNorm = osg::Vec3f(-1.0, 0.0, 0.0);

  bool colorScale = lastPrefs_->colorscale();
  osg::Vec4f color = ColorUtils::RgbaToVec4(lastPrefs_->commonprefs().color());
  if (lastPrefs_->commonprefs().useoverridecolor())
    color = ColorUtils::RgbaToVec4(lastPrefs_->commonprefs().overridecolor());
  osg::Vec4f scaleAltColor = osg::Vec4f(0.5, 0.0, 0.5, 1.0);

  int lastCount = 0;

  bool azimDone = false;
  for (double ii = startazim; !azimDone; ii += degDetail)
  {
    double width = beamDetail;
    if (ii + degDetail >= endazim)
    {
      width = simCore::DEG2RAD*(endazim - ii);
      azimDone = true;
    }
    azim = simCore::DEG2RAD*ii;

    bool elevDone = false;
    for (double jj = startelev; !elevDone; jj += degDetail)
    {
      if (jj >= endelev)
      {
        jj = endelev;
        elevDone = true;
      }
      elev = simCore::DEG2RAD*jj;

      // compute first point in t-strip
      osg::Vec3f pt;
      intGain = static_cast<int>(ComputeRadius_(azim, elev, polarity_, pt));

      osg::Vec3f ptNorm(pt);
      ptNorm.normalize();

      verts->push_back(pt);
      norms->push_back(ptNorm);

      if (colorScale)
      {
        colors->push_back(colorUtils_->GainThresholdColor(intGain));
      }
      else
      {
        colors->push_back(color);
      }

      // compute alternate point in t-strip
      osg::Vec3f ptne;
      intGain = static_cast<int>(ComputeRadius_((azim + width), elev, polarity_, ptne));

      osg::Vec3f ptneNorm(ptne);
      ptneNorm.normalize();

      verts->push_back(ptne);
      norms->push_back(ptneNorm);

      if (colorScale)
        colors->push_back(colorUtils_->GainThresholdColor(intGain));
      else
        colors->push_back(color);
    }

    antGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, lastCount, verts->size() - lastCount));
    lastCount = verts->size();
  }

  // draw top & bottom sides of pattern
  if (vRange < M_PI)
  {
    // draw bottom side of pattern
    elev = static_cast<float>(simCore::DEG2RAD*(startelev));

    osg::Vec3f bottomNormal = osg::Quat(elev, osg::Vec3f(1, 0, 0)) * osg::Vec3f(0, 0, -1);

    verts->push_back(zeroPt);
    norms->push_back(bottomNormal);

    if (colorScale)
      colors->push_back(scaleAltColor);
    else
      colors->push_back(color);

    bool azimDone = false;
    for (double ii = startazim; !azimDone; ii += degDetail)
    {
      if (ii >= endazim)
      {
        ii = endazim;
        azimDone = true;
      }
      azim = simCore::DEG2RAD*ii;

      osg::Vec3f pt;
      intGain = static_cast<int>(ComputeRadius_(azim, elev, polarity_, pt));

      verts->push_back(pt);
      norms->push_back(bottomNormal);

      if (colorScale)
        colors->push_back(colorUtils_->GainThresholdColor(intGain));
      else
        colors->push_back(color);
    }

    antGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, lastCount, verts->size() - lastCount));
    lastCount = verts->size();

    // draw top side of pattern
    elev = static_cast<float>(simCore::DEG2RAD*(endelev));

    osg::Vec3f topNormal = osg::Quat(elev, osg::Vec3f(1, 0, 0)) * osg::Vec3f(0, 0, 1);

    verts->push_back(zeroPt);
    norms->push_back(topNormal);

    if (colorScale)
      colors->push_back(scaleAltColor);
    else
      colors->push_back(color);

    azimDone = false;
    for (double ii = startazim; !azimDone; ii += degDetail)
    {
      if (ii >= endazim)
      {
        ii = endazim;
        azimDone = true;
      }
      azim = simCore::DEG2RAD*ii;

      osg::Vec3f pt;
      intGain = static_cast<int>(ComputeRadius_(azim, elev, polarity_, pt));

      verts->push_back(pt);
      norms->push_back(topNormal);

      if (colorScale)
        colors->push_back(colorUtils_->GainThresholdColor(intGain));
      else
        colors->push_back(color);
    }

    antGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, lastCount, verts->size() - lastCount));
    lastCount = verts->size();

  } // end of (vRange < M_PI)

  // draw back sides of pattern
  if (hRange < M_TWOPI)
  {
    // draw left side of pattern
    azim = static_cast<float>(simCore::DEG2RAD*(startazim));

    osg::Vec3f leftNormal = osg::Quat(azim, osg::Vec3f(0, 0, 1)) * osg::Vec3f(-1, 0, 0);

    verts->push_back(zeroPt);
    norms->push_back(leftNormal);

    if (colorScale)
      colors->push_back(scaleAltColor);
    else
      colors->push_back(color);

    bool elevDone = false;
    for (double jj = startelev; !elevDone; jj += degDetail)
    {
      if (jj >= endelev)
      {
        jj = endelev;
        elevDone = true;
      }
      elev = simCore::DEG2RAD*jj;

      osg::Vec3f pt;
      intGain = static_cast<int>(ComputeRadius_(azim, elev, polarity_, pt));

      verts->push_back(pt);
      norms->push_back(leftNormal);

      if (colorScale)
        colors->push_back(colorUtils_->GainThresholdColor(intGain));
      else
        colors->push_back(color);
    }

    antGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, lastCount, verts->size() - lastCount));
    lastCount = verts->size();

    // draw right side of pattern
    azim = static_cast<float>(simCore::DEG2RAD*(endazim));

    osg::Vec3f rightNormal = osg::Quat(azim, osg::Vec3f(0, 0, 1)) * osg::Vec3f(1, 0, 0);

    verts->push_back(zeroPt);
    norms->push_back(rightNormal);

    if (colorScale)
      colors->push_back(scaleAltColor);
    else
      colors->push_back(color);

    elevDone = false;
    for (double jj = startelev; !elevDone; jj += degDetail)
    {
      if (jj >= endelev)
      {
        jj = endelev;
        elevDone = true;
      }
      elev = simCore::DEG2RAD*jj;

      osg::Vec3f pt;
      intGain = static_cast<int>(ComputeRadius_(azim, elev, polarity_, pt));

      verts->push_back(pt);
      norms->push_back(rightNormal);

      if (colorScale)
        colors->push_back(colorUtils_->GainThresholdColor(intGain));
      else
        colors->push_back(color);
    }

    antGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, lastCount, verts->size() - lastCount));
  } // end of (hRange < T_PI)

  // optimize the geode:
  osgEarth::Symbology::MeshConsolidator::run(*geode);
  addChild(geode);
  applyScale_();
}
}
