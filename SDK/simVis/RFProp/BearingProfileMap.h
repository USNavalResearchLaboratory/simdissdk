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
#ifndef SIMVIS_RFPROP_BEARING_PROFILE_MAP_H
#define SIMVIS_RFPROP_BEARING_PROFILE_MAP_H

#include <map>

namespace simRF
{
class Profile;

/** A map from bearing to an RF Profile. */
class SDKVIS_EXPORT BearingProfileMap
{
public:
  /** Default constructor */
  BearingProfileMap();
  virtual ~BearingProfileMap();

  /** Map of time to Profile ref_ptr */
  typedef std::map<double, osg::ref_ptr<Profile> > TimeToProfileMap;
  /** Iterator into a TimeProfileMap */
  typedef TimeToProfileMap::iterator iterator;
  /** Const iterator into a TimeProfileMap */
  typedef TimeToProfileMap::const_iterator const_iterator;

  /** Returns the begin iterator for our time profile map */
  iterator begin();
  /** Returns the end iterator for our time profile map */
  iterator end();
  /** Returns true if there are no entries */
  bool empty() const;

  /**
   * Retrieve the profile for the specified bearing
   * @param bearingR bearing in radians
   * @return profile at specified bearing, or nullptr if none
   */
  Profile* getProfileByBearing(double bearingR) const;

  /**
  * Retrieve the bearing of the slot/profile that contains the specified bearing
  * @param bearingR bearing in radians
  * @return slot bearing (in radians) that contains the bearing, or input bearing if no matching slots found
  */
  double getSlotBearing(double bearingR) const;

  /// add the Profile to our set
  void addProfile(Profile &profile);

private:
  /// get an iterator to the slot/profile that contains the specified bearing
  const_iterator getSlot_(double bearingR) const;

private:
  TimeToProfileMap profiles_;
};

}

#endif

