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
#ifndef SIMVIS_GOG_PARSER_H
#define SIMVIS_GOG_PARSER_H

#include <map>
#include <memory>
#include <string>
#include <iostream>
#include "simCore/Common/Common.h"
#include "simCore/Calc/Coordinate.h"
#include "simVis/GOG/GOGNode.h"
#include "simVis/GOG/GOGRegistry.h"
#include "osgEarth/MapNode"
#include "osgEarthSymbology/Style"
#include "osgEarthSymbology/Color"

namespace simVis { namespace GOG
{
  class GogNodeInterface;
  class ErrorHandler;

  /**
   * Parses GOG files (streams).
   *
   * The GOG Parser will read a GOG file (or stream) and encode it as a
   * Config object (a general data container). It will then invoke the GOG
   * Registry, which will create the actual GOG Node from the config data.
   *
   * The GOG Registry has built-in support for documented GOG types. You can
   * pass in a custom GOG Registry if you wish to register addition, custom
   * GOG types.
   *
   * The Parser should be instantiated on demand, so as not to outlive its
   * reference to the osgEarth::MapNode it contains.
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
    Parser(osgEarth::MapNode* mapNode = NULL);

    /**
     * Constructs a GOG parser.
     * @param registry Custom GOG object registry to use
     */
    Parser(const GOGRegistry& registry);

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

    /**
     * Sets a style that will override style information found in the GOG input.
     * @param[in ] style Override style
     */
    void setStyle(const osgEarth::Symbology::Style& style) { style_ = style; }

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
     * @param[in ] input      Input stream
     * @param[in ] nodeType   Read GOGs as this type
     * @param[out] output     Resulting GOG collection
     * @param[out] followData Vector of the follow orientation data for attached GOGs, parallel vector to the output
     * @return True upon success, false upon failure
     */
    bool createGOGs(
      std::istream&                input,
      const GOGNodeType&           nodeType,
      OverlayNodeVector&           output,
      std::vector<GogFollowData>&  followData) const;

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
     * @param[in ] input  stream containing the serialized GOG
     * @param[in ] nodeType Read GOGs as this type
     * @param[out] output   Resulting GOG collection
     * @param[out] followData Vector of the follow orientation data for attached GOGs, parallel vector to the output
     * @return True upon success, false upon failure
     */
    bool loadGOGs(
      std::istream&                input,
      const GOGNodeType&           nodeType,
      OverlayNodeVector&           output,
      std::vector<GogFollowData>&  followData) const;

    /**
    * Add or overwrite a color key with a new color
    * @param[in ] key   GOG key like color1, color2, red, black,...
    * @param[in ] color The color to use for the given key
    */
    void addOverwriteColor(const std::string& key, osgEarth::Symbology::Color color);

    /**
    * Parses an input GOG stream into a Config structure and parallel vector of meta data.
    * The metadata contains attributes of the GOG shape that may be lost when converting to an osg::Node,
    * things like the GOG shape type (circle, polygon, etc.) and other information that is not in the node or its
    * osgEarth::Symbology::Style. All relevant lines are stored in a single string for each GOG.
    * @param[in ] input    GOG input data
    * @param[out] output   Config structure
    * @param[out] metaData Meta data about the GOG that needs to be stored with the resulting osg::Node
    */
    bool parse(
      std::istream&              input,
      osgEarth::Config&          output,
      std::vector<GogMetaData>&  metaData) const;

  private:

    /** Applies all the specified data to the meta data as appropriate  */
    void updateMetaData_(const ModifierState& state, const std::string& refOriginLine, const std::string& positionLines, bool relative, GogMetaData& currentMetaData) const;

    /** Initialize the default GOG colors */
    void initGogColors_();

    /**
     * Converts an GOG color into an HTML osgEarth::Color
     * @param[in ] c     GOG color
     * @param[in ] isHex If true than c is in Hex
     * @return GOG color into an HTML osgEarth::Color
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
     * Parses an input Config structure into a collection of GOG nodes.  Pass in the Config, and parallel vectors of meta data.
     * There must be an entry in the meta data vector for every item in the Config structure.
     * @param[in ] input    Config serialization
     * @param[in ] nodeType Read GOGs as this type
     * @param[in ] metaData Meta data about the GOG that is lost in the osg::Node
     * @param[out] output   Resulting GOG collection
     * @param[out] followData Vector of the follow orientation data for attached GOGs, parallel vector to the output
     * @return True upon success, false upon failure
     */
    bool createGOGs_(
      const osgEarth::Config&         input,
      const GOGNodeType&              nodeType,
      const std::vector<GogMetaData>& metaData,
      OverlayNodeVector&              output,
      std::vector<GogFollowData>&     followData) const;

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
    osgEarth::Symbology::Style           style_;
    std::map<std::string, osgEarth::Symbology::Color> colors_; // Key is GOG color like color1, color2
  };

} } // namespace simVis::GOG

#endif // SIMVIS_GOG_PARSER_H
