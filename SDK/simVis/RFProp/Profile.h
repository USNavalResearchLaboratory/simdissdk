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
#ifndef SIMVIS_RFPROP_PROFILE_H
#define SIMVIS_RFPROP_PROFILE_H

#include <map>
#include "osg/Group"
#include "osg/Geode"
#include "simCore/Common/Common.h"
#include "simCore/Calc/Vec3.h"
#include "simCore/Calc/Math.h"
#include "simVis/RFProp/ColorProvider.h"
#include "simVis/RFProp/ProfileDataProvider.h"

namespace osg {
  class MatrixTransform;
  class DrawElementsUInt;
}

namespace simRF
{
class CompositeProfileProvider;

/** Responsible for rendering a single profile of data. */
class SDKVIS_EXPORT Profile : public osg::Group
{
public:
  /**
   * Creates a new Profile
   * @param data The ProfileDataProvider to use for the Profile
   */
  Profile(CompositeProfileProvider* data);

  /** Draw style for the profile */
  enum DrawMode
  {
    DRAWMODE_2D_HORIZONTAL,
    DRAWMODE_2D_VERTICAL,
    DRAWMODE_2D_TEE,
    DRAWMODE_3D,
    DRAWMODE_3D_TEXTURE,
    DRAWMODE_3D_POINTS,
    DRAWMODE_RAE,
  };

  /** Adds a ProfileDataProvider to our CompositeProfileProvider */
  void addProvider(ProfileDataProvider* provider);

  /** Gets the DrawMode for this Profile */
  DrawMode getMode() const;

  /** Sets the DrawMode for this Profile */
  void setMode(DrawMode mode);

  /** Gets the DataProvider for this Profile */
  const CompositeProfileProvider* getDataProvider() const;

  /** Sets the DataProvider for this Profile */
  void setDataProvider(CompositeProfileProvider* dataProvider);

  /** Gets the display thickness, in meters for this Profile. */
  float getDisplayThickness() const;

  /** Sets the display thickness, in meters for this Profile. This setting effects the DRAWMODE_3D DisplayMode, as well as 3D Points and 3D Texture. */
  void setDisplayThickness(float displayThickness);

  /** Gets the alpha of this Profile */
  float getAlpha() const;

  /**
   * Sets the alpha of this Profile.
   * @param alpha The alpha value.  Valid ranges are 0 (completely transparent) to 1 (completely opaque)
   */
  void setAlpha(float alpha);

  /**
  * Sets the terrain heights for this Profile.
  * @param terrainHeights Map of terrain heights for this Profile keyed on ground range, both values in meters
  */
  void setTerrainHeights(const std::map<float, float>& terrainHeights);

  /** Gets the bearing of the Profile in radians */
  double getBearing() const;

  /** Sets the bearing of the Profile in radians */
  void setBearing(double bearing);

  /** Gets the half beam width in radians */
  double getHalfBeamWidth() const;

  /** Sets the half beam width in radians */
  void setHalfBeamWidth(double halfBeamWidth);

  /** Gets the height to use for this Profile */
  double getHeight() const;

  /**
   * Sets the height to use for this Profile
   * @param height The height to use when displaying this Profile.
   *       In DRAWMODE_2D_HORIZONTAL and DRAWMODE_2D_TEE this is the height that is sampled for the horizontal wedges.
   *       In DRAWMODE_3D this is the start height for the bottom of the voxels used to sample.  The range of sampled height indices will
   *       be in the range height, height+displayThickness
   */
  void setHeight(double height);

  /** Gets whether to treat the height value as AGL. */
  bool getAGL() const;

  /**
   * Sets whether to treat the height value as AGL.
   * @param agl If true, the height value is considered AGL for DRAWMODE_2D_HORIZONTAL and the terrainHeights array will be used to determine the actual height value to sample from the DataProvider.
   */
  void setAGL(bool agl);

  /** Gets the reference latitude in radians */
  double getRefLat() const;

  /** Gets the reference longitude in radians */
  double getRefLon() const;

  /** Gets the reference altitude in meters */
  double getRefAlt() const;

  /** Get whether this Profile should conform to a spherical earth */
  bool getSphericalEarth() const;

  /** Set whether this Profile should conform to a spherical earth */
  void setSphericalEarth(bool sphericalEarth);

  /** Get elevation angle in radians */
  double getElevAngle() const;

  /** Set elevation angle in radians */
  void setElevAngle(double elevAngleRad);

  /**
   * Sets the reference coordinate
   * @param latRad The latitude in radians
   * @param lonRad The longitude in radians
   * @param alt The altitude in meters
   */
  void setRefCoord(double latRad, double lonRad, double alt);

  /** Get current active threshold type */
  ProfileDataProvider::ThresholdType getThresholdType() const;

  /** Set threshold type, selects a data provider of that type, if one exists */
  void setThresholdType(ProfileDataProvider::ThresholdType type);

  /** On update visitor, re-initialize when dirty */
  virtual void traverse(osg::NodeVisitor& nv);

  /** Dirty this Profile causing it to be redrawn. */
  void dirty();

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simRF"; }

  /** Return the class name */
  virtual const char* className() const { return "Profile"; }

protected:
  /// osg::Referenced-derived
  virtual ~Profile();

  /** Performs initialization at construction time */
  void init_();

  /** Initializes as a 2D horizontal */
  void init2DHoriz_();
  /** Initializes as a 2D Vertical */
  void init2DVert_();
  /** Initializes as a 3D */
  void init3D_();
  /** Initializes as a 3D texture */
  void init3DTexture_();
  /** Initializes as a 3D points */
  void init3DPoints_();
  /** Initializes as RAE */
  void initRAE_();

  /** Gets the DataProvider for this Profile, non-const version */
  CompositeProfileProvider* getDataProvider_();

  /** Creates an image representing the loss values */
  osg::Image* createImage_();

  struct VoxelParameters;

  /** Creates a voxel (volume pixel) at the given location */
  const void buildVoxel_(const VoxelParameters& vParams, const simCore::Vec3& tpSphereXYZ, double heightRangeRatio, unsigned int rangeIndex, osg::Geometry* geometry);

  /** Fixes the orientation of the profile */
  void updateOrientation_();

  /** Adjusts based on spherical XYZ */
  void adjustSpherical_(osg::Vec3& v, const simCore::Vec3& tpSphereXYZ);

  /** Retrieves the profile height at the given ground range in meters */
  float getTerrainHgt_(float gndRng) const;

  /** Bearing of the profile in radians */
  double bearing_;

  /** Profile display thickness */
  float displayThickness_;
  /** Height of vertical slots */
  double height_;
  /** Half of the beam width in radians */
  double halfBeamWidth_;

  /** Transform for positioning the profile */
  osg::MatrixTransform* transform_;

  /** Range vs terrain heights; TODO: Ask Glenn why this is cached instead of requesting the hgts from osgEarth */
  std::map<float, float> terrainHeights_;

  /** Array of vertices for the profile */
  osg::ref_ptr<osg::Vec3Array> verts_;
  /** Holds the geode graphics */
  osg::ref_ptr<osg::Geode> geode_;
  /** Values for the profile */
  osg::ref_ptr<osg::FloatArray> values_;

  /** Data provider */
  osg::ref_ptr<CompositeProfileProvider> data_;
  /** Indicates profile needs updating */
  bool dirty_;
  /** Alpha value to apply to drawn pixels */
  float alpha_;
  /** Flags Above-Ground-Level mode */
  bool agl_;
  /** Draw mode */
  DrawMode mode_;
  /** Reference coordinate for placing the center of the profile */
  simCore::Vec3 refCoord_;
  /** Flags spherical vs flat earth */
  bool sphericalEarth_;
  /** Elevation angle in radians */
  double elevAngle_;

  /** Profile's horizontal beam extents */
  double cosTheta0_;
  double sinTheta0_;
  double cosTheta1_;
  double sinTheta1_;

  /** Texture for the textured mode */
  osg::ref_ptr<osg::Texture> texture_;
  /** Uniform shader value for adjusting the alpha */
  osg::ref_ptr<osg::Uniform> alphaUniform_;

private:
  /** Copy constructor, not implemented or available. */
  Profile(const Profile&);

  /** Tesselate the 2D Vertical with tringle strip */
  const void tesselate2DVert_(unsigned int numRanges, unsigned int numHeights, unsigned int startIndex, osg::ref_ptr<osg::FloatArray> values, osg::Geometry* geometry);
};
}

#endif /* SIMVIS_RFPROP_PROFILE_H */
