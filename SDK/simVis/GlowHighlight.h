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
 * not receive a LICENSE.txt with this code, email simdis@nrl.navy.mil.
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#ifndef SIMVIS_GLOWHIGHLIGHT_H
#define SIMVIS_GLOWHIGHLIGHT_H

#include "osg/observer_ptr"
#include "osg/StateSet"
#include "osg/Referenced"
#include "osg/NodeCallback"
#include "simVis/Types.h"
#include "simCore/Common/Common.h"

namespace osg { class StateSet; }

namespace simVis
{

/**
 * Responsible for applying to a node a highlight.  The highlight display corresponds to Circle
 * Highlight features.  Note that in the current state, there is no circle highlight, and instead the
 * highlight simply applies the color in varying shades of brightness to the model.
 */
class SDKVIS_EXPORT GlowHighlight : public osg::Referenced
{
public:
  /** Initialize on the given node (and implicitly its state set) */
  GlowHighlight(osg::Node* onNode);

  /** Changes the highlight color */
  void setColor(const simVis::Color& color);
  /** Retrieve the highlight color */
  simVis::Color color() const;

  /** Changes whether highlight is shown */
  void setEnabled(bool enable);
  /** Retrieves whether highlight is enabled */
  bool enabled() const;

protected:
  /** osg::Referenced-derived; cleans up */
  virtual ~GlowHighlight();

private:
  /** Creates the shader, lazily */
  void createShader_();

  /** Node on which we have our update callback, who hosts the virtual program in its stateset */
  osg::observer_ptr<osg::Node> node_;
  /** State set of the node */
  osg::observer_ptr<osg::StateSet> stateSet_;

  /** Color of the highlight */
  simVis::Color color_;
  /** Enabled flag for the highlight */
  bool enabled_;
  /** Cache from the registry as to whether this feature is supported by the graphics card */
  bool supported_;
  /** Flags true when the shader has been created, for lazy initialization */
  bool shaderCreated_;
};

}

#endif /* SIMVIS_GLOWHIGHLIGHT_H */
