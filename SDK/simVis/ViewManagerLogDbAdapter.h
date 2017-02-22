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
#ifndef SIMVIS_VIEWMANAGERLOGDBADAPTER_H
#define SIMVIS_VIEWMANAGERLOGDBADAPTER_H

#include <vector>
#include "osg/Referenced"
#include "osg/observer_ptr"
#include "osg/ref_ptr"
#include "simCore/Common/Common.h"
#include "simVis/ViewManager.h"

namespace osgEarth { namespace Util { class LogarithmicDepthBuffer; } }

namespace simVis
{

/**
 * Responsible for applying the Logarithmic Depth Buffer to views in a View Manager.
 * The Logarithmic Depth Buffer is an osgEarth utility that scales the culling range
 * logarithmically, permitting a better render on both close and far objects without
 * as much Z-fighting.
 */
class SDKVIS_EXPORT ViewManagerLogDbAdapter : public osg::Referenced
{
public:
  ViewManagerLogDbAdapter();

  /** Installs the LDB on the given view manager */
  void install(simVis::ViewManager* viewManager);
  /** Removes the LDB from the given view manager */
  void uninstall(simVis::ViewManager* viewManager);

  /** Returns true if installed on any view manager */
  bool isInstalled() const;
  /** Returns true if installed on given view manager */
  bool isInstalled(const simVis::ViewManager* viewManager) const;

protected:
  /** Derived from osg::Referenced */
  virtual ~ViewManagerLogDbAdapter();

private:
  /** Applies a logarithmic depth buffer when installed to view managers */
  osgEarth::Util::LogarithmicDepthBuffer* logDepthBuffer_;

  /** List of all view managers installed on */
  typedef std::vector<osg::observer_ptr<simVis::ViewManager> > ViewManagerList;
  /** List of all view managers installed on */
  ViewManagerList viewManagers_;

  /** Callback registered with view managers to insert and remove LDB (note: must come after LDB) */
  osg::ref_ptr<ViewManager::Callback> installCallback_;
};

}

#endif /* SIMVIS_VIEWMANAGERLOGDBADAPTER_H */
