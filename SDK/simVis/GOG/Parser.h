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
#ifndef SIMVIS_GOG_PARSER_H
#define SIMVIS_GOG_PARSER_H

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include "simCore/Common/Common.h"
#include "simCore/Calc/Coordinate.h"
#include "simVis/Types.h"
#include "simVis/GOG/GOGNode.h"
#include "simVis/GOG/GOGRegistry.h"
#include "osgEarth/MapNode"
#include "osgEarth/Style"

namespace simCore { class UnitsRegistry; }

namespace simVis { namespace GOG
{

  class ErrorHandler;
  class GogNodeInterface;
  class ParsedShape;

  /**
   * Parses GOG files (streams).
   *
   * The GOG Parser will read a GOG file or stream, and encode it into a vector
   * of ParsedShape objects.  This is an in-memory representation of the GOG shape
   * data for the input stream.  Using loadGOGs() and the configured GOG Registry,
   * this class will then create the actual GOG Node (GogNodeInterface) from the
   * configuration data in ParsedShape.
   *
   * The low level parsing is handled by the parse() method.  Typically you need
   * to call loadGOGs(), unless your goal is only to get the in-memory representation
   * of the GOG data.
   *
   * The GOG Registry has built-in support for documented GOG types. You can
   * pass in a custom GOG Registry if you wish to register additional, custom
   * GOG types.
   *
   * The Parser should be instantiated on demand, so as not to outlive its
   * reference to the osgEarth::MapNode it contains.
   *
   * @deprecated This class has been deprecated in favor of simCore::GOG::Parser
   *   combined with simVis::GOG::Loader. The GOG parsing capability has been extracted
   *   and moved to simCore::GOG::Parser, which creates in-memory representations of
   *   GOG shapes. The simVis::GOG::Loader then creates 3-D representations of those
   *   shapes. This simVis::GOG::Parser class will be removed in a future release.
   */
  class SDKVIS_EXPORT Parser
  {
  public:
    /**
     * A list of nodes.
     */
    typedef std::vector<GogNodeInterface*> OverlayNodeVector;

    /**
     * Constructs a GOG parser.
     * @param mapNode  MapNode that provides the context to GOG objects created by this parser.  Note
     *  that if the map node changes, the Parser will not pick up on the change and could cause problems
     *  in parsed items.  So it is recommended that the GOG parser be instantiated on demand.
     */
    SDK_DEPRECATE(explicit Parser(osgEarth::MapNode* mapNode = nullptr), "Use simVis::GOG::Loader with simCore::GOG::Parser instead.");

    /**
     * Constructs a GOG parser.
     * @param registry Custom GOG object registry to use
     */
    SDK_DEPRECATE(explicit Parser(const GOGRegistry& registry), "Use simVis::GOG::Loader with simCore::GOG::Parser instead.");

    /// Virtual destructor
    virtual ~Parser() {}

    /**
     * Sets the default reference location for relative GOG data loaded
     * from a file. The GOG specification allows for "relative" coordinates; when you
     * attach the GOG to an entity (like a platform), the GOG is relative to that
     * entity. Otherwise the GOG is placed on the map relative to this reference
     * location.
     * @param coord Reference location (must be absolute coord sys like LLA or ECEF)
     */
    void setReferenceLocation(const simCore::Coordinate& coord);

    /**
     * Sets the default reference location for relative GOG data loaded
     * from a file. The GOG specification allows for "relative" coordinates; when you
     * attach the GOG to an entity (like a platform), the GOG is relative to that
     * entity. Otherwise the GOG is placed on the map relative to this reference
     * location.
     * @param point Reference location
     */
    void setReferenceLocation(const osgEarth::GeoPoint& point);

    /**
     * Changes the error output handler to use when parsing GOG data.
     * @param errorHandler Error handler to use when parsing GOG data
     */
    void setErrorHandler(std::shared_ptr<ErrorHandler> errorHandler);

    /** Changes the Units Registry for unit conversions. */
    void setUnitsRegistry(const simCore::UnitsRegistry* registry);

    /**
     * Sets a style that will override style information found in the GOG input.
     * @param[in ] style Override style
     */
    void setStyle(const osgEarth::Style& style) { style_ = style; }

  public:
    /**
     * Parses a GOGParams into a GOG node.
     * @return a new instance of GogNodeInterface. Caller takes ownership of the memory
     */
    GogNodeInterface* createGOG(
      const std::vector<std::string>& lines,
      const GOGNodeType&              nodeType,
      GogFollowData&                  followData) const;

    /**
     * Parses an input stream into a collection of GOG nodes.
     * @param[in ] input        Input stream
     * @param[in ] nodeType     Read GOGs as this type
     * @param[out] output       Resulting GOG collection
     * @param[out] followData   Vector of the follow orientation data for attached GOGs, parallel vector to the output
     * @param[out] parsedShapes If supplied, is filled with the ParsedShapes parsed from the input stream
     * @param[out] metaData     If supplied, is filled with the GogMetaData parsed from the input stream
     * @return True upon success, false upon failure
     */
    bool createGOGs(
      std::istream&               input,
      const GOGNodeType&          nodeType,
      OverlayNodeVector&          output,
      std::vector<GogFollowData>& followData,
      std::vector<ParsedShape>*   parsedShapes = nullptr,
      std::vector<GogMetaData>*   metaData = nullptr) const;

    /**
    * Converts the GOG file shape keyword to a GogShape. Assumes keyword is all lower, does exact match
    * @param keyword for GOG shape
    * @return equivalent GogShape, GOG_UNKNOWN if no conversion found
    */
    static GogShape getShapeFromKeyword(const std::string& keyword);

    /**
    * Converts the GogShape enum to the GOG file keyword
    * @param shape
    * @return GOG file shape keyword, empty for unknown
    */
    static std::string getKeywordFromShape(GogShape shape);

    /**
     * Parses data from an input stream into a collection of GOG nodes.
     * @param[in ] input        stream containing the serialized GOG
     * @param[in ] nodeType     Read GOGs as this type
     * @param[out] output       Resulting GOG collection
     * @param[out] followData   Vector of the follow orientation data for attached GOGs, parallel vector to the output
     * @param[out] parsedShapes If supplied, is filled with the ParsedShapes parsed from the input stream
     * @param[out] metaData     If supplied, is filled with the GogMetaData parsed from the input stream
     * @return True upon success, false upon failure
     */
    bool loadGOGs(
      std::istream&               input,
      const GOGNodeType&          nodeType,
      OverlayNodeVector&          output,
      std::vector<GogFollowData>& followData,
      std::vector<ParsedShape>*   parsedShapes = nullptr,
      std::vector<GogMetaData>*   metaData = nullptr) const;

    /**
    * Add or overwrite a color key with a new color
    * @param[in ] key   GOG key like color1, color2, red, black,...
    * @param[in ] color The color to use for the given key
    */
    void addOverwriteColor(const std::string& key, simVis::Color color);

    /**
     * Parses an input GOG stream into a vector of ParsedShape entries, and a parallel vector of GogMetaData.
     * The metadata contains attributes of the GOG shape that may be lost when converting to an osg::Node,
     * things like the GOG shape type (circle, polygon, etc.) and other information that is not in the node or its
     * osgEarth::Style. All relevant lines are stored in a single string for each GOG.  Although
     * this is a public method, it is lower level than loadGOGs(), which uses the configured Registry to
     * create instances of GogNodeInterface representing each OSG node for the GOG.
     * @param[in ] input GOG input data
     * @param[out] output Vector that will contain a ParsedShape element for each GOG node (shape) in the input stream.
     * @param[out] metaData Meta data about the GOG that needs to be stored with the resulting osg::Node
     */
    bool parse(std::istream& input, std::vector<ParsedShape>& output,
      std::vector<GogMetaData>&  metaData) const;

    /**
     * Given an input vector of ParsedShapes and GogMetaData, create individual GogNodeInterface classes (using
     * the configured Registry), and return a vector of those instances, for every item in the ParsedShape list.
     * @param[in ] parsedShapes GOG data, deserialized as from parse()
     * @param[in ] nodeType Read GOGs as this type
     * @param[in ] metaData Meta data about the GOG that is lost in the osg::Node
     * @param[out] output Resulting GOG collection
     * @param[out] followData Vector of the follow orientation data for attached GOGs, parallel vector to the output
     * @return True upon success, false upon failure
     */
    bool createGOGsFromShapes(
      const std::vector<ParsedShape>& parsedShapes,
      const GOGNodeType&              nodeType,
      const std::vector<GogMetaData>& metaData,
      OverlayNodeVector&              output,
      std::vector<GogFollowData>&     followData) const;

  private:

    /** Applies all the specified data to the meta data as appropriate  */
    void updateMetaData_(const ModifierState& state, const std::string& refOriginLine, const std::string& positionLines, bool relative, GogMetaData& currentMetaData) const;

    /** Initialize the default GOG colors */
    void initGogColors_();

    /**
     * Converts an GOG color into an HTML simVis::Color
     * @param[in ] c     GOG color
     * @param[in ] isHex If true than c is in Hex
     * @return GOG color into an HTML simVis::Color
     */
    std::string parseGogColor_(const std::string& c, bool isHex) const;

    /**
    * Parses an angle string, which may be in DD, DDM, or DMS format, into a
    * decimal degrees string.
    * @param[in ] input String to be parsed
    * @return Parsed string in decimal degrees format
    */
    std::string parseGogGeodeticAngle_(const std::string& input) const;

    /**
     * Prints any GOG parsing error to simNotify
     * @param[in ] lineNumber  Line number of offending error
     * @param[in ] errorText   Details of error
     */
    void printError_(size_t lineNumber, const std::string& errorText) const;

  private:
    /// Note that the map node could change; generally though it will not change between when a parser is instantiated and used.
    osg::observer_ptr<osgEarth::MapNode> mapNode_;
    GOGRegistry                          registry_;
    GOGContext                           context_;
    osgEarth::Style           style_;
    std::map<std::string, simVis::Color> colors_; // Key is GOG color like color1, color2
  };

} } // namespace simVis::GOG

#endif // SIMVIS_GOG_PARSER_H
