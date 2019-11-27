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
#ifndef SIMVIS_RFPROP_PROFILE_MANAGER_H
#define SIMVIS_RFPROP_PROFILE_MANAGER_H

#include "osg/Group"
#include "simVis/RFProp/Profile.h"

namespace simRF
{
class BearingProfileMap;
class ColorProvider;

/**
 * Manages a collection of Profiles
 */
class SDKVIS_EXPORT ProfileManager : public osg::Group
{
public:
  /**
   * Creates a new ProfileManager
   */
  ProfileManager();

  /**
   * Create a new profile map for the given time
   */
  void addProfileMap(double time);

  /**
   * Remove a profile map for the given time
   */
  void removeProfileMap(double time);

  /**
   * update internals for the given time
   */
  void update(double time);

  /**
  * update internals for display state
  */
  void setDisplay(bool onOff);

  /**
  * gets display on/off state
  */
  bool display() const;

  /**
   * Sets the alpha on all profiles.  0.0 is transparent, 1.0 is opaque.
   */
  void setAlpha(float alpha);

  /**
   * Retrieves the alpha on profiles.  0.0 is transparent, 1.0 is opaque.
   */
  float getAlpha() const;

  /**
   * Gets the history in radians
   */
  double getHistory() const;

  /**
   * Sets the history in radians
   */
  void setHistory(double history);

  /**
   * Gets the active bearing, in radians
   */
  double getBearing() const;

  /**
   * Sets the active bearing
   * @param bearing bearing in radians
   */
  void setBearing(double bearing);

  /**
   * Gets the profile at the given bearing
   * @param bearingR bearing in radians
   * @return profile at specified bearing, or NULL if none
   */
  Profile* getProfileByBearing(double bearingR) const;

  /**
  * Gets the profile at the specified index, intended to support simple iteration through all profiles.
  * @param index of profile to return
  * @return profile at specified index, or NULL if none
  */
  const Profile* getProfile(unsigned int index) const;

  /**
   * Adds the Profile to the ProfileManager
   */
  void addProfile(Profile* profile);

  /**
   * Gets the ColorProvider for this ProfileManager.
   */
  ColorProvider* getColorProvider() const;

  /**
   * Sets the ColorProvider for this ProfileManager
   * @param colorProvider The ColorProvider for this ProfileManager.  All Profiles managed by this ProfileManager will have this ColorProvider assigned to them.
   */
  void setColorProvider(ColorProvider* colorProvider);

  /**
   * Gets the height of this ProfileManager.
   */
  double getHeight() const;

  /**
   * Sets the height of this ProfileManager
   * @param height The height of this ProfileManager.  All Profiles managed by this ProfileManager will have this height assigned to them.
   */
  void setHeight(double height);

  /**
   * Gets whether to treat the height value as AGL.
   */
  bool getAGL() const;

  /**
   * Sets whether to treat the height value as AGL.
   * @param agl Whether to treat the height value as AGL. All Profiles managed by this ProfileManager will have this height assigned to them.
   */
  void setAGL(bool agl);

  /**
   * Gets the DrawMode
   */
  Profile::DrawMode getMode() const;

  /**
   * Sets the DrawMode
   * @param mode The DrawMode.  All Profiles managed by this ProfileManager will have this draw mode assigned to them.
   */
  void setMode(Profile::DrawMode mode);

  /**
   * Gets the display thickness, a height in meters
   */
  float getDisplayThickness() const;

  /**
   * Sets the display thickness, i.e., the altitude span for 3D displays
   * @param displayThickness The display thickness in meters.  All Profiles managed by this ProfileManager will have this display thickness assigned to them.
   */
  void setDisplayThickness(float displayThickness);

  /**
   * Sets the display thickness in number of slots.  This call can fail if no profiles are loaded.
   * The actual height is calculated based on the height of a slot in the current profile.  See
   * also setDisplayThickness(float).
   * @param numSlots Number of slots of height to visualize
   * @return 0 on success; non-zero on failure, e.g. no profiles loaded
   */
  int setThicknessBySlots(int numSlots);

  /**
   * Gets the reference latitude in radians
   */
  double getRefLat() const;

  /**
   * Gets the reference longitude in radians
   */
  double getRefLon() const;

  /**
   * Gets the reference altitude in meters
   */
  double getRefAlt() const;

  /**
   * Sets the reference coordinate
   * @param latRad The latitude in radians
   * @param lonRad The longitude in radians
   * @param alt The altitude in meters
   */
  void setRefCoord(double latRad, double lonRad, double alt);

  /**
   * Get whether the Profiles should conform to a spherical earth
   */
  bool getSphericalEarth() const;

  /**
   * Set whether the Profiles should conform to a spherical earth
   */
  void setSphericalEarth(bool sphericalEarth);

  /**
   * Get elevation angle in radians
   */
  double getElevAngle() const;

  /**
   * Set elevation angle in radians
   */
  void setElevAngle(double elevAngleRad);

  /**
   * Get threshold type
   */
  ProfileDataProvider::ThresholdType getThresholdType() const;

  /**
   * Set threshold type
   */
  void setThresholdType(ProfileDataProvider::ThresholdType type);

  /**
   * Notifies the ProfileManager that it needs to re-render it's Profiles b/c something change effecting the rendering.
   */
  void dirty();

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simRF"; }

  /** Return the class name */
  virtual const char* className() const { return "ProfileManager"; }

protected:
  virtual ~ProfileManager();

private:
  void updateVisibility_();
  void initShaders_();

  osg::ref_ptr<ColorProvider> colorProvider_;

  std::map<double, BearingProfileMap*> timeBearingProfiles_; ///< map from time to profiles according to bearing
  BearingProfileMap *currentProfileMap_; ///< profile map corresponding to the current time

  double history_;          ///< number of bearing slices displayed
  double bearing_;          ///< current bearing of RF prop display
  double height_;           ///< 2D Horizontal display height
  float displayThickness_;  ///< display thickness for 3D displays
  bool agl_;                ///< whether height values for the 2D Horizontal display are referenced to height above ground level (AGL) or to mean sea level (MSL).
  bool displayOn_;          ///< whether the display is on or off
  float alpha_;             ///< Alpha value (1.0 opaque, 0.0 transparent)
  Profile::DrawMode mode_;  ///< Type of display, e.g. 2D, 3D
  osg::Vec3d refCoord_;     ///< Reference coordinate used for coordinate conversion used in the visualization
  bool sphericalEarth_;     ///< whether the profile data are specified for spherical earth or not
  double elevAngle_;        ///< elevation angle used in the current display
  ProfileDataProvider::ThresholdType type_;  ///< threshold type selected for display, e.g., POD, SNR, CNR
};
}

#endif /* SIMVIS_RFPROP_PROFILE_MANAGER_H */

