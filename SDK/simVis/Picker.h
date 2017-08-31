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
#ifndef SIMVIS_PICKER_H
#define SIMVIS_PICKER_H

#include <vector>
#include "osg/ref_ptr"
#include "osg/observer_ptr"
#include "osg/Referenced"
#include "osgEarthUtil/RTTPicker"
#include "simVis/ViewManager.h"

namespace osg {
  class Uniform;
  class Node;
}
namespace osgEarth { namespace Util { class RTTPicker; } }

namespace simVis {

class ScenarioManager;
class ViewManager;
class EntityNode;
class PlatformNode;

/** Facade to RTTPicker that ties in View Manager and other components. */
class SDKVIS_EXPORT Picker : public osg::Referenced
{
public:
  /** Constructs a new picker and associates with all views in the view manager. */
  Picker(simVis::ViewManager* viewManager, simVis::ScenarioManager* scenarioManager, int cameraSize);

  /** Callback that indicates when the picked object is changed. */
  class Callback : public osg::Referenced
  {
  public:
    /** Picked object has changed. */
    virtual void pickChanged(unsigned int pickedId, osg::Referenced* picked) = 0;
  protected:
    virtual ~Callback() {}
  };

  /** Sets the picked ID.  Note that this may be overridden on next mouse movement. */
  void setPickedId(unsigned int id);

  /** Retrieves the ID of the picked entity, as per osgEarth Registry's object index. 0 when none. */
  unsigned int pickedId() const;
  /** Object that corresponds to the picked ID. */
  osg::Referenced* picked() const;
  /** Attempts to convert picked() into an osg::Node. */
  osg::Node* pickedNode() const;
  /** Attempts to convert picked() into a simVis::EntityNode. */
  simVis::EntityNode* pickedEntity() const;
  /** Attempts to locate the simVis::PlatformNode associated with the pick. */
  simVis::PlatformNode* pickedPlatform() const;

  /** Installs the highlighting shader.  Without this, the highlighting will not apply to graphics. */
  void installHighlightShader();
  /** Turns on the highlighting feature.  Only functional if shader installed with installHighlightShader(). */
  void setHighlightEnabled(bool enabled);
  /** Returns true if the highlight is enabled.  Only functional if shader installed with installHighlightShader(). */
  bool isHighlightEnabled() const;

  /** Creates a texture representing the RTT display for the given view.  Use this for debugging purposes. */
  osg::Texture2D* getOrCreateTexture(simVis::View* fromView);
  /** Creates a texture for fromView and displays it in intoView.  Use this for debugging purposes. */
  void setUpViewWithDebugTexture(osgViewer::View* intoView, simVis::View* fromView);

  /** Retrieve the underlying RTT Picker. */
  osgEarth::Util::RTTPicker* rttPicker() const;

  /** Adds a callback that will trigger when the selected object changes. */
  void addCallback(Callback* callback);
  /** Removes a previously added callback. */
  void removeCallback(Callback* callback);

protected:
  /** osg::Referenced-derived, so protect destructor */
  virtual ~Picker();

private:
  /** Fires off all pick callbacks. */
  void firePickChanged_(unsigned int pickedId, osg::Referenced* picked);

  /** Last osgEarth::ObjectId that was picked. */
  unsigned int pickedId_;
  /** osg::Referenced from the Registry's object index that corresponds to pickedId_. */
  osg::observer_ptr<osg::Referenced> picked_;

  /** List of all callbacks registered. */
  std::vector<osg::ref_ptr<Callback> > callbacks_;

  /** Contains pickedId_ for the highlighting shader. */
  osg::ref_ptr<osg::Uniform> highlightIdUniform_;
  /** Flags whether the highlighting shader should run. */
  osg::ref_ptr<osg::Uniform> highlightEnabledUniform_;

  /** Underlying render-to-texture picker. */
  osg::ref_ptr<osgEarth::Util::RTTPicker> rttPicker_;
  /** Callback that is used to add the picker to SDK views. */
  osg::ref_ptr<simVis::ViewManager::Callback> viewManagerCallback_;

  /** Retain a pointer to the view manager to clean up callbacks. */
  osg::observer_ptr<simVis::ViewManager> viewManager_;
  /** Scenario Manager is the root of items that can be picked in the scene. */
  osg::observer_ptr<osg::Node> scenarioManager_;
};

}

#endif /* SIMVIS_PICKER_H */
