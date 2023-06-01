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
#ifndef SIMVIS_PROJECTOR_H
#define SIMVIS_PROJECTOR_H
#include <memory>
#include "simData/DataTypes.h"
#include "simVis/Constants.h"
#include "simVis/Entity.h"

#include "osgEarth/MapNodeObserver"

namespace osg { class Texture2D; }

#if OSGEARTH_SOVERSION >= 135
// Adapter for the deprecated EllipsoidIntersector class
#include "osgEarth/Ellipsoid"
namespace osgEarth {
  namespace Util {
    class EllipsoidIntersector {
    public:
      EllipsoidIntersector(const osgEarth::Ellipsoid& ellipsoid) : ellipsoid_(ellipsoid) { }
      bool intersectLine(const osg::Vec3d& p0, const osg::Vec3d& p1, osg::Vec3d& out) const {
        return ellipsoid_.intersectGeocentricLine(p0, p1, out);
      }
      osgEarth::Ellipsoid ellipsoid_;
    };
  }
}
#else
#include "osgEarth/EllipsoidIntersector"
#endif

namespace simVis
{
class EntityLabelNode;
struct LocatorCallback;
class LocatorNode;

/** Projector video interface on the MediaPlayer2 side */
class ProjectorTexture : public osg::Referenced
{
public:

  /** Set image to an underlying object like a texture */
  virtual void setImage(osg::Image *image) = 0;

protected:

  virtual ~ProjectorTexture() { }
};

/** Projector-video interface on the ProjectorNode side */
class SDKVIS_EXPORT ProjectorTextureImpl : public ProjectorTexture
{
public:
  /**
  * Construct a new ProjectorTextureImpl, used for holding a texture that will hold the video image.
  */
  ProjectorTextureImpl();

  /** Set image to the texture */
  virtual void setImage(osg::Image *image);

  /** Set texture from projector node */
  void setTexture(osg::Texture2D *texture);

protected:

  virtual ~ProjectorTextureImpl() { }

private:
  osg::observer_ptr<osg::Texture2D> texture_;
};

/** EntityNode that represents a projector */
class SDKVIS_EXPORT ProjectorNode : public EntityNode, public osgEarth::MapNodeObserver
{
public:
  /**
  * Construct a new node that projects an image or video on to the terrain.
  */
  ProjectorNode(const simData::ProjectorProperties& props,
    simVis::Locator* hostLocator,
    const simVis::EntityNode* host = nullptr);

  /**
  * Gets the last known properties of this object
  * @return Object properties
  */
  const simData::ProjectorProperties& getProperties() const;

  /// set preferences
  void setPrefs(const simData::ProjectorPrefs& prefs);

  /// get preferences
  const simData::ProjectorPrefs& getPrefs() const;

  /// get vertical field of view in degrees
  double getVFOV() const;

  /// Return texture
  osg::Texture2D* getTexture() const;

  /// Return shadow map
  osg::Texture2D* getShadowMap() const;

  /// Load image into texture
  void setImage(osg::Image *image);

  /// Gets the texture generation matrix
  const osg::Matrixd& getTexGenMatrix() const;

  /// Gets the shadow map generation matrix
  const osg::Matrixd& getShadowMapMatrix() const;

  /**
   * Gets a pointer to the last data store update, or nullptr if
   * none have been applied.
   */
  const simData::ProjectorUpdate* getLastUpdateFromDS() const;

  /// Add projector uniforms to the given StateSet
  void applyToStateSet(osg::StateSet* stateSet) const;

  /// Remove projector uniforms from the given StateSet
  void removeFromStateSet(osg::StateSet* stateSet) const;

  /// Copy uniform values to an array stateset
  void copyUniformsTo(osg::StateSet* stateSet, unsigned size, unsigned index) const;

  /// Set the calculator that can calculate the projector's ellipsoid intersection
  void setCalculator(std::shared_ptr<osgEarth::Util::EllipsoidIntersector> calculator);

  /** Configure an entity to accept the texture projected by this projector.  An entity can accept up to 4 projectors.  Returns 0 on success. */
  int addProjectionToNode(osg::Node* entity, osg::Node* attachmentPoint);

  /** Remove the setup configured by addProjectionToNode.  Returns 0 on success. */
  int removeProjectionFromNode(osg::Node* entity);

  /**
  * Get the traversal mask for this node type
  * @return a traversal mask
  */
  static unsigned int getMask() { return simVis::DISPLAY_MASK_PROJECTOR; }

  /**
  * Updates the projection uniforms. This called automatically when the locator moves; you
  * do not need to call it directly.
  */
  void syncWithLocator();

  /** Traverse the node during visitor pattern */
  virtual void traverse(osg::NodeVisitor& nv) override;

  /// Override from MapNodeObserver
  virtual void setMapNode(osgEarth::MapNode*) override;
  virtual osgEarth::MapNode* getMapNode() override;

public: // EntityNode interface
  /**
  * Whether the entity is active within the scenario at the current time.
  * The entity is considered active if it has a valid position update for the
  * current scenario time, and has not received a command to turn off
  * @return true if active; false if not
  */
  virtual bool isActive() const override;

  /**
  * Whether this entity is visible.
  */
  virtual bool isVisible() const override;

  /**
  * Returns the entity name. Can be used to get the actual name always or the
  * actual/alias depending on the commonprefs.usealias flag.
  * @param nameType  enum option to always return real/alias name or name based on
  *            the commonprefs usealias flag.
  * @param allowBlankAlias If true DISPLAY_NAME will return blank if usealias is true and alias is blank
  * @return    actual/alias entity name string
  */
  virtual const std::string getEntityName(EntityNode::NameType nameType, bool allowBlankAlias = false) const override;

  /// Returns the pop up text based on the label content callback, update and preference
  virtual std::string popupText() const override;
  /// Returns the hook text based on the label content callback, update and preference
  virtual std::string hookText() const override;
  /// Returns the legend text based on the label content callback, update and preference
  virtual std::string legendText() const override;

  /** Retrieve the object index tag for picking this projector. */
  virtual unsigned int objectIndexTag() const override;

  /** @copydoc EntityNode::getPosition() */
  virtual int getPosition(simCore::Vec3* out_position, simCore::CoordinateSystem coordsys = simCore::COORD_SYS_ECEF) const override;

  /** @copydoc EntityNode::getPositionOrientation() */
  virtual int getPositionOrientation(simCore::Vec3* out_position, simCore::Vec3* out_orientation,
    simCore::CoordinateSystem coordsys = simCore::COORD_SYS_ECEF) const override;

  /**
  * Get the object ID of the beam rendered by this node
  * @return object ID
  */
  virtual simData::ObjectId getId() const override;

  /** Get the projector's host's ID */
  virtual bool getHostId(simData::ObjectId& out_hostId) const override;

  /**
  * Updates the entity based on the bound data store.
  * @param updateSlice  Data store update slice (could be nullptr)
  * @param force true to force the update to be applied; false allows entity to use its own internal logic to decide whether the update should be applied
  * @return true if update applied, false if not
  */
  virtual bool updateFromDataStore(const simData::DataSliceBase* updateSlice, bool force = false) override;

  /**
  * Flushes all the entity's data point visualization.
  */
  virtual void flush() override;

  /**
  * Returns a range value (meters) used for visualization.  Will return zero for platforms and projectors.
  */
  virtual double range() const override;

  /** Return the proper library name */
  virtual const char* libraryName() const override { return "simVis"; }
  /** Return the class name */
  virtual const char* className() const override { return "ProjectorNode"; }

protected:
  /// osg::Referenced-derived; destructor body needs to be in the .cpp
  virtual ~ProjectorNode();

private:
  /** Copy constructor, not implemented or available. */
  ProjectorNode(const ProjectorNode&);

  void init_();

  /// Read video file
  bool readVideoFile_(const std::string& filename);
  /// Read raster file
  bool readRasterFile_(const std::string& filename);
  /// Load new file
  void loadRequestedFile_(const std::string& newFilename);

  /// Update label
  void updateLabel_(const simData::ProjectorPrefs& prefs);
  /// Update override color
  void updateOverrideColor_(const simData::ProjectorPrefs& prefs);
  /// Calculate the vertical FOV (deg) and aspect ratio
  int calculatePerspectiveComponents_(double& vFovDeg, double& aspectRatio) const;

  /// returns true if the user changes a preference that requires the projector manager to update the rendering state
  bool isStateDirty_() const;
  /// clears the dirty flag
  void resetStateDirty_();

  /// get horizontal field of view in degrees; returns -1 if unset, which means that aspect ratio should be used
  double getHFOVDegrees_() const;

  simData::ProjectorProperties  lastProps_;
  simData::ProjectorPrefs       lastPrefs_;
  simData::ProjectorUpdate      lastUpdate_;
  osg::observer_ptr<const EntityNode> host_;
  osg::observer_ptr<Locator>    hostLocator_; ///< locator that tracks the projector origin
  osg::ref_ptr<LocatorCallback> locatorCallback_; ///< notifies when projector host (& origin) has moved
  osg::ref_ptr<LocatorNode>     projectorLocatorNode_; ///< locator node that tracks the projector/ellipsoid intersection
  osg::ref_ptr<EntityLabelNode> label_;
  unsigned int                  objectIndexTag_ = 0;
  bool                          hasLastUpdate_ = false;
  bool                          hasLastPrefs_ = false;
  mutable bool                  stateDirty_ = false;

  osg::Matrixd texGenMatrix_;
  osg::ref_ptr<osg::Texture2D> texture_;
  osg::Matrixd shadowMapMatrix_;
  osg::ref_ptr<osg::Texture2D> shadowMap_;
  osg::ref_ptr<osg::Camera> shadowCam_;
  osg::ref_ptr<osg::Uniform> shadowToPrimaryMatrix_;
  osg::Matrixd viewMat_;

  // Projector video interface for transferring video image.
  osg::ref_ptr<ProjectorTextureImpl> projectorTextureImpl_;
  // Playlist node that holds the video images that will be read into
  // the texture; loaded from "osgDB::readNodeFile".
  osg::ref_ptr<osg::Referenced> imageProvider_;
  osg::ref_ptr<osg::MatrixTransform> graphics_;
  osg::ref_ptr<osg::Uniform> projectorActive_;
  osg::ref_ptr<osg::Uniform> projectorAlpha_;
  osg::ref_ptr<osg::Uniform> texProjPosUniform_;
  osg::ref_ptr<osg::Uniform> texProjDirUniform_;
  osg::ref_ptr<osg::Uniform> texProjSamplerUniform_;
  osg::ref_ptr<osg::Uniform> useColorOverrideUniform_;
  osg::ref_ptr<osg::Uniform> colorOverrideUniform_;
  osg::ref_ptr<osg::Uniform> projectorMaxRangeSquaredUniform_;
  osg::ref_ptr<osg::Uniform> doubleSidedUniform_;

  // Keep track of the nodes projected onto so the projections can be removed when projector is deleted.
  // The key is the entity the value is the attachment point
  std::map<osg::observer_ptr<osg::Node>, osg::observer_ptr<osg::Node> > projectedNodes_;

  std::shared_ptr<osgEarth::Util::EllipsoidIntersector> calculator_;



  friend class ProjectorManager;
};

} //namespace simVis

#endif // SIMVIS_PROJECTOR_H

