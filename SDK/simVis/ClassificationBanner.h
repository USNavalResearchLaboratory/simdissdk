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
#include <osgText/Text>
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
  class SDKVIS_EXPORT ClassificationBanner : public osg::Group
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

    /** Set the font file of the banner, can include full path (e.g. "arial.ttf", "full/path/to/arialbd.ttf") */
    void setFontFile(const std::string& fontFile);

    /** Set the font size of the banner */
    void setFontSize(unsigned int fontSize);

  private:
    /** Class implements data store listener */
    class ScenarioListenerImpl;
    /** Callback that checks for screen resize on each frame and notifies banner if needed */
    class FrameResizeCallback;

    /** Build the label objects */
    void createClassLabels_();

    /** Set the position of the top classification label */
    void setTopPosition_(const osg::Vec3& topPos);

    /** Set the position of the bottom classification label */
    void setBottomPosition_(const osg::Vec3& bottomPos);

    /** Create a classification banner label control */
    osgText::Text* createText_(const std::string& classLabel,
      const osg::Vec4f& classColor,
      osgText::Font* fontFile,
      osgText::Text::AlignmentType alignment) const;

    /** Retrieves the current classification label and color from the DataStore */
    void getCurrentClassification_(std::string& classLabel, osg::Vec4f& classColor);

    /** Update the ClassificationLabels on callback from the data store */
    void updateClassLabel_();

    simData::DataStore*            dataStore_;                              ///< Reference to the data store
    unsigned int fontSize_;
    std::string fontFile_;
    osg::ref_ptr<osgText::Text> classLabelUpper_;
    osg::ref_ptr<osgText::Text> classLabelLower_;
    simData::DataStore::ScenarioListenerPtr listener_;                      ///< Listener to the data store
    /// Callback to reposition the classification banners when screen size changes
    osg::ref_ptr<FrameResizeCallback> resizeCallback_;
  };

} // namespace simVis

#endif // SIMVIS_CLASSIFICATION_BANNER_H
