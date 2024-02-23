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
#ifndef SIMVIS_PLANETARIUM_VIEW_TOOL_H
#define SIMVIS_PLANETARIUM_VIEW_TOOL_H

#include <memory>
#include "osg/Group"
#include "osg/Texture2D"
#include "osg/observer_ptr"
#include "simData/DataSlice.h"
#include "simVis/EntityFamily.h"
#include "simVis/Tool.h"
#include "simVis/Types.h"

namespace osg {
class Geometry;
class TransferFunction1D;
class Vec4f;
}

namespace osgEarth { class LineDrawable; }

namespace simCore { class TimeStamp; }

namespace simData
{
class BeamUpdate;
class DataStore;
}

namespace simVis
{
class BeamNode;
class HorizonGeoFence;
class LocatorNode;
class PlatformNode;
class ScenarioManager;
class TargetDelegation;

/**
 * PlanetariumViewTool is a tool that draws a translucent dome
 * or sphere around a platform and "projects" the platform's
 * sensors and targets onto the surface of the dome.
 */
class SDKVIS_EXPORT PlanetariumViewTool : public ScenarioTool
{
public:
  /**
   * Constructs a new dome/sensor viewing tool.
   * @param[in ] host View will center on this host.
   * @param[in ] ds Reference to data store for usage by some beam history nodes
   */
  PlanetariumViewTool(PlatformNode* host, simData::DataStore& ds);

  /**
   * Range of the sensor intersection dome from the host
   * @param[in ] range Range from the host in meters
   */
  void setRange(double range);
  /** Retrieve the range of sensor intersection dome from the host, in meters */
  double getRange() const { return range_; }

  /**
   * Color of the transparent dome/sphere
   * @param[in ] color Color
   */
  void setColor(const osg::Vec4f& color);
  /** Retrieves the color of the transparent dome/sphere (RGBA OSG style) */
  const osg::Vec4f& getColor() const { return domeColor_; }

  /**
   * Whether to display target vectors - vectors from the dome's
   * surface to the target.
   */
  void setDisplayTargetVectors(bool value);
  /** Retrieves whether to show target vectors */
  bool getDisplayTargetVectors() const { return displayTargetVectors_; }

  /**
   * Sets a Beam Prefs template to use for the beam display
   * when projected on to the dome.
   * @param[in ] prefs Beam prefs template
   */
  void setBeamPrefs(const simData::BeamPrefs& prefs);
  /** Retrieves a reference to the beam prefs template for beam display */
  const simData::BeamPrefs& getBeamPrefs() const { return beamPrefs_; }

  /**
   * Sets a Gate Prefs template to use for the gate display
   * when projected on to the dome.
   * @param[in ] prefs Gate prefs template
   */
  void setGatePrefs(const simData::GatePrefs& prefs);
  /** Retrieve a reference to the gate prefs template for gate display */
  const simData::GatePrefs& getGatePrefs() const { return gatePrefs_; }

  /** Set whether beam history is displayed on the planetarium */
  void setDisplayBeamHistory(bool display);
  /** Get whether beam history is displayed on the planetarium */
  bool getDisplayBeamHistory() const;

  /** Set beam history length in seconds */
  void setBeamHistoryLength(double history);
  /** Get beam history length in seconds */
  double getBeamHistoryLength() const;

  /** Set whether to display gates on the planetarium */
  void setDisplayGates(bool display);
  /** Get whether gates are displayed */
  bool getDisplayGates() const;

  /** Set whether to use a gradient when displaying history points */
  void setUseGradient(bool useGradient);
  /** Whether to use a gradient when displaying history points */
  bool getUseGradient() const;

  /** Set whether to draw a sector instead of a full planetarium */
  void setUseSector(bool useSector);
  /** Get whether to draw a sector instead of a full planetarium */
  bool getUseSector() const;

  /** Set the pointing azimuth for the sector in degrees */
  void setSectorAzimuth(double azDeg);
  /** Get the pointing azimuth for the sector in degrees */
  double getSectorAzimuth() const;

  /** Set the pointing elevation for the sector in degrees */
  void setSectorElevation(double elDeg);
  /** Get the pointing elevation for the sector in degrees */
  double getSectorElevation() const;

  /** Set the sector width in degrees */
  void setSectorWidth(double widthDeg);
  /** Get the sector width in degrees */
  double getSectorWidth() const;

  /** Set the sector height in degrees */
  void setSectorHeight(double heightDeg);
  /** Get the sector height in degrees */
  double getSectorHeight() const;

  /** Planetarium can be textured with up to 4 textures */
  enum class TextureUnit {
    UNIT0 = 0,
    UNIT1,
    UNIT2,
    UNIT3
  };

  /** Changes the texture overlay for the given texture unit. Textures are off by default, use setTextureActive() as needed. */
  void setTextureImage(TextureUnit texUnit, osg::Image* image);
  /** Retrieves the texture overlay for the given texture unit */
  osg::Image* getTextureImage(TextureUnit texUnit) const;

  /** Changes texture coordinates for a given image unit; Defaults are latitude from -90 to +90, longitude from -180 to +180. */
  void setTextureCoords(TextureUnit texUnit, double minLat, double maxLat, double minLon, double maxLon);
  /** Retrieves texture coordinates for the given image units. */
  void getTextureCoords(TextureUnit texUnit, double& minLat, double& maxLat, double& minLon, double& maxLon) const;

  /** Changes the alpha for applying the texture (multiplied against the alpha of the image); value is 0.0 to 1.0. */
  void setTextureAlpha(TextureUnit texUnit, float alpha);
  /** Retrieves the alpha value for a given texture */
  float getTextureAlpha(TextureUnit texUnit) const;

  /** Changes whether a texture is drawn or not. Textures default off. */
  void setTextureEnabled(TextureUnit texUnit, bool active);
  /** Returns true if the texture has been enabled on the dome. */
  bool getTextureEnabled(TextureUnit texUnit) const;

  /** Turns on "texture only" mode, where sphere background is not drawn if there are textures */
  void setTextureOnlyMode(bool textureOnlyMode);
  /** Retrieves state of texture-only mode */
  bool getTextureOnlyMode() const;

public: // ScenarioTool

  /** @see ScenarioTool::onInstall() */
  virtual void onInstall(const ScenarioManager& scenario) override;

  /** @see ScenarioTool::onUninstall() */
  virtual void onUninstall(const ScenarioManager& scenario) override;

  /** @see ScenarioTool::onEntityAdd() */
  virtual void onEntityAdd(const ScenarioManager& scenario, EntityNode* entity) override;

  /** @see ScenarioTool::onEntityRemove() */
  virtual void onEntityRemove(const ScenarioManager& scenario, EntityNode* entity) override;

  /** @see ScenarioTool::onUpdate() */
  virtual void onUpdate(const ScenarioManager& scenario, const simCore::TimeStamp& timeStamp, const EntityVector& updates) override;

  /** @see ScenarioTool::onFlush() */
  virtual void onFlush(const ScenarioManager& scenario, simData::ObjectId flushedId) override;

public: // Tool

  /// returns the node to display in the scenario graph
  osg::Node* getNode() const;

public: // internal

  /// Updates the geometries on the dome when target delegation's UpdateGeometryCallback detects geometry changes
  void updateTargetGeometry(osg::MatrixTransform*, const osg::Vec3d&) const;

protected:
  virtual ~PlanetariumViewTool();

private:
  /** Group that stores and manages a beam's history points on a planetarium */
  class BeamHistory : public osg::Group
  {
  public:
    BeamHistory(simVis::BeamNode* beam, simData::DataStore& ds, double range);

    /** Update the beam history to the specified time */
    void updateBeamHistory(double time);
    /** Set history length in seconds */
    void setHistoryLength(double historyLength);
    /** Set whether to use a gradient when displaying history points */
    void setUseGradient(bool useGradient);
    /** Set the range of the planetarium. Used to correctly position history points */
    void setRange(double range);

  protected:
    /** Protect osg::Referenced-derived destructor */
    virtual ~BeamHistory();

  private:
    /// sentinel value for no commanded color
    const simVis::Color NO_COMMANDED_COLOR = osg::Vec4f(
      std::numeric_limits<float>::max(),
      std::numeric_limits<float>::max(),
      std::numeric_limits<float>::max(),
      std::numeric_limits<float>::max());

    /// sentinel value for no commanded hbw or vbw
    const double NO_COMMANDED_BEAMWIDTH = std::numeric_limits<double>::max();

    /** Represents a history point node and its original color */
    struct HistoryPoint
    {
      osg::ref_ptr<simVis::LocatorNode> node; ///< Node representing the beam history point
      simVis::Color color; ///< Used to preserve color when history point was created. Alpha is subject to change based on current time
      bool hasCommandedHbw;
      bool hasCommandedVbw;
    };

    /** Find all beam updates from dataStore/slice in interval (lastTime, currentTime] to add to the beam's history visualization */
    void backfill_(double lastTime, double currentTime);
    /** Add the specified update to a beam's history visualization */
    void addPointFromUpdate_(const simData::BeamPrefs& prefs,
      bool hasCommandedHbw, bool hasCommandedVbw, const simVis::Color& color,
      const simData::BeamUpdate* update, double updateTime);
    /** Limit history points according to time and point limit prefs */
    void applyDataLimiting_(const simData::BeamPrefs& prefs);
    /** Initialize the gradient used for history point colors */
    void initGradient_();

    osg::observer_ptr<simVis::BeamNode> beam_;
    const simData::BeamUpdateSlice* beamUpdateSlice_;
    const simData::BeamCommandSlice* beamCommandSlice_;
    /** History points, keyed by time in seconds since ref year */
    std::map<double, std::unique_ptr<HistoryPoint> > historyPoints_;
    /** History length to show in seconds */
    double historyLength_;
    /** Whether to show history points in a gradient */
    bool useGradient_;
    /** Whether to limit data in history */
    bool limitingData_;
    /** Gradient for history points. Used when useGradient_ is true. nullptr until first needed */
    osg::ref_ptr<osg::TransferFunction1D> gradientFunction_;
    /** Cached time of the first data point for the associated beam */
    double firstTime_;
    /** Current range of the planetarium, updated via setRange() */
    double range_;
    /** cache of the last time history was updated */
    double lastUpdateTime_;
  };

  /** Represents a single texture overlay */
  struct TextureData
  {
    /** Pointer to the image in the texture */
    osg::ref_ptr<osg::Image> image;
    /** Pointer to the texture being used */
    osg::ref_ptr<osg::Texture2D> texture;
    /** Latitude values for texture on the dome; e.g. -90. is bottom of dome, 90 is top. */
    osg::Vec2d latitudeSpan = { -90., 90. };
    /** Longitude values for texture on the dome; e.g. -180. is back (left) of dome, 90 is halfway from front to back on right. */
    osg::Vec2d longitudeSpan = { -180., 180. };
    /** Alpha value to apply on top of the texture when mixing colors. */
    float alpha = 1.f;
    /** Toggle this to make the image not be rendered, without removing it entirely from the dome. */
    bool enabled = false;
  };

  void applyOverrides_(bool enable);
  void applyOverrides_(EntityNode* node, bool enable) const;
  void updateDome_();
  void scaleTargetGeometry_(double range) const;
  osg::Node* buildVectorGeometry_() const;
  void addBeamToBeamHistory_(simVis::BeamNode* beam);
  void flushFamilyEntity_(const EntityNode* entity);

  /** Retrieves a single texture, given a unit */
  TextureData& getTexture_(TextureUnit texUnit);
  /** Retrieves a single texture, given a unit (const version) */
  const TextureData& getTexture_(TextureUnit texUnit) const;
  /** Sets shader values for the given unit, pulling them from the saved texture info */
  void applyTexture_(TextureUnit texUnit);
  /** Sets all shader values for all texture units */
  void applyAllTextures_();
  /** Sets shader value for texture-only mode */
  void applyTextureOnlyMode_();

  EntityFamily                    family_;
  osg::observer_ptr<PlatformNode> host_;
  simData::DataStore&             ds_;
  osg::ref_ptr<LocatorNode>       locatorRoot_;
  osg::ref_ptr<osg::Group>        root_;
  double                          range_; ///< planetarium radius, in meters
  osg::Vec4f                      domeColor_;
  simData::BeamPrefs              beamPrefs_;
  simData::GatePrefs              gatePrefs_;
  bool                            displayTargetVectors_;
  bool                            displayBeamHistory_;
  bool                            displayGates_;
  double                          historyLength_; ///< seconds
  double                          lastUpdateTime_; ///< seconds-since-ref-year
  bool                            useGradient_;
  bool                            useSector_;
  double                          sectorAzDeg_;
  double                          sectorElDeg_;
  double                          sectorWidthDeg_;
  double                          sectorHeightDeg_;
  osg::observer_ptr<const ScenarioManager> scenario_;
  osg::ref_ptr<TargetDelegation>  targets_;
  osg::ref_ptr<HorizonGeoFence>   fence_;
  osg::ref_ptr<osg::Geometry>     dome_;
  osg::ref_ptr<osg::Node>         targetGeom_;
  std::map<simData::ObjectId, osg::ref_ptr<BeamHistory> > history_;
  std::vector<TextureData> textures_;
  bool                            textureOnlyMode_;

  class ProjectorMonitor;
  std::unique_ptr<ProjectorMonitor> projectorMonitor_;
};

} // namespace simVis

#endif // SIMVIS_PLANETERIUM_VIEW_TOOL_H
