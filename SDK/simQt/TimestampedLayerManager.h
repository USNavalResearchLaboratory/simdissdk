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
#ifndef SIMQT_TIMESTAMPEDLAYERMANAGER_H
#define SIMQT_TIMESTAMPEDLAYERMANAGER_H

#include <memory>
#include <QObject>
#include "osgEarth/MapNodeObserver"
#include "simCore/Time/TimeClass.h"

namespace simCore { class Clock; }

namespace osgEarth { class ImageLayer; }

namespace simQt {

/**
 * TimestampedLayerManager allows for image layers which are shown or hidden depending on the current time.  Timed layers are created by
 * giving the layer a <time> property in the configuration file.  This property takes a UTC time reference as the layer's start time.
 * Timed layers can also specify a group name with the <time_group> option that takes a nonempty string as the group name.  Layers with
 * no group specified will be considered in the same group.  At most one layer from each group will be shown at a time.  This class watches the
 * map for any image layers that have a time configuration and the clock for time updates.  On time updates, it shows or hides the layers
 * it finds so that, for each group, only the layer with greatest time that is less than or equal to current time is shown.  No timed layers
 * are shown before the earliest layer's time. If image layers are set invisible by another class, this class will not set them visible.
 */
class SDKQT_EXPORT TimestampedLayerManager : public QObject
{
  Q_OBJECT;
public:

  /// This will be used as the group name for timed layers which don't specify any group
  static const std::string DEFAULT_LAYER_TIME_GROUP;

  /**
   * Constructor
   * @param clock Clock to watch for time updates
   * @param attachPoint Attach point for an osgEarth::MapNodeObserver
   * @param parent Parent object
   */
  TimestampedLayerManager(simCore::Clock& clock, osg::Group* attachPoint, QObject* parent = NULL);

  /** Destructor */
  virtual ~TimestampedLayerManager();

  /**
   * Indicates whether given layer is one of the timed layers being maintained by this object
   * If timing is not active, this method will return false for all layers
   * @param layer Layer to check for
   * @return True if layer is being maintained as a timed layer, false else
   */
  bool layerIsTimed(const osgEarth::ImageLayer* layer) const;

  /// Gets the time group string from the given layer.  Returns empty string if layer is not timed
  std::string getLayerTimeGroup(const osgEarth::ImageLayer* layer) const;
  /** Returns the time associated with the given layer, if layer is timed.  Returns simCore::INFINITE_TIME_STAMP if not timed. */
  simCore::TimeStamp getLayerTime(const osgEarth::ImageLayer* layer) const;

  /**
   * Returns the layer that has time value closest after the current time for the given time group.
   * If timing is not active, the current timed layer will always be null.
   * @param timeGroup Name of the time group to get the current layer from.
   * @return The layer that has time value closest after the current time
   */
  const osgEarth::ImageLayer* getCurrentTimedLayer(const std::string& timeGroup = DEFAULT_LAYER_TIME_GROUP) const;

  /**
   * Sets the map node which will have its timed image layers managed.
   * Restores visibility settings of previous map's image layers if they still exist
   * @param mapNode New map node to get image layers from
   */
  void setMapNode(osgEarth::MapNode* mapNode);

  /// Set the active state of timed layer processing.  When moving from active to inactive, original visibility of timed layers is restored
  void setTimingActive(bool active);
  bool timingActive() const;

signals:
  /**
   * Emitted when the current layer changes.   New layer or old layer can be NULL.  If non-NULL, newLayer
   * and oldLayer are guaranteed to be part of the map associated with this object at time of emission.  This
   * signal indicates that visibility of timed layers has changed.  Further processing is not needed for simple
   * timed showing or hiding.
   * @param newLayer New current layer
   * @param previousLayer Previous current layer
   */
  void currentTimedLayerChanged(const osgEarth::ImageLayer* newLayer = NULL, const osgEarth::ImageLayer* previousLayer = NULL);

private:

  /** Inner class to keep track of a time group's list of tracked layers and current layer */
  class TimeGroup
  {
  public:
    /// Constructor
    TimeGroup();
    /// Destructor
    virtual ~TimeGroup();

    /// All layers in this time group mapped by their start time
    std::map<simCore::TimeStamp, osg::observer_ptr<osgEarth::ImageLayer> > layers;
    /// Image layer with the highest time value that is less than or equal to current time
    osg::observer_ptr<osgEarth::ImageLayer> currentLayer;
  };

  /**
   * Check the given layer for time values, if any are found, adds it to the layers being watched
   * @param Layer to begin managing if time configuration is found
   */
  void addLayerWithTime_(osgEarth::ImageLayer* newLayer);

  /**
   * Called by mapChangeObserver when it receives a new map
   * @param mapNode New map node
   */
  void setMapNode_(osgEarth::MapNode* mapNode);

  /**
   * Respond to changes in current time from the clock.  Updates current layer and visibility if needed
   * @param stamp New current timestamp
   */
  void setTime_(const simCore::TimeStamp& stamp);

  /// Restore the original visibility of all layers tracked by the manager
  void restoreOriginalVisibility_();
  /// Set all timed layers but the current (if there is one) invisible
  void useTimedVisibility_();

  /** Inner class to act as an osgEarth::MapNodeObserver for this class */
  class MapChangeObserver;
  /** Class to listen to the map for new image layers and add them to the manager if needed */
  class MapListener;
  /** Class to listen to the clock for changes in current time */
  class ClockListener;

  osg::ref_ptr<MapListener> mapListener_;
  std::shared_ptr<ClockListener> clockListener_;
  std::map<std::string, TimeGroup*> groups_;
  /** NOTE: Keys are unowned, naked pointers.  Do not dereference. */
  std::map<const osgEarth::ImageLayer*, bool> originalVisibility_;
  simCore::Clock& clock_;
  simCore::TimeStamp currTime_;
  osg::ref_ptr<osg::Node> mapChangeObserver_;
  osg::ref_ptr<osg::Group> attachPoint_;
  bool timingActive_;
};

}


#endif
