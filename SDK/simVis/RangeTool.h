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
#ifndef SIMVIS_RANGETOOL_H
#define SIMVIS_RANGETOOL_H

#include <memory>
#include <sstream>

#include "osg/Group"
#include "osg/MatrixTransform"
#include "osgEarth/Revisioning"
#include "osgEarth/Units"

#include "simCore/Common/Common.h"
#include "simCore/Calc/Math.h"
#include "simCore/Calc/Calculations.h"
#include "simData/ObjectId.h"
#include "simVis/Measurement.h"
#include "simVis/Platform.h"
#include "simVis/RangeToolState.h"
#include "simVis/Scenario.h"
#include "simVis/Tool.h"
#include "simVis/Utils.h"

namespace simCore { class DatumConvert; }
namespace simRF { class RFPropagationFacade; }

namespace simVis
{

  /**
  * RangeTool is a subsystem for drawing range measurements.
  *
  * Range visualization represent properties relating two platforms
  * in the scene- like distance, angles, etc.
  */
  class SDKVIS_EXPORT RangeTool : public ScenarioTool
  {
  public:
    /**
    * Describes how to draw a Graphic.
    */
    struct SDKVIS_EXPORT GraphicOptions
    {
      GraphicOptions();

      /**@name display settings
       *@{
       */
      osg::Vec4f     lineColor1_, lineColor2_;
      unsigned short lineStipple1_, lineStipple2_;
      unsigned int   lineWidth_;

      osg::Vec4f     pieColor_;
      unsigned int   pieSegments_;
      bool           usePercentOfSlantDistance_;
      float          pieRadiusPercent_;
      float          pieRadiusValue_;
      osgEarth::Units pieRadiusUnits_;

      bool           useDepthTest_;
      bool           showGraphics_;
      ///@}
    };

    /**
    * Describes how to draw the label for a Measurement.
    */
    struct SDKVIS_EXPORT TextOptions
    {
      TextOptions();

      /// outline drawn around text
      enum OutlineType
      {
        OUTLINE_NONE,
        OUTLINE_THIN,
        OUTLINE_THICK
      };

      /// Defines what text to show
      enum ShowText
      {
        NONE,  ///< Show no text
        FULL,  ///< Show description with value and units
        VALUES_ONLY ////< Show only the value and units
      };

      /// Defines where to show the text
      enum TextLocation
      {
        PAIRING_LINE,  ///< Show all text on the pairing line between the entities
        ALL ///< Show all text at their default location
      };

      /**@name text settings
       *@{
       */
      bool         displayAssociationName_;
      bool         useScaleFont_;
      bool         dynamicScaleFont_;
      OutlineType  outlineType_;
      osg::Vec4f   outlineColor_;
      std::string  font_;
      float        fontSize_;
      float        scaleFontSize_;
      float        xOffset_;
      float        yOffset_;
      osg::Vec4f   color_;
      ShowText     showText_;
      TextLocation textLocation_;
      ///@}
    };

    /**
    * Entity state needed to do Range calculations
    */
    struct SDKVIS_EXPORT EntityState
    {
      simCore::Vec3 lla_;  ///< Lat, lon, alt in rad, rad, m
      simCore::Vec3 ypr_;  ///< Yaw, pitch, roll in rad, rad, rad
      simCore::Vec3 vel_;  ///< X, Y and Z velocities in m/s
      osg::ref_ptr<const simVis::EntityNode> node_; ///< The node of the entity
      simData::ObjectId hostId_;   ///< Unique ID of the host entity; for platforms and custom renderings hostId_ == id_
      osg::ref_ptr<const simVis::PlatformNode> platformHostNode_; ///< The node of the host platform; for platforms platformHostNode_ == node_
      simRF::RFPropagationFacade* rfPropagation_;  ///< If the entity is a beam this MAY BE set

      EntityState()
        : node_(NULL),
        hostId_(0),
        platformHostNode_(NULL),
        rfPropagation_(NULL)
      {
      }
    };


    /**
    * Base class for a Graphic Primitive. Included are a set of built-in primitives
    * (see below), but you can use this base class to create custom primitives
    * as well.
    */
    class SDKVIS_EXPORT Graphic : public osg::Referenced, public osgEarth::DirtyNotifier
    {
    public:
      /** Define the type of graphic */
      enum GraphicType
      {
        LINE = 0,       ///< uses all line fields of the GraphicOptions
        PIE_SLICE,      ///< uses all the pie fields of the GraphicOptions
        CUSTOM          ///< custom graphic shape
      };

    protected:
      /// protected constructor for internal use
      Graphic(const std::string& typeName, GraphicType graphicType)
        : typeName_(typeName), graphicType_(graphicType) { }

    public:

      /**
      * The type name of the graphic; must be unique
      * @return String type name
      */
      const std::string& typeName() const { return typeName_; }

      /**
      * Graphic options to use when drawing this primitive
      * NOTE: If you use the non-const accessor and change a property, call setDirty().
      * @return Graphic options structure
      */
      GraphicOptions& graphicOptions() { return options_; }
      /// const version
      const GraphicOptions& graphicOptions() const { return options_; }

      /**
      * Indicates the type of graphic options applicable to this graphic
      * @return GraphicType of the object
      */
      GraphicType graphicType() const { return graphicType_; }

      /**
      * Renders this primitive into OSG geometry.
      * @param geode Geode to which to render
      * @param state Context object
      */
      virtual void render(osg::Geode* geode, RangeToolState& state) = 0;

      /**
      * Gets a coordinate at which to place a text label for this primitive.
      * @param state context object
      * @return A local (tangent-plane) space coordinate
      */
      virtual osg::Vec3 labelPos(RangeToolState& state) = 0;

    protected:
      /// osg::Referenced-derived
      virtual ~Graphic() {}

    protected:
      GraphicOptions options_; ///< controls for drawing

      /**
      * Returns true if the type is Platform or Custom Rendering
      * @param type Object type
      * @return True if the type is Platform or Custom Rendering
      */
      bool hasPosition_(simData::ObjectType type) const;

    private:
      std::string typeName_;
      GraphicType graphicType_;
    };

    /// a vector of Graphic pointers
    typedef std::vector< osg::ref_ptr<Graphic> > GraphicVector;


    /**
     * Calculation is a visual representation of a collection of Graphics
     * with optional labeling.
     */
    class SDKVIS_EXPORT Calculation : public osg::Referenced, public osgEarth::DirtyNotifier
    {
    public:
      /** Define the type of angle */
      enum AngleType
      {
        AZIMUTH = 0,       ///< azimuth angle of the calculated objects
        ELEVATION,         ///< elevation angle of the calculated objects
        COMPOSITE          ///< composite of the azimuth and elevation angles
      };

      /// constructor with the name of the measurement
      Calculation(const std::string& name);

      /// name of the calculation being performed
      const std::string& name() const { return name_; }

      /// control drawing
      void setVisible(bool value);
      /// current drawn state
      bool visible() const { return visible_; }

      /// add a graphic
      void addGraphic(Graphic* graphic, bool useAsLabelGraphic = false);

      /** Accesses the graphics list. If you modify the list manually you must call setDirty() */
      GraphicVector& graphics() { return graphics_; }
      /// const version
      const GraphicVector& graphics() const { return graphics_; }

      /// set the label
      void setLabelGraphic(Graphic* graphic);
      /// get the label
      Graphic* labelGraphic() const { return labelGraphic_.get(); }

      /// set the label measurement
      void setLabelMeasurement(Measurement* measurement);
      /// get the label measurement
      Measurement* labelMeasurement() const { return labelMeasurement_.get(); }

      /// set the label units
      void setLabelUnits(const osgEarth::Units& units);
      /// get the label units
      const osgEarth::optional<osgEarth::Units>& labelUnits() const { return labelUnits_; }

      /// set the label precision
      void setLabelPrecision(unsigned int precision);
      /// get the label precision
      double labelPrecision() const { return labelPrecision_; }

      /** Text options - call setDirty() if you modify the object. */
      TextOptions& textOptions() { return textOptions_; }
      /// const version
      const TextOptions& textOptions() const { return textOptions_; }

      /// set the angle type
      void setAngleType(AngleType type);
      /// get the angle type
      AngleType angleType() const { return angleType_; }

      /// set the most recently calculated value
      void setLastValue(double value);

      /**
      * Returns the last calculated value.
      * @return Calculated value
      */
      double lastValue() const { return lastValue_; }

      /**
      * Returns the last calculated value converted to the specified units.
      * @return Calculated value in target units
      */
      double lastValue(const osgEarth::Units& outputUnits) const;

      /**
      * Returns true if the call to lastValue will return a valid value
      * @return True if the call to lastValue will return a valid value
      */
      bool valid() const { return valid_; }

      /**
      * Sets the state of the lastValue method
      * @param value The state of the lastValue
      */
      void setValid(bool value);

    protected:
      /// osg::Referenced-derived
      virtual ~Calculation() {}

    protected: // data
      std::string                 name_; ///< name of the measurement
      GraphicVector               graphics_; ///< displays the graphics
      osg::ref_ptr< Graphic >     labelGraphic_; ///< displays the label
      osg::ref_ptr< Measurement > labelMeasurement_; ///< class to make calculation
      osgEarth::optional<osgEarth::Units> labelUnits_; ///< units to display
      unsigned int                labelPrecision_; ///< number of decimal points to display
      TextOptions                 textOptions_; ///< control label appearance
      AngleType                   angleType_; ///< store angle type, used when setting color
      bool                        visible_; ///< enable draw
      bool                        valid_; ///< True if the lastValue_ holds a valid value.
      double                      lastValue_;  ///< The last calculated value
    };

    /// vector of Calculation pointers
    typedef std::vector< osg::ref_ptr<Calculation> > CalculationVector;

    /**
    * Associated two entities from the scenario, and draws one or more
    * calculations applied to those entities.
    */
    class SDKVIS_EXPORT Association : public osg::Referenced, public osgEarth::DirtyNotifier
    {
    public:
      /**
      * Constructs a new association between two entities.
      * @param id1 ID of first object
      * @param id2 ID of second object
      */
      Association(simData::ObjectId id1, simData::ObjectId id2);

      /**
      * Adds a calculation to this association.
      * @param calc Calculation to add
      */
      void add(Calculation* calc);

      /**
      * Removes a calculation from this association (by name)
      * @param calc Calculation to remove
      */
      void remove(Calculation* calc);

      /**
      * Accesses the set of calculations to draw for this association.
      */
      const CalculationVector& calculations() const { return calculations_; }

      /**
      * Gets the first object ID
      */
      simData::ObjectId getFirstObjectId() const { return id1_; }

      /**
      * Gets the second object ID
      */
      simData::ObjectId getSecondObjectId() const { return id2_; }

      /**
      * Whether the association is visible
      */
      bool isVisible() const { return visible_; }

      /**
      * Toggles the visibility of the association
      */
      void setVisible(bool value) { visible_ = value; }

      /**
      * Updates the range tool based on a new time stamp
      * (Called internally)
      * @return true if update processed normally, false if scenario is null or association was not valid
      */
      bool update(const ScenarioManager& scenario, const simCore::TimeStamp& timestamp);

      /**
      * Sets dirty flag and clears labels_ cache to force text color update
      */
      virtual void setDirty();

      /**
      * Gets the root node representing this association
      */
      osg::Node* getNode() const { return xform_.get(); }

    private:
      simData::ObjectId                  id1_, id2_;             // id's of the associated entities
      bool                               dirty_;                 // whether the scene geometry needs rebuilding
      bool                               visible_;               // whether to render the association
      osg::ref_ptr<osg::MatrixTransform> xform_;                 // local-to-world transform
      osg::Geode*                        geode_;                 // scene geometry
      osg::Group*                        labels_;                // label controls
      osg::observer_ptr<EntityNode>      obj1_obs_;              // cached pointer to first entity
      osg::observer_ptr<EntityNode>      obj2_obs_;              // cached pointer to second entity
      osgEarth::Revision                 obj1LocatorRev_;        // tracks whether entity 1 is up to date with scenario data
      osgEarth::Revision                 obj2LocatorRev_;        // tracks whether entity 2 is up to date with scenario data
      CalculationVector                  calculations_;          // calculations to render
      RangeToolState                     state_;                 // the calc state for this assoc
      osg::ref_ptr<Graphic>              labelPos_;              // Use the mid-point of the slant line for the text

    protected:
      /// osg::Referenced-derived
      virtual ~Association() {}

    private:
      // regenerates scene geometry
      void refresh_(
        EntityNode*      obj1,
        EntityNode*      obj2,
        const ScenarioManager& scenario,
        const simCore::TimeStamp& timeStamp);
    };

    /// vector of Association pointers
    typedef std::vector< osg::ref_ptr<Association> > AssociationVector;

  public:
    /** Constructs a new range tool. */
    RangeTool();

    /**
    * Adds a new association to the range tool.
    * @param obj1 first object in the new association to add
    * @param obj2 second object in the new association to add
    */
    Association* add(simData::ObjectId obj1, simData::ObjectId obj2);

    /**
    * Removes an association from the range tool.
    * @param assoc association to remove
    */
    void remove(Association* assoc);

    /**
    * Gets the associations currently active in the range tool
    * @return Vector of Association pointers
    */
    const AssociationVector& getAssociations() const { return associations_; }

    /**
     * Range Tool updates require a full timestamp, but do not use/require EntityVector.
     */
    void update(const ScenarioManager* scenario, const simCore::TimeStamp& timeStamp);

    /**
    * Gets the node representing the range tool's graphics.
    * @returns an OSG node
    */
    osg::Node* getNode() const { return root_.get(); }


  public: // ScenarioTool interface

    /** @see ScenarioTool::onInstall() */
    virtual void onInstall(const ScenarioManager& scenario);

    /** @see ScenarioTool::onUninstall() */
    virtual void onUninstall(const ScenarioManager& scenario);

    /**
    * Updates the range tool based on a new time stamp
    */
    virtual void onUpdate(const ScenarioManager& scenario, const simCore::TimeStamp& timeStamp, const EntityVector& updates);

  public:
    /// @copydoc osgEarth::setDirty()
    virtual void setDirty();

  protected:
    /// osg::Referenced-derived
    virtual ~RangeTool() {}

  private: // Private helper classes
    struct RefreshGroup : public osg::Group
    {
      osg::observer_ptr<RangeTool> tool_;
      RefreshGroup(RangeTool* tool) : tool_(tool) { }
      void traverse(osg::NodeVisitor& nv);
      void scheduleRefresh();
    };

  private:
    AssociationVector                  associations_;         // all active associations
    osg::ref_ptr<RefreshGroup>         root_;                 // scene graph container
    osg::observer_ptr<const ScenarioManager> lastScenario_;   // saves a scenario pointer

  public: // Helper Graphics classes
    /// a stippled line between two points
    class SDKVIS_EXPORT LineGraphic : public Graphic
    {
    protected:
      /// constructor with name of the measurement type
      LineGraphic(const std::string& typeName, GraphicType graphicType)
        : Graphic(typeName, graphicType)
      {
      }

      /** add our geometry to 'geode'
       * @param verts vertex array to use (shared reference)
       * @param mode primitive set type to use
       * @param geode root node to attach to
       * @param state control display settings
       */
      void createGeometry(
        osg::Vec3Array*    verts,
        GLenum             mode,
        osg::Geode*        geode,
        RangeToolState&             state);

      /// osg::Referenced-derived
      virtual ~LineGraphic() {}
    };

    /// a filled in arc
    class SDKVIS_EXPORT PieSliceGraphic : public Graphic
    {
    public:
      virtual osg::Vec3 labelPos(RangeToolState& state);

      /// PieSliceGraphics cache their measured value here
      virtual void setMeasuredValue(double value) { measuredValue_ = value; }

    protected:
      /// constructor with name of the measurement type
      PieSliceGraphic(const std::string& typeName)
        : Graphic(typeName, PIE_SLICE)
      {
      }

      /// add our geometry to 'geode'
      void createGeometry(
        const osg::Vec3&    originVec,
        osg::Vec3d    startVec,
        osg::Vec3d    endVec,
        double       angle,
        osg::Geode*  geode,
        RangeToolState&       state);

      /// osg::Referenced-derived
      virtual ~PieSliceGraphic() {}

      osgEarth::optional<osg::Vec3> labelPos_; ///< label position
      double measuredValue_;                  ///< value of calc's measurement
    };

  public: // Built-in Graphics

    /// Graphics
    class SDKVIS_EXPORT GroundLineGraphic : public LineGraphic
    {
    public:
      GroundLineGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);

    protected:
      /// osg::Referenced-derived
      virtual ~GroundLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT SlantLineGraphic : public LineGraphic
    {
    public:
      SlantLineGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~SlantLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeginAltitudeLineGraphic : public LineGraphic
    {
    public:
      BeginAltitudeLineGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeginAltitudeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT EndAltitudeLineGraphic : public LineGraphic
    {
    public:
      EndAltitudeLineGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~EndAltitudeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeginAltitudeLineToEndAltitudeGraphic : public LineGraphic
    {
    public:
      BeginAltitudeLineToEndAltitudeGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeginAltitudeLineToEndAltitudeGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT EndAltitudeLineToBeginAltitudeGraphic : public LineGraphic
    {
    public:
      EndAltitudeLineToBeginAltitudeGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~EndAltitudeLineToBeginAltitudeGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeginToEndLineAtBeginAltitudeGraphic : public LineGraphic
    {
    public:
      BeginToEndLineAtBeginAltitudeGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeginToEndLineAtBeginAltitudeGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeginToEndLineAtEndAltitudeGraphic : public LineGraphic
    {
    public:
      BeginToEndLineAtEndAltitudeGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeginToEndLineAtEndAltitudeGraphic() {}
    };

    /// Graphics
    class SDKVIS_EXPORT BeamGroundLineGraphic : public LineGraphic
    {
    public:
      BeamGroundLineGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamGroundLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeamSlantLineGraphic : public LineGraphic
    {
    public:
      BeamSlantLineGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamSlantLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeamBeginAltitudeLineGraphic : public LineGraphic
    {
    public:
      BeamBeginAltitudeLineGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamBeginAltitudeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeamEndAltitudeLineGraphic : public LineGraphic
    {
    public:
      BeamEndAltitudeLineGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamEndAltitudeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeamBeginAltitudeLineToEndAltitudeGraphic : public LineGraphic
    {
    public:
      BeamBeginAltitudeLineToEndAltitudeGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamBeginAltitudeLineToEndAltitudeGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeamEndAltitudeLineToBeginAltitudeGraphic : public LineGraphic
    {
    public:
      BeamEndAltitudeLineToBeginAltitudeGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamEndAltitudeLineToBeginAltitudeGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeamBeginToEndLineAtBeginAltitudeGraphic : public LineGraphic
    {
    public:
      BeamBeginToEndLineAtBeginAltitudeGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamBeginToEndLineAtBeginAltitudeGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeamBeginToEndLineAtEndAltitudeGraphic : public LineGraphic
    {
    public:
      BeamBeginToEndLineAtEndAltitudeGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamBeginToEndLineAtEndAltitudeGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT CrossRangeLineGraphic : public LineGraphic
    {
    public:
      CrossRangeLineGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~CrossRangeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT DownRangeLineGraphic : public LineGraphic
    {
    public:
      DownRangeLineGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~DownRangeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT VelAzimDownRangeLineGraphic : public LineGraphic
    {
    public:
      VelAzimDownRangeLineGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~VelAzimDownRangeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT VelAzimCrossRangeLineGraphic : public LineGraphic
    {
    public:
      VelAzimCrossRangeLineGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~VelAzimCrossRangeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT DownRangeCrossRangeDownLineGraphic : public LineGraphic
    {
    public:
      DownRangeCrossRangeDownLineGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
      osg::Vec3 labelPos(RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~DownRangeCrossRangeDownLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT TrueAzimuthPieSliceGraphic : public PieSliceGraphic
    {
    public:
      TrueAzimuthPieSliceGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~TrueAzimuthPieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT TrueElevationPieSliceGraphic : public PieSliceGraphic
    {
    public:
      TrueElevationPieSliceGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~TrueElevationPieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT TrueCompositeAnglePieSliceGraphic : public PieSliceGraphic
    {
    public:
      TrueCompositeAnglePieSliceGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~TrueCompositeAnglePieSliceGraphic() {}
    };

    struct SDKVIS_EXPORT MagneticAzimuthPieSliceGraphic : public PieSliceGraphic
    {
    public:
      MagneticAzimuthPieSliceGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~MagneticAzimuthPieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT RelOriAzimuthPieSliceGraphic : public PieSliceGraphic
    {
    public:
      RelOriAzimuthPieSliceGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~RelOriAzimuthPieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT RelOriElevationPieSliceGraphic : public PieSliceGraphic
    {
    public:
      RelOriElevationPieSliceGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~RelOriElevationPieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT RelOriCompositeAnglePieSliceGraphic : public PieSliceGraphic
    {
    public:
      RelOriCompositeAnglePieSliceGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~RelOriCompositeAnglePieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT RelAspectAnglePieSliceGraphic : public PieSliceGraphic
    {
    public:
      RelAspectAnglePieSliceGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
    private:
      /// osg::Referenced-derived
      virtual ~RelAspectAnglePieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT RelVelAzimuthPieSliceGraphic : public PieSliceGraphic
    {
    public:
      RelVelAzimuthPieSliceGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~RelVelAzimuthPieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT RelVelElevationPieSliceGraphic : public PieSliceGraphic
    {
    public:
      RelVelElevationPieSliceGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~RelVelElevationPieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT RelVelCompositeAnglePieSliceGraphic : public PieSliceGraphic
    {
    public:
      RelVelCompositeAnglePieSliceGraphic();
      void render(osg::Geode* geode, RangeToolState& state);
    protected:
      /// osg::Referenced-derived
      virtual ~RelVelCompositeAnglePieSliceGraphic() {}
    };
  };
} // namespace simVis

#endif // SIMVIS_RANGETOOL_H

