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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMUTIL_VELOCITYPARTICLELAYER_H
#define SIMUTIL_VELOCITYPARTICLELAYER_H

#include "osg/ref_ptr"
#include "osg/Vec4"
#include "osg/Texture2D"
#include "osgEarth/Bounds"
#include "osgEarth/Color"
#include "osgEarth/ImageLayer"
#include "simCore/Common/Common.h"
#include "simVis/GradientShader.h"

namespace simUtil {

class VelocityTextureNode;
using osgEarth::optional; // required for OE_OPTION

/**
 * Layer that generates a particle system to represent velocity data in a patch on the world.  Velocity
 * data is fed using either a URI or in-memory texture.  The texture data encodes velocity in the red
 * and green elements of the image, such that red corresponds to X velocity and green is Y velocity.
 * The velocity values are scaled between -25 and +25 m/s, corresponding to pixel values [0.,1.] (or
 * [0,255]).  The output is a series of particles color coded based on velocity.  The particles update
 * per frame based on speed.  The particles are rendered as circles by default, or could be rendered
 * using a sprite texture that is rotated in the direction of velocity.
 *
 * This was designed for use with wind velocities, but could easily be adapted to ocean currents or
 * even vehicle traffic.
 *
 * Adapted from osgearth_wind example by Jason Beverage, Pelican Mapping, with permission.
 */
class SDKUTIL_EXPORT VelocityParticleLayer : public osgEarth::ImageLayer
{
public:
  /** Options related to velocity texture layers. */
  class SDKUTIL_EXPORT Options : public osgEarth::ImageLayer::Options
  {
  public:
    META_LayerOptions(osgEarth, Options, osgEarth::ImageLayer::Options);

    /** Dimension of particles texture (width and height).  Squared, this is the number of particles to render. */
    OE_OPTION(unsigned int, particleDimension);
    /** Zero: Particles do not slowly die; non-zero: speed at which particles die.  Lower values reduce life faster. */
    OE_OPTION(float, dieSpeed);
    /** Factor multiplied against the particles to slow down movement. */
    OE_OPTION(float, speedFactor);
    /** Size of the particle on screen */
    OE_OPTION(float, pointSize);
    /** Likelihood that particle will spontaneously drop, outside its typical life reduction from dieSpeed. */
    OE_OPTION(float, dropChance);
    /** Altitude of the particle in meters.  Particles are obscured by terrain. */
    OE_OPTION(float, particleAltitude);
    /** Bounding lat/lon values for the velocity texture.  Values are in degrees. */
    OE_OPTION(osgEarth::Bounds, boundingBox);
    /** URI for the sprite for particle points. May be blank to use dots instead. */
    OE_OPTION(osgEarth::URI, spriteUri);
    /** URI for the velocity texture.  Velocity textures encode R=X velocity, G=Y velocity, on a scale from [0,1], mapping to velocities [-25,25]. */
    OE_OPTION(osgEarth::URI, velocityTextureUri);
    /** GLSL code fragment to convert velocity texel "t" into a vec2 velocity, e.g.: "mix(vec2(-25.0, -25.0), vec2(25.0, 25.0), t.rg)" */
    OE_OPTION(std::string, texelToVelocityFragment);

    /** Gradient shader defines the gradient to use when rendering. */
    OE_OPTION(simVis::GradientShader, gradient);

    /** Create the Config from this options structure. */
    virtual osgEarth::Config getConfig() const override;

  private:
    /** Called by constructors to initialize options */
    void fromConfig(const osgEarth::Config& conf);
  };

  META_Layer(osgEarth, VelocityParticleLayer, Options, ImageLayer, VelocityParticleImage);

  /** Sets the velocity data texture from an in-memory texture object. Clears the Velocity URI. */
  void setVelocityTexture(osg::Texture2D* texture);
  /** Sets the point sprite texture from an in-memory texture object. Clears the Sprite URI. */
  void setPointSprite(osg::Texture2D* pointSprite);

  unsigned int getParticleDimension() const;
  void setParticleDimension(unsigned int value);
  float getDieSpeed() const;
  void setDieSpeed(float value);
  float getSpeedFactor() const;
  void setSpeedFactor(float value);
  float getPointSize() const;
  void setPointSize(float value);
  float getDropChance() const;
  void setDropChance(float value);
  float getParticleAltitude() const;
  void setParticleAltitude(float value);
  osgEarth::Bounds getBoundingBox() const;
  void setBoundingBox(const osgEarth::Bounds& bounds);
  const osgEarth::URI& getVelocityTexture() const;
  void setVelocityTexture(const osgEarth::URI& uri);
  const osgEarth::URI& getPointSprite() const;
  void setPointSprite(const osgEarth::URI& uri);
  const simVis::GradientShader& getGradient() const;
  void setGradient(const simVis::GradientShader& gradient);
  std::string getTexelToVelocityFragment() const;

  /**
   * Sets the GLSL fragment that is used to convert from the velocity texel to an absolute X-east Y-north velocity vec2.
   * The texel is extracted from the Velocity Texture and is a vec4 containing the red, green, blue, and alpha components
   * in typical [0.0, 1.0] range.  The fragment is expected to calculate the velocity in meters per second given that texel.
   * This is a single GLSL statement that is expected to create a vec2 value.  The default implementation is:
   *
   * <code>
   * mix(vec2(-25.0, -25.0), vec2(25.0, 25.0), t.rg)
   * </code>
   *
   * In the <VelocityParticleImage> the tag might look like:
   *
   * <code>
   * <VelocityParticleImage>
   *   ...
   *   <texel_to_velocity_fragment>mix(vec2(-25.0, -25.0), vec2(25.0, 25.0), t.rg)</texel_to_velocity_fragment>
   * </VelocityParticleImage>
   * </code>
   *
   * Important: The texel is always available as the variable "t".  For details on integration, refer to
   * VelocityParticleLayer.compute.pos.frag.glsl and VelocityParticleLayer.compute.dir.frag.glsl.
   *
   * The default implementation above presumes that the red pixels encode velocity X with 0 as -25 m/s, and 1.0 (full red)
   * as +25 m/s; it presumes the same for the green pixels, encoding velocity Y. If empty or unspecified, this default mapping
   * is used.
   *
   * This fragment can be adapted to read images that encode with different minimum or maximum values, and can also be
   * adapted to convert from other encodings.  For example, one might encode their image with red as a clockwise direction
   * and green as a speed; this fragment could perform the math to convert these values into a velocity X and Y value.
   */
  void setTexelToVelocityFragment(const std::string& glslFragment);

  // From ImageLayer:
  virtual osgEarth::Status openImplementation() override;
  // From VisibleLayer:
  virtual void setOpacity(float value) override;
  // From Layer:
  virtual osg::Node* getNode() const override;

protected:
  virtual ~VelocityParticleLayer();

private:
  /** Helper function to cast node_; due to ref_ptr issues, can't store node_ as a VelocityTextureNode */
  VelocityTextureNode* getNode_() const;
  /** Recreates the point sprite from options if needed */
  void recreatePointSprite_();

  /** Using _options->velocityTextureUri(), attempts to create the velocity texture, returning 0 on success.  velocityTexture_ is updated. */
  int readAndSetVelocityTexture_();

  osg::ref_ptr<osg::Group> node_;
  osg::ref_ptr<osg::Texture2D> velocityTexture_;
  osg::ref_ptr<osg::Texture2D> pointSprite_;
};

}

#endif /* SIMUTIL_VELOCITYPARTICLELAYER_H */
