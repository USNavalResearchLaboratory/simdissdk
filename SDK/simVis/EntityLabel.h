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
#ifndef SIMVIS_ENTITY_LABEL_H
#define SIMVIS_ENTITY_LABEL_H

#include <string>
#include "osg/Referenced"
#include "osg/ref_ptr"
// osg::ref_ptr does not play nicely with forward declarations in the SDK DLL build
#include "osgEarthAnnotation/LabelNode"
#include "simData/DataTypes.h"

namespace osg { class Group; }
namespace simVis
{
/// Class for managing the labels; using osg::Referenced purely for smart pointer, as this class is not added to the scenegraph
class SDKVIS_EXPORT EntityLabelNode : public osg::Referenced
{
public:
  /// Constructor
  explicit EntityLabelNode(osg::Group* root);

  /// Update the label with the given preferences and text
  void update(const simData::CommonPrefs& commonPrefs, const std::string& text, float zOffset=0.f);

  /// @see osg::Node::addCullCallback()
  void addCullCallback(osg::Callback* callback);

protected:
  virtual ~EntityLabelNode();

private:
  osg::ref_ptr<osg::Group> root_;
  osg::ref_ptr<osgEarth::Annotation::LabelNode> label_;  ///< The actual label
  simData::CommonPrefs lastCommonPrefs_;  ///< The last preferences to check for changes
  bool hasLastPrefs_; ///< Whether lastCommonPrefs_ has been set by prefs we received
  std::string lastText_; ///< The last text to check for change
};

}

#endif

