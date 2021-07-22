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
#ifndef SIMVIS_GOG_LOADER_H
#define SIMVIS_GOG_LOADER_H

#include <iostream>
#include <vector>
#include "simCore/Common/Common.h"
#include "simCore/GOG/Parser.h"
#include "simVis/GOG/GogNodeInterface.h"

namespace simCore { class UnitsRegistry; }

namespace simVis { namespace GOG
{

/**
* Parses GOG files (streams).
*
* The GOG Loader will read a GOG file or stream using a simCore::GOG::Parser,
* and generate a list of GogNodeInterface objects as output.
*
* The Loader should be instantiated on demand, so as not to outlive its
* reference to the osgEarth::MapNode it contains.
*/
class SDKVIS_EXPORT Loader
{
public:
  /// A list of GOG nodes.
  typedef std::vector<GogNodeInterfacePtr> GogNodeVector;

  /// Constructor takes a parser and map node for constructing the osgEarth GOG nodes
  Loader(const simCore::GOG::Parser& parser, osgEarth::MapNode* mapNode = nullptr);

  /// Virtual destructor
  virtual ~Loader();

  /**
  * Set the default reference position for fallback when parsing GOGs
  * @param referencePosition lla radians
  */
  void setReferencePosition(const simCore::Vec3& referencePosition);

  /**
  * Parses data from an input stream into a collection of GOG nodes.
  * @param input stream containing the serialized GOG
  * @param filename identifies the source GOG file or shape group
  * @param attached true if GOG is attached to a platform
  * @param output resulting GOG collection
  */
  void loadGogs(std::istream& input, const std::string& filename, bool attached, GogNodeVector& output) const;

  /**
  * Parses data from a single input shape block into a GOG node.
  * @param gogShapeBlock  string containing the serialized GOG shape, consisting of one start/end block
  * @param filename  identifies the source GOG file or the shape's group
  * @param shapeNumber  unique number for this shape within the parent GOG
  * @param attached  true if GOG is attached to a platform
  * @param output  resulting GOG collection
  */
  void loadShape(const std::string& gogShapeBlock, const std::string& filename, size_t shapeNumber, bool attached, GogNodeVector& output) const;

private:
  /// build a GOG node object from the specified GogShape; can return NULL if failed to build the node
  GogNodeInterfacePtr buildGogNode_(simCore::GOG::GogShapePtr gog, const std::string& filename, bool attached) const;

  /// Parser for converting the input stream into simCore::GOG::GogShape objects
  const simCore::GOG::Parser& parser_;
  /// Map node for use when creating osgEarth nodes
  osg::observer_ptr<osgEarth::MapNode> mapNode_;
  /// Default reference position to use as fallback
  simCore::Vec3 referencePosition_;
};

} } // namespace simVis::GOG

#endif // SIMVIS_GOG_LOADER_H
