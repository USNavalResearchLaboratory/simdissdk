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
#ifndef SIMQT_TIMESTAMPEDLAYERMANAGER_H
#define SIMQT_TIMESTAMPEDLAYERMANAGER_H

#include <QObject>
#include "osgEarth/MapNodeObserver"
#include "simCore/Common/Memory.h"
#include "simCore/Time/TimeClass.h"

namespace simCore { class Clock; }

namespace osgEarth { class ImageLayer; }

namespace simQt {

/**
 * TimestampedLayerManager allows for image layers which are shown or hidden depending on the current time.
 * It watches the map for any image layers that have a time configuration and the clock for time updates.  On time updates,
 * it shows or hides the layers it finds so that only the layer with greatest time that is less than or equal to current time is shown.
 * No timed layers are shown before the earliest layer's time.
 * If image layers are set invisible by another class, this class will not set them visible.
 */
class SDKQT_EXPORT TimestampedLayerManager : public QObject
{
  Q_OBJECT;
public:
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
   * @param layer Layer to check for
   * @return True if layer is being maintained as a timed layer, false else
   */
  bool layerIsTimed(const osgEarth::ImageLayer* layer) const;

  /**
   * Returns the layer that has time value closest after the current time
   * @return The layer that has time value closest after the current time
   */
  const osgEarth::ImageLayer* currentTimedLayer() const;

  /**
   * Sets the map node which will have its timed image layers managed.
   * Restores visibility settings of previous map's image layers if they still exist
   * @param mapNode New map node to get image layers from
   */
  void setMapNode(osgEarth::MapNode* mapNode);

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

  /** Inner class to act as an osgEarth::MapNodeObserver for this class */
  class MapChangeObserver;
  /** Class to listen to the map for new image layers and add them to the manager if needed */
  class MapListener;
  /** Class to listen to the clock for changes in current time */
  class ClockListener;

  osg::ref_ptr<MapListener> mapListener_;
  std::tr1::shared_ptr<ClockListener> clockListener_;
  std::map<simCore::TimeStamp, osg::observer_ptr<osgEarth::ImageLayer> > layers_;
  /** NOTE: Keys are unowned, naked pointers.  Do not dereference. */
  std::map<const osgEarth::ImageLayer*, bool> originalVisibility_;
  /** Image layer with the highest time value that is less than or equal to current time */
  osg::observer_ptr<osgEarth::ImageLayer> currentLayer_;
  simCore::Clock& clock_;
  simCore::TimeStamp currTime_;
  osg::ref_ptr<osg::Node> mapChangeObserver_;
  osg::ref_ptr<osg::Group> attachPoint_;
};

}


#endif
