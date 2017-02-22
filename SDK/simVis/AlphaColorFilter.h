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
#ifndef SIMVIS_ALPHA_COLOR_FILTER_H
#define SIMVIS_ALPHA_COLOR_FILTER_H

#include "simVis/Entity.h"
#include <osgEarthUtil/Common>
#include <osgEarth/ColorFilter>
#include <osg/Uniform>

namespace simVis
{
  /**
  * Implementation of an osgEarth::ColorFilter that applies a filter to alpha values.
  *
  * rescale the alpha value to match the new 'clear' (minimum) and 'opaque' (maximum) values set.
  * Updates the pixel's alpha value, such that any pixel's alpha < 'clear' is set to 0.0, any pixel's alpha value > 'opaque'
  * is set to 1.0, and any pixel's alpha between 'clear' and 'opaque' will be recalculated to the
  * new scale defined by the clear/opaque values, based on original scale of 0.0-1.0.
  * Note that the 'clear' value must be < the 'opaque' value, or it will reset them to 0.0 and 1.0
  */
  class SDKVIS_EXPORT AlphaColorFilter : public osgEarth::ColorFilter
  {
  public:
    AlphaColorFilter();

    /**
     * Whether the system supports this filter
     */
    static bool isSupported();

    /**
    * Sets the values of the 'clear' and 'opaque' values based on
    * what is passed in the Config object.  If the 'clear' <= 'opaque'
    * resets the values to 0.0 and 1.0 respectively.  'clear' is
    * set using "c", 'opaque' is set using "o".
    * @param[in] conf the Config defining 'clear' and 'opaque'
    */
    AlphaColorFilter(const osgEarth::Config& conf);

    /**
    * Set the 'clear' and 'opaque' values using a Vec2f.  Expects the value at index 0 is 'clear',
    * value at index 1 is 'opaque'.
    * @param[in] clearOpaqueValues the new 'clear' and 'opaque' values
    */
    void setAlphaOffset(const osg::Vec2f& clearOpaqueValues);

    /**
    * Get the 'clear' and 'opaque' values in a Vec2f.  The value at index 0 is 'clear', the value
    * at index 1 is 'opaque'.
    * @return Vec2f, a float vector with 2 values, 'clear' and 'opaque'
    */
    osg::Vec2f getAlphaOffset() const;

    /**
    * Enable/disable the color filter.
    * @param enabled whether the filter is to be enabled or disabled
    */
    void setEnabled(bool enabled);

    /**
    * Returns whether the filter is enabled.
    * @return true if filter is enabled
    */
    bool getEnabled() const;

    //---------------------------------------------------------------------
    // inherited from ColorFilter

    /**
     * The name of the function to call in the custom shader. This function
     * must have the signature:
     *
     *    void function(in int slot, inout vec4 color)
     *
     * Failure to match this signature will result in a shader compilation error.
     */
    virtual std::string getEntryPointFunctionName() const;

    /**
     * Installs any uniforms or other bindings required by this filter on the
     * provided state set.
     */
    virtual void install(osg::StateSet* stateSet) const;

    /**
     * Serializes this object to a Config (optional).
     */
    virtual osgEarth::Config getConfig() const;

  private: // methods
    virtual ~AlphaColorFilter() {} // osg::Referenced object
    void init_();

  private: // data
    unsigned int instanceId_; /// local instance id counter
    osg::ref_ptr<osg::Uniform> alpha_; /// the local alpha 'clear' and 'opaque' values
  };
}

#endif

