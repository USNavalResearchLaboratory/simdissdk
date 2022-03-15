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
#ifndef SIMVIS_PLATFORM_MODEL_H
#define SIMVIS_PLATFORM_MODEL_H

#include "osg/Vec2f"
#include "simCore/Common/Common.h"
#include "simCore/EM/RadarCrossSection.h"
#include "simVis/Constants.h"
#include "simData/DataTypes.h"
#include "simVis/LocatorNode.h"
#include "simVis/BillboardAutoTransform.h"

namespace simVis
{

class EntityLabelNode;
class DynamicScaleTransform;
class OverrideColor;
class RCSNode;

/**
 * Scene graph node that represents the platform model and all its attachments.
 */
class SDKVIS_EXPORT PlatformModelNode : public LocatorNode
{
public:
  static const unsigned int TRAVERSAL_MASK;

  /** Interface for activity callbacks. */
  class Callback : public osg::Referenced
  {
  public:
    /// Events that this callback processes
    enum EventType
    {
      /** Platform bounds changed. */
      BOUNDS_CHANGED,
    };

    /// Provide this method to receive an event
    virtual void operator()(simVis::PlatformModelNode* modelNode, EventType eventType) = 0;

  protected:
    /// osg::Referenced-derived
    virtual ~Callback() {}
  };

public:
  /**
   * Constructs a new platform model node.
   * @param locator Parent locator from which to inherit position/orientation
   */
  PlatformModelNode(Locator* locator = nullptr);

  /**
  * Gets the bounds of the 3D model, possible scaled
  * @return 3D bounding box in object space
  */
  const osg::BoundingBox& getScaledIconBounds() const { return bounds_; }

  /**
  * Gets the actual bounds of the 3D model
  * @return 3D bounding box in object space
  */
  const osg::BoundingBox& getUnscaledIconBounds() const { return unscaledBounds_; }

  /**
  * Sets a dynamically scaled platform model/icon to automatically rotate to face
  * the screen. This will ONLY take effect if dynamic scaling is enabled.
  * @param value True or False
  */
  void setRotateToScreen(bool value);
  /** Retrieves flag for whether to rotate to face viewer */
  bool getRotateToScreen() const { return autoRotate_; }

  /** addChild(), but adds to the dynamic-scaled portion of model; returns true on success */
  bool addScaledChild(osg::Node* node);
  /** Removes previously added scaled child */
  bool removeScaledChild(osg::Node* node);

  /** Returns true if the model is a 2D image, false otherwise */
  bool isImageModel() const;

  /** Retrieves the traversal mask for platform models */
  static unsigned int getMask() { return TRAVERSAL_MASK; }

  /** Retrieves the node for the entity's label */
  EntityLabelNode* label() const { return label_.get(); }

  /** Sets the RCS data */
  void setRcsData(simCore::RadarCrossSectionPtr rcsData);

  /** Retrieves the bounding box radius */
  double getBoundsRadius() const { return unscaledBounds_.radius(); }

  /** Retrieves the offsetXform node */
  osg::Node* offsetNode() const;

  /** Retrieves the platform's tag ID in the registry object index, for picking. */
  unsigned int objectIndexTag() const;

  /** Adds a callback to the platform */
  void addCallback(Callback* callback);
  /** Removes a callback from the platform */
  void removeCallback(Callback* callback);

  /**
   * Override the platform icon with the given node.  This node will be replaced on the
   * next setPrefs() call if the prefs.icon() changes.  This is useful for temporarily
   * overriding a platform's model with a custom texture or icon.  This is a relatively
   * low level method that is also used internally by the model loading code.
   * @param node Node to use for model
   * @param isImage True if image icon transformations should be valid
   */
  void setModel(osg::Node* node, bool isImage);

public: // PlatformAttachment interface

  /** Sets the properties for the platform */
  void setProperties(const simData::PlatformProperties& props);

  /** Sets the prefs for the platform */
  void setPrefs(const simData::PlatformPrefs& prefs);

public: // LocatorNode interface

  /** Override to keep image icons rotated toward eye */
  virtual void syncWithLocator(); //override

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simVis"; }

  /** Return the class name */
  virtual const char* className() const { return "PlatformModelNode"; }

protected:
  virtual ~PlatformModelNode();

private:
  PlatformModelNode(const PlatformModelNode& rhs); // not implemented
  PlatformModelNode& operator=(const PlatformModelNode& rhs); // not implemented

  /// Callback to the Model Cache that will set our platform model on asynchronous load
  class SetModelCallback;
  /// Member pointer to the most recent set-model-callback
  osg::ref_ptr<SetModelCallback> lastSetModelCallback_;

  simData::PlatformProperties        lastProps_;
  simData::PlatformPrefs             lastPrefs_;
  simData::LabelPrefs                lastLabelPrefs_;
  osg::BoundingBox                   bounds_;
  osg::BoundingBox                   unscaledBounds_;
  /// Points to a (likely) shared instance of a single 3-D Model
  osg::ref_ptr<osg::Node>            model_;
  /// True if the URI of the model points to a 2-D image that needs billboarding
  bool                               isImageModel_;
  osg::ref_ptr<RCSNode>              rcs_;
  osg::ref_ptr<EntityLabelNode>      label_;
  osg::ref_ptr<simVis::DynamicScaleTransform> dynamicXform_;
  osg::ref_ptr<simVis::BillboardAutoTransform> imageIconXform_;
  osg::ref_ptr<osg::MatrixTransform> offsetXform_;
  osg::Vec2f                         imageOriginalSize_;
  bool                               autoRotate_;
  bool                               lastPrefsValid_;
  osg::ref_ptr<OverrideColor>        overrideColor_;
  osg::ref_ptr<osg::Uniform>         brightnessUniform_;
  osg::ref_ptr<osg::Group>           alphaVolumeGroup_;
  unsigned int                       objectIndexTag_;

  /** Potentially null pointer to the fast-path icon render node. */
  osg::ref_ptr<osg::Node> fastPathIcon_;
  /** If the model is externally provided, then turn off fast path */
  bool allowFastPath_;

  /** Contains list of platform callbacks */
  std::vector<osg::ref_ptr<Callback> > callbacks_;
  /** Fires off callbacks with the given event type */
  void fireCallbacks_(Callback::EventType eventType);

  /** Print a warning for invalid offsets with image icons to catch an error in old SIMDIS rules. */
  void warnOnInvalidOffsets_(const simData::PlatformPrefs& prefs, bool modelChanged) const;

  /// May changes the model based on prefs and returns true if the model was changed
  bool updateModel_(const simData::PlatformPrefs& prefs);
  enum ModelUpdate
  {
    NO_UPDATE,  ///< No further update required, model is correctly set
    FORCE_UPDATE, ///< Continue processing to force an update of the model
    CHECK_FOR_UPDATE,  ///< Check preferences to see if the model should be updated
  };
  /// Called by updateModel_() to try to update the fast-path icon; returns if the model was updated or if more processing is needed
  ModelUpdate updateFastPathModel_(const simData::PlatformPrefs& prefs);
  /// Updates the orientation offset  based on prefs or if force is set true; returns true if changed
  bool updateOffsets_(const simData::PlatformPrefs& prefs, bool force);
  /// Updates the scale based on pref; returns true if changed
  bool updateScale_(const simData::PlatformPrefs& prefs);
  /// Updates the XYZ scale based on pref; returns true when scale changes
  bool updateScaleXyz_(const simData::PlatformPrefs& prefs);
  /// Updates the dynamic scale based on pref; returns true when dynamic scale changes
  bool updateDynamicScale_(const simData::PlatformPrefs& prefs);
  /// Updates the icon rotation based on pref or if force is set to true when the model has changed
  void updateImageIconRotation_(const simData::PlatformPrefs& prefs, bool force);
  /// Updates the depth buffer when the nodepthicons has changed or if force is set to true when the model has changed
  void updateImageDepth_(const simData::PlatformPrefs& prefs, bool force) const;
  /// Updates the RCS based on pref
  void updateRCS_(const simData::PlatformPrefs& prefs);
  /// Updates the bounds
  void updateBounds_();
  /// Updates the stippling based on prefs
  void updateStippling_(const simData::PlatformPrefs& prefs);
  /// Updates the culling based on prefs
  void updateCulling_(const simData::PlatformPrefs& prefs);
  /// Updates the polygon mode based on prefs
  void updatePolygonMode_(const simData::PlatformPrefs& prefs);
  /// Updates the lighting based on prefs or if force is set to true when the model has changed
  void updateLighting_(const simData::PlatformPrefs& prefs, bool force);
  /// Updates the override color based on prefs
  void updateOverrideColor_(const simData::PlatformPrefs& prefs);
  /// Updates the alpha volume based on prefs
  void updateAlphaVolume_(const simData::PlatformPrefs& prefs);
  /// Updates the DOF Transform animation based on prefs
  void updateDofTransform_(const simData::PlatformPrefs& prefs, bool force) const;
  /// Internal version of set setModel();
  void setModel_(osg::Node* node, bool isImage);
};

} // namespace simVis

#endif //SIMVIS_PLATFORM_MODEL_H
