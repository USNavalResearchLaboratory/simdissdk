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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_RCS_H
#define SIMVIS_RCS_H

#include "osg/Group"
#include "simCore/EM/RadarCrossSection.h"
#include "simCore/EM/Constants.h"
#include "simData/DataTypes.h"
#include "simVis/Utils.h"

namespace simVis
{

/// render a radar cross section (RCS) visually
class SDKVIS_EXPORT RCSRenderer
{
public:
  /** Constructor */
  RCSRenderer(
    double frequency = simCore::DEFAULT_FREQUENCY,
    simCore::PolarityType polarity = simCore::POLARITY_UNKNOWN,
    float elevation = 0.0f,
    float detail = 1.0f,
    const osg::Vec4& color = simVis::Color::White,
    bool colorOverride = false);

  virtual ~RCSRenderer();

  /// do the rendering
  void RenderRCS(simCore::RadarCrossSection* rcs, float scale = 1.0f, const osg::Quat& rot = osg::Quat());

public:  //runtime properties

  /**@name setters and getters
   *@{
   */
  bool setFrequency(double frequency);
  double getFrequency() const { return freq_; }

  bool setPolarity(simCore::PolarityType polarity);
  simCore::PolarityType getPolarity() const { return polarity_; }

  bool setElevation(float elevation);
  float getElevation() const { return elev_; }

  bool setDetail(float detail);
  float getDetail() const { return detail_; }

  bool setColor(const osg::Vec4& color);
  const osg::Vec4& getColor() const { return color_; }

  bool setColorOverride(bool colorOverride);
  bool getColorOverride() const { return colorOverride_; }

public:  //rendered node accessors
  osg::Node* getRCS2D() const { return rcs2D_.get(); }
  osg::Node* getRCS3D() const { return rcs3D_.get(); }
  ///@}

private:
  simVis::ColorUtils* colorUtils_;

  simCore::RadarCrossSection* rcs_;
  float scale_;
  osg::Quat rot_;

  simCore::PolarityType polarity_;      // polarization of pattern
  double freq_;                 // frequency of pattern
  float elev_;                  // elevation value to use for 2-D slice
  float detail_;                // angular resolution of pattern
  double min_, max_;            // rcs bounds of entire pattern
  int offset_;                  // pattern offset
  int zeroRing_;                // ring number for 0 dB
  bool colorOverride_;          // flag to denote if color_ should be used instead of default gradient
  osg::Vec4 color_;             // Color of RCS data
  float z_;                     // Z offset of 2D RCS
  bool useAlpha_;               // Whether to use alpha blending instead of stippling

  osg::ref_ptr<osg::Node> rcs2D_;
  osg::ref_ptr<osg::Node> rcs3D_;

  void initValues_();
  int computeRadius_(double azim, double elev, osg::Vec3f &p, float *rcsdB);

  void renderRcs_();
  osg::Node* render2D_();
  osg::Node* render3D_();
};

//-----------------------------------------------------------------------

/** Node that holds the Radar Cross Section graphics */
class SDKVIS_EXPORT RCSNode : public osg::Group
{
public:
  RCSNode(); // Locator* locator );

  ///@return true if the node loaded properly
  bool isValid() const { return loadedOK_; }

public:
  /**@name set properties
   *@{
   */
  void setScale(float scale) { scale_ = scale; }

  void setPrefs(const simData::PlatformPrefs& prefs);
  void setRcs(simCore::RadarCrossSectionPtr newRcs);
  ///@}

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "RCSNode"; }

protected:
  /// osg::Referenced-derived
  virtual ~RCSNode();

  /** Rebuilds the display of the node */
  void rebuild();
  /** Converts the simData polarity to a simCore polarity */
  simCore::PolarityType convertPolarity(simData::Polarity pol) const;

private:
  simCore::RadarCrossSectionPtr rcsData_;
  bool                          loadedOK_;
  float                         scale_;

  osgEarth::optional<simData::PlatformPrefs>      lastPrefs_;
  bool hasLastPrefs_;
};

} // namespace simVis

#endif  /* SIMVIS_RCS_H */
