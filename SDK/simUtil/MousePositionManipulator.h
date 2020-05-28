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
#ifndef SIMUTIL_MOUSEPOSITIONMANIPULATOR_H
#define SIMUTIL_MOUSEPOSITIONMANIPULATOR_H

#include "osg/Node"
#include "osg/observer_ptr"
#include "osgEarth/GeoData"
#include "simCore/Common/Common.h"
#include "simUtil/MouseManipulator.h"

namespace osg { class Group; }
namespace osgViewer { class View; }
namespace osgEarth {
  class MapNode;
  class TerrainEngineNode;
}
namespace simVis { class ElevationQueryProxy; }

namespace simUtil {

/** Implements MouseManipulator to provide a method for dealing with
  * mouse clicks, to integrate with priority into a MouseDispatcher.  If you
  * don't care about priority integration, then an osgGA::GUIEventHandler
  * might be more appropriate.
  */
class SDKUTIL_EXPORT MousePositionManipulator : public simUtil::MouseManipulatorAdapter
{
public:
  /// Sentinel value for invalid latitude, longitude, or altitude values
  static const double INVALID_POSITION_VALUE;

  /// Observer for getting updates on mouse position on sphere
  class Listener
  {
  public:
    virtual ~Listener(){}
    /// Called whenever the mouse moves and is at the passed in lat/lon (degrees) and alt (meters). Notification also happens in the FRAME event, for pending elevation queries.
    virtual void mouseOverLatLon(double lat, double lon, double alt) = 0;
  };

  /// Constructor; requires map node for picking points, and scene attachment for listening to map changes
  MousePositionManipulator(osgEarth::MapNode* mapNode, osg::Group* scene);
  /// Destructor
  virtual ~MousePositionManipulator();

  /** Mouse button released, returns non-zero on handled */
  virtual int release(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
  /** Mouse being moved, returns non-zero on handled */
  virtual int move(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
  /** Mouse being dragged, returns non-zero on handled */
  virtual int drag(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
  /** Frame event, returns non-zero on handled */
  virtual int frame(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

  /// Sets the map node
  void setMapNode(osgEarth::MapNode* mapNode);

  /// Returns the last LLA point in degrees and absolute altitude in meters that the mouse moved to
  osgEarth::GeoPoint lastLLA() const;
  /**
  * Fills in the last cached x and y mouse coordinates. units are screen coordinates or 0.0 if the mouse has not yet entered the screen
  * 0,0 is the lower left corner
  */
  void getLastXY(float& lastX, float& lastY) const;

  /// Returns an LLA point in degrees and absolute altitude in meters based on the mouse x,y point passed in or -DBL_MAX if the point is off the globe
  osgEarth::GeoPoint getLLA(float mx, float my, bool queryElevation) const;
  /// Returns an LLA point in degrees and absolute altitude in meters based on the cached lastMouseX_,lastMouseY_ point or -DBL_MAX if the point is off the globe
  osgEarth::GeoPoint getLLA(bool queryElevation) const;
  /// Adds a listener for mouse over lat long events
  void addListener(Listener* listener, bool queryElevation);
  /// Removes a listener for mouse over lat long events, memory for listener is not deallocated here
  void removeListener(Listener* listener);

  /// Changes the resolution of the elevation query in radians. This affects how expensive the call to the elevation query will be, more precise resolutions will take more time
  void setTerrainResolution(double resolutionRadians);
  /// Retrieves the terrain resolution to use in elevation query
  double getTerrainResolution() const;

  /**
  * Retrieve the terrain elevation of currently loaded map data in meters, into elevationMeters; will do nothing to elevationMeters if query fails for any reason.
  * @return 0 on success, non-zero on failure
  */
  int getElevation(const osgEarth::GeoPoint& lonLatAlt, double& elevationMeters) const;

private:

  /**
  * Gets the elevation from an elevation query, into elevationMeters if blocking is true. If not blocking, elevationMeters gets the last cached elevation value
  * and elevation will be returned in the frame() event.
  * @return 0 on success, non-zero on failure
  */
  int getElevation_(const osgEarth::GeoPoint& lonLatAlt, double& elevationMeters, bool blocking) const;

  /**
  * Returns an LLA point in degrees and absolute altitude in meters based on the mouse x,y point passed in or -DBL_MAX if the point is off the globe
  * If blocking is true, will block querying elevation. Otherwise, will use the asynchronous elevation query, that will eventually return final
  * elevation value in the frame() event.
  */
  osgEarth::GeoPoint getLLA_(float mx, float my, bool queryElevation, bool blocking) const;


  // Reference pointers
  osg::observer_ptr<osgEarth::MapNode> mapNode_;
  osg::NodePath mapNodePath_;
  osg::observer_ptr<osgEarth::TerrainEngineNode> terrainEngineNode_;

  /// Last view from mouse movement
  osg::observer_ptr<osgViewer::View> lastView_;
  /// Last mouse x from mouse movement
  float lastMouseX_;
  /// Last mouse y from mouse movement
  float lastMouseY_;
  /// Last latitude longitude altitude point in degrees and absolute altitude in meters from mouse movement
  osgEarth::GeoPoint lastLLA_;
  /// Vector of listeners for mouse-over lat long events
  std::vector<Listener*> llListeners_;
  /// Vector of listeners for mouse-over lat long events, that also care about altitude
  std::vector<Listener*> llaListeners_;
  /// Calculates the elevation at a point on the map
  simVis::ElevationQueryProxy* elevationQuery_;
  /// Terrain resolution to pass to the elevation query; appears to be in radians
  double terrainResolution_;

  /// Specialization of an osgEarth::MapNodeReplacer
  class MapChangeListener;
  /// Responsible for notifying when the map changes
  osg::ref_ptr<MapChangeListener> mapChangeListener_;
  /// Holds the reference to the scene, under which the Map Change Listener is listening
  osg::observer_ptr<osg::Group> scene_;
};

}

#endif /* SIMUTIL_MOUSEPOSITIONMANIPULATOR_H */
