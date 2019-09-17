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
#include <vector>
#include "osg/ref_ptr"
#include "osg/MatrixTransform"
#include "simCore/Common/Export.h"

namespace osg {
  class Geode;
  class Group;
  class Texture2D;
}
namespace simData { class DataStore; }
namespace simVis
{
class Locator;
class PlatformNode;
class OverrideColor;

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
    bool isWake;                    ///< the trail will not be billboarded, but rendered flat wrt earth

    /// default constructor gives reasonable values
    VaporTrailData();
  };

  /**
  * Construct a new vapor trail. Adds to the scene.
  * @param dataStore needed for the limits
  * @param expireModeGroup the ExpireModeGroup that attaches this vaporTrail to the scenegraph.
  * @param hostPlatform platform the vapor trail is connected to.
  * @param vaporTrailData data used to construct the vapor trail.
  * @param vaporPuffData data used to specify the vapor puff.
  * @param textures vector of textures to use for alternating puffs.
  */
  VaporTrail(const simData::DataStore& dataStore, osg::Group* expireModeGroup, PlatformNode& hostPlatform, const VaporTrailData& vaporTrailData, const VaporPuffData& vaporPuffData, const std::vector< osg::ref_ptr<osg::Texture2D> >& textures);

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
  * Adds one new puff (corresponding to vapor trail start time) to the trail.
  * @return 0 on success, non-zero on failure
  */
  int addFirstPuff_();

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
  * Returns a matrix with the position and corrected orientation for a puff.
  * @param[in] position ECEF position for this puff.
  * @return position and orientation matrix for the puff.
  */
  static osg::Matrixd calcWakeMatrix_(const simCore::Vec3& ecefPosition);

  /**
  * Process all specified textures into reusable geodes that are managed internally.
  * @param textures vector of textures to process.
  */
  void processTextures_(const std::vector<osg::ref_ptr<osg::Texture2D> >& textures);

  /**
  * Create a geometry from the specified texture in the specified geode.
  * @param geode geode to which to add the geometry.
  * @param texture texture to use.
  */
  void createTexture_(osg::Geode& geode, osg::Texture2D* texture) const;

  /// DataStore for getting the limits
  const simData::DataStore& dataStore_;

  /// the scenegraph attachment for the vaporTrail
  osg::observer_ptr<osg::Group> expireModeGroup_;

  /// the platform for this vapor trail
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
  std::vector< osg::ref_ptr<osg::Geode> > textures_;
};

/**
* Class that holds a visual representation of a single vapor trail component.
*/
class VaporTrail::VaporTrailPuff : public osg::MatrixTransform
{
public:
  /**
  * Construct a vapor trail puff.
  * @param graphic the puff graphic.
  * @param matrix position and orientation for the puff.
  * @param startTime time that this puff is created.
  */
  VaporTrailPuff(osg::Geode* graphic, const osg::Matrixd& matrix, double startTime);

#ifdef USE_DEPRECATED_SIMDISSDK_API
  /**
  * Construct a vapor trail puff.
  * @param graphic the puff graphic.
  * @param position ECEF position at which this puff will be located.
  * @param startTime time that this puff is created.
  */
  SDK_DEPRECATE(VaporTrailPuff(osg::Geode* graphic, const simCore::Vec3& position, double startTime), "Method will be removed in future SDK release.");
#endif

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
  * @param matrix pos and ori for the puff.
  * @param startTime time that this puff is created.
  */
  void set(const osg::Matrixd& matrix, double startTime);

#ifdef USE_DEPRECATED_SIMDISSDK_API
  /**
  * Turns the puff back on with the given values
  * @param position ECEF position at which this puff will be located.
  * @param startTime time that this puff is created.
  */
  SDK_DEPRECATE(void set(const simCore::Vec3& position, double startTime), "Method will be removed in future SDK release.");
#endif

protected:
  /// osg::Referenced-derived
  virtual ~VaporTrailPuff();

private:
  /// Controls the shader that will apply fading to all puffs in this vaporTrail
  osg::ref_ptr<OverrideColor> overrideColor_;
  /// the puff's current scale
  double scale_;
  /// the puff's start time, for modeling fade and expand
  double startTime_;
  /// if true the puff is active and should be updated
  bool active_;
};

}

#endif
