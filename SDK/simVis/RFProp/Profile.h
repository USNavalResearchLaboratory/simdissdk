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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_RFPROP_PROFILE_H
#define SIMVIS_RFPROP_PROFILE_H

#include <map>
#include <memory>
#include "osg/MatrixTransform"
#include "simCore/Common/Common.h"
#include "simVis/RFProp/ColorProvider.h"
#include "simVis/RFProp/ProfileDataProvider.h"

namespace simCore { class DatumConvert; }
namespace simRF
{
class CompositeProfileProvider;
struct ProfileContext;

/** Responsible for rendering a single profile of data. */
class SDKVIS_EXPORT Profile : public osg::MatrixTransform
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

  /** Gets the DataProvider for this Profile */
  const CompositeProfileProvider* getDataProvider() const;

  /** Sets the DataProvider for this Profile */
  void setDataProvider(CompositeProfileProvider* dataProvider);

  /** Sets the shared context for all profiles */
  void setProfileContext(std::shared_ptr<ProfileContext> profileContext);

  /**
   * Set threshold type, selects a data provider of that type, if one exists
   * Special setter to handle behavior not automatically handled by shared ProfileContext
   * @param type The data/data provider to use for the display
   */
  void setThresholdType(ProfileDataProvider::ThresholdType type);

  /** Gets the bearing of the Profile in radians */
  double getBearing() const;

  /** Sets the bearing of the Profile in radians */
  void setBearing(double bearing);

  /** Gets the half beam width in radians */
  double getHalfBeamWidth() const;

  /** Sets the half beam width in radians */
  void setHalfBeamWidth(double halfBeamWidth);

  /**
  * Sets the terrain heights for this Profile.
  * @param terrainHeights Map of terrain heights for this Profile keyed on ground range, both values in meters
   */
  void setTerrainHeights(const std::map<float, float>& terrainHeights);

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

private:
  /** Copy constructor, not implemented or available. */
  Profile(const Profile&);

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

  /** Creates an image representing the loss values */
  osg::Image* createImage_();

  class VoxelProcessor;
  class RahVoxelProcessor;
  class RaeVoxelProcessor;

  /** Creates a voxel (volume pixel) at the given location */
  int buildVoxel_(VoxelProcessor& vProcessor, unsigned int rangeIndex, osg::Geometry* geometry);

  /** Fixes the orientation of the profile */
  void updateOrientation_();

  /** Adjusts point height if it was originally based on spherical XYZ or MSL */
  double adjustHeight_(double x, double y, double z) const;

  /** Tesselate the 2D Vertical with triangle strip */
  void tesselate2DVert_(unsigned int numRanges, unsigned int numHeights, unsigned int startIndex, osg::ref_ptr<osg::FloatArray> values, osg::Geometry* geometry) const;

  /** Retrieves the profile height at the given ground range in meters */
  float getTerrainHgt_(float gndRng) const;

  /** Range vs terrain heights; TODO: Ask Glenn why this is cached instead of requesting the hgts from osgEarth */
  std::map<float, float> terrainHeights_;

  /** Array of vertices for the profile */
  osg::ref_ptr<osg::Vec3Array> verts_;
  /** Holds the graphics */
  osg::ref_ptr<osg::Group> group_;
  /** Values for the profile */
  osg::ref_ptr<osg::FloatArray> values_;
  /** Texture for the textured mode */
  osg::ref_ptr<osg::Texture> texture_;

  /** Bearing of the profile in radians */
  double bearing_;

  /** Half of the beam width in radians */
  double halfBeamWidth_;

  /** Profile's horizontal beam extents */
  double cosTheta0_;
  double sinTheta0_;
  double cosTheta1_;
  double sinTheta1_;

  /** Data provider */
  osg::ref_ptr<CompositeProfileProvider> data_;

  /** Indicates profile needs updating */
  bool dirty_;

  /** context owned by manager but shared with each profile */
  std::shared_ptr<ProfileContext> profileContext_;
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
