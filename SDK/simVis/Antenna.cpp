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
#include "simCore/EM/AntennaPattern.h"

#include "simData/DataTypes.h"
#include "simVis/AxisVector.h"
#include "simVis/Constants.h"
#include "simVis/Registry.h"
#include "simVis/Utils.h"
#include "simVis/Antenna.h"

// enable this to draw axes at beam origin and at pattern face vertices, for testing only
//#define DRAW_AXES 1

namespace {
  /**
  * Calculate a normal to the input vector, where the vector represents a vertex in the top or bottom face
  * The normal is calculated as the input vector rotated 90 degrees around an axis in the XY plane
  * where that axis is normal to the vector projected on the XY plane.
  * @param[in] vec a vector in the top or bottom face of the antenna pattern
  * @return the normal vector
  */
  osg::Vec3f calcNormalXY(const osg::Vec3f& vec)
  {
    osg::Vec3f vecNorm(vec);
    // make a unit vector out of the original vector
    vecNorm.normalize();
    // construct a 2d normal to the vector, in the x-y plane
    const osg::Vec3f axis(-vec.y(), vec.x(), 0.f);
    // rotate the unit vector pi/2 around the 2d-normal-axis
    const osg::Quat& normalRot = osg::Quat(M_PI_2, axis);
    vecNorm = normalRot * vecNorm;
    return vecNorm;
  }

  /**
  * Calculate a normal to the input vector, where the vector represents a vertex in the left or right face
  * The normal is calculated as the input vector rotated 90 degrees around an axis in the XZ plane
  * where that axis is normal to the vector projected on the XZ plane.
  * @param[in] vec a vector in the left or right face of the antenna pattern
  * @return the normal vector
  */
  osg::Vec3f calcNormalXZ(const osg::Vec3f& vec)
  {
    osg::Vec3f vecNorm(vec);
    // make a unit vector out of the original vector
    vecNorm.normalize();
    // construct a 2d normal to the vector, in the x-z plane
    const osg::Vec3f axis(vec.z(), 0.f, -vec.x());
    // rotate the unit vector pi/2 around the 2d-normal-axis
    const osg::Quat& normalRot = osg::Quat(M_PI_2, axis);
    vecNorm = normalRot * vecNorm;
    return vecNorm;
  }
}
namespace simVis
{
// AntennaNode hierarchy:
//  this (LocatorNode) - responsible for beam position and orientation
//    antenna_ (MatrixTransform) - responsible for antenna visual scaling
//      Geode - contains the antenna geometry
//        Geometry - contains the antenna primitives

AntennaNode::AntennaNode(simVis::Locator* locator, const osg::Quat& rot)
  : LocatorNode(locator),
    antennaPattern_(NULL),
    loadedOK_(false),
    beamRange_(1.0f),
    beamScale_(1.0f),
    scaleFactor_(-1.0f),
    rot_(rot),
    min_(HUGE_VAL),
    max_(-HUGE_VAL)
{
  colorUtils_ = new ColorUtils(0.3);
  antenna_ = new osg::MatrixTransform();
  addChild(antenna_);
}

AntennaNode::~AntennaNode()
{
  delete colorUtils_;
  delete antennaPattern_;
}

void AntennaNode::syncWithLocator()
{
  // if not drawing, we don't need to update this
  if (!loadedOK_ || !lastPrefs_.isSet() || !lastPrefs_->drawtype() == simData::BeamPrefs_DrawType_ANTENNA_PATTERN)
    return;

  // call the base class to update the matrix.
  LocatorNode::syncWithLocator();
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

  const bool requiresRebuild =
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
    const double freq = prefs.frequency() > 0 ? prefs.frequency() : simCore::DEFAULT_FREQUENCY;
    antennaPattern_ = simCore::loadPatternFile(patternFile_, freq);
    loadedOK_ = (antennaPattern_ != NULL);
  }

  polarity_ = static_cast<simCore::PolarityType>(prefs.polarity());

  const bool drawAntennaPattern = loadedOK_ &&
    (prefs.drawtype() == simData::BeamPrefs_DrawType_ANTENNA_PATTERN);

  const bool requiresRedraw = drawAntennaPattern &&
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
    antenna_->removeChildren(0, getNumChildren());
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

    // ensure that locator is synced with beam parent
    syncWithLocator();

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
  const float gain = PatternGain(azim, elev, polarity);
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
  antenna_->setMatrix(osg::Matrixf::scale(newScale, newScale, newScale) * osg::Matrix::rotate(rot_));
  if (newScale != 1.0f)
  {
    antenna_->getOrCreateStateSet()->setMode(GL_RESCALE_NORMAL, 1);
  }
}

void AntennaNode::drawAxes_(const osg::Vec3f& pos, const osg::Vec3f& vec)
{
  AxisVector* axes = new AxisVector();
  axes->setPositionOrientation(pos, vec);
  antenna_->addChild(axes);
}

void AntennaNode::render_()
{
  // render should never be called unless a valid pattern is set. if assert fails, check logic in setPrefs
  assert(loadedOK_);
  // lastPrefs_ must be valid before a pattern can be rendered; if assert fails, check for changes in setPrefs
  assert(lastPrefs_.isSet());

  antenna_->removeChildren(0, getNumChildren());

  osg::ref_ptr<osg::Geometry> antGeom = new osg::Geometry();
  antGeom->setDataVariance(osg::Object::DYNAMIC);
  antGeom->setUseVertexBufferObjects(true);

  osg::Vec3Array* verts = new osg::Vec3Array();
  antGeom->setVertexArray(verts);

  osg::Vec3Array* norms = new osg::Vec3Array();
  antGeom->setNormalArray(norms);
  antGeom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

  osg::Vec4Array* colors = new osg::Vec4Array;
  antGeom->setColorArray(colors);
  antGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

  // expected range for vRange is (0, M_PI]
  const double vRange = osg::clampBetween(lastPrefs_->fieldofview(), std::numeric_limits<double>::min(), M_PI);
  // expected range for vRange is (0, M_TWOPI]
  const double hRange = osg::clampBetween(lastPrefs_->fieldofview(), std::numeric_limits<double>::min(), M_TWOPI);

  // detail is in degrees, determines the step size between az and el points, expected value is [1, 10] degrees
  const double degDetail = osg::clampBetween(lastPrefs_->detail(), 1.0, 10.0);

  // determine pattern bounds in order to normalize
  if (scaleFactor_ < 0.0)
  {
    antennaPattern_->minMaxGain(&min_, &max_, simCore::AntennaGainParameters(0, 0, polarity_, simCore::angFix2PI(lastPrefs_->horizontalwidth()), osg::absolute(simCore::angFixPI(lastPrefs_->verticalwidth())), lastPrefs_->gain(), -23.2f, -20.0f, lastPrefs_->frequency() * 1e6, lastPrefs_->weighting()));

    // prevent divide by zero error for OMNI case
    scaleFactor_ = (max_ == min_) ? 1.0f/max_ : 1.0f/(max_ - min_);
  }

  // elevationoffset is in radians, expected limits are [-90,90] (degrees)
  const double elevationoffset = osg::clampBetween(lastPrefs_->elevationoffset(), -M_PI_2, M_PI_2);
  const double endelev = simCore::RAD2DEG * (elevationoffset + (vRange * 0.5));
  const double startelev = simCore::RAD2DEG * (elevationoffset - (vRange * 0.5));
  // pre-calculate the elev points we are using
  std::vector<float> elevPoints;
  bool elevDone = false;
  for (double elev = startelev; !elevDone; elev += degDetail)
  {
    if (elev >= endelev)
    {
      elev = endelev;
      elevDone = true;
    }
    elevPoints.push_back(static_cast<float>(simCore::DEG2RAD * elev));
  }

  // azimuthoffset is in radians, expected limits are [0,360) (degrees)
  const double azimuthoffset = osg::clampBetween(lastPrefs_->azimuthoffset(), 0.0, M_TWOPI);
  const double endazim = simCore::RAD2DEG * (azimuthoffset + (hRange * 0.5));
  const double startazim = simCore::RAD2DEG * (azimuthoffset - (hRange * 0.5));
  // pre-calculate the azim points we are using
  std::vector<float> azimPoints;
  bool azimDone = false;
  for (double azim = startazim; !azimDone; azim += degDetail)
  {
    if (azim >= endazim)
    {
      azim = endazim;
      azimDone = true;
    }
    azimPoints.push_back(static_cast<float>(simCore::DEG2RAD * azim));
  }
  // algorithms below require azimPoints > 1, so break out if we don't meet that requirement
  if (azimPoints.size() < 2)
  {
    return;
  }

  const osg::Vec3f& zeroPt = osg::Vec3f(0.0f, 0.0f, 0.0f);
  const bool colorScale = lastPrefs_->colorscale();
  const osg::Vec4f color = (lastPrefs_->commonprefs().useoverridecolor()) ? ColorUtils::RgbaToVec4(lastPrefs_->commonprefs().overridecolor()) : ColorUtils::RgbaToVec4(lastPrefs_->commonprefs().color());
  // this is the color that corresponds to minimum gain (-100)
  const osg::Vec4f scaleAltColor = colorUtils_->GainThresholdColor(-100);
  int lastCount = 0;

  #ifdef DRAW_AXES
  // draw axes to represent beam orientation
  {
    AxisVector* vec = new AxisVector();
    addChild(vec);
  }
  #endif

  azimDone = false;
  // unconventional iteration, due to algorithm needing *iiter and *(iiter+1)
  for (std::vector<float>::const_iterator iiter = azimPoints.begin(); !azimDone; ++iiter)
  {
    const float azim = *iiter;
    auto nextIter = iiter + 1;
    // azimDone condition should break iteration before this can occur
    assert(nextIter != azimPoints.end());
    const float azim2 = *nextIter;
    azimDone = (nextIter + 1 == azimPoints.end());

    for (std::vector<float>::const_iterator jiter = elevPoints.begin(); jiter != elevPoints.end(); ++jiter)
    {
      const float elev = *jiter;

      // compute first point in t-strip
      osg::Vec3f pt;
      float gain = ComputeRadius_(azim, elev, polarity_, pt);
      osg::Vec3f ptNorm(pt);
      ptNorm.normalize();
      verts->push_back(pt);
      norms->push_back(ptNorm);
      if (colorScale)
        colors->push_back(colorUtils_->GainThresholdColor(static_cast<int>(gain)));
      else
        colors->push_back(color);

      // compute alternate point in t-strip
      // TODO: this calculated result could potentially be reused in the next azim iteration; consider using an index array.
      osg::Vec3f ptne;
      gain = ComputeRadius_(azim2, elev, polarity_, ptne);
      osg::Vec3f ptneNorm(ptne);
      ptneNorm.normalize();
      verts->push_back(ptne);
      norms->push_back(ptneNorm);
      if (colorScale)
        colors->push_back(colorUtils_->GainThresholdColor(static_cast<int>(gain)));
      else
        colors->push_back(color);
    }

    antGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, lastCount, verts->size() - lastCount));
    lastCount = verts->size();
  }

  // draw top & bottom sides of pattern
  if (vRange < M_PI)
  {
    // draw near face/bottom side of pattern
    // TODO: for some patterns (gaussian), all bottom side points will be zero, and the entire side can be skipped
    {
      const float elev = static_cast<float>(simCore::DEG2RAD * startelev);
      // determine a normal for the face at the beam origin - rotate the beam unit vector (x-axis) around y-axis by (elev + PI/2) radians
      const osg::Quat& normalRot = osg::Quat(M_PI_2 - elev, osg::Y_AXIS);
      const osg::Vec3f& zeroNormal = normalRot * osg::X_AXIS;

      verts->push_back(zeroPt);
      norms->push_back(zeroNormal);
      if (colorScale)
        colors->push_back(scaleAltColor);
      else
        colors->push_back(color);

      // reverse iteration to set correct polygon facing
      for (std::vector<float>::const_reverse_iterator iriter = azimPoints.rbegin(); iriter != azimPoints.rend(); ++iriter)
      {
        const float azim = *iriter;
        osg::Vec3f pt;
        const float gain = ComputeRadius_(azim, elev, polarity_, pt);
        verts->push_back(pt);
        const osg::Vec3f& normalVec = calcNormalXY(pt);
        norms->push_back(normalVec);

        if (colorScale)
          colors->push_back(colorUtils_->GainThresholdColor(static_cast<int>(gain)));
        else
          colors->push_back(color);

#ifdef DRAW_AXES
        // draw axes to visualize the vertex normals, every 10th point of the triangle fan
        if (fmod(std::distance(static_cast<std::vector<float>::const_reverse_iterator>(azimPoints.rbegin()), iriter), 10.0) == 0)
        {
          drawAxes_(pt, normalVec);
        }
#endif
      }
      antGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, lastCount, verts->size() - lastCount));
      lastCount = verts->size();
    }

    // draw near face/top side of pattern
    // TODO: for some patterns (gaussian), all top side points will be zero, and the entire side can be skipped
    {
      const float elev = static_cast<float>(simCore::DEG2RAD * endelev);

      // determine a normal for the face at the beam origin - rotate the beam unit vector (x-axis) around y-axis by (-pi/2 - elev) radians
      const osg::Quat& normalRot = osg::Quat(-M_PI_2 - elev, osg::Y_AXIS);
      const osg::Vec3f& zeroNormal = normalRot * osg::X_AXIS;
      verts->push_back(zeroPt);
      norms->push_back(zeroNormal);
      if (colorScale)
        colors->push_back(scaleAltColor);
      else
        colors->push_back(color);

      for (std::vector<float>::const_iterator iiter = azimPoints.begin(); iiter != azimPoints.end(); ++iiter)
      {
        const float azim = *iiter;
        osg::Vec3f pt;
        const float gain = ComputeRadius_(azim, elev, polarity_, pt);
        verts->push_back(pt);
        // sign change is required for top side
        const osg::Vec3f& normalVec = -calcNormalXY(pt);
        norms->push_back(normalVec);

        if (colorScale)
          colors->push_back(colorUtils_->GainThresholdColor(static_cast<int>(gain)));
        else
          colors->push_back(color);

#ifdef DRAW_AXES
        // draw axes to visualize the vertex normals, every 10th point of the triangle fan
        if (fmod(std::distance(static_cast<std::vector<float>::const_iterator>(azimPoints.begin()), iiter), 10.0) == 0)
        {
          drawAxes_(pt, normalVec);
        }
#endif
      }
      antGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, lastCount, verts->size() - lastCount));
      lastCount = verts->size();
    }
  } // end of (vRange < M_PI)


  // draw right and left sides of pattern
  if (hRange < M_TWOPI)
  {
    // draw right side of pattern
    // TODO: for some patterns (pedestal), all right side points will be zero, and the entire right side can be skipped
    {
      const float azim = static_cast<float>(simCore::DEG2RAD * startazim);
      // determine a normal for the face at the beam origin - rotate the beam unit vector (x-axis) around z-axis by azim - pi/2 radians
      const osg::Quat& normalRot = osg::Quat(-M_PI_2 + azim, osg::Z_AXIS);
      const osg::Vec3f& zeroNormal = normalRot * osg::X_AXIS;

      verts->push_back(zeroPt);
      norms->push_back(zeroNormal);
      if (colorScale)
        colors->push_back(scaleAltColor);
      else
        colors->push_back(color);

      for (std::vector<float>::const_iterator jiter = elevPoints.begin(); jiter != elevPoints.end(); ++jiter)
      {
        const float elev = *jiter;
        osg::Vec3f pt;
        const float gain = ComputeRadius_(azim, elev, polarity_, pt);
        verts->push_back(pt);
        const osg::Vec3f& normalVec = calcNormalXZ(pt);
        norms->push_back(normalVec);

        if (colorScale)
          colors->push_back(colorUtils_->GainThresholdColor(static_cast<int>(gain)));
        else
          colors->push_back(color);

#ifdef DRAW_AXES
        // draw axes to visualize the vertex normals, every 10th point of the triangle fan
        if (fmod(std::distance(static_cast<std::vector<float>::const_iterator>(elevPoints.begin()), jiter), 10.0) == 0)
        {
          drawAxes_(pt, normalVec);
        }
#endif
      }
      antGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, lastCount, verts->size() - lastCount));
      lastCount = verts->size();
    }

    // draw left side of pattern
    // TODO: for some patterns (pedestal), all left side points will be zero, and the entire left side can be skipped
    {
      const float azim = static_cast<float>(simCore::DEG2RAD * endazim);
      // determine a normal for the face at the beam origin - rotate the beam unit vector (x-axis) around z-axis by azim + pi/2 radians
      const osg::Quat& normalRot = osg::Quat(M_PI_2 + azim, osg::Z_AXIS);
      const osg::Vec3f& zeroNormal = normalRot * osg::X_AXIS;
      verts->push_back(zeroPt);
      norms->push_back(zeroNormal);
      if (colorScale)
        colors->push_back(scaleAltColor);
      else
        colors->push_back(color);

      // reverse iteration to set correct polygon facing
      for (std::vector<float>::const_reverse_iterator jriter = elevPoints.rbegin(); jriter != elevPoints.rend(); ++jriter)
      {
        const float elev = *jriter;
        osg::Vec3f pt;
        const float gain = ComputeRadius_(azim, elev, polarity_, pt);
        verts->push_back(pt);
        // sign change is required for left side
        const osg::Vec3f& normalVec = -calcNormalXZ(pt);
        norms->push_back(normalVec);

        if (colorScale)
          colors->push_back(colorUtils_->GainThresholdColor(static_cast<int>(gain)));
        else
          colors->push_back(color);

#ifdef DRAW_AXES
        // draw axes to visualize the vertex normals, every 10th point of the triangle fan
        if (fmod(std::distance(static_cast<std::vector<float>::const_reverse_iterator>(elevPoints.rbegin()), jriter), 10.0) == 0)
        {
          drawAxes_(pt, normalVec);
        }
#endif
      }
      antGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN, lastCount, verts->size() - lastCount));
    }
  } // end of (hRange < T_PI)

  osg::ref_ptr<osg::Geode> geode = new osg::Geode();
  geode->addDrawable(antGeom);

  // optimize the geode:
  osgEarth::Symbology::MeshConsolidator::run(*geode);
  antenna_->addChild(geode);
  applyScale_();
}
}
