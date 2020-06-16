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
#ifndef SIMVIS_RADIAL_LOS_H
#define SIMVIS_RADIAL_LOS_H

#include "simCore/Common/Common.h"
#include "simCore/Calc/Coordinate.h"
#include "simVis/Types.h"
#include "osgEarth/MapNode"
#include "osgEarth/GeoData"
#include "osgEarth/SpatialReference"
#include "osg/Node"

namespace simVis
{

/**
 * Samples the terrain in a radial pattern around an origin point.
 */
class SDKVIS_EXPORT RadialLOS
{
public:
  /**
   * Terrain sample at a given relative location.
   */
  struct SDKVIS_EXPORT Sample
  {
    /** Constructs an invalid sample */
    Sample(double range, const osgEarth::GeoPoint& point);

    /** Constructs a valid sample */
    Sample(double range, const osgEarth::GeoPoint& point, double hamsl, double hae, double dot, bool visible);

    /** Copy constructor */
    Sample(const Sample& rhs);

    /** dtor */
    virtual ~Sample() { }

    /** Indicates whether sample is valid */
    bool     valid_;
    /** Range of the sample in meters */
    double   range_m_;
    /** Height above mean sea level, in meters */
    double   hamsl_m_;
    /** Height above ellipsoid, in meters */
    double   hae_m_;
    /** Elevation from LOS origin to sample point */
    double   elev_rad_;
    /** Flags whether sample is visible */
    bool     visible_;
    /** Map data point */
    osgEarth::GeoPoint point_;
  };

  /** Vector of Samples */
  typedef std::vector<Sample> SampleVector;

public:
  /**
   * Constructs a new RLOS computer
   */
  RadialLOS();

  /**
   * Copy constructor
   */
  RadialLOS(const RadialLOS& rhs);

  /**
   * Dtor
   */
  virtual ~RadialLOS() { }

  /**
   * Assignment
   */
  RadialLOS& operator = (const RadialLOS& rhs);

  /**
   * Sets the maximum range of the sample
   * @param[in ] value Maximum range
   */
  void setMaxRange(const osgEarth::Distance& value);

  /**
   * Gets the maximum range of the sample
   * @return Maximum range
   */
  const osgEarth::Distance& getMaxRange() const { return range_max_; }

  /**
   * Sets the azimuth of center of the LOS's field of view.
   * @param[in ] value LOS field of view azimuth
   */
  void setCentralAzimuth(const osgEarth::Angle& value);

  /**
   * Gets the azimuth of the center of the field of view.
   * @return An angle
   */
  const osgEarth::Angle& getCentralAzimuth() const { return azim_center_; }

  /**
   * Sets the field of view.
   * @param[in ] value An angle measuring the field of view of the calculation
   *   about the central azimuth
   */
  void setFieldOfView(const osgEarth::Angle& value);

  /**
   * Gets the field of view
   * @return An angle
   */
  const osgEarth::Angle& getFieldOfView() const { return fov_; }

  /**
   * Sets the range resolution (max distance between range samples)
   * @param[in ] value Range resolution
   */
  void setRangeResolution(const osgEarth::Distance& value);

  /**
   * Gets the range resolution (max distance between range samples)
   * @return Range step
   */
  const osgEarth::Distance& getRangeResolution() const { return range_resolution_; }

  /**
   * Sets the azimuthal resolution (max angle between radials)
   * @param[in ] value Azimuthal resolution
   */
  void setAzimuthalResolution(const osgEarth::Angle& value);

  /**
   * Gets the azimuthal resolution (max angle between radials)
   * @return Azimuthal resolution
   */
  const osgEarth::Angle& getAzimuthalResolution() const { return azim_resolution_; }

  /**
   * Gets whether to perform LOS computation against the live scene
   * graph (versus the elevation model).
   */
  bool getUseSceneGraph() const { return use_scene_graph_; }

public:

  /**
   * Compute the entire set of terrain samples using the current settings.
   * @param[in ] mapNode Map interface to use for sampling
   * @param[in ] origin  Origin point for the LOS computation
   * @return True upon success
   */
  bool compute(osgEarth::MapNode* mapNode, const simCore::Coordinate& origin);

  /**
   * Re-samples the terrain for all sample points that fall within the specified extent.
   * @param[in ] mapNode Map interface to use for sampling
   * @param[in ] extent  Geospatial extent within which to update the samples
   * @param patch Patch node, possibly NULL
   * @return True upon success
   */
  bool update(osgEarth::MapNode* mapNode, const osgEarth::GeoExtent& extent, osg::Node* patch = NULL);

  /**
   * Gets the number of samples in each radial
   * @return Sample count
   */
  unsigned int getNumSamplesPerRadial() const;

  /**
   * Given an azimuth, compute the minimum and maximum elevations along
   * its radial.
   * @param[in ] azimuth   Azimuthal radial along which to test
   * @param[out] minHeight Minimum sample along the radial
   * @param[out] maxHeight Maximum sample along the radial
   * @return True if and only if data exists for the specified azimuth.
   */
  bool getMinMaxHeight(const osgEarth::Angle& azimuth, osgEarth::Distance& minHeight, osgEarth::Distance& maxHeight) const;

  /**
   * Given a target coordinate, compute whether there is a clear line of sight from
   * the LOS origin to that coordinate.
   * @param[in ] target    Target coordinate for LOS computation
   * @param[out] isVisible Whether the target location is visible
   * @return True if and only if data exists for the specified calculation.
   */
  bool getLineOfSight(const simCore::Coordinate& target, bool& isVisible) const;


public:

  /**
   * Data for a single radial corresponding to an azimuthal offset
   */
  struct Radial
  {
    /** Constructor */
    Radial(double azim_rad) : azim_rad_(azim_rad) { }
    /** Azimuth of radial in radians */
    double       azim_rad_;
    /** Samples along the radial */
    SampleVector samples_;
  };

  /** Vector of Radial */
  typedef std::vector<Radial> RadialVector;

  /**
   * Gets the collection of radials computed for this model
   */
  const RadialVector& getRadials() const { return radials_; }

private:
  bool                dirty_;
  RadialVector        radials_;
  osgEarth::GeoPoint  originMap_;
  osgEarth::Distance  range_max_;
  osgEarth::Distance  range_resolution_;
  osgEarth::Angle     azim_center_;
  osgEarth::Angle     fov_;
  osgEarth::Angle     azim_resolution_;
  osg::ref_ptr<const osgEarth::SpatialReference> srs_;
  osgEarth::ElevationPool::WorkingSet elevationWorkingSet_;
  bool use_scene_graph_;

  bool getBoundingRadials_(double azim_rad, const Radial*& out_r0, const Radial*& out_r1, double& out_mix) const;

  bool makeRadial_(Radial& out_radial) const;
};

} // namespace simVis


#endif // SIMVIS_RADIAL_LOS_H
