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
#ifndef SIMVIS_RFPROP_RFPROPAGATIONFACADE_H
#define SIMVIS_RFPROP_RFPROPAGATIONFACADE_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "osg/ref_ptr"
#include "simCore/Common/Export.h"
#include "simData/ObjectId.h"
#include "simVis/RFProp/CompositeColorProvider.h"
#include "simVis/RFProp/CompositeProfileProvider.h"
#include "simVis/RFProp/ProfileManager.h"
#include "simVis/RFProp/PODProfileDataProvider.h"

namespace simCore
{
  class TimeStamp;
  class DatumConvert;
}

namespace simRF
{
class Profile;

/** Facade to the simRF module, managing RF data for a single beam. */
class SDKVIS_EXPORT RFPropagationFacade
{
public:
  /**
   * Construct an RF Propagation beam handler for the specified beam
   * @param beamId Beam to configure
   * @param parent node to which the visual display's locator is attached; if NULL, no display will be created
   * @param datumConvert converter for MSL heights
   */
  RFPropagationFacade(simData::ObjectId beamId, osg::Group* parent, std::shared_ptr<simCore::DatumConvert> datumConvert);
  virtual ~RFPropagationFacade();

  /**
   * Sets the propagation model for a given beam (not implemented yet)
   * @return 1; not yet implemented
   */
  int setModelType();

  /**
   * Sets the propagation model RADAR parameters for a given beam
   * @param radarParams radar parameter structure
   * @return 0 on success, !0 on error
   */
  int setRadarParams(const simCore::RadarParameters& radarParams);

  /**
  * Gets the  propagation model RADAR parameters for a given beam
  * @return const ptr to RadarParameters structure
  */
  const RadarParametersPtr radarParams() const;

  /**
   * Sets the probability of detection thresholds for a given beam
   * @param podLoss Vector of 100 positive (implicitly negative) Loss thresholds (dB) for a probability of detection from 0% to 100%;
   *  podLoss must contain 100 elements, and elements are expected to be ordered as positive decreasing values (implicitly negative increasing)
   * @return 0 on success, !0 on error
   */
  int setPODLossThreshold(const std::vector<float>& podLoss);

  /**
  * Gets the probability of detection thresholds for a given beam
  * @return const ptr to POD Loss vector
  */
  const PODVectorPtr getPODLossThreshold() const;

  /**
   * Sets the propagation model threshold color map for a given beam
   * @param type Color map type, only valid for: POD, Loss and SNR
   * @param colorMap Map of dB values to corresponding colors that constitutes the color map
   * @return 0 on success, !0 on error
   */
  int setColorMap(simRF::ProfileDataProvider::ThresholdType type, const std::map<float, osg::Vec4>& colorMap);

  /**
   * Set the slot data for a given beam
   * @param profile Pointer to Profile containing slot data
   * @return 0 on success, !0 on error
   */
  int setSlotData(simRF::Profile* profile);

  /**
   * Gets the valid (in use) slot data for a given beam
   * @param azRad Azimuth bearing relative to true north (rad) to retrieve data
   * @return 0 on success, !0 on error
   */
  const simRF::Profile* getSlotData(double azRad) const;

  /**
   * Add input AREPS RF Propagation files for a given beam
   * @param time Time reference for the data in the files
   * @param filenames Reference to vector containing the filename(s) of AREPS .txt files
   * @return 0 on success, !0 on error
   */
  int loadArepsFiles(const simCore::TimeStamp& time, const std::vector<std::string>& filenames);

  /**
   * Get AREPS RF Propagation files for a given beam
   * @param time Time reference for the files which are requested
   * @param filenames Pointer to vector to return the filename(s) of AREPS .txt files
   * @return 0 on success, !0 on error
   */
  int getInputFiles(const simCore::TimeStamp& time, std::vector<std::string>& filenames) const;

  /**
   * Controls the display of the specified RF propagation data
   * @param option on(true) or off(false)
   * @return 0 on success, !0 on error
   */
  int setDisplay(bool option);

  /**
   * Returns the display state
   * @return the display state
   */
  bool display() const;

  /**
   * Controls the display of RF propagation data based on selected Range Tool association
   * A Range Tool association must exist between an RF Emitter as the
   * "begin entity" and a target as the "end entity".  When enabled,
   * the propagation display will be controlled by the currently selected
   * Range Tool association
   * @param option on(true) or off(false)
   * @return 0 on success, !0 on error
   */
  int setRangeTool(bool option);

  /**
   * Returns the aglActive flag; the routine assumes the valid() returns true
   * @return The aglActive flag
   */
  bool aglActive() const;

  /**
   * Turns on or off AGL height.  When on, AGL height flag changes the interpretation
   * of the Height parameter.  Loaded terrain should impact both the data read-outs
   * and the display of the horizontal slice graphic.
   * @param aglActive If true, turn on AGL mode.  If false, turn off AGL mode.
   * @return 0 on success, !0 on error.
   */
  int setAglActive(bool aglActive);

  /**
   * Controls the type of drawing space for the propagation data
   * @param drawMode DrawMode to draw
   * @return 0 on success, !0 on error
   */
  int setDrawMode(simRF::Profile::DrawMode drawMode);

  /**
  * Returns the current  type of drawing space for the propagation data
  * @return current draw mode to draw
  */
  simRF::Profile::DrawMode drawMode() const;

  /**
   * Controls display of RF propagation data height
   * This option controls the 2D Horizontal display height in meters of the
   * propagation data and is only available when the propagation Draw Space
   * is set to 2D Horizontal.  Display heights will vary based on quantized
   * height values.  These discrete heights are determined by the difference
   * of the maximum and minimum height values divided by the number of height
   * points (nzout) in the profile.
   * @param height Height of the 2D propagation slice in meters referenced to HAE
   * @return 0 on success, !0 on error
   */
  int setHeight(double height);

  /**
   * Returns the height in meters; the routine assumes that valid() returns true
   * @return The height in meters
   */
  double height() const;

 /**
   * Controls display of RF propagation data thickness.
   * This option controls the 3D display thickness and is only available when
   * the propagation Draw Space is set to 3D, 3D Points, or 3D Texture.
   * @param thickness 3D display thickness of the propagation data, in # height steps
   * @return 0 on success, !0 on error
   */
  int setThickness(unsigned int thickness);

  /**
   * Returns the thickness in # height steps; this routine assumes that valid() returns true
   * @return The thickness, in # height steps
   */
  unsigned int thickness() const;

  /**
   * Controls the number of bearing slices to display
   * Range of values are 1 to 360 degrees.  Display history may vary due to the horizontal beam widths
   * @param length Arc length of history in deg
   * @return 0 on success, !0 on error
   */
  int setHistory(int length);

  /**
   * Returns the history length; the routine assumes the valid() returns true
   * @return History length in number of bearing slices
   */
  int history() const;

  /**
   * Controls the visibility of the propagation data
   * Range of values 0 (opaque) to 100 (transparent)
   * @param transparency Transparency percentage value for drawn propagation data
   * @return 0 on success, !0 on error
   */
  int setTransparency(int transparency);

  /**
   * Returns the transparency; the routine assumes the valid() returns true
   * @return Transparency between 0 and 100
   */
  int transparency() const;

  /**
   * Controls the propagation data threshold display options
   * Applies the selected type of threshold test on propagation data
   * @param mode Propagation data threshold display options
   * @return 0 on success, !0 on error
   */
  int setThresholdMode(simRF::ColorProvider::ColorMode mode);

  /**
   * Returns the threshold mode; the routine assumes the valid() returns true
   * @return The threshold mode
   */
  simRF::ColorProvider::ColorMode thresholdMode() const;

  /**
   * Controls the type of propagation data in which threshold setting will be applied.
   * For now, this call will update the color provider gradient colors with default values based on the type
   * @param type Type of threshold test to perform on propagation data
   * @return 0 on success, !0 on error
   */
  int setThresholdType(simRF::ProfileDataProvider::ThresholdType type);

  /**
   * Returns the threshold type; the routine assumes the valid() returns true
   * @return The Threshold type
   */
  simRF::ProfileDataProvider::ThresholdType thresholdType() const;

  /**
   * Controls display percentage threshold above or below in which data is drawn
   * This mode is not available when the Threshold display is set to Entire Spectrum
   * @param value Threshold value above or below of which data is drawn
   * @return 0 on success, !0 on error
   */
  int setThresholdValue(int value);

  /**
   * Returns the threshold value; the routine assumes the valid() returns true
   * @return The Threshold value
   */
  int threshold() const;

  /**
   * Controls the above threshold color
   * This setting only applies to above- and above-and-below threshold display types
   * @param color Color to use for values above threshold
   * @return 0 on success, !0 on error
   */
  int setAboveColor(const osg::Vec4f& color);

  /**
  * Returns the above threshold color
  * @param color Color used for values above threshold
  * @return 0 on success, !0 on error
  */
  int aboveColor(osg::Vec4f& color);

  /**
   * Controls the below threshold color
   * This setting only applies to below- and above-and-below threshold display types
   * @param color Color to use for values at or below threshold
   * @return 0 on success, !0 on error
   */
  int setBelowColor(const osg::Vec4& color);

  /**
  * Returns the below threshold color
  * @param color Color used for values at or below threshold
  * @return 0 on success, !0 on error
  */
  int belowColor(osg::Vec4f& color);

  /**
   * Clears all RF propagation data from the beam's cache
   * Deletes all cached propagation data in the propagation library.  If reset is true,
   * then the RF and environmental parameters are also cleared.
   * @param reset Resets the RF and environmental parameters
   * @return 0 on success, !0 on error
   */
  int clearCache(bool reset);

  /**
   * Return the probability of detection for a given beam with RF Prop parameters
   * @param azimRad Azimuth angle referenced to True North in radians
   * @param gndRngMeters Ground range from emitter source, meters
   * @param hgtMeters Height, above surface referenced to HAE, meters
   * @return Probability of detection [0, 100]
   */
  double getPOD(double azimRad, double gndRngMeters, double hgtMeters) const;

  /**
   * Return the propagation loss for a given beam with RF Prop parameters
   * @param azimRad Azimuth angle referenced to True North in radians
   * @param gndRngMeters Ground range from emitter source, meters
   * @param hgtMeters Height, above surface referenced to HAE, meters
   * @return Propagation loss [-300: error or invalid data]
   */
  double getLoss(double azimRad, double gndRngMeters, double hgtMeters) const;

  /**
   * Return the pattern propagation factor for a given beam with RF Prop parameters
   * @param azimRad Azimuth angle referenced to True North in radians
   * @param gndRngMeters Ground range from emitter source, meters
   * @param hgtMeters Height, above surface referenced to HAE, meters
   * @return Pattern propagation factor [-300: error or invalid data]
   */
  double getPPF(double azimRad, double gndRngMeters, double hgtMeters) const;

  /**
   * Return the signal to noise ratio of detection for a given beam with RF Prop parameters
   * @param azimRad Azimuth angle referenced to True North in radians
   * @param slantRngMeters Slant range from emitter source, meters
   * @param hgtMeters Height, above surface referenced to HAE, meters
   * @param xmtGaindB Transmitter antenna gain, dB
   * @param rcvGaindB Receiver antenna gain, dB
   * @param rcsSqm Target RADAR cross section, sqm
   * @param gndRngMeters Ground range from emitter source, meters, used to look up PPF in RF propagation array
   * @return Signal to noise ratio [-300: error or invalid data]
   */
  double getSNR(double azimRad, double slantRngMeters, double hgtMeters, double xmtGaindB, double rcvGaindB, double rcsSqm, double gndRngMeters) const;

  /**
   * Return the clutter to noise ratio for a given beam with RF Prop parameters
   * @param azimRad Azimuth angle referenced to True North in radians
   * @param gndRngMeters Ground range from emitter source, meters
   * @return Clutter to noise ratio for gndRngMeters values clamped to [min, max] table limits [-300: error or invalid data]
   */
  double getCNR(double azimRad, double gndRngMeters) const;

  /**
   * Return the one way power for a given beam with RF Prop parameters
   * @param azimRad Azimuth angle referenced to True North in radians
   * @param slantRngMeters Slant range from emitter source, meters
   * @param hgtMeters Height, above surface referenced to HAE, meters
   * @param xmtGaindB Transmitter antenna gain, dB
   * @param gndRngMeters Ground range from emitter source, meters, used to look up PPF in RF propagation array
   * @param rcvGaindB Receiver antenna gain, dB
   * @return One way received power [-300: error or invalid data]
   */
  double getOneWayPower(double azimRad, double slantRngMeters, double hgtMeters, double xmtGaindB, double gndRngMeters, double rcvGaindB) const;

  /**
   * Return the two way received power for a given beam with RF Prop parameters
   * @param azimRad Azimuth angle referenced to True North in radians
   * @param slantRngMeters Slant range from emitter source, meters
   * @param hgtMeters Height, above surface referenced to HAE, meters
   * @param xmtGaindB Transmitter antenna gain, dB
   * @param rcvGaindB Receiver antenna gain, dB
   * @param rcsSqm Target RADAR cross section, sqm
   * @param gndRngMeters Ground range from emitter source, meters, used to look up PPF in RF propagation array
   * @return Two way received power [-300: error or invalid data]
   */
  double getReceivedPower(double azimRad, double slantRngMeters, double hgtMeters, double xmtGaindB, double rcvGaindB, double rcsSqm, double gndRngMeters) const;

  /**
   * Returns valid propagation state for given beam
   * @return true for valid, false otherwise
  */
  bool valid() const;

public:
  /**
  * Gets the composite provider for the specified azimuth the antenna height that will be used for the display
  * @param azimRad Azimuth angle referenced to True North in radians
  * @return the composite provider , or NULL if no provider exists at the specified azimuth
  */
  const simRF::CompositeProfileProvider* getProfileProvider(double azimRad) const;

  /**
  * Sets the antenna height that will be used for the display
  * @param antennaHeightM height in meters
  */
  void setAntennaHeight(float antennaHeightM);

  /**
  * Gets the antenna height
  * @return antenna height in meters
  */
  float antennaHeight() const;

  /**
  * Gets the min data height
  * @return min height in meters
  */
  float minHeight() const;

  /**
  * Gets the max data height
  * @return max height in meters
  */
  float maxHeight() const;

  /**
  * Gets the number of height steps in the data
  * @return number of steps
  */
  unsigned int heightSteps() const;

  /**
   * Gets the active bearing
   * @return current active bearing in radians
   */
  double getBearing() const;

  /**
   * Sets the active bearing
   * @param bearing active bearing in radians
   */
  void setBearing(double bearing);

  /**
   * Sets the active elevation; used in RAE mode
   * @param elevation in radians
   */
  void setElevation(double elevation);

  /**
   * Set whether the data are specified for spherical or WGS84 earth
   * @param sphericalEarth  true if data is spherical earth data, false if WGS84
   */
  void setSphericalEarth(bool sphericalEarth);

  /**
  * Gets the number of profiles available in the profile manager
  * @return number of profiles
  */
  unsigned int numProfiles() const;

  /**
  * Gets the profiles at the specified index
  * @return requested profile, or NULL if the index was not valid
  */
  const simRF::Profile* getProfile(unsigned int index) const;

  /**
  * Updates the position of the display to specified lat/lon
  * @param latRad latitude in radians
  * @param lonRad longitude in radians
  */
  void setPosition(double latRad, double lonRad);

  /** Enables or disables the depth buffer */
  void enableDepthBuffer(bool enabled);

  /** Returns true if the depth buffer is enabled */
  bool isDepthBufferEnabled() const;

private:
  /// set some reasonable defaults in our default color maps
  void initializeDefaultColors_();
  /// update the gradient color map based on threshold type
  void setGradientByThresholdType_(simRF::ProfileDataProvider::ThresholdType type);

  /// The beam id for which this display is specified
  simData::ObjectId id_;

  /// antenna height used to create rf propagation data
  float antennaHeightMeters_;

  /// indicates whether RF Parameters have been set
  bool rfParamsSet_;

  /// profile manager manages all the profiles that hold the rf prop data
  osg::ref_ptr<simRF::ProfileManager> profileManager_;

  /// parent node in the scene graph of our profileManager
  osg::observer_ptr<osg::Group> parent_;

  /// color provider to manager which color
  osg::ref_ptr<simRF::CompositeColorProvider> colorProvider_;

  /// map of filesets loaded, keyed by the timestamp for which they were specified
  std::map<simCore::TimeStamp, std::vector<std::string> > arepsFilesetTimeMap_;

  /// shared ptr to the POD Loss thresholds
  PODVectorPtr podLossThresholds_;

  /// shared ptr to the RF RADAR Parameters
  RadarParametersPtr radarParameters_;

  /// color maps by threshold type
  std::map<simRF::ProfileDataProvider::ThresholdType, simRF::GradientColorProvider::ColorMap> colorMaps_;

  /// default color map
  simRF::GradientColorProvider::ColorMap defaultColors_;
};

}

#endif /* SIMVIS_RFPROP_RFPROPAGATIONFACADE_H */
