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
#ifndef SIMUTIL_VELOCITYPARTICLELAYER_H
#define SIMUTIL_VELOCITYPARTICLELAYER_H

#include "osg/ref_ptr"
#include "osg/Vec4"
#include "osg/Texture2D"
#include "osgEarth/Bounds"
#include "osgEarth/Color"
#include "osgEarth/ImageLayer"
#include "simCore/Common/Common.h"

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
    /** Output color for minimum velocity in the velocity texture. */
    OE_OPTION(osgEarth::Color, minColor);
    /** Output color for maximum velocity in the velocity texture. */
    OE_OPTION(osgEarth::Color, maxColor);
    /** URI for the sprite for particle points. May be blank to use dots instead. */
    OE_OPTION(osgEarth::URI, spriteUri);
    /** URI for the velocity texture.  Velocity textures encode R=X velocity, G=Y velocity, on a scale from [0,1], mapping to velocities [-25,25]. */
    OE_OPTION(osgEarth::URI, velocityTextureUri);

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
  const osg::Vec4& getMinColor() const;
  void setMinColor(const osg::Vec4& minColor);
  const osg::Vec4& getMaxColor() const;
  void setMaxColor(const osg::Vec4& maxColor);
  const osgEarth::URI& getVelocityTexture() const;
  void setVelocityTexture(const osgEarth::URI& uri);
  const osgEarth::URI& getPointSprite() const;
  void setPointSprite(const osgEarth::URI& uri);

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
