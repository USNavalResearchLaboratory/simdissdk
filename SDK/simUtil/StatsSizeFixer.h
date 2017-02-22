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
#ifndef SIMUTIL_STATSSIZEFIXER_H
#define SIMUTIL_STATSSIZEFIXER_H

#include "osg/Camera"
#include "osgGA/GUIEventHandler"
#include "osgViewer/ViewerEventHandlers"

namespace simUtil {

/**
 * GUI Event Adapter that modifies the Stats handler's camera to have consistent graphics sizes even on resize.
 * By default, an osgViewer::StatsHandler (and by extension simUtil::StatsHandler) will resize the text to
 * keep a consistent aspect ratio, but also shrink and grow the text based on window height.  This event handler
 * will fix the camera on the StatsHandler to provide a consistent and unchanging font size.
 *
 * To use:
 * <code>
 * osgViewer::StatsHandler* stats = new osgViewer::StatsHandler;
 * viewer_->addEventHandler(stats);
 * viewer_->addEventHandler(new simUtil::StatsSizeFixer(stats));
 * </code>
 */
class StatsSizeFixer : public osgGA::GUIEventHandler
{
public:
  StatsSizeFixer(osgViewer::StatsHandler* stats)
    : stats_(stats),
      init_(false),
      sizeMultiplier_(1.0)
  {
  }

  /** Override handle() to change the StatsHandler's camera metrics to get text to not change size */
  virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
  {
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::KEYDOWN:
      // Fall-through intentional, just as it seems to be in osgViewer::StatsHandler::handle()
    case osgGA::GUIEventAdapter::RESIZE:
      // Always fix the camera on resize
      fixCameraSize_(ea.getWindowWidth(), ea.getWindowHeight());
      break;
    case osgGA::GUIEventAdapter::FRAME:
      // Need to be able to initialize the GUI at least once
      if (!init_)
      {
        // ea will not have valid width/height here, use graphics context's value
        const osg::GraphicsContext* gc = stats_->getCamera()->getGraphicsContext();
        if (gc && gc->getTraits())
          fixCameraSize_(gc->getTraits()->width, gc->getTraits()->height);
      }
      break;
    default:
      break;
    }
    return false;
  }

  /** Changes the overall scale for the stats.  1.0 is default size, 2.0 is twice as big */
  void setScale(double scale)
  {
    if (scale == sizeMultiplier_ || scale <= 0.0)
      return;
    sizeMultiplier_ = scale;
    const osg::GraphicsContext* gc = stats_->getCamera()->getGraphicsContext();
    if (gc && gc->getTraits())
      fixCameraSize_(gc->getTraits()->width, gc->getTraits()->height);
  }

private:
  /** Given a canvas width/height, adjusts camera matrices on stats handler to have consistent text size */
  void fixCameraSize_(int windowWidth, int windowHeight)
  {
    // an idealized screen ratio for good screen text sizing
    static const double SCREEN_RATIO = 1024.0 / 768.0;
    // StatsHandler's expected viewport width and height
    static const double STATS_WIDTH = 1024;
    static const double STATS_HEIGHT = 1024;

    osg::Camera* camera = stats_->getCamera();
    if (camera->getGraphicsContext() == NULL)
      return;
    camera->setProjectionMatrixAsOrtho2D(0, windowWidth / sizeMultiplier_,
      STATS_HEIGHT - windowHeight * SCREEN_RATIO / sizeMultiplier_, STATS_HEIGHT);

    // Need to set the viewport and view matrix at least once
    if (!init_)
    {
      camera->setViewport(0, 0, windowWidth, windowHeight);
      camera->setViewMatrixAsLookAt(
        osg::Vec3d(0, 0, 100),
        osg::Vec3d(0, 0, 0),
        osg::Vec3d(0, 1, 0));

      // Won't need to set again
      init_ = true;
    }
  }

  osgViewer::StatsHandler* stats_;
  bool init_;
  double sizeMultiplier_;
};

}

#endif /* SIMUTIL_STATSSIZEFIXER_H */
