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
 * not receive a LICENSE.txt with this code, email simdis@us.navy.mil.
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
#include "osg/Version"
#include "osgText/Text"
#include "simCore/Common/Common.h"

namespace simVis
{
  /**
  * Replacement for osgText::Text class that addresses a sizing bug when
  * using the SCREEN_COORDS mode.
  */
  class [[deprecated("Deprecated, use osgText::Text instead")]] SDKVIS_EXPORT Text : public osgText::Text
  {
  public:
    Text() : osgText::Text(), x_(0.0f), y_(0.0f) { }
    Text(const Text& text, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY) : osgText::Text(text, copyop), x_(0.0f), y_(0.0f) { }

    virtual osg::Object* cloneType() const { return new Text(); }
    virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new Text(*this,copyop); }
    virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const Text*>(obj)!=nullptr; }
    virtual const char* className() const { return "Text"; }
    virtual const char* libraryName() const { return "simVis"; }

    /**
     * Adds an offset, in screen coordinates, to text
     * @param x Adds to the x coordinate, positive values moves the text to the right
     * @param x Adds to the y coordinate, positive values moves the text up
     */
    void setScreenOffset(float x, float y);

  public: // osgText::Text
#if OSG_VERSION_LESS_THAN(3,5,0)
    virtual void computePositions(unsigned int contextID) const;
#endif

  protected:
    virtual ~Text() { }

  private:
    float x_;
    float y_;
  };

} // namespace simVis

#endif // SIMVIS_TEXT_H
