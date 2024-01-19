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
#ifndef SIMVIS_PICKER_H
#define SIMVIS_PICKER_H

#include <vector>
#include "osg/ref_ptr"
#include "osg/observer_ptr"
#include "osg/Referenced"
#include "osgEarth/RTTPicker"
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

/** Highlight shader for making selected entities or objects glow. */
class SDKVIS_EXPORT PickerHighlightShader : public osg::Referenced
{
public:
  /** Declares uniform variables for using and setting the highlight values. */
  PickerHighlightShader(osg::StateSet* stateset);

  /** Installs the highlighting shader.  Without this, the highlighting will not apply to graphics. If a shaderPrefix is specified, will load a new non-default shader. */
  static void installShaderProgram(osg::StateSet* intoStateSet, bool defaultEnabled, const std::string& shaderPrefix = "");
  /** Installs the highlighting shader (non-static version).  Applies to stateset supplied at construction. */
  void installShaderProgram(bool defaultEnabled);

  /** Returns true if the shader is current enabled on the stateset */
  bool isEnabled() const;

  /** Turns on the highlighting feature.  Only functional if shader installed with installHighlightShader(). */
  void setEnabled(bool enabled);
  /** Changes the Tag ID that is currently enabled.  Corresponds to the ID from osgEarth::Registry::objectIndex. */
  void setId(unsigned int tagId);

  /** Setting a prefix allows for reuse of this code in contexts other than typical picking. Typically this is empty string. */
  void setShaderPrefix(const std::string& shaderPrefix);

protected:
  /** osg::Referenced-derivced, so protected destructor. */
  virtual ~PickerHighlightShader();

private:
  osg::observer_ptr<osg::StateSet> stateset_;
  /// Prefix for adding non-default shaders to the picker
  std::string shaderPrefix_;
};

/** Abstract base class for pickers in SIMDIS SDK */
class SDKVIS_EXPORT Picker : public osg::Referenced
{
public:
  struct PickedEntity
  {
  public:
    unsigned int id = 0;
    osg::observer_ptr<osg::Referenced> picked;

    bool operator==(const PickedEntity& rhs) const
    {
      return id == rhs.id && picked.get() == rhs.picked.get();
    }
    bool operator!=(const PickedEntity& rhs) const
    {
      return !operator==(rhs);
    }
  };

  /** Retrieves the ID of the picked entity, as per osgEarth Registry's object index. 0 when none. */
  unsigned int pickedId() const;

  /** Object that corresponds to the first picked ID. */
  virtual osg::Referenced* picked() const;

  /** Attempts to convert picked() into an osg::Node. */
  osg::Node* pickedNode() const;
  /** Attempts to convert picked() into a simVis::EntityNode. */
  simVis::EntityNode* pickedEntity() const;
  /** Attempts to locate the simVis::PlatformNode associated with picked(). */
  simVis::PlatformNode* pickedPlatform() const;

  /** Callback that indicates when the picked object is changed. */
  class Callback : public osg::Referenced
  {
  public:
    /** First picked object has changed. */
    virtual void pickChanged(unsigned int pickedId, osg::Referenced* picked) = 0;
    /** List of picked objects has changed. */
    virtual void pickChanged(const std::vector<PickedEntity>& picked) = 0;

  protected:
    virtual ~Callback() {}
  };

  /** Adds a callback that will trigger when the selected object changes. */
  void addCallback(Callback* callback);
  /** Removes a previously added callback. */
  void removeCallback(Callback* callback);

protected:
  /** This is intended to be an abstract class, so protected.  Accepts stateset for ID uniform. */
  Picker(osg::StateSet* stateSet);
  /** Derived from osg::Referenced */
  virtual ~Picker();

  /** Packages the single entity into a vector and calls the other setPicked_ */
  void setPicked_(unsigned int pickedId, osg::Referenced* picked);
  /** Fires off all pick callbacks. */
  void setPicked_(const std::vector<PickedEntity>& picked);

private:
  /** Vector of osgEarth::ObjectId and corresponding osg::Referenced that were picked during the last pick */
  std::vector<PickedEntity> pickedEntities_;

  /** Allows us to change the picked ID on the scenario manager */
  osg::ref_ptr<PickerHighlightShader> shaderValues_;

  /** List of all callbacks registered. */
  std::vector<osg::ref_ptr<Callback> > callbacks_;
};

/** Picker that uses an intersection test to pick at most once per frame */
class SDKVIS_EXPORT IntersectPicker : public Picker
{
public:
  IntersectPicker(simVis::ViewManager* viewManager, simVis::ScenarioManager* scenarioManager);

protected:
  /** Derived from osg::Referenced, protect destructor */
  virtual ~IntersectPicker();

private:
  /** Performs the actual intersection pick. */
  void pickThisFrame_();

  class IntersectEventHandler;

  /** View that the mouse was last over from a MOVE/DRAG */
  osg::observer_ptr<simVis::View> lastMouseView_;
  /** Mouse X coordinate in OSG coordinates */
  double mx_;
  /** Mouse Y coordinate in OSG coordinates */
  double my_;
  /** Flags whether the pick has already occurred for this frame */
  bool pickedThisFrame_;

  /** Callback that is used to add the picker to SDK views. */
  osg::ref_ptr<AddEventHandlerToViews> addHandlerToViews_;
  /** Event handler for Intersection picking */
  osg::ref_ptr<osgGA::GUIEventHandler> guiEventHandler_;

  /** Retain a pointer to the view manager to clean up callbacks. */
  osg::observer_ptr<simVis::ViewManager> viewManager_;
  /** Pointer to the scenario manager */
  osg::observer_ptr<simVis::ScenarioManager> scenario_;
};

/** Facade to RTTPicker that ties in View Manager and other components. */
class SDKVIS_EXPORT RTTPicker : public Picker
{
public:
  /** Constructs a new picker and associates with all views in the view manager. */
  RTTPicker(simVis::ViewManager* viewManager, simVis::ScenarioManager* scenarioManager, int cameraSize);

  /** Sets the picked ID.  Note that this may be overridden on next mouse movement. */
  void setPickedId(unsigned int id);

  /** Creates a texture representing the RTT display for the given view.  Use this for debugging purposes. */
  osg::Texture2D* getOrCreateTexture(simVis::View* fromView);
  /** Creates a texture for fromView and displays it in intoView.  Use this for debugging purposes. */
  void setUpViewWithDebugTexture(osgViewer::View* intoView, simVis::View* fromView);

  /** Retrieve the underlying RTT Picker. */
  osgEarth::Util::RTTPicker* rttPicker() const;

protected:
  /** osg::Referenced-derived, so protect destructor */
  virtual ~RTTPicker();

private:
  /** Underlying render-to-texture picker. */
  osg::ref_ptr<osgEarth::Util::RTTPicker> rttPicker_;
  /** Callback that is used to add the picker to SDK views. */
  osg::ref_ptr<simVis::ViewManager::Callback> viewManagerCallback_;

  /** Retain a pointer to the view manager to clean up callbacks. */
  osg::observer_ptr<simVis::ViewManager> viewManager_;
};

}

#endif /* SIMVIS_PICKER_H */
