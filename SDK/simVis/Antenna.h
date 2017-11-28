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
#ifndef SIMVIS_ANTENNA_H
#define SIMVIS_ANTENNA_H

#include "osg/MatrixTransform"
#include "osg/ref_ptr"
#include "osgEarth/Config"
#include "simCore/Common/Common.h"
#include "simCore/EM/Constants.h"
#include "simData/DataTypes.h"
#include "simVis/LocatorNode.h"

namespace simCore
{
  class AntennaPattern;
}

namespace simVis
{
  struct ColorUtils;

  /**
   * Represents an antenna pattern.
   */
  class SDKVIS_EXPORT AntennaNode : public simVis::LocatorNode
  {
  public:
    /**
     * Constructs an antenna node
     * @param[in ] rot   Rotation (optional)
     */
    explicit AntennaNode(simVis::Locator* locator, const osg::Quat& rot = osg::Quat());

    /**
     * Whether the antenna pattern loaded OK.
     */
    bool isValid() const { return loadedOK_; }

    /**
     * The range/scale of the antenna pattern
     * @param[in ] range Range for the antenna in meters
     */
    void setRange(float range);

    /**
    * Configures the antenna pattern from the beam prefs
    * @param[in] prefs preferences to use in configuring the antenna
    * @return flag that indicates whether the antenna graphic was rebuilt
    */
    bool setPrefs(const simData::BeamPrefs& prefs);

    /** calculate the antenna gain for given parameters */
    float PatternGain(float azim, float elev, simCore::PolarityType polarity) const;

    /** Return the proper library name */
    virtual const char* libraryName() const { return "simVis"; }

    /** Return the class name */
    virtual const char* className() const { return "AntennaNode"; }

  public: // LocatorNode interface
    virtual void syncWithLocator(); //override

  protected:
    /// osg::Referenced-derived
    virtual ~AntennaNode();

  private:
    /// apply the lighting pref
    void updateLighting_(bool shaded);

    /// apply the blending pref
    void updateBlending_(bool blending);

    // antennaPattern is scaled by the product of update range (in m) and pref beamScale (no units, 1.0 default)
    void applyScale_();

    /**
    * Draw axes at the specified pt, orienting the x-axis along the specified vector
    * @param[in] pos the position for the axes origin
    * @param[in] vec the orientation for the x-axis
    */
    void drawAxes_(const osg::Vec3f& pos, const osg::Vec3f& vec);

    float ComputeRadius_(float azim, float elev, simCore::PolarityType polarity, osg::Vec3f &p) const;
    void render_();

  private:
    simCore::AntennaPattern* antennaPattern_;
    bool                     loadedOK_;
    std::string              patternFile_;
    simCore::PolarityType    polarity_;

    float                    beamRange_;
    float                    beamScale_;
    float                    scaleFactor_;
    osg::Quat                rot_;
    float                    min_;
    float                    max_;
    osg::ref_ptr<osg::MatrixTransform> antenna_;
    osgEarth::optional<simData::BeamPrefs>      lastPrefs_;

    ColorUtils*      colorUtils_;
  };

} // namespace simVis

#endif  //SIMVIS_ANTENNA_H

