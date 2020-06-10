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
#ifndef SIMCORE_EM_RADAR_CROSS_SECTION_H
#define SIMCORE_EM_RADAR_CROSS_SECTION_H

#include <ostream>
#include <map>
#include <memory>
#include <vector>
#include <string>
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Random.h"
#include "simCore/EM/Constants.h"

namespace simCore
{
  /**
  * @brief Base class for loading RCS data files.
  */
  class SDKCORE_EXPORT RadarCrossSection
  {
  public:
    RadarCrossSection() : filename_("") {}
    virtual ~RadarCrossSection() {}

    /**
    * This method returns the type of radar cross section
    * @return radar cross section type
    */
    virtual RCSType type() const { return NO_RCS; }

    /**
    * This method sets the file name of the radar cross section (RCS)
    * @param[in ] fname RCS file name.
    */
    void setFilename(const std::string& fname)
    {
      if (!fname.empty())
        filename_ = fname;
    }

    /**
    * This method returns the file name of the radar cross section (RCS)
    * @return RCS file name.
    */
    std::string filename() const { return filename_; }

    /**
    * This method computes the RCS value in dB for the requested parameters
    * @param[in ] freq Frequency of radar in MHz
    * @param[in ] azim Relative azimuth angle, referenced to host platform (rad)
    * @param[in ] elev Relative elevation angle, referenced to host platform (rad)
    * @param[in ] pol Radar polarity
    * @return RCS value (dB)
    */
    virtual float RCSdB(float freq, double azim, double elev, PolarityType pol) = 0;

    /**
    * This method computes the RCS value in square meters for the requested parameters
    * @param[in ] freq Frequency of radar in MHz
    * @param[in ] azim Relative azimuth angle, referenced to host platform (rad)
    * @param[in ] elev Relative elevation angle, referenced to host platform (rad)
    * @param[in ] pol Radar polarity
    * @return RCS value (square meters)
    */
    virtual float RCSsm(float freq, double azim, double elev, PolarityType pol) = 0;

    /**
    * This method checks the incoming RCS data filename, opens a file stream and parses the RCS data
    * @param[in ] fname Input file name
    * @return 0 on success.
    */
    virtual int loadRCSFile(const std::string& fname) = 0;

  private:
    ///Filename of RCS pattern
    std::string filename_;
  };

  /// Shared pointer of a Radar Cross Section
  typedef std::shared_ptr<RadarCrossSection> RadarCrossSectionPtr;

  /** RCS data keyed on host body azimuth (rad) */
  typedef std::map<float, float> AZIM_RCS_MAP;

  /**
   * @brief Storage class used for low level single point RCS data
   *
   * RCS data typically found in elevation and aspect charts.  This
   * class assumes that only one frequency, one elevation one
   * time/range value and one polarization is valid per table.
   * All angular data is stored in radians
   */
  class SDKCORE_EXPORT RCSTable
  {
  public:
    RCSTable();
    virtual ~RCSTable() {}

    /**
    * This method retrieves the radar cross section value for the requested azimuth
    * @param[in ] azim Azimuth value relative to host (rad)
    * @return RCS value in square meters
    */
    float RCS(double azim) const;

    /**
    * This method sets the radar cross section value for the given azimuth
    * @param[in ] azim Azimuth value relative to host (rad)
    * @param[in ] rcs Radar cross section value in square meters
    */
    void setRCS(float azim, float rcs);

    /**
    * This method retrieves the measured frequency associated to this RCSTable
    * @return measured frequency (MHz)
    */
    float freq() const { return freq_; }

    /**
    * This method sets the measured frequency associated to this RCSTable
    * @param[in ] val Measured frequency (MHz)
    */
    void setFreq(float val) { freq_ = fabs(val); }

    /**
    * This method retrieves the elevation value associated to this RCSTable
    * @return elevation value (rad)
    */
    float elev() const { return elev_; }

    /**
    * This method sets the elevation associated to this RCSTable
    * @param[in ] val Elevation value (rad)
    */
    void setElev(float val) { elev_ = val; }

    /**
    * This method retrieves the polarity associated to this RCSTable
    * @return polarity value
    */
    PolarityType polarity() const { return polarity_; }

    /**
    * This method sets the polarity associated to this RCSTable
    * @param[in ] val Polarity value
    */
    void setPolarity(PolarityType val) { polarity_ = val; }

  protected:
    float freq_;              ///< RCS measured frequency (MHz)
    float elev_;              ///< elevation angle (rad)
    PolarityType polarity_;   ///< RCS polarization
    AZIM_RCS_MAP azMap_;      ///< RCS data (sqm) container keyed on host body azimuth (rad)
  };

  /* ************************************************************************ */
  /* Utility types and classes                                                */
  /* ************************************************************************ */

  /** RCS tables keyed on host body elevation (rad) */
  typedef std::map<float, RCSTable*> ELEV_RCSTABLE_MAP;

  /** @brief Container of RCSTables sorted on elevation angle */
  struct SDKCORE_EXPORT ELEVMAP
  {
    ELEV_RCSTABLE_MAP eMap;   ///< Container of RCS tables keyed on host body elevation (rad)
    ELEVMAP() {}
    ~ELEVMAP();
  };

  /** RCS elevation tables keyed on RCS frequency */
  typedef std::map<float, ELEVMAP*> FREQ_ELEV_MAP;

  /** @brief Container of ELEVMAPs sorted on frequency */
  struct SDKCORE_EXPORT FREQMAP
  {
    FREQ_ELEV_MAP freqMap;   ///< Container of ELEVMAPs keyed on frequency
    FREQMAP() {}
    ~FREQMAP();
  };

  /** RCS frequency tables keyed on RCS polarity */
  typedef std::map<PolarityType, FREQMAP*> POLARITY_FREQ_ELEV_MAP;

  /**
   * @brief Storage class used for multiple sub-tables of RCS values associated to an azimuth value.
   *
   * The sub-tables are organized  into hierarchical containers stored under specified polarity,
   * frequency and elevation values.  A given polarity can have one or more frequencies associated
   * with it.  A given frequency can have one or more elevation values, and an elevation can have
   * one or more data pairings of azimuth and RCS values.  The azimuthal data can be to any desired
   * degree of resolution and can be irregularly spaced.  If a requested polarity is not found in
   * the file -300 dB is returned.  Frequency selection is based on a nearest neighbor selection.
   * Elevation and azimuth values are interpolated, if the data allows.  This class also has the
   * ability to perform various types of distributions on the RCSTable data.  Currently Gaussian,
   * Rayleigh and Log normal distributions are supported.
   */
  class SDKCORE_EXPORT RCSLUT : public RadarCrossSection
  {
  public:
    RCSLUT();
    virtual ~RCSLUT();

    /**
    * This method checks the incoming RCS data filename, opens a file stream and parses the RCS data.
    * This is a convenience method that will determine the data type based on filename extension.
    * @param[in ] fname Input file name
    * @return 0 on success.
    */
    virtual int loadRCSFile(const std::string& fname);

    /**
     * Stream version of RCSLUT::loadRCSFile().  Loads RCS from a stream.  RCS type is determined
     * by inspecting the contents of the stream before reading the data.
     * @param istream Input stream containing RCS data
     * @return 0 on successful load
     */
    int loadRCSFile(std::istream& istream);

    /**
    * This method returns the type of radar cross section
    * @return radar cross section type
    */
    virtual RCSType type() const { return RCS_LUT; }

    /**
    * This method computes the RCS value in square meters for the requested parameters
    * @param[in ] freq Frequency of radar in MHz
    * @param[in ] azim Relative azimuth angle, referenced to host platform (rad)
    * @param[in ] elev Relative elevation angle, referenced to host platform (rad)
    * @param[in ] pol Radar polarity
    * @return RCS value (square meters)
    */
    virtual float RCSsm(float freq, double azim, double elev, PolarityType pol=POLARITY_UNKNOWN);

    /**
    * This method computes the RCS value in dB for the requested parameters
    * @param[in ] freq Frequency of radar in MHz
    * @param[in ] azim Relative azimuth angle, referenced to host platform (rad)
    * @param[in ] elev Relative elevation angle, referenced to host platform (rad)
    * @param[in ] pol Radar polarity
    * @return RCS value (dB)
    */
    virtual float RCSdB(float freq, double azim, double elev, PolarityType pol=POLARITY_UNKNOWN);

    /**
    * This method sets the radar cross section modulation value
    * @param[in ] mod Radar cross section modulation value (sq meters)
    */
    void modulation(float mod) { modulation_ = mod; }

    /**
    * This method returns the radar cross section modulation value
    * @return radar cross section modulation (sq meters)
    */
    float modulation() const { return modulation_; }

    /**
    * This method returns the radar cross section mean value
    * @return radar cross section mean (dB)
    */
    float mean() const { return mean_; }

    /**
    * This method returns the radar cross section median value
    * @return radar cross section median (dB)
    */
    float median() const { return median_; }

    /**
    * This method returns the radar cross section minimum value
    * @return radar cross section minimum (dB)
    */
    float min() const { return min_; }

    /**
    * This method returns the radar cross section maximum value
    * @return radar cross section maximum (dB)
    */
    float max() const { return max_; }

  protected:
    std::string description_;           ///< description of RCS data
    RCSTableType tableType_;            ///< RCS table type
    RCSFuncType functionType_;          ///< RCS distribution function
    NormalVariable gaussian_;           ///< random process applied to rcs values
    float modulation_;                  ///< scintillation modulation applied to rcs (sq meter)
    float mean_;                        ///< mean cross section (dBsm) arithmetical average of all RCS
    float median_;                      ///< median cross section (dBsm) center or midpoint of sorted RCS
    float min_;                         ///< min cross section (dBsm)
    float max_;                         ///< max cross section (dBsm)
    POLARITY_FREQ_ELEV_MAP rcsMap_;     ///< RCS data

    float lastFreq_;                    ///< last frequency used for look up
    float lastElev_[2];                 ///< two last elevations used for look up
    PolarityType lastPolarity_;         ///< last polarity used for look up
    RCSTable *loTable_[2];              ///< pointers to two last accessed lo tables
    RCSTable *hiTable_[2];              ///< pointers to two last accessed hi tables

    /**
    * This method returns an azimuth based RCSTable
    * @param[in ] freq Frequency of radar in MHz
    * @param[in ] elev Relative elevation angle, referenced to host platform (rad)
    * @param[in ] pol Radar polarity
    * @param[in ] create Boolean flag, if true a table is created if not found
    * @return RCSTable.
    */
    RCSTable* getTable_(float freq, float elev, PolarityType pol, bool create);

    /**
    * This method returns a RCS value (sq meter) based on input parameters
    * @param[in ] freq Frequency of radar in MHz
    * @param[in ] azim Relative azimuth angle, referenced to host platform (rad)
    * @param[in ] elev Relative elevation angle, referenced to host platform (rad)
    * @param[in ] pol Radar polarity
    * @return RCS value in square meters.
    */
    float calcTableRCS_(float freq, double azim, double elev, PolarityType pol);

    /**
    * This method parses and loads a RCS table file (RCS_LUT type)
    * @param[in ] inFile Input stream
    * @return 0 on success
    */
    int loadRcsLutFile_(std::istream &inFile);

    /**
    * This method parses and loads a XPatch RCS file
    * @param[in ] inFile Input stream
    * @return 0 on success
    */
    int loadXPATCHRCSFile_(std::istream &inFile);

    /**
    * This method parses and loads a SADM RCS file
    * @param[in ] inFile Input stream
    * @return 0 on success
    */
    int loadSADMRCSFile_(std::istream &inFile);

    /**
    * This method resets the RCS data, deleting any allocated memory
    */
    void reset_();

    /**
    * This method verifies a parsed line of string tokens
    * @param[in ] val Number on incoming tokens
    * @param[in ] min Minimum number of tokens required
    * @param[in ] param Current parameter name being checked
    * @return 0 on success
    */
    int checkTokens_(size_t val, size_t min, const std::string& param);

    /**
    * This method computes the min, max, mean, and median RCS values
    * medianVec must not be empty
    */
    void computeStatistics_(std::vector<float>* medianVec);
  };

  /** @brief Contains static methods for loading RCS data files. */
  class SDKCORE_EXPORT RcsFileParser
  {
  public:
    /**
    * This method opens a file stream and parses the RCS data
    * @param[in ] fname Input file name
    * @return valid RadarCrossSection on success, NULL otherwise.
    */
    static RadarCrossSection* loadRCSFile(const std::string& fname);
  };

} // namespace simcore

#endif  /* SIMCORE_EM_RADAR_CROSS_SECTION_H */
