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
#ifndef SIMVIS_DBFORMAT_H
#define SIMVIS_DBFORMAT_H

#include <string>
#include "osgEarth/TileSourceImageLayer"
#include "osgEarth/TileSourceElevationLayer"
#include "osgEarth/URI"
#include "simCore/Common/Common.h"
#include "simCore/Common/Export.h"
#include "simCore/Time/TimeClass.h"
#include "simVis/DBOptions.h"

namespace simVis
{

// OE_OPTION uses unqualified optional<> type
using osgEarth::optional;

class SDKVIS_EXPORT DBImageLayer : public osgEarth::ImageLayer
{
public: // serialization
  class SDKVIS_EXPORT Options : public osgEarth::ImageLayer::Options {
  public:
    META_LayerOptions(simVis, Options, osgEarth::ImageLayer::Options);
    OE_OPTION(osgEarth::URI, url);
    OE_OPTION(unsigned, deepestLevel);
    virtual osgEarth::Config getConfig() const;
  private:
    void fromConfig(const osgEarth::Config&);
  };

public:
  META_Layer(simVis, DBImageLayer, Options, osgEarth::ImageLayer, DBImage);

public:
  /// Base URL of TileCache endpoint
  void setURL(const osgEarth::URI& value);
  const osgEarth::URI& getURL() const;

  /// Maximum level to use in DB file
  void setDeepestLevel(const unsigned int& value);
  const unsigned int& getDeepestLevel() const;

public: // Layer

  /// Establishes a connection to the database
  virtual osgEarth::Status openImplementation();

  /// Creates a raster image for the given tile key
  virtual osgEarth::GeoImage createImageImplementation(const osgEarth::TileKey& key, osgEarth::ProgressCallback* progress) const;

protected: // Layer

  /// Called by constructors
  virtual void init();

  /// Destructor
  virtual ~DBImageLayer();

  void* context_;
};

/**
  * Elevation layer connected to a DB file
  */
class SDKVIS_EXPORT DBElevationLayer : public osgEarth::ElevationLayer
{
public: // serialization
  class SDKVIS_EXPORT Options : public osgEarth::ElevationLayer::Options {
  public:
    META_LayerOptions(simVis, Options, osgEarth::ElevationLayer::Options);
    OE_OPTION(osgEarth::URI, url);
    OE_OPTION(unsigned, deepestLevel);
    virtual osgEarth::Config getConfig() const;
  private:
    void fromConfig(const osgEarth::Config&);
  };

public:
  META_Layer(simVis, DBElevationLayer, Options, osgEarth::ElevationLayer, DBElevation);

  /// URL of the database file
  void setURL(const osgEarth::URI& value);
  const osgEarth::URI& getURL() const;

  /// Maximum level to use in DB file
  void setDeepestLevel(const unsigned int& value);
  const unsigned int& getDeepestLevel() const;

public: // Layer

  /// Establishes a connection to the database
  virtual osgEarth::Status openImplementation();

  /// Creates a heightfield for the given tile key
  virtual osgEarth::GeoHeightField createHeightFieldImplementation(const osgEarth::TileKey& key, osgEarth::ProgressCallback* progress) const;

protected: // Layer

  /// Called by constructors
  virtual void init();

  /// Destructor
  virtual ~DBElevationLayer();

  void* context_;
};

}

#endif /* SIMVIS_DBFORMAT_H */
