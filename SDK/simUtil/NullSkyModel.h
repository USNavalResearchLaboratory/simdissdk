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
#ifndef SIMUTIL_NULLSKYMODEL_H
#define SIMUTIL_NULLSKYMODEL_H

#include "osg/Light"
#include "osgEarth/Sky"

namespace simUtil {

/**
 * Null Object pattern instance for a Sky Model.  Useful to prevent the Simple Sky
 * model from being used in views.
 */
class /*SIMUTIL_EXPORT*/ NullSkyModel : public osgEarth::SkyNode  // Header-only
{
public:
  NullSkyModel()
    : light_(new osg::Light)
  {
  }

  /** Returns a sun light (supports after 2.8, when signature on pure virtual was changed */
  virtual osg::Light* getSunLight() const
  {
    return light_.get();
  }

  /** Return a sun light (supports up to and including 2.8 -- changed post-2.8) */
  virtual osg::Light* getSunLight()
  {
    return light_.get();
  }

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }

  /** Return the class name */
  virtual const char* className() const { return "NullSkyModel"; }

private:
  // Need a light to avoid crash in some osgEarth versions
  osg::ref_ptr<osg::Light> light_;
};

}

#endif /* SIMUTIL_NULLSKYMODEL_H */
