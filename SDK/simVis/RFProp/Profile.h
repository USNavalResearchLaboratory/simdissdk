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

  /** Gets the display thickness, in # of height steps for this Profile. */
  unsigned int getDisplayThickness() const;

  /** Sets the display thickness, in # of height steps for this Profile. This setting effects the DRAWMODE_3D DisplayMode, as well as 3D Points and 3D Texture. */
  void setDisplayThickness(unsigned int displayThickness);

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

  class VoxelProcessor;
  class RahVoxelProcessor;
  class RaeVoxelProcessor;

  /** Creates a voxel (volume pixel) at the given location */
  int buildVoxel_(VoxelProcessor& vProcessor, const simCore::Vec3& tpSphereXYZ, unsigned int rangeIndex, osg::Geometry* geometry);

  /** Fixes the orientation of the profile */
  void updateOrientation_();

  /** Adjusts based on spherical XYZ */
  void adjustSpherical_(osg::Vec3& v, const simCore::Vec3& tpSphereXYZ);

  /** Retrieves the profile height at the given ground range in meters */
  float getTerrainHgt_(float gndRng) const;

  /** Bearing of the profile in radians */
  double bearing_;

  /** Profile display thickness, in # height steps */
  unsigned int displayThickness_;
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

  /** Tesselate the 2D Vertical with triangle strip */
  const void tesselate2DVert_(unsigned int numRanges, unsigned int numHeights, unsigned int startIndex, osg::ref_ptr<osg::FloatArray> values, osg::Geometry* geometry);
};

//----------------------------------------------------------------------------

// Interface for VoxelProcessors that help generate DRAWMODE_RAE visualizations
class Profile::VoxelProcessor
{
public:
  struct VoxelRange
  {
    float valNear;
    float valFar;
    unsigned int indexNear;
    unsigned int indexFar;
  };
  struct VoxelHeight
  {
    float valBottom;
    float valTop;
    unsigned int indexBottom;
    unsigned int indexTop;
  };
  struct VoxelIndexCache
  {
    unsigned int i2;
    unsigned int i3;
    unsigned int i6;
    unsigned int i7;
  };

  /**
  * Determines if the data for this profile can be used to generate voxels
  * @return profile is valid for voxel visualization
  */
  virtual bool isValid() const = 0;

  /**
  * Calculates the voxel parameters for the specified range index
  * @param rangeIndex the index into the profile's range data
  * @param voxelRange the range parameters for the voxel
  * @param nearHeight the height parameters for the near edge of the voxel
  * @param farHeight the height parameters for the far edge of the voxel
  * @return -1 if not valid; 1 if valid voxel at that should be the last voxel drawn; otherwise, 0 for valid voxel
  */
  virtual int calculateVoxel(unsigned int rangeIndex, VoxelRange& voxelRange, VoxelHeight& nearHeight, VoxelHeight& farHeight) const = 0;

  /**
  * Sets the specified index values into the index cache for optimized generation of next voxel
  * @param i2 the i2 index to cache
  * @param i3 the i3 index to cache
  * @param i6 the i6 index to cache
  * @param i7 the i7 index to cache
  */
  virtual void setIndexCache(unsigned int i2, unsigned int i3, unsigned int i6, unsigned int i7) = 0;

  /**
  * Clears the index cache and marks its is invalid
  */
  virtual void clearIndexCache() = 0;

  /**
  * Returns the index cache if it is valid
  * @param[out] if valid, cache struct filled with cached indices
  * @return 0 if cache is valid, non-zero if not valid
  */
  virtual int indexCache(VoxelIndexCache& cache) const = 0;
};
}

#endif /* SIMVIS_RFPROP_PROFILE_H */
