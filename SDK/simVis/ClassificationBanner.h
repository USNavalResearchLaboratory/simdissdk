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
#ifndef SIMVIS_CLASSIFICATION_BANNER_H
#define SIMVIS_CLASSIFICATION_BANNER_H

#include "osg/observer_ptr"
#include "osg/ref_ptr"
#include "osgText/Text"
#include "simData/DataStore.h"

namespace simVis
{

class View;

/** Given a text, updates its text and color to data store's classification fields. */
class SDKVIS_EXPORT SetToClassificationTextCallback : public simData::DataStore::ScenarioListener
{
public:
  /** Construct to update a text string */
  explicit SetToClassificationTextCallback(osgText::Text* text);
  /** Override from ScenarioListener. */
  virtual void onScenarioPropertiesChange(simData::DataStore* source);

private:
  osg::observer_ptr<osgText::Text> text_;
};

/**
 * osgText::Text specialization that defaults the settings to look like a SIMDIS classification
 * string, and provides utility methods to bind to a data store.  Like most nodes in OSG, the
 * position of this node is controlled externally and not internally.
 */
class SDKVIS_EXPORT ClassificationLabelNode : public osgText::Text
{
public:
  /** Constructs a Text with default settings that look like SIMDIS classification text. */
  ClassificationLabelNode();
  /** OSG-like copy constructor. */
  ClassificationLabelNode(const ClassificationLabelNode& node, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

  /**
   * Binds the label to the data store, so that the label's content and color represents
   * the values in the data store's scenario properties.  Pass in nullptr to unbind.  The
   * label can only be bound to a single data store.
   * @param ds Data store that supplies the classification text and color through scenario
   *    properties.  Use nullptr to unbind from the data store.
   */
  void bindTo(simData::DataStore* ds);

  // Override osgText::Text methods
  virtual osg::Object* cloneType() const { return new ClassificationLabelNode(); }
  virtual osg::Object* clone(const osg::CopyOp& copyop) const { return new ClassificationLabelNode(*this, copyop); }
  virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const ClassificationLabelNode*>(obj) != nullptr; }
  virtual const char* className() const { return "ClassificationLabelNode"; }
  virtual const char* libraryName() const { return "simVis"; }

protected:
  /** Virtual destructor protected to avoid ref_ptr double delete issues. */
  virtual ~ClassificationLabelNode();

private:
  simData::DataStore* dataStore_;
  std::shared_ptr<SetToClassificationTextCallback> listener_;
};

/**
 * Keeps two classification banner texts, synchronized with the data store, and aligns
 * them along the top and bottom of the screen.
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

  /**
   * Add the ClassificationBanner to a managed view
   * @param managedView View onto which to add the classification
   */
  void addToView(simVis::View* managedView);

  /**
   * Remove the ClassificationBanner from a managed view
   * @param managedView View from which to remove the classification
   */
  void removeFromView(simVis::View* managedView);

  /** Set the font file of the banner, can include full path (e.g. "arial.ttf", "full/path/to/arialbd.ttf") */
  void setFontFile(const std::string& fontFile);

  /** Set the font size of the banner */
  void setFontSize(unsigned int fontSize);

protected:
  /** Destructor is protected to avoid ref_ptr double delete issues. */
  virtual ~ClassificationBanner();

private:
  /** Callback that checks for screen resize on each frame and notifies banner if needed */
  class FrameResizeCallback;

  /// Reference to the data store
  simData::DataStore* dataStore_;
  osg::ref_ptr<ClassificationLabelNode> classLabelUpper_;
  osg::ref_ptr<ClassificationLabelNode> classLabelLower_;
  /// Callback to reposition the classification banners when screen size changes
  osg::ref_ptr<FrameResizeCallback> resizeCallback_;
};

} // namespace simVis

#endif // SIMVIS_CLASSIFICATION_BANNER_H
