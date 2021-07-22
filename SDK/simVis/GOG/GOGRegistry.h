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
#ifndef SIMVIS_GOG_PARSER_REGISTRY_H
#define SIMVIS_GOG_PARSER_REGISTRY_H

#include "simCore/Common/Common.h"
#include "simVis/GOG/GOGNode.h"
#include "simVis/GOG/Utils.h"

namespace simVis { namespace GOG {

class GogNodeInterface;
class ParsedShape;

/**
 * Place to register GOG object parser functions. The GOG::Parser
 * will call the parsing function registered for a particular
 * keyword when it encounters it in the GOG source file.
 */
class SDKVIS_EXPORT GOGRegistry
{
public:
  /**
   * Constructs a new GOG registry
   * @param[in ] mapNode map node for georeferencing
   */
  GOGRegistry(osgEarth::MapNode* mapNode);

  /**
   * Copy constructor
   * @param[in ] rhs Object to copy
   */
  GOGRegistry(const GOGRegistry& rhs);

  /// Virtual destructor
  virtual ~GOGRegistry() {}

  /**
   * Creates a single GOG node by parsing input data that corresponds to the
   * specified shape tag. The parse shape's shape keyword must be registered
   * here in order to locate and invoke the appropriate factory method.
   * @param[in ] parsedShape Serialized object data, output from Parser::parse()
   * @param[in ] nodeType   Whether to create a geographic or hosted GOG
   * @param[in ] style      Override style parameters
   * @param[in ] context    Common GOG information
   * @param[in ] metaData    meta data describing shape
   * @param[out] followData filled in with the follow orientation data for an attached GOG
   * @return a new instance of GogNodeInterface. Caller takes ownership of the memory
   */
  GogNodeInterface* createGOG(
    const ParsedShape& parsedShape,
    const GOGNodeType& nodeType,
    const osgEarth::Style& style,
    const GOGContext&  context,
    const GogMetaData& metaData,
    GogFollowData&     followData) const;

  /**
   * Fetched the map node associated with the registry
   * @return A map node, or nullptr if this is a localized object
   */
  osgEarth::MapNode* getMapNode() const { return mapNode_.get(); }


public:
  /** Pure virtual class that defines a method for deserializing GOG objects into the scene graph */
  struct Deserializer : public osg::Referenced
  {
    /** Functor method to deserialize a GOG into the scene */
    virtual GogNodeInterface* operator()(
      const ParsedShape&  parsedShape,
      simVis::GOG::ParserData& p,
      const GOGNodeType&       nodeType,
      const GOGContext&        context,
      const GogMetaData&       metaData,
      osgEarth::MapNode*       mapNode) const = 0;
  };

  /**
   * Adds a deserialization functor to the registry
   * @param[in ] tag  Tag that will trigger this factory method
   * @param[in ] func Functor that creates the node
   */
  void add(const std::string& tag, Deserializer* func);

protected:
  /** Typedef of a map of GOG types (string) to their Deserializer ref_ptr */
  typedef std::map<std::string, osg::ref_ptr<Deserializer> > DeserializerTable;

  /** Observer to the map node */
  osg::observer_ptr<osgEarth::MapNode> mapNode_;
  /** Maps GOG types as strings, to Deserializer instances */
  DeserializerTable          deserializers_;
};

}
} // namespace simVis::GOG

#endif // SIMVIS_GOG_PARSER_REGISTRY_H
