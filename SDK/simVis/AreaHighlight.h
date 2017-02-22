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

#ifndef SIMVIS_AREA_HIGHLIGHT_H
#define SIMVIS_AREA_HIGHLIGHT_H

#include "osg/ref_ptr"
#include "osg/Geode"
#include "osg/Vec4f"
#include "simCore/Common/Common.h"

namespace osg { class Uniform; }

namespace simVis
{

/// Attachment node for a circular highlight display.
class SDKVIS_EXPORT AreaHighlightNode : public osg::Geode
{
public:
  /** Declare boilerplate code */
  META_Node(simVis, AreaHighlightNode);
  /** Constructor */
  AreaHighlightNode();
  /** OSG Copy constructor */
  AreaHighlightNode(const AreaHighlightNode &rhs, const osg::CopyOp& copyOp=osg::CopyOp::SHALLOW_COPY);

  /** Apply a color to the highlight */
  void setColor(const osg::Vec4f& rgba);

  /** Changes the radius of the highlight in meters */
  void setRadius(float radius);

protected:

  /// osg::Referenced-derived
  virtual ~AreaHighlightNode();

private:
  /// Create the geometry of the highlight
  void init_();

  osg::ref_ptr<osg::Uniform> color_;
  osg::ref_ptr<osg::Uniform> radius_;
};

} // namespace simVis

#endif // SIMVIS_AREA_HIGHLIGHT_H
