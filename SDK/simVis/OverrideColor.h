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
#ifndef SIMVIS_OVERRIDE_COLOR_H
#define SIMVIS_OVERRIDE_COLOR_H

#include "osgEarth/VirtualProgram"
#include "simCore/Common/Common.h"
#include "simVis/Types.h"

namespace simVis
{

/**
 * Injects a color override shader component into a state set.
 */
class SDKVIS_EXPORT OverrideColor : public osg::Referenced
{
public:
  /**
   * Construct a color overrider and install it in a state set.
   * It will automatically uninstall when this object destructs.
   */
  explicit OverrideColor(osg::StateSet* stateset);

  /**
   * Sets the override color.
   */
  void setColor(const simVis::Color& color);

#ifdef USE_DEPRECATED_SIMDISSDK_API
  /**
   * Gets the override color.
   * @deprecated Color may not be stored in future versions of SIMDIS SDK.  If still
   *   required in the future, this may be replaced with a more expensive call into
   *   the shader uniform value, returning a copy of the color instead of a reference.
   */
  const simVis::Color& getColor() const;
#endif

  static const std::string OVERRIDECOLOR_UNIFORM;
protected:
  /// osg::Referenced-derived
  virtual ~OverrideColor();

private:
  /** Lazy creation on the shader */
  void createShader_();

  osg::observer_ptr<osg::StateSet> stateset_;
#ifdef USE_DEPRECATED_SIMDISSDK_API
  simVis::Color color_;
#endif
  bool supported_;
  bool shaderCreated_;
};

} // namespace simVis

#endif // SIMVIS_OVERRIDE_COLOR_H

