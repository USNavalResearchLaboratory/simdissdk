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
#ifndef SIMVIS_CLASSIFICATION_BANNER_H
#define SIMVIS_CLASSIFICATION_BANNER_H

#include <osg/ref_ptr>
#include <osg/Vec4f>
#include <osgEarthUtil/Controls>
#include "simData/DataStore.h"

namespace osgText { class Font; }
namespace osgEarth { namespace Util { namespace Controls {
  class ControlCanvas;
  class LabelControl;
} } }
namespace osgViewer { class View; }

namespace simVis
{
  class View;

  /**
  * Creates the ClassificationBanner, and keeps it synchronized with the DataStore
  */
  class SDKVIS_EXPORT ClassificationBanner
  {

  public:

    /**
    * Constructs a new ClassificationBanner
    * @param dataStore  ptr to the current data store
    * @param fontSize  point size of the font to display
    * @param fontFile  file name of the font to display, can include full path (e.g. "arial.ttf", "full/path/to/arialbd.ttf")
    */
    ClassificationBanner(simData::DataStore* dataStore, unsigned int fontSize, const std::string& fontFile);

    /** Destructor */
    virtual ~ClassificationBanner();

    /**
    * Add the ClassificationBanner to a managed view
    * @param managedView
    */
    void addToView(simVis::View* managedView);

    /**
    * Remove the ClassificationBanner from a managed view
    * @param managedView
    */
    void removeFromView(simVis::View* managedView);

    /**
    * Add the ClassificationBanner to a control canvas
    * @param controlCanvas
    */
    void addToView(osgEarth::Util::Controls::ControlCanvas* controlCanvas);

    /**
    * Remove the ClassificationBanner from a control canvas
    * @param controlCanvas
    */
    void removeFromView(osgEarth::Util::Controls::ControlCanvas* controlCanvas);

    /** Set the font file of the banner, can include full path (e.g. "arial.ttf", "full/path/to/arialbd.ttf") */
    void setFontFile(const std::string& fontFile);

    /** Set the font size of the banner */
    void setFontSize(unsigned int fontSize);

  private:
    /** Class implements data store listener */
    class ScenarioListenerImpl;

    /** Build the label objects */
    void createClassLabels_();

    /** Create a classification banner label control */
    osgEarth::Util::Controls::LabelControl* createControl_(const std::string& classLabel,
      const osg::Vec4f& classColor,
      osgText::Font* fontFile,
      osgEarth::Util::Controls::Control::Alignment vertAlign) const;

    /** Retrieves the current classification label and color from the DataStore */
    void getCurrentClassification_(std::string& classLabel, osg::Vec4f& classColor);

    /** Update the ClassificationLabels on callback from the data store */
    void updateClassLabel_();

    simData::DataStore*            dataStore_;                              ///< Reference to the data store
    unsigned int fontSize_;
    std::string fontFile_;
    osg::ref_ptr<osgEarth::Util::Controls::LabelControl> classLabelUpper_;  ///< Upper classification label
    osg::ref_ptr<osgEarth::Util::Controls::LabelControl> classLabelLower_;  ///< Lower classification label
    simData::DataStore::ScenarioListenerPtr listener_;                      ///< Listener to the data store
  };

} // namespace simVis

#endif // SIMVIS_CLASSIFICATION_BANNER_H
