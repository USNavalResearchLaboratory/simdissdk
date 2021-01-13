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
#ifndef SIMVIS_PROJECTOR_H
#define SIMVIS_PROJECTOR_H
#include <memory>
#include "simData/DataTypes.h"
#include "simVis/Constants.h"
#include "simVis/Entity.h"

namespace osg { class Texture2D; }
namespace osgEarth { namespace Util { class EllipsoidIntersector; } }
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
class SDKVIS_EXPORT ProjectorNode : public EntityNode
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
  const simData::ProjectorProperties& getProperties() const { return lastProps_; }

  /// set preferences
  void setPrefs(const simData::ProjectorPrefs& prefs);

  /// get preferences
  const simData::ProjectorPrefs& getPrefs() const { return lastPrefs_; }

  /// get field of view in degrees
  double getVFOV() const;

  /// Return texture
  osg::Texture2D* getTexture() const;

  /// Load image into texture
  void setImage(osg::Image *image);

  /// Gets the texture generation matrix
  const osg::Matrixd& getTexGenMatrix() const { return texGenMatrix_; }

  /**
   * Gets a pointer to the last data store update, or nullptr if
   * none have been applied.
   */
  const simData::ProjectorUpdate* getLastUpdateFromDS() const;

  /// Add projector uniforms to the given StateSet
  void addUniforms(osg::StateSet* stateSet) const;
  /// Remove projector uniforms from the given StateSet
  void removeUniforms(osg::StateSet* stateSet) const;

  /// Set the calculator that can calculate the projector's ellipsoid intersection
  void setCalculator(std::shared_ptr<osgEarth::Util::EllipsoidIntersector> calculator);

  /** Configure a node to accept the texture projected by this projector.  A node can accept only one projector.  Returns 0 on success. */
  int addProjectionToNode(osg::Node* node);

  /** Remove the setup configured by addProjectionToNode.  Returns 0 on success. */
  int removeProjectionFromNode(osg::Node* node);

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

  /** This entity type is, at this time, unpickable. */
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

  void getMatrices_(
    osg::Matrixd& out_projection,
    osg::Matrixd& out_locator,
    osg::Matrixd& out_modelView) const;

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


  simData::ProjectorProperties  lastProps_;
  simData::ProjectorPrefs       lastPrefs_;
  simData::ProjectorUpdate      lastUpdate_;
  osg::observer_ptr<const EntityNode> host_;
  osg::observer_ptr<Locator>    hostLocator_; ///< locator that tracks the projector origin
  osg::ref_ptr<LocatorCallback> locatorCallback_; ///< notifies when projector host (& origin) has moved
  osg::ref_ptr<LocatorNode>     projectorLocatorNode_; ///< locator node that tracks the projector/ellipsoid intersection
  osg::ref_ptr<EntityLabelNode> label_;
  bool                          hasLastUpdate_;
  bool                          hasLastPrefs_;

  osg::Matrixd texGenMatrix_;
  osg::ref_ptr<osg::Texture2D> texture_;
  // Projector video interface for transferring video image.
  osg::ref_ptr<ProjectorTextureImpl> projectorTextureImpl_;
  // Playlist node that holds the video images that will be read into
  // the texture; loaded from "osgDB::readNodeFile".
  osg::ref_ptr<osg::Referenced> imageProvider_;
  osg::MatrixTransform* graphics_;
  osg::ref_ptr<osg::Uniform> projectorActive_;
  osg::ref_ptr<osg::Uniform> projectorAlpha_;
  osg::ref_ptr<osg::Uniform> texProjPosUniform_;
  osg::ref_ptr<osg::Uniform> texProjDirUniform_;
  osg::ref_ptr<osg::Uniform> texProjSamplerUniform_;
  osg::ref_ptr<osg::Uniform> useColorOverrideUniform_;
  osg::ref_ptr<osg::Uniform> colorOverrideUniform_;

  osg::ref_ptr<osg::NodeCallback> projectOnNodeCallback_;
  std::shared_ptr<osgEarth::Util::EllipsoidIntersector> calculator_;
};

} //namespace simVis

#endif // SIMVIS_PROJECTOR_H

