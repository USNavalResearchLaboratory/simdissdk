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
#ifndef SIMVIS_VAPOR_TRAIL_H
#define SIMVIS_VAPOR_TRAIL_H
#include <map>
#include "osg/observer_ptr"
#include "osg/MatrixTransform"
#include "osg/Texture2D"
#include "osg/Billboard"
#include "simCore/Common/Export.h"
#include "simData/DataStore.h"
#include "simVis/Locator.h"
#include "simVis/OverrideColor.h"

namespace simVis
{
class PlatformNode;

/**
 * Class that holds a visual representation of a vapor trail.
 */
class SDKVIS_EXPORT VaporTrail : public osg::Referenced
{
public:
  /// Describes the user-defined parameters of each vapor trail puff
  struct SDKVIS_EXPORT VaporPuffData
  {
    double initialRadiusM;        ///< initial puff radius, in meters
    double radiusExpansionRate;   ///< puff expansion with time, in meters/second
    double fadeTimeS;             ///< puff fading with time, in seconds

    /// default constructor gives reasonable values
    VaporPuffData();
  };

  /// Describes the user-defined parameters of the vapor trail
  struct SDKVIS_EXPORT VaporTrailData
  {
    double startTime;             ///< start time for the trail
    double endTime;               ///< end time for the trail
    double numRadiiFromPreviousSmoke;    ///< distance from last puff for new puff, in number of radii
    double metersBehindCurrentPosition;  ///< distance behind platform for closest puff, in meters

    /// default constructor gives reasonable values
    VaporTrailData();
  };

  /**
  * Construct a new vapor trail. Adds to the scene.
  * @param dataStore needed for the limits
  * @param hostPlatform platform the vapor trail is connected to.
  * @param vaporTrailData data used to construct the vapor trail.
  * @param vaporPuffData data used to specify the vapor puff.
  * @param textures vector of textures to use for alternating puffs.
  */
  VaporTrail(const simData::DataStore& dataStore, PlatformNode& hostPlatform, const VaporTrailData& vaporTrailData, const VaporPuffData& vaporPuffData, const std::vector< osg::ref_ptr<osg::Texture2D> >& textures);

  /**
   * Add new puffs, update all existing puffs in the vapor trail.
   * Update does data limiting to prevent spikes when time jumps in file mode
   */
  void update(double time);

protected:
  /// osg::Referenced-derived
  virtual ~VaporTrail();

private:
  class VaporTrailPuff;

  /**
   * Gets the data limiting values
   * @param pointLimit Maximum number of points
   * @param timeLimit Maximum length of vapor trail in seconds
   */
  void getLimits_(unsigned int& pointLimit, double& timeLimit);

  /**
   * Remove the specified number of puffs from the front of the deque
   * @param dropAmount Number of puffs to drop
   */
  void dropPuffsFromFront_(size_t dropAmount);

  /**
   * Return the adjusted puffsToAdd so that the limits are respected and
   * removes puffs as needed to make sure there is room in puffs_
   * @param puffsToAdd The desired number of puffs to add
   * @param time The current time
   * @param prevPuffTime The time of the last puff
   * @return The number of puffs that actual can be added
   */
  unsigned int applyDataLimiting_(unsigned int puffsToAdd, double time, double prevPuffTime);

  /**
  * Adds new puffs to the trail if conditions require it.
  * @param time time to evaluate vapor trail parameters.
  */
  void addNewPuffs_(double time);

  /**
  * Creates a puff and adds it to the vapor trail.
  * @param position ECEF position at which this puff will be located.
  * @param startTime time of puff start.
  */
  void addPuff_(const simCore::Vec3& position, double startTime);

  /**
  * Create a reusable billboard with specified texture.
  * @param texture texture to use.
  * @return the billboard created
  */
  osg::Billboard* createTextureBillboard_(osg::Texture2D* texture) const;

  /// DataStore for getting the limits
  const simData::DataStore& dataStore_;

  /// the host platform for this vapor trail
  osg::observer_ptr<simVis::PlatformNode> hostPlatform_;

  /// locator to track the host and calculate the puff offset
  osg::ref_ptr<Locator> locator_;

  /// the container for the vapor trail graphic
  osg::ref_ptr<osg::Group> vaporTrailGroup_;

  /// the container for puffs
  typedef std::deque<osg::ref_ptr<VaporTrailPuff> > Puffs;

  /// active puffs
  Puffs puffs_;

  /// inactive puffs waiting to be re-used
  Puffs recyclePuffs_;

  /// Specification for this vapor trail
  VaporTrailData vaporTrailData_;
  VaporPuffData vaporPuffData_;

  /// counter used to iterate through list of textures to cyclically initialize new puffs
  unsigned int textureCounter_;

  /// the list of textures that cyclically initialize new puffs
  std::vector< osg::ref_ptr<osg::Billboard> > textureBillboards_;

  /// the shader that will apply fading to all puffs in this vaporTrail
  osg::ref_ptr<OverrideColor> overrideColor_;
};

/**
* Class that holds a visual representation of a single vapor trail component.
*/
class VaporTrail::VaporTrailPuff : public osg::Referenced
{
public:
  /**
  * Construct a vapor trail puff.
  * @param puffTransform the container for the puff graphic.
  * @param position ECEF position at which this puff will be located.
  * @param startTime time that this puff is created.
  */
  VaporTrailPuff(osg::MatrixTransform* puffTransform, const simCore::Vec3& position, double startTime);

  /**
  * Update the puff representation for elapsing time.
  * @param currentTime current time.
  * @param puffData the VaporPuffData structure that specifies puff behavior
  */
  void update(double currentTime, const VaporTrail::VaporPuffData& puffData);

  /**
  * Return the ECEF position of the puff.
  * @return the puff position as an ECEF coordinate
  */
  simCore::Vec3 position() const;

  /**
   * Return the start time
   * @return the start time
   */
  double time() const;

  /**
   * Turns the puff off
   */
  void clear();

  /**
   * Turns the puff back on with the given values
   * @param position ECEF position at which this puff will be located.
   * @param startTime time that this puff is created.
   */
  void set(const simCore::Vec3& position, double startTime);

protected:
  /// osg::Referenced-derived
  virtual ~VaporTrailPuff();

private:
  osg::ref_ptr<osg::MatrixTransform> puff_;
  osg::ref_ptr<osg::Uniform> overrideColor_;

  /// the puff's ECEF position
  simCore::Vec3 position_;

  /// the puff's start time, for modeling fade and expand
  double startTime_;
  /// if true the puff is active and should be updated
  bool active_;
};

}

#endif
