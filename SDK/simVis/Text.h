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
#ifndef SIMVIS_TEXT_H
#define SIMVIS_TEXT_H

/**@file
* Replacement for osgText::Text that fixes a bug in screen-coords character size
*/
#include "simCore/Common/Common.h"

#include "osgText/Text"

namespace simVis
{
  /**
  * Replacement for osgText::Text class that addresses a sizing bug when
  * using the SCREEN_COORDS mode.
  */
  class SDKVIS_EXPORT Text : public osgText::Text
  {
  public:
    Text() : osgText::Text() { }
    Text(const Text& text,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) : osgText::Text(text, copyop) { }

    virtual osg::Object* cloneType() const { return new Text(); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new Text(*this,copyop); }
    virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const Text*>(obj)!=NULL; }
    virtual const char* className() const { return "Text"; }
    virtual const char* libraryName() const { return "simVis"; }

  public: // osgText::Text
    virtual void computePositions(unsigned int contextID) const;

  protected:
      virtual ~Text() { }
  };

} // namespace simVis

#endif // SIMVIS_TEXT_H

