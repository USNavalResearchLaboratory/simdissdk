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
#include "osg/LineWidth"
#include "osg/MatrixTransform"
#include "osg/Depth"
#include "osgEarth/LineDrawable"

#include "simNotify/Notify.h"
#include "simCore/Calc/Angle.h"
#include "simCore/Calc/Math.h"
#include "simCore/EM/Decibel.h"
#include "simCore/EM/RadarCrossSection.h"

#include "simVis/Constants.h"
#include "simVis/PolygonStipple.h"
#include "simVis/Registry.h"
#include "simVis/Types.h"
#include "simVis/Utils.h"
#include "simVis/RCS.h"

#define LC "[simVis::RCSNode] "

namespace simVis {
//-------------------------------------------------------------------------

RCSNode::RCSNode() :
  loadedOK_(false),
  scale_(1.0f),
  hasLastPrefs_(false)
{
  auto* stateSet = getOrCreateStateSet();
  // Note that traversal order is needed to avoid issues with color blending when 2D and 3D both active
  stateSet->setRenderBinDetails(simVis::BIN_RCS, simVis::BIN_TRAVERSAL_ORDER_SIMSDK);
  // Turn off depth reads
  stateSet->setAttributeAndModes(new osg::Depth(osg::Depth::ALWAYS));
  // Lighting never affects RCS
  simVis::setLighting(stateSet, osg::StateAttribute::OFF);
}

RCSNode::~RCSNode()
{
}


void RCSNode::setPrefs(const simData::PlatformPrefs& prefs)
{
  const bool rebuildRequired =
    PB_FIELD_CHANGED(&lastPrefs_.get(), &prefs, drawrcs) ||
    PB_FIELD_CHANGED(&lastPrefs_.get(), &prefs, draw3drcs) ||
    PB_FIELD_CHANGED(&lastPrefs_.get(), &prefs, rcsfrequency) ||
    PB_FIELD_CHANGED(&lastPrefs_.get(), &prefs, rcscolor) ||
    PB_FIELD_CHANGED(&lastPrefs_.get(), &prefs, rcscolorscale) ||
    PB_FIELD_CHANGED(&lastPrefs_.get(), &prefs, rcsdetail) ||
    PB_FIELD_CHANGED(&lastPrefs_.get(), &prefs, rcselevation) ||
    PB_FIELD_CHANGED(&lastPrefs_.get(), &prefs, rcspolarity);

  lastPrefs_ = prefs;
  hasLastPrefs_ = true;

  if (rebuildRequired)
  {
    rebuild();
  }

  if (this->getNumChildren() == 2)
  {
    // use platform node mask?
    this->getChild(0)->setNodeMask(lastPrefs_->drawrcs() ? simVis::DISPLAY_MASK_PLATFORM : simVis::DISPLAY_MASK_NONE);
    this->getChild(1)->setNodeMask(lastPrefs_->draw3drcs() ? simVis::DISPLAY_MASK_PLATFORM : simVis::DISPLAY_MASK_NONE);
  }
}

simCore::PolarityType RCSNode::convertPolarity(simData::Polarity pol) const
{
  switch (pol)
  {
  case simData::POL_HORIZONTAL:
    return simCore::POLARITY_HORIZONTAL;
  case simData::POL_VERTICAL:
    return simCore::POLARITY_VERTICAL;
  case simData::POL_CIRCULAR:
    return simCore::POLARITY_CIRCULAR;
  case simData::POL_HORZVERT:
    return simCore::POLARITY_HORZVERT;
  case simData::POL_VERTHORZ:
    return simCore::POLARITY_VERTHORZ;
  case simData::POL_LEFTCIRC:
    return simCore::POLARITY_LEFTCIRC;
  case simData::POL_RIGHTCIRC:
    return simCore::POLARITY_RIGHTCIRC;
  case simData::POL_LINEAR:
    return simCore::POLARITY_LINEAR;
  case simData::POL_UNKNOWN:
    break;
  }
  return simCore::POLARITY_UNKNOWN;
}

void RCSNode::setRcs(simCore::RadarCrossSectionPtr newRcs)
{
  rcsData_ = newRcs;
  if (hasLastPrefs_)
    rebuild();
}

void RCSNode::rebuild()
{
  loadedOK_ = (rcsData_ != NULL) ? true : false;
  this->removeChildren(0, this->getNumChildren());

  if (rcsData_ && (lastPrefs_->drawrcs() || lastPrefs_->draw3drcs()))
  {
    const double frequency = lastPrefs_->rcsfrequency();
    const simCore::PolarityType polarity = convertPolarity(lastPrefs_->rcspolarity());
    const float elevation = lastPrefs_->rcselevation();
    const float detail = lastPrefs_->rcsdetail();
    osg::Vec4f color = ColorUtils::RgbaToVec4(lastPrefs_->rcscolor());
    // rcscolorscale protobuf controls use of gradient color scale, RCSRender wants a bool to set use of rcscolor
    // invert pref value for use of rcscolor
    const bool useOverrideColor = !lastPrefs_->rcscolorscale();

    RCSRenderer renderer(frequency, polarity, elevation, detail, color, useOverrideColor);

    // rotate 90 deg to match model rotation
    osg::Quat rot(osg::PI_2, osg::Vec3(0, 0, 1));
    renderer.RenderRCS(rcsData_.get(), scale_, rot);

    this->addChild(renderer.getRCS2D());
    this->getChild(0)->setNodeMask(lastPrefs_->drawrcs() ? simVis::DISPLAY_MASK_PLATFORM : simVis::DISPLAY_MASK_NONE);

    this->addChild(renderer.getRCS3D());
    this->getChild(1)->setNodeMask(lastPrefs_->draw3drcs() ? simVis::DISPLAY_MASK_PLATFORM : simVis::DISPLAY_MASK_NONE);
  }
}

//-------------------------------------------------------------------------

RCSRenderer::RCSRenderer(double frequency,
  simCore::PolarityType polarity,
  float elevation,
  float detail,
  const osg::Vec4& color,
  bool colorOverride) :
  rcs_(NULL),
  polarity_(polarity),
  freq_(frequency),
  elev_(elevation),
  detail_(10.0),
  min_(HUGE_VAL),
  max_(-HUGE_VAL),
  offset_(-1),
  zeroRing_(0),
  colorOverride_(colorOverride),
  color_(color),
  z_(0.0f),
  useAlpha_(false)
{
  setDetail(detail);
  colorUtils_ = new simVis::ColorUtils(useAlpha_ ? 0.3f : 1.0f);
}

RCSRenderer::~RCSRenderer()
{
  delete colorUtils_;
}

void RCSRenderer::RenderRCS(simCore::RadarCrossSection* rcs, float scale, const osg::Quat& rot)
{
  rcs_   = rcs;
  scale_ = scale;
  rot_   = rot;

  renderRcs_();
}

void RCSRenderer::renderRcs_()
{
  if (rcs_)
  {
    initValues_();

    osg::Node* rcs3D = render3D_();
    osg::Node* rcs2D = render2D_();

    double scaleVal = osg::absolute(max_) + offset_;
    scaleVal = (scaleVal <=0) ? scale_ : scale_ / scaleVal;

    osg::MatrixTransform* mt2D = new osg::MatrixTransform;
    mt2D->setMatrix(osg::Matrixf::scale(scaleVal, scaleVal, 1.0f) * osg::Matrix::rotate(rot_));
    mt2D->addChild(rcs2D);
    rcs2D_ = mt2D;

    osg::MatrixTransform* mt3D = new osg::MatrixTransform;
    mt3D->setMatrix(osg::Matrixf::scale(scaleVal, scaleVal, scaleVal) * osg::Matrix::rotate(rot_));
    mt3D->addChild(rcs3D);
    rcs3D_ = mt3D;
  }
  else
  {
    rcs3D_ = NULL;
    rcs2D_ = NULL;
  }
}

void RCSRenderer::initValues_()
{
  zeroRing_ = 0;
  min_ = HUGE_VAL;
  max_ = -HUGE_VAL;

  offset_ = 0;
  simCore::RCSLUT *rcslut = dynamic_cast<simCore::RCSLUT*>(rcs_);
  if (rcslut)
  {
    min_ = rcslut->min();
    max_ = rcslut->max();
  }
  else
  {
    // calculate RCS bounds
    for (int i = -180; i <= 180; i++)
    {
      double azim = simCore::DEG2RAD*(i);
      for (int j = -90; j <= 90; j++)
      {
        double elev = simCore::DEG2RAD*(j);
        double radius = rcs_->RCSdB(freq_, azim, elev, polarity_);
        if (radius > simCore::SMALL_DB_COMPARE)
          min_ = osg::minimum(min_, radius);
        max_ = osg::maximum(max_, radius);
      }
    }
  }

  if (min_ > 0)
  {
    zeroRing_ = 0;
    offset_ = 10;
  }
  else
  {
    do
    {
      offset_ += 10;
      zeroRing_++;
    } while (offset_ < -min_);

    offset_ += 10;
  }
}

int RCSRenderer::computeRadius_(double azim, double elev, osg::Vec3f &p, float *rcsdB)
{
  // values returned from RCS lookup table are in dB
  float rcsValue = rcs_->RCSdB(freq_, azim, elev, polarity_);
  *rcsdB = rcsValue;
  rcsValue = osg::maximum((float)offset_ + rcsValue, 0.0f);

  // convert azim & elev to a rectangular coordinate
  // course is off Y, elevation off horizon...
  p.set(rcsValue * cos(-azim) * cos(elev),
    rcsValue * sin(-azim) * cos(elev),
    rcsValue * sin(elev));

  return static_cast<int>(rcsValue);
}

osg::Node* RCSRenderer::render2D_()
{
  osgEarth::LineGroup* lineGroup = new osgEarth::LineGroup();
  lineGroup->setName("simVis::RCS");

  if (rcs_)
  {
    osgEarth::LineDrawable* crosshair = new osgEarth::LineDrawable(GL_LINES);
    crosshair->setDataVariance(osg::Object::DYNAMIC);
    crosshair->setUseVertexBufferObjects(true);
    crosshair->setLineWidth(3);
    lineGroup->addDrawable(crosshair);

    double elev = simCore::DEG2RAD * elev_;
    int end = 2 + static_cast<int>(osg::absolute(max_)/10.0);

    // draw cross hair
    float chLength = (float)((zeroRing_ + end + 1) * 10);
    crosshair->pushVertex(osg::Vec3(0.0f, chLength, 0.0f + z_));
    crosshair->pushVertex(osg::Vec3(0.0f, -chLength, 0.0f + z_));
    crosshair->pushVertex(osg::Vec3(chLength, 0.0f, 0.0f + z_));
    crosshair->pushVertex(osg::Vec3(-chLength, 0.0f, 0.0f + z_));
    crosshair->setColor(osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f));
    crosshair->dirty();

    // draw polar rings
    float rcsValue = 10;

    const osg::Vec4 grey(0.4f, 0.4f, 0.4f, 1.0f);

    for (int j = 0; j < (zeroRing_ + end); j++)
    {
      osgEarth::LineDrawable* polarRing = new osgEarth::LineDrawable(GL_LINE_LOOP);
      polarRing->setDataVariance(osg::Object::DYNAMIC);
      polarRing->setUseVertexBufferObjects(true);
      polarRing->setLineWidth(3);
      lineGroup->addDrawable(polarRing);

      for (int i = 0; i < 36; i ++)
      {
        double azim = simCore::DEG2RAD * i * 10;
        polarRing->pushVertex(osg::Vec3(rcsValue * cos(azim), rcsValue * sin(azim), 0.0f + z_));
      }

      if (j == zeroRing_)
        polarRing->setColor(simVis::Color::White);  // white 0 dB ring
      else
        polarRing->setColor(grey); // gray rings every 10 dB

      polarRing->dirty();
      rcsValue += 10;
    }

    // draw RCS
    osgEarth::LineDrawable* rcs = new osgEarth::LineDrawable(GL_LINE_LOOP);
    rcs->setDataVariance(osg::Object::DYNAMIC);
    rcs->setUseVertexBufferObjects(true);
    rcs->setLineWidth(3);
    lineGroup->addDrawable(rcs);

    for (int i = 0; i < 360; i++)
    {
      double azim = simCore::DEG2RAD * i;

      rcsValue = rcs_->RCSdB(freq_, azim, elev, polarity_);
      // offset RCS value by specified dB for plot
      rcsValue = osg::maximum((float)offset_ + rcsValue, 0.0f);

      // course is off Y, elevation off horizon...
      float x = rcsValue * cos(-azim);
      float y = rcsValue * sin(-azim);
      rcs->pushVertex(osg::Vec3(x, y, 0 + z_));
    }

    if (colorOverride_)
      rcs->setColor(color_);
    else
      rcs->setColor(simVis::Color::Yellow);
    rcs->dirty();
  }

  return lineGroup;
}

osg::Node* RCSRenderer::render3D_()
{
  osg::Geode* geode = new osg::Geode();

  if (!useAlpha_)
    simVis::PolygonStipple::setValues(geode->getOrCreateStateSet(), true, 0);

  if (rcs_)
  {
    osg::Geometry* rcsGeom = new osg::Geometry();
    rcsGeom->setName("simVis::RCS");
    rcsGeom->setDataVariance(osg::Object::DYNAMIC);

    rcsGeom->setUseVertexBufferObjects(true);

    geode->addDrawable(rcsGeom);

    osg::Vec3Array* verts = new osg::Vec3Array();
    rcsGeom->setVertexArray(verts);

    osg::Vec3Array* norms = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    rcsGeom->setNormalArray(norms);

    osg::Vec4Array* colors = new osg::Vec4Array(osg::Array::BIND_PER_VERTEX);
    rcsGeom->setColorArray(colors);

    double radDetail = simCore::DEG2RAD * detail_;
    float rcsdB;

    int lastCount = 0;
    for (int jj = -90; jj < 90; jj += static_cast<int>(detail_))
    {
      double elev = simCore::DEG2RAD * jj;
      for (int ii = 0; ii <= 360; ii += static_cast<int>(detail_))
      {
        double azim = simCore::DEG2RAD * ii;

        // compute first point in t-strip
        osg::Vec3f pt;
        computeRadius_(azim, elev, pt, &rcsdB);

        osg::Vec3f ptNorm(pt);
        ptNorm.normalize();

        verts->push_back(pt);
        norms->push_back(ptNorm);

        if (colorOverride_)
          colors->push_back(color_);
        else
          colors->push_back(colorUtils_->GainThresholdColor(static_cast<int>(rcsdB)));

        // compute alternate point in t-strip
        osg::Vec3f ptne;
        computeRadius_(azim, (elev + radDetail), ptne, &rcsdB);

        osg::Vec3f ptneNorm(ptne);
        ptneNorm.normalize();

        verts->push_back(ptne);
        norms->push_back(ptneNorm);

        if (colorOverride_)
          colors->push_back(color_);
        else
          colors->push_back(colorUtils_->GainThresholdColor(static_cast<int>(rcsdB)));
      }

      rcsGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, lastCount, verts->size() - lastCount));
      lastCount = verts->size();
    }
  }

  return geode;
}

bool RCSRenderer::setFrequency(double frequency)
{
  if (freq_ != frequency)
  {
    freq_ = frequency;
    renderRcs_();

    return true;
  }

  return false;
}

bool RCSRenderer::setPolarity(simCore::PolarityType polarity)
{
  if (polarity_ != polarity)
  {
    polarity_ = polarity;
    renderRcs_();

    return true;
  }

  return false;
}

bool RCSRenderer::setElevation(float elevation)
{
  if (elev_ != elevation)
  {
    elev_ = elevation;
    renderRcs_();

    return true;
  }

  return false;
}

bool RCSRenderer::setDetail(float detail)
{
  if (detail < 1.0)
    detail = 1.0;

  if (detail_ != detail)
  {
    detail_ = detail;
    renderRcs_();

    return true;
  }

  return false;
}

bool RCSRenderer::setColor(const osg::Vec4& color)
{
  if (color_ != color)
  {
    color_.set(color.r(), color.g(), color.b(), color.a());

    if (colorOverride_)
      renderRcs_();

    return true;
  }

  return false;
}

bool RCSRenderer::setColorOverride(bool colorOverride)
{
  if (colorOverride_ != colorOverride)
  {
    colorOverride_ = colorOverride;
    renderRcs_();

    return true;
  }

  return false;
}

}
