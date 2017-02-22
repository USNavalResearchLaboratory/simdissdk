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
#include <osgText/Font>
#include "simVis/View.h"
#include "simVis/Utils.h"
#include "simVis/Registry.h"
#include "ClassificationBanner.h"

namespace simVis
{

/** Data Store listener that updates the classification label contents when scenario properties change */
class ClassificationBanner::ScenarioListenerImpl : public simData::DataStore::ScenarioListener
{
public:
  /** Constructor */
  explicit ScenarioListenerImpl(ClassificationBanner* parent) :parent_(parent) {}
  virtual ~ScenarioListenerImpl(){}

  /** Updates label when the scenario properties change */
  void onScenarioPropertiesChange(simData::DataStore *source)
  {
    parent_->updateClassLabel_();
  }

private:
  ClassificationBanner* parent_;
};

//////////////////////////////////////////////

ClassificationBanner::ClassificationBanner(simData::DataStore* dataStore, unsigned int fontSize, const std::string& fontFile)
  :dataStore_(dataStore),
  fontSize_(fontSize),
  fontFile_(fontFile)
{
  createClassLabels_();
  listener_ = simData::DataStore::ScenarioListenerPtr(new ScenarioListenerImpl(this));
  if (dataStore_)
    dataStore_->addScenarioListener(listener_);
}

ClassificationBanner::~ClassificationBanner()
{
  if (dataStore_)
    dataStore_->removeScenarioListener(listener_);
}

void ClassificationBanner::addToView(simVis::View* managedView)
{
  updateClassLabel_(); // update before adding
  if (managedView) // only add if we have main view
  {
    managedView->addOverlayControl(classLabelUpper_);
    managedView->addOverlayControl(classLabelLower_);
  }
}

void ClassificationBanner::removeFromView(simVis::View* managedView)
{
  if (managedView) // only remove if we have main view
  {
    managedView->removeOverlayControl(classLabelUpper_);
    managedView->removeOverlayControl(classLabelLower_);
  }
}

void ClassificationBanner::setFontFile(const std::string& fontFile)
{
  fontFile_ = fontFile;
  osgText::Font* foundFont = simVis::Registry::instance()->getOrCreateFont(fontFile_);
  if (classLabelUpper_)
    classLabelUpper_->setFont(foundFont);
  if (classLabelLower_)
    classLabelLower_->setFont(foundFont);
}

void ClassificationBanner::setFontSize(unsigned int fontSize)
{
  fontSize_ = fontSize;
  if (classLabelUpper_)
    classLabelUpper_->setFontSize(simVis::osgFontSize(fontSize_));
  if (classLabelLower_)
    classLabelLower_->setFontSize(simVis::osgFontSize(fontSize_));
}

void ClassificationBanner::createClassLabels_()
{
  // get current classification
  std::string classLabel;
  osg::Vec4f classColor;
  getCurrentClassification_(classLabel, classColor);
  // load default font, make sure we found the file
  osgText::Font* foundFont = simVis::Registry::instance()->getOrCreateFont(fontFile_);
  assert(foundFont);

  // create upper and lower label controls
  classLabelUpper_ = createControl_(classLabel, classColor, foundFont, osgEarth::Util::Controls::Control::ALIGN_TOP);
  classLabelUpper_->setName("Classification Banner Upper");
  classLabelLower_ = createControl_(classLabel, classColor, foundFont, osgEarth::Util::Controls::Control::ALIGN_BOTTOM);
  classLabelLower_->setName("Classification Banner Lower");
}

osgEarth::Util::Controls::LabelControl* ClassificationBanner::createControl_(const std::string& classLabel,
  const osg::Vec4f& classColor,
  osgText::Font* fontFile,
  osgEarth::Util::Controls::Control::Alignment vertAlign) const
{
  osgEarth::Util::Controls::LabelControl* classControl = new osgEarth::Util::Controls::LabelControl(classLabel, simVis::osgFontSize(fontSize_), classColor);
  classControl->setHaloColor(osg::Vec4f(0.f, 0.f, 0.f, classColor.a()));
  classControl->setHorizAlign(osgEarth::Util::Controls::Control::ALIGN_CENTER);
  classControl->setVertAlign(vertAlign);
  classControl->setPadding(0);
  classControl->setPadding(osgEarth::Util::Controls::Control::SIDE_TOP, 10);
  classControl->setPadding(osgEarth::Util::Controls::Control::SIDE_BOTTOM, 10);
  classControl->setFont(fontFile);
  classControl->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
  classControl->setTextBackdropType(osgText::Text::OUTLINE);
  return classControl;
}

void ClassificationBanner::getCurrentClassification_(std::string& classLabel, osg::Vec4f& classColor)
{
  if (!dataStore_)
    return;
  simData::DataStore::Transaction transaction;
  const simData::ScenarioProperties_Classification& classification =
    dataStore_->scenarioProperties(&transaction)->classification();
  classLabel = classification.label();
  classColor = simVis::ColorUtils::RgbaToVec4(classification.fontcolor());
}

void ClassificationBanner::updateClassLabel_()
{
  // get current classification
  std::string classLabel;
  osg::Vec4f classColor;
  getCurrentClassification_(classLabel, classColor);
  if (classLabel.empty())
    return;
  if (!classLabelUpper_.valid()) // assume upper and lower are always same state
    return;

  if (classLabelUpper_->text() != classLabel)
    classLabelUpper_->setText(classLabel);
  if (classLabelUpper_->foreColor() != classColor)
  {
    classLabelUpper_->setForeColor(classColor);
    classLabelUpper_->setHaloColor(osg::Vec4f(0.f, 0.f, 0.f, classColor.a()));
  }
  if (classLabelLower_->text() != classLabel)
    classLabelLower_->setText(classLabel);
  if (classLabelLower_->foreColor() != classColor)
  {
    classLabelLower_->setForeColor(classColor);
    classLabelLower_->setHaloColor(osg::Vec4f(0.f, 0.f, 0.f, classColor.a()));
  }
}

} // namespace simVis
