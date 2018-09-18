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
#ifndef SIMDIS_DB_H
#define SIMDIS_DB_H 1

#include <string>
#include "osgEarth/TileSource"
#include "osgEarth/URI"
#include "simCore/Common/Common.h"
#include "simVis/osgEarthVersion.h"

namespace simVis
{
  /**
  * Configuration options for the "DB" tile source driver.
  */
  class DBOptions : public osgEarth::TileSourceOptions // header-only (no export)
  {
  public:
    /** Location of the DB file to load (mutable) */
    osgEarth::optional<osgEarth::URI>& url() { return _url; }
    /** Location of the DB file to load (immutable) */
    const osgEarth::optional<osgEarth::URI>& url() const { return _url; }

    /** Deepest level (in .db depth) for reading data from the .db file. (mutable) */
    osgEarth::optional<unsigned int>& deepestLevel() { return _deepestLevel; }
    /** Deepest level (in .db depth) for reading data from the .db file. (immutable) */
    const osgEarth::optional<unsigned int>& deepestLevel() const { return _deepestLevel; }

  public:
    /**
    * Construct a new DB options structure
    * @param opt Options data from which to deserialize configuration
    */
    DBOptions(const osgEarth::ConfigOptions &opt = osgEarth::ConfigOptions())
      : TileSourceOptions(opt)
    {
      setDriver("db");
      fromConfig_(_conf);
    }

    // Virtual destructor for virtual functions
    virtual ~DBOptions() {}

  protected:
    /// get the current configuration
    // (override from osgEarth::TileSourceOptions)
    virtual osgEarth::Config getConfig() const
    {
      osgEarth::Config conf = osgEarth::TileSourceOptions::getConfig();
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,10,0)
      conf.set("url", _url);
      conf.set("deepest_level", _deepestLevel);
#else
      conf.updateIfSet("url", _url);
      conf.updateIfSet("deepest_level", _deepestLevel);
#endif
      return conf;
    }

    /// add in the settings from 'conf' to current
    // (override from osgEarth::TileSourceOptions)
    virtual void mergeConfig(const osgEarth::Config& conf)
    {
      osgEarth::TileSourceOptions::mergeConfig(conf);
      fromConfig_(conf);
    }

  private:
    /// set current settings to 'conf'
    void fromConfig_(const osgEarth::Config& conf)
    {
#if SDK_OSGEARTH_MIN_VERSION_REQUIRED(1,10,0)
      conf.get("url", _url);
      conf.get("deepest_level", _deepestLevel);
#else
      conf.getIfSet("url", _url);
      conf.getIfSet("deepest_level", _deepestLevel);
#endif
    }

    osgEarth::optional<osgEarth::URI> _url;
    osgEarth::optional<unsigned int> _deepestLevel;
  };
}

#endif // SIMDIS_DB_H
