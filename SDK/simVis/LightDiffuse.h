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
#ifndef SIMVIS_LIGHTDIFFUSE_H
#define SIMVIS_LIGHTDIFFUSE_H

#ifdef USE_DEPRECATED_SIMDISSDK_API

#include "osg/StateAttribute"
#include "simCore/Common/Common.h"
#include "simVis/Types.h"

namespace osg { class Light; }

namespace simVis
{

/**
 * State attribute that controls the diffuse value of a given light.  Can be used
 * to override the diffuse value in a particular scene graph node.  Similar to
 * implementation of osg::Light, but without requiring specification of all light-
 * related parameters.
 *
 * @deprecated Use osg::Uniform("osg_LightSource[0].diffuse", osg::Vec4f(...)); instead
 */
class SDKVIS_EXPORT LightDiffuse : public osg::StateAttribute
{
public:
  /** OSG boilerplate */
  META_StateAttribute(simVis, LightDiffuse, simVis::LIGHT_DIFFUSE);

  /** Default constructor on light 0 */
  LightDiffuse();
  /** Constructor for a specific diffuse value */
  LightDiffuse(const simVis::Color& diffuse, unsigned int lightNum=0);
  /** Act as a slave to the provided light, using its values for diffuse. */
  LightDiffuse(osg::Light* light);
  /** OSG copy constructor */
  LightDiffuse(const LightDiffuse& rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY);

  /** Set the diffuse light value to provided color; clears the master light. */
  void setDiffuse(const simVis::Color& color);
  /** Set the diffuse light value using the provided magnitude for RGB, with 1.f alpha. */
  void setDiffuse(float magnitude);
  /** Retrieves the diffuse color setting */
  const simVis::Color& diffuse() const;

  /** Changes the light number being updated; clears the master light. */
  void setLightNum(unsigned int lightNumber);
  /** Retrieves the light number being updated */
  unsigned int lightNum() const;

  /** Use values from the light instead of internal values. */
  void setLightMaster(osg::Light* lightMaster);
  /** Returns the pointer to the light master, if any */
  osg::Light* lightMaster() const;
  /** Returns true if using the light master. */
  bool useLightMaster() const;

  /** Override getMember() to return the light number */
  virtual unsigned int getMember() const;

  /** @see osg::StateAttribute::compare() */
  virtual int compare(const StateAttribute& sa) const;

  /** Apply the light color state to the OpenGL state machine. */
  virtual void apply(osg::State& state) const;

protected:
  virtual ~LightDiffuse();

private:
  /// Color to apply to the light diffuse value
  simVis::Color diffuse_;
  /// OpenGL light number being modified
  unsigned int lightNum_;

  /// Slave our values to this light
  osg::observer_ptr<osg::Light> lightMaster_;

  /// Indicates we should be using the light master; note that this can be true and light set to NULL
  bool useLightMaster_;
};

}

#endif /* USE_DEPRECATED_SIMDISSDK_API */

#endif /* SIMVIS_LIGHTDIFFUSE_H */
