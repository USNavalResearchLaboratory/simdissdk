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
 * not receive a LICENSE.txt with this code, email simdis@enews.nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_GOG_GOGNODEINTERFACE_H
#define SIMVIS_GOG_GOGNODEINTERFACE_H

#include <string>
#include <vector>
#include <osg/ref_ptr>
#include "simCore/Calc/Coordinate.h"
#include "simCore/Calc/Units.h"
#include "simCore/GOG/GogShape.h"
#include "simData/DataTypes.h"
#include "simVis/Utils.h"
#include "simVis/GOG/GOGNode.h"
#include "simVis/GOG/Utils.h"

namespace osgEarth{
  class GeoPoint;
  class FeatureNode;
  class GeometryNode;
  class GeoPositionNode;
  class ImageOverlay;
  class LabelNode;
  class LocalGeometryNode;
  class PlaceNode;
}

//----------------------------------------------------------------------------
namespace simVis { namespace GOG {

// enum describing the depth buffer override state
enum DepthBufferOverride
{
  DEPTHBUFFER_FORCE_OFF,
  DEPTHBUFFER_FORCE_ON,
  DEPTHBUFFER_IGNORE_OVERRIDE
};

/// enum describing Overlay tessellation style
enum TessellationStyle
{
  TESSELLATE_NONE,
  TESSELLATE_RHUMBLINE,
  TESSELLATE_GREAT_CIRCLE_PROJECTION
};

/// enum describing the altitude mode
enum AltitudeMode
{
  ALTITUDE_NONE,
  ALTITUDE_GROUND_RELATIVE,
  ALTITUDE_GROUND_CLAMPED,
  ALTITUDE_EXTRUDE
};

/**
* Base class to wrap the osg::Node that draws an overlay shape. This class and its implementations provide
* efficient access to the overlay attributes. The GogNodeInterface is a base class that manages the
* generic attributes common to all the overlay nodes. For attributes specific to a particular node type
* (FeatureNode, LocalGeometryNode, LabelNode), specific implementations re-implement the GogNodeInterface
*
* The osgEarth::Style object is where most common attributes exist. The GogNodeInterface
* provides an interface for getStyle_() and setStyle_(), and each implementation uses these to provide access to
* their respective Style object.
*/
class SDKVIS_EXPORT GogNodeInterface
{
public:
  /** Listener that gets alerted when the GogNodeInterface changes */
  class GogNodeListener
  {
  public:
    virtual ~GogNodeListener() {}
    virtual void drawChanged(const GogNodeInterface* nodeChanged) = 0;
  };
  typedef std::shared_ptr<GogNodeListener> GogNodeListenerPtr;

  /**
  * Constructor
  * @param OverlayNode  the Overlay node
  * @param metaData  the meta data returned by the Parser
  */
  GogNodeInterface(osg::Node* OverlayNode, const simVis::GOG::GogMetaData& metaData);

  /** Destructor */
  virtual ~GogNodeInterface() {}

  /** Get a pointer to the shape object, may be NULL */
  const simCore::GOG::GogShape* shapeObject() const;
  /** Set the GogShape object, which updates the node's style */
  void setShapeObject(simCore::GOG::GogShapePtr shape);

  /** Font to use if not defined in annotation block */
  void setDefaultFont(const std::string& fontName);
  /** Text Size to use if not defined in annotation block */
  void setDefaultTextSize(int textSize);
  /** Text Color to use if not defined in annotation block */
  void setDefaultTextColor(const osg::Vec4f& textColor);

  /** Store the current style as a default style. Revert to this style at any time using revertToDefaultStyle(). */
  void storeDefaultStyle();
  /** Revert to the default style set by storeDefaultStyle(). */
  void revertToDefaultStyle();

  /** Apply a ParsedShape object to the GOG's style */
  virtual void applyToStyle(const ParsedShape& parsedShape, const UnitsState& units);

  /**
  * Get the altitude mode of the Overlay, returns false if the Overlay does not support an altitude mode
  * @param altMode  returns current valid altitude mode
  * @return 0 if this Overlay supports altitude mode, non-zero otherwise
  */
  virtual int getAltitudeMode(AltitudeMode& altMode) const;

  /**
  * Get the altitude offset of the Overlay in meters, returns false if the Overlay does not support an altitude offset
  * @param altOffset  set to the altitude offset of the Overlay in meters
  * @return 0 if this Overlay supports altitude offset, non-zero otherwise
  */
  virtual int getAltOffset(double& altOffset) const;

  /**
  * Get the depth buffer state of the Overlay, returns false if the Overlay does not support depth buffer.
  * Note that actual depth buffer state may be overridden by the depth buffer override, and not consistent
  * with the value returned by this method
  * @param depthBuffer  set to true if the Overlay has a depth buffer
  * @return 0 if this Overlay supports depth buffer, non-zero otherwise
  */
  virtual int getDepthBuffer(bool& depthBuffer) const;

  /**
  * Get the depth buffer override state
  * @param state  will be set to the current depth buffer override state
  * @return 0 if this Overlay supports depth buffer, non-zero otherwise
  */
  int getDepthBufferOverrideState(DepthBufferOverride& state);

  /**
  * Get the draw state of the Overlay
  * @return true if the draw flag is set
  */
  virtual bool getDraw() const;

  /**
  * Get the extruded state of the Overlay, returns false if the Overlay does not support extrusion.
  * @return extruded  set to true if the Overlay is extruded
  * @return 0 if this Overlay can be extruded, non-zero otherwise
  */
  virtual int getExtruded(bool& extruded) const;

  /**
  * Get the extrude height of the Overlay, returns nonzero if the Overlay does not have an extrusion height.
  * @param[out] extrudeHeightM the extrude height in meters
  * @return 0 if this Overlay has an extrude height set, non-zero otherwise
  */
  virtual int getExtrudedHeight(double& extrudeHeightM) const;

  /**
  * Get the fill state of the Overlay, returns false if the Overlay does not support being filled
  * @param state  set to true if the Overlay is filled
  * @param color  set to the fill color in osg format (r,g,b,a) between 0.0 - 1.0
  * @return 0 if this Overlay can be filled, non-zero otherwise
  */
  virtual int getFilledState(bool& state, osg::Vec4f& color) const;

  /**
  * Get the font of the Overlay, returns false if the Overlay does not support a font
  * @param fontFile  set to the Overlay font file name
  * @param fontSize  set to the Overlay's font size
  * @param fontColor  set to the Overlay's font color in osg format (r,g,b,a) between 0.0 - 1.0
  * @return 0 if this Overlay supports font, non-zero otherwise
  */
  virtual int getFont(std::string& fontFile, int& fontSize, osg::Vec4f& fontColor) const;

  /**
   * Get the declutter priority for text in the overlay, returning non-zero if Overlay does not have a declutter priority.
   * @param[out] priority Priority values, where -1 is no-declutter, and lower non-negative values are less likely to be occluded than higher values.
   * @return 0 if this Overlay has a priority set, non-zero otherwise
   */
  virtual int getDeclutterPriority(int& priority) const;

  /**
   * Get the line attributes of the Overlay
   * @param outlineState  set to true if there is a line symbol
   * @param color  set to the color of the line in osg format (r,g,b,a) between 0.0 - 1.0
   * @param lineStyle  set to the line style (solid, dashed, dotted)
   * @param lineWidth  set to the lineWidth (0-10)
   * @return 0 if this Overlay has lines, non-zero otherwise
   */
  virtual int getLineState(bool& outlineState, osg::Vec4f& color, Utils::LineStyle& lineStyle, int& lineWidth) const;

  /**
  * Get the point size of the Overlay, returns false if this is Overlay doesn't have points
  * @param pointSize  set to the Overlay point size
  * @return 0 if this Overlay has points, non-zero otherwise
  */
  virtual int getPointSize(int& pointSize) const;

  /**
  * Get the position of the shape on the map, in osgEarth::GeoPoint position format, lon/lat/alt degrees/degrees/meters. Will use the provided
  * referencePosition param as the reference position if it is valid, otherwise will use the node's internal position if it has one.
  * @param position to be filled in
  * @param referencePosition will use this for ref position if it is valid, when applying local offset
  * @return 0 if position was found, non-zero otherwise
  */
  virtual int getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition = nullptr) const = 0;

  /**
  * Get the reference position of the shape on the map, in osgEarth::GeoPoint position format, lon/lat/alt degrees/degrees/meters.
  * Return 0 if the node has a reference position and it was filled in, non-zero if no reference position exists for the node.
  * @param referencePosition to be filled in
  * @return 0 if reference position was found, non-zero otherwise
  */
  virtual int getReferencePosition(osg::Vec3d& referencePosition) const;

  /**
  * Get the tessellation style of the Overlay, returns false if the Overlay doesn't support tessellation
  * @param style  set to the Overlay's tessellation style
  * @return 0 if this Overlay supports tessellation, non-zero otherwise
  */
  virtual int getTessellation(TessellationStyle& style) const;

  /**
  * Get the text outline style and color of the Overlay, returns false if the Overlay does not support a text outline
  * @param outlineColor  set to the Overlay's outline color in osg format (r,g,b,a) between 0.0 - 1.0
  * @param outlineThickness set to the Overlay's outline thickness (thick, thin, or none)
  * @return 0 if this Overlay has an outline style, non-zero otherwise
  */
  virtual int getTextOutline(osg::Vec4f& outlineColor, simData::TextOutline& outlineThickness) const;

  /**
  * Retrieves the opacity value assigned to the GOG object.  By default, this is 1.0 (opaque).
  * @param opacity Opacity for the overlay; 0.0 for transparent, 1.0 for fully opaque.
  * @return 0 if this overlay has an opacity value, non-zero otherwise.
  */
  int getOpacity(float& opacity) const;

  /**
  * Get the underlying osg::Node that represents the Overlay in the scene graph
  * @return the Overlay osg::Node
  */
  osg::Node* osgNode() const;

  /**
   * Serialize the GogNodeInterface into an ostream
   * @param gogOutputStream  ostream to hold serialized Overlay
   */
  virtual void serializeToStream(std::ostream& gogOutputStream);

  /** Update the altitude mode of the Overlay */
  virtual void setAltitudeMode(AltitudeMode altMode);

  /**
  * Update altitude offset of Overlay, in meters, to prevent Z-buffer fighting
  * @param altOffsetMeters Altitude in meters
  */
  virtual void setAltOffset(double altOffsetMeters);

  /**
  * Update whether to enable depth buffer, only applies to Overlays that have no altitude
  * @param depthBuffer  if true OverlayNode will enable depth buffer
  */
  virtual void setDepthBuffer(bool depthBuffer);

  /**
  * Set the depth buffer override state. This will force the depth buffer on or off, or return control to the manual setting
  * Note that the return from getDepthBuffer does not reflect the override state
  * @param state Desired state for depth buffer override
  */
  void setDepthBufferOverrideState(DepthBufferOverride state);

  /**
  * Update draw state of the Overlay
  * @param draw true if Overlay should be drawn
  * @return 0 if successful, non-zero if update failed or no change required
  */
  virtual int setDrawState(bool draw);

  /** update whether to extend the object towards the Earth's surface */
  virtual void setExtrude(bool extrude);

  /**
  * Update the extrude height of the Overlay
  * @param extrudeHeightM the extrude height in meters; value of 0.0 resets to default extrude (to ground)
  */
  virtual void setExtrudedHeight(double extrudeHeightM);

  /**
  * Update the filled state of the Overlay adding a new PolygonSymbol and applying current fill color if state is true.
  * Sets fill color alpha to 0 if false. Note that if current fill color's alpha value is 0, no change to fill state will occur
  * @param state  if true Overlay will be filled by color (if current color alpha is > 0)
  */
  virtual void setFilledState(bool state);

  /**
  * Update the filled state of the Overlay, only applicable to closed shapes
  * @param color  Fill color in osg format (r,g,b,a) between 0.0 - 1.0
  */
  virtual void setFillColor(const osg::Vec4f& color);

  /**
  * Update font of Overlay, only applies to Annotations
  * @param fontName file name of the font file
  * @param fontSize point size of the font
  * @param color of the font
  */
  virtual void setFont(const std::string& fontName, int fontSize, const osg::Vec4f& color);

  /** Sets the priority for decluttering priority; -1 for no declutter, lower non-negative values for higher priority */
  virtual void setDeclutterPriority(int priority);

  /** Update the line color of the Overlay in osg format (r,g,b,a) between 0.0 - 1.0 */
  virtual void setLineColor(const osg::Vec4f& color);

  /**
  * Update the outline state of the Overlay adding a new LineSymbol and applying current line color if state is true.
  * Sets line color alpha to 0 if false. Note that if current line color's alpha value is 0, no change to outline state will occur
  * @param outlineState  if true Overlay will display an outline with the current line color
  */
  virtual void setOutlineState(bool outlineState);

  /**
  * Update the line style of the Overlay
  * @param style  updates the line stipple
  */
  virtual void setLineStyle(Utils::LineStyle style);

  /** Update the line width of the Overlay */
  virtual void setLineWidth(int lineWidth);

  /** Update the point size of the Overlay */
  virtual void setPointSize(int pointSize);

  /** Update tessellation of the Overlay */
  virtual void setTessellation(TessellationStyle style);

  /**
  * Update text outline of Overlay, only applies to Annotations
  * @param outlineColor  color of the outline
  * @param outlineThickness thickness fo the outline
  */
  virtual void setTextOutline(const osg::Vec4f& outlineColor, simData::TextOutline outlineThickness);

  /**
   * Sets an overriding opacity value on the GOG object.  This opacity is multiplied against any other
   * opacities for foreground color, outline, image alpha value, etc. and is efficient for applying
   * an overarching transparency level for the GOG.
   * @param opacity Alpha value from 0.0 (transparent) to 1.0 (opaque)
   */
  virtual void setOpacity(float opacity);

  /**
   * Indicates if the shape has a properly formatted AltitudeSymbol or an ExtrusionSymbol based
   * on the altitude mode combinations applied in the setAltitudeMode  method. Will return true
   * if the AltitudeSymbol and ExtrusionSymbol attributes match a known state, false otherwise.
   * Note that other combinations are not necessarily wrong, but their behavior is not well defined
   * and may not display as expected.
   * @return true if altitude mode is valid, false otherwise
   */
  bool hasValidAltitudeMode() const;

  /** Set backface culling based on shape state */
  void applyBackfaceCulling();

  /**
   * Get the shape's original load format, which is defined in the meta data
   * @return load format enum
   */
  simVis::GOG::LoadFormat loadFormat() const;

  /** Sets the units that were specified for "xy" commands (default to YARDS) */
  void setRangeUnits(const simCore::Units& rangeUnits);
  /** Retrieves the units for "xy" commands (default to YARDS) */
  const simCore::Units& rangeUnits() const;

  /**
   * Get the shape type of this Overlay, which is defined in the meta data
   * @return shape type enum
   */
  simVis::GOG::GogShape shape() const;

  /// Set flag indicating if shape's yaw component is locked to a reference orientation
  void setFollowYaw(bool follow);
  /// Set flag indicating if shape's pitch component is locked to a reference orientation
  void setFolloPitch(bool follow);
  /// Set flag indicating if shape's roll component is locked to a reference orientation
  void setFollowRoll(bool follow);

  /// Set the yaw angular offset from a reference orientation in radians
  void setYawOffset(double offsetRad);
  /// Set the pitch angular offset from a reference orientation in radians
  void setPitchOffset(double offsetRad);
  /// Set the roll angular offset from a reference orientation in radians
  void setRollOffset(double offsetRad);


  /** Return the starting line number from the source GOG file*/
  size_t lineNumber() const;

  /** Add the specified listener */
  void addGogNodeListener(GogNodeListenerPtr listener);
  /** Remove the specified listener */
  void removeGogNodeListener(GogNodeListenerPtr listener);

protected: // methods

  /**
  * Child classes implement this to serialize their geometry into a GOG format string
  */
  virtual void serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const = 0;

  /** Serialize the keyword to the specified output stream */
  virtual void serializeKeyword_(std::ostream& gogOutputStream) const;

  /**
  * Child classes can override this to return a reference to their Style object
  * @return the current Style object reference
  */
  virtual const osgEarth::Style& getStyle_() const;

  /**
  * Child classes implement this to apply the Style object to their node
  * @param style  the new Style to apply
  */
  virtual void setStyle_(const osgEarth::Style& style) = 0;

  /** Called whenever altitude needs to be adjusted, based on altitude offset and altitude mode */
  virtual void adjustAltitude_() = 0;

  /** Initialize altitude symbol to clamp per vertex to work properly with altitude modes. Will have no effect unless called after style_ object has been assigned */
  void initializeAltitudeSymbol_();

  /** Initialize fill color. Will have no effect unless called after style_ object has been assigned */
  void initializeFillColor_();

  /** Initialize line color. Will have no effect unless called after style_ object has been assigned */
  void initializeLineColor_();

  /** Begin batched updates to the Style, subsequent sets will not apply the style to the GOG */
  void beginStyleUpdates_();

  /** End batched updates to the Style, subsequent sets will apply the style to the GOG */
  void endStyleUpdates_();

  /**
  * Return the current status of GOG style updating
  * @return whether updates to GOG are deferred in a begin/end batch update
  */
  bool deferringStyleUpdates_() const;

  /** Notify listeners that the draw state has changed */
  void fireDrawChanged_() const;

  /**
  * Helper method for setting the altitude on a geo position node. Determines the altitude based on original altitude, altitude mode,
  * and altitude offset. The altitudeAdjustment param is for offsets specific to the node (not the shape altitude offset), is typically 0.
  */
  void setGeoPositionAltitude_(osgEarth::GeoPositionNode& node, double altitudeAdjustment);

  /** Helper method for initializing hasMapNode_ and altitude_ from the specified GeoPosition node. */
  void initializeFromGeoPositionNode_(const osgEarth::GeoPositionNode& node);

protected: // data
  osg::ref_ptr<osg::Node> osgNode_;  ///< reference to the basic osg::Node. Keep in ref_ptr so this instance will hold on the memory even if it's removed from the scene
  simVis::GOG::GogMetaData metaData_;  ///< meta data returned by the Parser
  simCore::GOG::GogShapePtr shape_; ///< parsed shape object
  bool filled_; ///< cache fill state of the shape for quick reference
  bool extruded_; ///< cache extruded state of the shape for quick reference
  bool outlined_; ///< cache outlined state of the shape for quick reference
  bool depthBuffer_; ///< cache depth buffer state so that we can locally override it without losing state
  DepthBufferOverride depthBufferOverride_; ///< determines the state of depth buffer override
  double extrudedHeight_; ///< cache the extruded height value, in meters
  osgEarth::Style style_; ///< style for this node
  osgEarth::Style defaultStyle_; ///< stored default style for this node
  bool hasDefaultStyle_; ///< tracks if a default style has been stored
  osg::Vec4f fillColor_;  ///< fill color; saved because setFilledState can be destructive on shape's fill color
  osg::Vec4f lineColor_; ///< line color needs to be stored in case LineSymbol is turned off
  double altitude_; ///< cache the original altitude, in meters
  double altOffset_; ///< cache the altitude offset, in meters
  AltitudeMode altMode_; ///< cache the altitude mode
  bool hasMapNode_; ///< indicates if this shape has a map node

private:

  /**
  * Check the metadata string for the specified flag keyword. If it is present, erase the keyword from the
  * metadata and return true, otherwise return false
  * @param flag  keyword to match in metadata
  * @param metaData  value to search
  */
  bool getMetaDataFlag_(const std::string& flag, std::string& metaData);

  /** Check which shapes are valid to use with the GogNodeInterface base class fill methods */
  bool isFillable_(simVis::GOG::GogShape shape) const;

  /** Check which shapes are valid to use with the GogNodeInterface base class line methods */
  bool isLined_(simVis::GOG::GogShape shape) const;

  /** Check if this shape should only be filled when extruded */
  bool fillOnlyWhenExtruded_(simVis::GOG::GogShape shape) const;

  /** indicates whether updates to GOG are deferred in a begin/end batch update */
  bool deferringStyleUpdate_;

  /** Font to use if not defined in annotation block */
  std::string defaultFont_;
  /** Text Size to use if not defined in annotation block */
  int defaultTextSize_;
  /** Text Color to use if not defined in annotation block */
  osg::Vec4f defaultTextColor_;

  /** Range units specified by user in file */
  simCore::Units rangeUnits_;

  /** cache the opacity value, for efficiency */
  float opacity_;

  /** listeners for updates */
  std::vector<GogNodeListenerPtr> listeners_;
};

/// Shared ptr wrapper for the GogNodeInterface object
typedef std::shared_ptr<GogNodeInterface> GogNodeInterfacePtr;

/**
 * Implementation of GogNodeInterface for AnnotationNodes, which are a base class of all GOG node types.
 * Used when less generic GogNodeInterface can't be found for a given osg node
 */
class SDKVIS_EXPORT AnnotationNodeInterface : public GogNodeInterface
{
public:
  AnnotationNodeInterface(osgEarth::AnnotationNode* annotationNode, const simVis::GOG::GogMetaData& metaData);
  virtual ~AnnotationNodeInterface();
  virtual int getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition = nullptr) const;

protected:
  virtual void adjustAltitude_();
  virtual void serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const;
  virtual void setStyle_(const osgEarth::Style& style);

private:
  osg::observer_ptr<osgEarth::AnnotationNode> annotationNode_;
};

/**
* Implementation of GogNodeInterface for FeatureNodes, which represent absolute line type Overlays
*/
class SDKVIS_EXPORT FeatureNodeInterface : public GogNodeInterface
{
public:
  /** Constructor */
  FeatureNodeInterface(osgEarth::FeatureNode* featureNode, const simVis::GOG::GogMetaData& metaData);
  /** Constructor with parent group node */
  FeatureNodeInterface(osg::Group* node, osgEarth::FeatureNode* featureNode, const simVis::GOG::GogMetaData& metaData);
  virtual ~FeatureNodeInterface() {}
  virtual int getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition = nullptr) const;
  virtual int getTessellation(TessellationStyle& style) const;
  virtual void setAltitudeMode(AltitudeMode altMode);
  virtual void setAltOffset(double altOffsetMeters);
  virtual void setExtrude(bool extrude);
  virtual void setTessellation(TessellationStyle style);

protected:
  virtual void adjustAltitude_();
  virtual void serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const;
  virtual void setStyle_(const osgEarth::Style& style);

  osg::observer_ptr<osgEarth::FeatureNode> featureNode_;
  /// cache the original altitude values, to apply altitude offset dynamically
  std::vector<double> originalAltitude_;

private:
  void init_();
};

/**
* Implementation of GogNodeInterface for LocalGeometryNodes, which represent both absolute Overlays with relative coordinates and attached Overlays
*/
class SDKVIS_EXPORT LocalGeometryNodeInterface : public GogNodeInterface
{
public:
  /** Constructor */
  LocalGeometryNodeInterface(osgEarth::LocalGeometryNode* localNode, const simVis::GOG::GogMetaData& metaData);
  virtual ~LocalGeometryNodeInterface() {}
  virtual int getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition = nullptr) const;
  /// override the get reference position
  virtual int getReferencePosition(osg::Vec3d& referencePosition) const;

protected:
  virtual void adjustAltitude_();
  virtual void serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const;
  virtual void setStyle_(const osgEarth::Style& style);

  /** LocalGeometryNode that this interface represents */
  osg::observer_ptr<osgEarth::LocalGeometryNode> localNode_;
};

/**
 * Implementation of GogNodeInterface for LabelNode and PlaceNode, which represent Annotation Overlays.
 * PlaceNodes have the same relevant settings, and are used in KML.
 */
class SDKVIS_EXPORT LabelNodeInterface : public GogNodeInterface
{
public:
  /** Constructor */
  LabelNodeInterface(osgEarth::GeoPositionNode* labelNode, const simVis::GOG::GogMetaData& metaData);
  virtual ~LabelNodeInterface() {}
  virtual int getFont(std::string& fontFile, int& fontSize, osg::Vec4f& fontColor) const;
  virtual int getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition = nullptr) const;
  virtual int getTextOutline(osg::Vec4f& outlineColor, simData::TextOutline& outlineThickness) const;
  virtual int getDeclutterPriority(int& priority) const;
  virtual void setFont(const std::string& fontName, int fontSize, const osg::Vec4f& color);
  virtual void setTextOutline(const osg::Vec4f& outlineColor, simData::TextOutline outlineThickness);
  virtual void setDeclutterPriority(int priority);

protected:
  virtual void adjustAltitude_();
  virtual void serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const;
  virtual void serializeKeyword_(std::ostream& gogOutputStream) const;
  virtual void setStyle_(const osgEarth::Style& style);

private:
  // Stores either a LabelNode or PlaceNode
  osg::observer_ptr<osgEarth::GeoPositionNode> labelNode_;
  // Cache the outline color for case when outline is turned off
  osg::Vec4f outlineColor_;
  // Cache outline thickness
  simData::TextOutline outlineThickness_;
};

/**
* Implementation of GogNodeInterface for a cylinder, which is a special case. A cylinder is represented by three LocalGeometryNodes.
* The first node is the sides, and cannot have a LineSymbol in its style, and the other nodes are the top and bottom caps, which cannot have
* an extrusion symbol
*/
class SDKVIS_EXPORT CylinderNodeInterface : public GogNodeInterface
{
public:
  /** Constructor */
  CylinderNodeInterface(osg::Group* groupNode, osgEarth::LocalGeometryNode* sideNode, osgEarth::LocalGeometryNode* topCapNode, osgEarth::LocalGeometryNode* bottomCapNode, const simVis::GOG::GogMetaData& metaData);
  virtual ~CylinderNodeInterface();
  virtual int getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition = nullptr) const;
  virtual void setAltitudeMode(AltitudeMode altMode);

protected:
  virtual void adjustAltitude_();
  virtual void serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const;
  virtual void setStyle_(const osgEarth::Style& style);

private:

  osg::observer_ptr<osgEarth::LocalGeometryNode> sideNode_; ///< draws the sides
  osg::observer_ptr<osgEarth::LocalGeometryNode> topCapNode_; ///< draws the top cap
  osg::observer_ptr<osgEarth::LocalGeometryNode> bottomCapNode_; ///< draws the bottom cap
  /// height of the cylinder in meters
  float height_;
};

/**
* Implementation of GogNodeInterface for an arc, which is a special case. An arc is represented by two LocalGeometryNodes.
* The first node is the arc itself, and cannot have a PolygonSymbol in its style, and the second nodes is the filled node,
* which cannot have a LineSymbol, and is only visible when fill state is on
*/
class SDKVIS_EXPORT ArcNodeInterface : public GogNodeInterface
{
public:
  /** Constructor */
  ArcNodeInterface(osg::Group* groupNode, osgEarth::LocalGeometryNode* shapeNode, osgEarth::LocalGeometryNode* fillNode, const simVis::GOG::GogMetaData& metatData);
  virtual ~ArcNodeInterface() {}
  virtual int getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition = nullptr) const;
  virtual void setFilledState(bool state);

protected:
  virtual void adjustAltitude_();
  virtual void serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const;
  virtual void setStyle_(const osgEarth::Style& style);

private:
  osg::observer_ptr<osgEarth::LocalGeometryNode> shapeNode_; ///< draws the arc
  osg::observer_ptr<osgEarth::LocalGeometryNode> fillNode_; ///< draws the filled pie shape
};

/**
* Implementation of GogNodeInterface for a spherical object (sphere, hemisphere, ellipsoid), which is a special case. The spherical shapes
* color is applied by a color array internally in the osg::Geometry object, not through the Style, so their color setters have to be implemented
* specially. These shape's color is defined by either line color, or if filled is set to true, then by fill color
*/
class SDKVIS_EXPORT SphericalNodeInterface : public LocalGeometryNodeInterface
{
public:
  /** Constructor */
  SphericalNodeInterface(osgEarth::LocalGeometryNode* localNode, const simVis::GOG::GogMetaData& metaData);
  virtual ~SphericalNodeInterface() {}
  virtual int getFilledState(bool& state, osg::Vec4f& color) const;
  virtual int getLineState(bool& outlineState, osg::Vec4f& color, Utils::LineStyle& lineStyle, int& lineWidth) const;
  virtual void setFillColor(const osg::Vec4f& color);
  virtual void setFilledState(bool state);
  virtual void setLineColor(const osg::Vec4f& color);

protected:
  /** Override setStyle to fix the depth */
  virtual void setStyle_(const osgEarth::Style& style);

private:
  void setColor_(const osg::Vec4f& color);
};

/**
* Implementation of GogNodeInterface for a cone object. The cone's color is applied by a color
* array internally in the osg::Geometry object, not through the Style, so the color setter has
* to be implemented specially. The cone's color is defined by the fill color.
*/
class SDKVIS_EXPORT ConeNodeInterface : public LocalGeometryNodeInterface
{
public:
  ConeNodeInterface(osgEarth::LocalGeometryNode* localNode, const simVis::GOG::GogMetaData& metaData);
  virtual ~ConeNodeInterface() {}
  virtual void setFillColor(const osg::Vec4f& color);
};

/**
* Implementation of GogNodeInterface for an image overlay object, which is equivalent to a KML ground overlay.
* Basic implementation since editing KML objects is limited.
*/
class SDKVIS_EXPORT ImageOverlayInterface : public GogNodeInterface
{
public:
  ImageOverlayInterface(osgEarth::ImageOverlay* imageNode, const simVis::GOG::GogMetaData& metaData);
  virtual ~ImageOverlayInterface() {}
  virtual int getPosition(osg::Vec3d& position, osgEarth::GeoPoint* referencePosition = nullptr) const;

  /** Override opacity, since the override color approach doesn't work */
  virtual void setOpacity(float opacity) override;

protected:
  virtual void adjustAltitude_();
  virtual void serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const;
  virtual void setStyle_(const osgEarth::Style& style);

private:
  osg::observer_ptr<osgEarth::ImageOverlay> imageNode_;
};

/**
* Implementation of GogNodeInterface for a LatLonAltBox, which requires multiple FeatureNodes to display correctly
*/
class SDKVIS_EXPORT LatLonAltBoxInterface : public FeatureNodeInterface
{
public:
  LatLonAltBoxInterface(osg::Group* node, osgEarth::FeatureNode* topNode, osgEarth::FeatureNode* bottomNode, const simVis::GOG::GogMetaData& metaData);
  virtual ~LatLonAltBoxInterface() {}
  // handle the special case for MultiGeometry
  virtual void setAltOffset(double altOffsetMeters);

protected:
  virtual void serializeGeometry_(bool relativeShape, std::ostream& gogOutputStream) const;
  virtual void serializeKeyword_(std::ostream& gogOutputStream) const;
  virtual void setStyle_(const osgEarth::Style& style);

private:
  /// store the altitudes of the specified node into the specified altitudes vector, handles iterating through MultiGeometry
  void initAltitudes_(osgEarth::FeatureNode& node, std::vector<double>& altitudes) const;
  /// apply the altitude offsets, handles iterating through MultiGeometry
  void applyAltOffsets_(osgEarth::FeatureNode& node, const std::vector<double>& altitudes) const;

  osg::observer_ptr<osgEarth::FeatureNode> bottomNode_;
  /// original altitudes of the bottom node
  std::vector<double> bottomAltitude_;
};


}}

#endif /* SIMVIS_GOG_GOGNODEINTERFACE_H */
