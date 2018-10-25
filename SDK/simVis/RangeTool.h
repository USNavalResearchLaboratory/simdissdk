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
#include "simVis/Scenario.h"
#include "simVis/Platform.h"
#include "simVis/Tool.h"
#include "simVis/Utils.h"

namespace simCore { class DatumConvert; }
namespace simRF { class RFPropagationFacade; }

namespace simVis
{
  /// Units for calculations that have no units
  const osgEarth::Units UNITLESS = osgEarth::Units("nounits", "", osgEarth::Units::TYPE_INVALID, 1.0);
  /// Units in db
  const osgEarth::Units LOG10  = osgEarth::Units("log10", "dB", osgEarth::Units::TYPE_INVALID, 1.0);
  /// Units for RF Power in dBW
  const osgEarth::Units RF_POWER = osgEarth::Units("rf_power", "dBW", osgEarth::Units::TYPE_INVALID, 1.0);
  /// Units for RF Power in dBsm (square meters)
  const osgEarth::Units RF_POWER_SM = osgEarth::Units("rf_power_sm", "dBsm", osgEarth::Units::TYPE_INVALID, 1.0);
  /// Units for %
  const osgEarth::Units PERCENTAGE = osgEarth::Units("percentage", "%", osgEarth::Units::TYPE_INVALID, 1.0);

  /// Default effective Earth radius scalar for optical horizon measurement
  const double DEFAULT_OPTICAL_RADIUS = 1.06;

  /// Default effective Earth radius scalar for RF horizon measurement
  const double DEFAULT_RF_RADIUS = 4. / 3.;

  /// Number of enumerations in State::Coord
  const size_t COORD_CACHE_SIZE = 16;
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
      osg::Vec4f     lineColor1_,   lineColor2_;
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
    * Internal state class for Graphic rendering. Graphic primitives and
    * Measurements receive a State object internally when rendering in order
    * to track object locations and other shared data.
    */
    struct SDKVIS_EXPORT State
    {
      /**
       * Coordinate data saved in the coord_ member variable for later use
       * Local coordinate mean LTP with OBJ 0 at the origin
       */
      enum Coord
      {
        COORD_OBJ_0,  ///< The "to" object in local coordinates
        COORD_OBJ_1,  ///< The "from" object in local coordinates
        COORD_OBJ_0_0HAE,  ///< The "to" object forced to zero altitude, in local coordinates
        COORD_OBJ_1_0HAE,  ///< The "from" object forced to zero altitude, in local coordinates
        COORD_OBJ_0_AT_OBJ_1_ALT,  ///< The "to" object at the "from" object altitude, in local coordinates
        COORD_OBJ_1_AT_OBJ_0_ALT,  ///< The "from" object at the "to" object altitude, in local coordinates
        COORD_DR,  ///< Down Range inflection point (the corner of the "L") in local coordinates
        COORD_VEL_AZIM_DR, ///< Velocity Azimuth Down Range inflection point (the corner of the "L") in local coordinates
        COORD_BEAM_LLA_0,  ///< The "to" object for beam calculation (closest point) in LLA (rad, rad, m)
        COORD_BEAM_LLA_1,  ///< TThe "from" object for beam calculation (closest point) in LLA (rad, rad, m)
        COORD_BEAM_0,  ///< The "to" object for beam calculation (closest point) in local coordinates
        COORD_BEAM_1,  ///< TThe "from" object for beam calculation (closest point) in local coordinates
        COORD_BEAM_0_0HAE,  ///< The "to" beam forced to zero altitude, in local coordinates
        COORD_BEAM_1_0HAE,  ///< The "from" beam forced to zero altitude, in local coordinates
        COORD_BEAM_0_AT_BEAM_1_ALT,  ///< The "to" beam at the "from" object altitude, in local coordinates
        COORD_BEAM_1_AT_BEAM_0_ALT,  ///< The "from" beam at the "to" object altitude, in local coordinates
      };

      /**
       * Calculates and caches the requested values
       * @param coord the type value to calculate and cache
       * @return the requested values, the type of values detailed in Coord
       */
      osg::Vec3d coord(Coord coord);

      /**
       * Converts osg::Vec3d to simCore::Vec3
       * @param point the data to convert
       * @return simCore::Vec3 of the given point
       */
      simCore::Vec3 osg2simCore(const osg::Vec3d& point) const;

      /**
       * Converts simCore::Vec3 to osg::Vec3d
       * @param point the data to convert
       * @return osg::Vec3d of the given point
       */
      ///@return osg:Vec3d of the given point
      osg::Vec3d simCore2osg(const simCore::Vec3& point) const;

      /// Interpolate 'numSeg' positions between 'lla0' and 'lla1', adding them to 'verts'
      void line(
        const simCore::Vec3& lla0,
        const simCore::Vec3& lla1,
        double               altOffset,
        osg::Vec3Array*      verts);

      /**
       * Generate a list of lat/lon points between lla0 and lla1 at intervals of at most distDelta.  List does not include lla0 or lla1.
       * If lla0 == lla1, list will be empty.
       * @param lla0 Start point to generate intermediate points from
       * @param lla1 End point to generate intermediate points towards
       * @param distDelta Maximum distance between intermediate points.  Actual distance between may be lower.
       * @param[out] llaPointsOut Vector of intermediate points.  All points have an altitude of 0
       */
      void intermediatePoints(const simCore::Vec3& lla0, const simCore::Vec3& lla1, double distDelta, std::vector<simCore::Vec3>& llaPointsOut) const;

      /// Returns the midpoint between the two given positions
      simCore::Vec3 midPoint(const simCore::Vec3& lla0, const simCore::Vec3& lla1, double altOffset);

      ///@return the given lla to relative values scaled to the local frame (xyz)
      osg::Vec3 lla2local(double lat_rad, double lon_rad, double alt_m) const;

      ///@return lla values for the given position relative to the local frame
      simCore::Vec3 local2lla(const osg::Vec3d& local);

      ///@return the local/ENU vector produced by rotating the start->end vector by specified az, rotated in the ltp
      osg::Vec3d rotateEndVec(double az);

      /**
      * Fills in a entity state based on the given scenario and entity node
      * @param scenario The scenario for getting the host platform of node
      * @param node The node to extract information from
      * @param state Range Tool state information needed to do the calculations
      * @return zero on success and non-zero on failure
      */
      int populateEntityState(const simVis::ScenarioManager& scenario, const simVis::EntityNode* node, EntityState& state);

      /**
      * Resets the coord cache to initial state
      */
      void resetCoordCache();

      /**@name internal state (TODO: make private)
       *@{
       */
      osg::Matrixd                     world2local_;     // world to local tangent plane
      osg::Matrixd                     local2world_;     // reverse of above
      EntityState                      beginEntity_;
      EntityState                      endEntity_;
      simCore::EarthModelCalculations  earthModel_;
      simCore::CoordinateConverter     coordConv_;
      osgEarth::optional<osg::Vec3d>   coord_[COORD_CACHE_SIZE];  // number of enumerations in State::Coord
      simCore::TimeStamp timeStamp_; // the timeStamp of the last update
      osg::observer_ptr<osgEarth::MapNode>  mapNode_;
      ///@}
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
      virtual void render(osg::Geode* geode, State& state) =0;

      /**
      * Gets a coordinate at which to place a text label for this primitive.
      * @param state context object
      * @return A local (tangent-plane) space coordinate
      */
      virtual osg::Vec3 labelPos(State& state) = 0;

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
    * Base class for formatting values into a string
    */
    class SDKVIS_EXPORT ValueFormatter : public osg::Referenced
    {
    public:
      virtual ~ValueFormatter() {}
      /**
      * Formats the value into a string
      * @param value The value that needs to be converted into a string.
      * @param precision The number of digits after the decimal point
      * @return The value as a string
      */
      virtual std::string stringValue(double value, int precision) const;
    };

    /**
    * Class for formatting Above/Below into a string.
    * Intended for use with RadioHorizonMeasurement and OpticalHorizonMeasurement
    */
    class SDKVIS_EXPORT HorizonFormatter : public ValueFormatter
    {
    public:
      virtual ~HorizonFormatter() {}
      /**
      * Formats the value into a string
      * @param value The value that needs to be converted into a string.
      * @param precision Ignored
      * @return The value as a string
      */
      virtual std::string stringValue(double value, int precision) const;
    };

    /**
    * Base class for a Measurement. A Measurement is a value derived from the
    * simulation state data (object positions, orientations, velocities, etc.).
    * Each calculation consists of multiple Graphic primitives and a single
    * Measurement, which is the value displayed in the text label.
    *
    * You can create custom measurements by sub-classing Measurement.
    */
    class SDKVIS_EXPORT Measurement : public osg::Referenced
    {
    protected:
      /**
      * Constructs a new Measurement
      * @param typeName    Unique type name of this measurement.
      * @param typeAbbr    Prefix string for labeling.
      * @param units       Units of the measurement value.
      */
      Measurement(const std::string &typeName, const std::string &typeAbbr, const osgEarth::Units &units);

    public:
      /**
      * Gets the unique type name of the measurement
      */
      const std::string& typeName() const { return typeName_; }

      /**
      * The abbreviation string to use for labeling/UI.
      * @return A short string
      */
      const std::string& typeAbbr() const { return typeAbbr_; }

      /**
      * The Units in which value() is expressed.
      * @return Units
      */
      const osgEarth::Units& units() const { return units_; }

      /**
      * Returns the calculated value of the measurement
      * @param state Context object
      * @return Calculated value
      */
      virtual double value(State& state) const = 0;

      /**
      * Returns if the calculation is valid for the given types
      * @param state All the state information for both entities
      * @return True if the types are valid for the calculation otherwise false
      */
      virtual bool willAccept(const simVis::RangeTool::State& state) const = 0;

      /**
      * Returns the calculated value converted to the specified units.
      * @param outputUnits units to which to convert
      * @param state Context object
      * @return Calculated value in target units
      */
      double value(const osgEarth::Units& outputUnits, State& state) const;

      /**
      * Returns the formatter for the measurement
      * @return The formatter for the measurement
      */
      ValueFormatter* formatter() const { return formatter_.get(); }

    protected:
      /// osg::Referenced-derived
      virtual ~Measurement() {}

      /// Returns true if the type is a beam, gate, laser or lob group
      bool isRaeObject_(simData::ObjectType type) const;
      /// Returns true if both types are either platform, beam, gate, laser or lob group
      bool isEntityToEntity_(simData::ObjectType fromType, simData::ObjectType toType) const;
      /// Returns true if both types are platforms
      bool isPlatformToPlatform_(simData::ObjectType fromType, simData::ObjectType toType) const;
      /// Returns true if both types are either platforms or custom rendering
      bool isLocationToLocation_(simData::ObjectType fromType, simData::ObjectType toType) const;
      /// Returns true if one type is a beam and the other is a non-beam
      bool isBeamToNonBeamAssociation_(simData::ObjectType fromType, simData::ObjectType toType) const;
      /// Returns true if the fromType is a beam and the toType is a valid entity
      bool isBeamToEntity_(simData::ObjectType fromType, simData::ObjectType toType) const;
      /// Returns true if the nodes are valid for a angle calculation
      bool isAngle_(simData::ObjectType fromType, simData::ObjectId fromHostId, simData::ObjectType toType, simData::ObjectId toHostId) const;
      /// Returns true if the nodes are valid for velocity angle calculation
      bool isVelocityAngle_(simData::ObjectType fromType, simData::ObjectId fromHostId, simData::ObjectType toType, simData::ObjectId toHostId) const;
      /// Returns the composite angle (rad) for the given angles (rad) for entities on the SAME platform
      double getCompositeAngle_(double bgnAz, double bgnEl, double endAz, double endEl) const;
      /// Returns the true angles (rad) for the given state
      void calculateTrueAngles_(const RangeTool::State& state, double* az, double* el, double* cmp) const;
      /**
      * Calculates the relative angles between entities (for the given state)
      * @param state current range tool state upon to use for calculation
      * @param az Relative azimuth angle to calculate (rad)
      * @param el Relative elevation angle to calculate (rad)
      * @param cmp Relative composite angle to calculate (rad)
      */
      void calculateRelativeAngles_(const RangeTool::State& state, double* az, double* el, double* cmp) const;
      /// Formats the double value to a string
      osg::ref_ptr< ValueFormatter > formatter_;

    private:
      std::string     typeName_;
      std::string     typeAbbr_;
      osgEarth::Units units_;
    };

    /// a vector of Measurement pointers
    typedef std::vector< osg::ref_ptr<Measurement> > MeasurementVector;

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
      void addGraphic(Graphic* graphic, bool useAsLabelGraphic =false);

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
      State                              state_;                 // the calc state for this assoc
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
        State&             state);

      /// osg::Referenced-derived
      virtual ~LineGraphic() {}
    };

    /// a filled in arc
    class SDKVIS_EXPORT PieSliceGraphic : public Graphic
    {
    public:
      virtual osg::Vec3 labelPos(State& state);

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
        State&       state);

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
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);

    protected:
      /// osg::Referenced-derived
      virtual ~GroundLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT SlantLineGraphic : public LineGraphic
    {
    public:
      SlantLineGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~SlantLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeginAltitudeLineGraphic : public LineGraphic
    {
    public:
      BeginAltitudeLineGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeginAltitudeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT EndAltitudeLineGraphic : public LineGraphic
    {
    public:
      EndAltitudeLineGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~EndAltitudeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeginAltitudeLineToEndAltitudeGraphic : public LineGraphic
    {
    public:
      BeginAltitudeLineToEndAltitudeGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeginAltitudeLineToEndAltitudeGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT EndAltitudeLineToBeginAltitudeGraphic : public LineGraphic
    {
    public:
      EndAltitudeLineToBeginAltitudeGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~EndAltitudeLineToBeginAltitudeGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeginToEndLineAtBeginAltitudeGraphic : public LineGraphic
    {
    public:
      BeginToEndLineAtBeginAltitudeGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeginToEndLineAtBeginAltitudeGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeginToEndLineAtEndAltitudeGraphic : public LineGraphic
    {
    public:
      BeginToEndLineAtEndAltitudeGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeginToEndLineAtEndAltitudeGraphic() {}
    };

    /// Graphics
    class SDKVIS_EXPORT BeamGroundLineGraphic : public LineGraphic
    {
    public:
      BeamGroundLineGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamGroundLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeamSlantLineGraphic : public LineGraphic
    {
    public:
      BeamSlantLineGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamSlantLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeamBeginAltitudeLineGraphic : public LineGraphic
    {
    public:
      BeamBeginAltitudeLineGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamBeginAltitudeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeamEndAltitudeLineGraphic : public LineGraphic
    {
    public:
      BeamEndAltitudeLineGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamEndAltitudeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeamBeginAltitudeLineToEndAltitudeGraphic : public LineGraphic
    {
    public:
      BeamBeginAltitudeLineToEndAltitudeGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamBeginAltitudeLineToEndAltitudeGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeamEndAltitudeLineToBeginAltitudeGraphic : public LineGraphic
    {
    public:
      BeamEndAltitudeLineToBeginAltitudeGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamEndAltitudeLineToBeginAltitudeGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeamBeginToEndLineAtBeginAltitudeGraphic : public LineGraphic
    {
    public:
      BeamBeginToEndLineAtBeginAltitudeGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamBeginToEndLineAtBeginAltitudeGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT BeamBeginToEndLineAtEndAltitudeGraphic : public LineGraphic
    {
    public:
      BeamBeginToEndLineAtEndAltitudeGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~BeamBeginToEndLineAtEndAltitudeGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT CrossRangeLineGraphic : public LineGraphic
    {
    public:
      CrossRangeLineGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~CrossRangeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT DownRangeLineGraphic : public LineGraphic
    {
    public:
      DownRangeLineGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~DownRangeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT VelAzimDownRangeLineGraphic : public LineGraphic
    {
    public:
      VelAzimDownRangeLineGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~VelAzimDownRangeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT VelAzimCrossRangeLineGraphic : public LineGraphic
    {
    public:
      VelAzimCrossRangeLineGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~VelAzimCrossRangeLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT DownRangeCrossRangeDownLineGraphic : public LineGraphic
    {
    public:
      DownRangeCrossRangeDownLineGraphic();
      void render(osg::Geode* geode, State& state);
      osg::Vec3 labelPos(State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~DownRangeCrossRangeDownLineGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT TrueAzimuthPieSliceGraphic : public PieSliceGraphic
    {
    public:
      TrueAzimuthPieSliceGraphic();
      void render(osg::Geode* geode, State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~TrueAzimuthPieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT TrueElevationPieSliceGraphic : public PieSliceGraphic
    {
    public:
      TrueElevationPieSliceGraphic();
      void render(osg::Geode* geode, State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~TrueElevationPieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT TrueCompositeAnglePieSliceGraphic : public PieSliceGraphic
    {
    public:
      TrueCompositeAnglePieSliceGraphic();
      void render(osg::Geode* geode, State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~TrueCompositeAnglePieSliceGraphic() {}
    };

    struct SDKVIS_EXPORT MagneticAzimuthPieSliceGraphic : public PieSliceGraphic
    {
    public:
      MagneticAzimuthPieSliceGraphic();
      void render(osg::Geode* geode, State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~MagneticAzimuthPieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT RelOriAzimuthPieSliceGraphic : public PieSliceGraphic
    {
    public:
      RelOriAzimuthPieSliceGraphic();
      void render(osg::Geode* geode, State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~RelOriAzimuthPieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT RelOriElevationPieSliceGraphic : public PieSliceGraphic
    {
    public:
      RelOriElevationPieSliceGraphic();
      void render(osg::Geode* geode, State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~RelOriElevationPieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT RelOriCompositeAnglePieSliceGraphic : public PieSliceGraphic
    {
    public:
      RelOriCompositeAnglePieSliceGraphic();
      void render(osg::Geode* geode, State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~RelOriCompositeAnglePieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT RelAspectAnglePieSliceGraphic : public PieSliceGraphic
    {
    public:
      RelAspectAnglePieSliceGraphic();
      void render(osg::Geode* geode, State& state);
    private:
      /// osg::Referenced-derived
      virtual ~RelAspectAnglePieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT RelVelAzimuthPieSliceGraphic : public PieSliceGraphic
    {
    public:
      RelVelAzimuthPieSliceGraphic();
      void render(osg::Geode* geode, State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~RelVelAzimuthPieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT RelVelElevationPieSliceGraphic : public PieSliceGraphic
    {
    public:
      RelVelElevationPieSliceGraphic();
      void render(osg::Geode* geode, State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~RelVelElevationPieSliceGraphic() {}
    };

    /// Graphics
    struct SDKVIS_EXPORT RelVelCompositeAnglePieSliceGraphic : public PieSliceGraphic
    {
    public:
      RelVelCompositeAnglePieSliceGraphic();
      void render(osg::Geode* geode, State& state);
    protected:
      /// osg::Referenced-derived
      virtual ~RelVelCompositeAnglePieSliceGraphic() {}
    };

  public: // Built-in Measurements

    /// Ground Range
    class SDKVIS_EXPORT GroundDistanceMeasurement : public Measurement
    {
    public:
      GroundDistanceMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~GroundDistanceMeasurement() {}
    };

    /// Slant Range
    class SDKVIS_EXPORT SlantDistanceMeasurement : public Measurement
    {
    public:
      SlantDistanceMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    public:
      /// osg::Referenced-derived
      virtual ~SlantDistanceMeasurement() {}
    };

    /// Altitude
    class SDKVIS_EXPORT AltitudeDeltaMeasurement : public Measurement
    {
    public:
      AltitudeDeltaMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~AltitudeDeltaMeasurement() {}
    };

    /// Beam Ground Range
    class SDKVIS_EXPORT BeamGroundDistanceMeasurement : public Measurement
    {
    public:
      BeamGroundDistanceMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~BeamGroundDistanceMeasurement() {}
    };

    /// Beam Slant Range
    class SDKVIS_EXPORT BeamSlantDistanceMeasurement : public Measurement
    {
    public:
      BeamSlantDistanceMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~BeamSlantDistanceMeasurement() {}
    };

    /// Beam Altitude
    class SDKVIS_EXPORT BeamAltitudeDeltaMeasurement : public Measurement
    {
    public:
      BeamAltitudeDeltaMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~BeamAltitudeDeltaMeasurement() {}
    };

    /// Down Range
    class SDKVIS_EXPORT DownRangeMeasurement : public Measurement
    {
    public:
      DownRangeMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~DownRangeMeasurement() {}
    };

    /// Cross Range
    class SDKVIS_EXPORT CrossRangeMeasurement : public Measurement
    {
    public:
      CrossRangeMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~CrossRangeMeasurement() {}
    };

    /// Down Value
    class SDKVIS_EXPORT DownRangeCrossRangeDownValueMeasurement : public Measurement
    {
    public:
      DownRangeCrossRangeDownValueMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~DownRangeCrossRangeDownValueMeasurement() {}
    };

    /// Geodesic Down Range
    class SDKVIS_EXPORT GeoDownRangeMeasurement : public Measurement
    {
    public:
      GeoDownRangeMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~GeoDownRangeMeasurement() {}
    };

    /// Geodesic Cross Range
    class SDKVIS_EXPORT GeoCrossRangeMeasurement : public Measurement
    {
    public:
      GeoCrossRangeMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~GeoCrossRangeMeasurement() {}
    };

    // True angles

    /// True Azimuth
    class SDKVIS_EXPORT TrueAzimuthMeasurement : public Measurement
    {
    public:
      TrueAzimuthMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    public:
      /// osg::Referenced-derived
      virtual ~TrueAzimuthMeasurement() {}
    };

    /// True Elevation
    class SDKVIS_EXPORT TrueElevationMeasurement : public Measurement
    {
    public:
      TrueElevationMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    public:
      /// osg::Referenced-derived
      virtual ~TrueElevationMeasurement() {}
    };

    /// True Composite Angle Calculation
    class SDKVIS_EXPORT TrueCompositeAngleMeasurement : public Measurement
    {
    public:
      TrueCompositeAngleMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    public:
      /// osg::Referenced-derived
      virtual ~TrueCompositeAngleMeasurement() {}
    };

    class SDKVIS_EXPORT MagneticAzimuthMeasurement : public Measurement
    {
    public:
      explicit MagneticAzimuthMeasurement(std::shared_ptr<simCore::DatumConvert> datumConvert);
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    public:
      /// osg::Referenced-derived
      virtual ~MagneticAzimuthMeasurement() {}
    private:
      std::shared_ptr<const simCore::DatumConvert> datumConvert_;
    };

    // Orientation-relative angles

    /// helper class - internal
    struct RelOriMeasurement : public Measurement
    {
    protected:
      /**
       * Constructor.
       * @param name The name of the type.
       * @param abbr The type abbr.
       * @param units The units.
       */
      RelOriMeasurement(const std::string& name, const std::string& abbr, const osgEarth::Units& units)
        : Measurement(name, abbr, units) { }

      /**
       * Calculates the angles for the given state
       * @param az If non-null, the azimuth (radians).
       * @param el If non-null, the elevation (radians).
       * @param cmp If non-null, the composite angle (radians).
       * @param state The state information for both the begin and end entities
       */
      void getAngles(double* az, double* el, double* cmp, State& state) const;

      /// osg::Referenced-derived
      virtual ~RelOriMeasurement() {}
    };

    /// Orientation Relative Azimuth
    class SDKVIS_EXPORT RelOriAzimuthMeasurement : public RelOriMeasurement
    {
    public:
      RelOriAzimuthMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    public:
      /// osg::Referenced-derived
      virtual ~RelOriAzimuthMeasurement() {}
    };

    /// Orientation Relative Elevation
    class SDKVIS_EXPORT RelOriElevationMeasurement : public RelOriMeasurement
    {
    public:
      RelOriElevationMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    public:
      /// osg::Referenced-derived
      virtual ~RelOriElevationMeasurement() {}
    };

    /// Orientation Relative Composite Angle
    class SDKVIS_EXPORT RelOriCompositeAngleMeasurement : public RelOriMeasurement
    {
    public:
      RelOriCompositeAngleMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    public:
      /// osg::Referenced-derived
      virtual ~RelOriCompositeAngleMeasurement() {}
    };

    // Velocity vector-relative angles

    /// helper class - internal
    struct RelVelMeasurement : public Measurement
    {
    protected:

      /**
       * Constructor.
       * @param name The name of the type.
       * @param abbr The type abbr.
       * @param units The units.
      */
      RelVelMeasurement(const std::string& name, const std::string& abbr, const osgEarth::Units& units)
        : Measurement(name, abbr, units) { }

      /**
       * Calculates the angles for the given state
       * @param az If non-null, the azimuth (radians).
       * @param el If non-null, the elevation (radians).
       * @param cmp If non-null, the composite angle (radians).
       * @param state The state information for both the begin and end entities
       */
      void getAngles(double* az, double* el, double* cmp, State& state) const;

      /// osg::Referenced-derived
      virtual ~RelVelMeasurement() {}
    };

    /// Velocity Relative Azimuth
    class SDKVIS_EXPORT RelVelAzimuthMeasurement : public RelVelMeasurement
    {
    public:
      RelVelAzimuthMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    public:
      /// osg::Referenced-derived
      virtual ~RelVelAzimuthMeasurement() {}
    };

    /// Velocity Relative Elevation
    class SDKVIS_EXPORT RelVelElevationMeasurement : public RelVelMeasurement
    {
    public:
      RelVelElevationMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    public:
      /// osg::Referenced-derived
      virtual ~RelVelElevationMeasurement() {}
    };

    /// Velocity Relative Composite Angle
    class SDKVIS_EXPORT RelVelCompositeAngleMeasurement : public RelVelMeasurement
    {
    public:
      RelVelCompositeAngleMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    public:
      /// osg::Referenced-derived
      virtual ~RelVelCompositeAngleMeasurement() {}
    };

    // Velocity measures

    /// Closing Velocity
    class SDKVIS_EXPORT ClosingVelocityMeasurement : public Measurement
    {
    public:
      ClosingVelocityMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~ClosingVelocityMeasurement() {}
    };

    /// Separation Velocity
    class SDKVIS_EXPORT SeparationVelocityMeasurement : public Measurement
    {
    public:
      SeparationVelocityMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~SeparationVelocityMeasurement() {}
    };

    /// Velocity Delta
    class SDKVIS_EXPORT VelocityDeltaMeasurement : public Measurement
    {
    public:
      VelocityDeltaMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~VelocityDeltaMeasurement() {}
    };

    /// Velocity Azimuth Down Range
    class SDKVIS_EXPORT VelAzimDownRangeMeasurement : public Measurement
    {
    public:
      VelAzimDownRangeMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~VelAzimDownRangeMeasurement() {}
    };

    /// Velocity Azimuth Cross Range
    class SDKVIS_EXPORT VelAzimCrossRangeMeasurement : public Measurement
    {
    public:
      VelAzimCrossRangeMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~VelAzimCrossRangeMeasurement() {}
    };

    /// Velocity Azimuth Geodesic Down Range
    class SDKVIS_EXPORT VelAzimGeoDownRangeMeasurement : public Measurement
    {
    public:
      VelAzimGeoDownRangeMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~VelAzimGeoDownRangeMeasurement() {}
    };

    /// Velocity Azimuth Geodesic Cross Range
    class SDKVIS_EXPORT VelAzimGeoCrossRangeMeasurement : public Measurement
    {
    public:
      VelAzimGeoCrossRangeMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~VelAzimGeoCrossRangeMeasurement() {}
    };

    /// Aspect Angle
    class SDKVIS_EXPORT AspectAngleMeasurement : public Measurement
    {
    public:
      AspectAngleMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    private:
      /// osg::Referenced-derived
      virtual ~AspectAngleMeasurement() {}
    };

    /// Base class for RF calculations
    class SDKVIS_EXPORT RfMeasurement : public RelOriMeasurement
    {
    public:
      /**
       * Constructor.
       * @param name Name of the type.
       * @param abbr The type abbr.
       * @param units The units.
       */
      RfMeasurement(const std::string& name, const std::string& abbr, const osgEarth::Units& units);

    protected:
      /// osg::Referenced-derived
      virtual ~RfMeasurement() {}
      /**
       * Calculates RF parameters from the given state
       * @param state State information for both the begin and end entities
       * @param azAbs The absolute true azimuth, in radians, between the begin entity and the end entity
       * @param elAbs The absolute elevation, in radians, between the begin entity and the end entity
       * @param hgtMeters The height, in meters, of the antenna
       * @param xmtGaindB The gain, in dB, of the transmit antenna
       * @param rcvGaindB The gain, in dB, of the receive antenna
       * @param rcsSqm RCS db if useDb is true or RCS dBsm if useDb is false;
       * @param useDb Flag for selecting type of rcs value to return
       * @param freqMHz The frequency, in MHz, of the RF signal
       * @param powerWatts The power, in watts, of the RF signal
       */
      void getRfParameters_(State& state, double* azAbs, double* elAbs, double *hgtMeters, double* xmtGaindB, double* rcvGaindB, double* rcsSqm, bool useDb,
        double* freqMHz, double* powerWatts) const;
    };

    /// Antenna Gain
    class SDKVIS_EXPORT RFGainMeasurement : public RfMeasurement
    {
    public:
      RFGainMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~RFGainMeasurement() {}
    };

    /// Received Power
    class SDKVIS_EXPORT RFPowerMeasurement : public RfMeasurement
    {
    public:
      RFPowerMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~RFPowerMeasurement() {}
    };

    /// One-Way Power
    class SDKVIS_EXPORT RFOneWayPowerMeasurement : public RfMeasurement
    {
    public:
      RFOneWayPowerMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~RFOneWayPowerMeasurement() {}
    };

    /// Base class for Horizon calculations
    class SDKVIS_EXPORT HorizonMeasurement : public Measurement
    {
    public:

      /**
       * Constructor.
       * @param typeName Name of the type.
       * @param typeAbbr The type abbr.
       * @param units The units.
       */
      HorizonMeasurement(const std::string &typeName, const std::string &typeAbbr, const osgEarth::Units &units);
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

      /**
      * Set effective Earth radius scalars for optical and rf horizon measurement.
      * @param opticalRadius new Earth radius scalar for optical horizon calculations
      * @param rfRadius new Earth radius scalar for rf horizon calculations
      */
      void setEffectiveRadius(double opticalRadius, double rfRadius);

    protected:
      /// osg::Referenced-derived
      virtual ~HorizonMeasurement() {}

      /**
       * Calculates if the end entity is above or below the horizon
       * @param state Information on both the begin entity and end entity
       * @param horizon Type of calculation
       * @return 0 = below horizon and 1 = above horizon
       */
      virtual double calcAboveHorizon_(State& state, simCore::HorizonCalculations horizon) const;

    private:
      double opticalEffectiveRadius_;
      double rfEffectiveRadius_;
    };

    /// Radio Horizon
    class SDKVIS_EXPORT RadioHorizonMeasurement : public HorizonMeasurement
    {
    public:
      RadioHorizonMeasurement();
      virtual double value(State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~RadioHorizonMeasurement() {}
    };

    /// Optical Horizon
    class SDKVIS_EXPORT OpticalHorizonMeasurement : public HorizonMeasurement
    {
    public:
      OpticalHorizonMeasurement();
      virtual double value(State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~OpticalHorizonMeasurement() {}
    };

    /// Probability of Detection (PoD)
    class SDKVIS_EXPORT PodMeasurement : public RfMeasurement
    {
    public:
      PodMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~PodMeasurement() {}
    };

    /// Propagation Loss
    class SDKVIS_EXPORT LossMeasurement : public RfMeasurement
    {
    public:
      LossMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~LossMeasurement() {}
    };

    /// Pattern Propagation Factor (PPF)
    class SDKVIS_EXPORT PpfMeasurement : public RfMeasurement
    {
    public:
      PpfMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~PpfMeasurement() {}
    };

    /// Signal to Noise (SNR)
    class SDKVIS_EXPORT SnrMeasurement : public RfMeasurement
    {
    public:
      SnrMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~SnrMeasurement() {}
    };

    /// Clutter to Noise (CNR)
    class SDKVIS_EXPORT CnrMeasurement : public RfMeasurement
    {
    public:
      CnrMeasurement();
      //unlike other RF - related calculations, CNR doesn't have a height component
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~CnrMeasurement() {}
    };

    /// Radar Cross Section (RCS)
    class SDKVIS_EXPORT RcsMeasurement : public RfMeasurement
    {
    public:
      RcsMeasurement();
      virtual double value(State& state) const;
      virtual bool willAccept(const simVis::RangeTool::State& state) const;

    protected:
      /// osg::Referenced-derived
      virtual ~RcsMeasurement() {}
    };
  };

} // namespace simVis

#endif // SIMVIS_RANGETOOL_H

