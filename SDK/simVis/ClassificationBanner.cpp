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
#include "osgText/Font"
#include "osgText/Text"
#include "simCore/Calc/Math.h"
#include "simVis/DevicePixelRatioUtils.h"
#include "simVis/Registry.h"
#include "simVis/Utils.h"
#include "simVis/View.h"
#include "simVis/ClassificationBanner.h"

namespace simVis
{

//////////////////////////////////////////////

SetToClassificationTextCallback::SetToClassificationTextCallback(osgText::Text* text)
  : text_(text)
{
}

void SetToClassificationTextCallback::onScenarioPropertiesChange(simData::DataStore* source)
{
  // Nothing to do
  osg::ref_ptr<osgText::Text> text;
  if (!source || !text_.lock(text))
    return;

  // Pull out the properties
  simData::DataStore::Transaction txn;
  const simData::ScenarioProperties* props = source->scenarioProperties(&txn);
  text->setText(props->classification().label());
  text->setColor(simVis::ColorUtils::RgbaToVec4(props->classification().fontcolor()));
  txn.release(&props);
}

//////////////////////////////////////////////

/// Classification banner outline thickness
static const float OUTLINE_THICKNESS = 0.03f;

ClassificationLabelNode::ClassificationLabelNode()
  : dataStore_(nullptr)
{
  listener_.reset(new simVis::SetToClassificationTextCallback(this));

  // Configure text defaults that are good for classification strings
  setFont("arialbd.ttf");

  // Scale font resolution for crispness
  const double dpr = simVis::DevicePixelRatioUtils::getDpr();
  unsigned int res = static_cast<unsigned int>(simCore::sdkMax(32.0, 24.0 * dpr));
  setFontResolution(res, res);

  setCharacterSize(simVis::osgFontSize(24.f));
  setAxisAlignment(osgText::TextBase::SCREEN);
  setBackdropType(osgText::Text::OUTLINE);
  setBackdropColor(osg::Vec4f(0.f, 0.f, 0.f, 1.f));
  setBackdropOffset(OUTLINE_THICKNESS);
  setDataVariance(osg::Object::DYNAMIC);
}

ClassificationLabelNode::ClassificationLabelNode(const ClassificationLabelNode& node, const osg::CopyOp& copyop)
  : Text(node, copyop),
    dataStore_(nullptr)
{
  listener_.reset(new simVis::SetToClassificationTextCallback(this));
  bindTo(node.dataStore_);
}

ClassificationLabelNode::~ClassificationLabelNode()
{
}

void ClassificationLabelNode::bindTo(simData::DataStore* ds)
{
  if (ds == dataStore_)
    return;
  if (dataStore_)
    dataStore_->removeScenarioListener(listener_);
  dataStore_ = ds;
  if (dataStore_)
  {
    // Add the listener, then update the text
    dataStore_->addScenarioListener(listener_);
    listener_->onScenarioPropertiesChange(dataStore_);
  }
}

//////////////////////////////////////////////

class ClassificationBanner::FrameResizeCallback : public osg::NodeCallback
{
public:
  /** Constructor */
  explicit FrameResizeCallback(ClassificationBanner* parent) :parent_(parent) {}
  virtual ~FrameResizeCallback(){}

  /** Updates banner positions when the screen size changes */
  void operator()(osg::Node *node, osg::NodeVisitor *nv) override
  {
    if (nv->getVisitorType() != osg::NodeVisitor::CULL_VISITOR)
    {
      traverse(node, nv);
      return;
    }

    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
    if (!cv)
    {
      traverse(node, nv);
      return;
    }

    // Since this is drawn in the HUD camera, the viewport represents physical pixels,
    // but the coordinate system of this node is logical.
    const double dpr = simVis::DevicePixelRatioUtils::getDpr();
    const double logicalWidth = cv->getViewport()->width() / dpr;
    const double logicalHeight = cv->getViewport()->height() / dpr;

    // Banners should be horizontally centered and ten logical pixels from the top and bottom
    const osg::Vec3 topLocalPos(logicalWidth / 2.0, logicalHeight - 10.0, 0.0);
    const osg::Vec3 bottomLocalPos(logicalWidth / 2.0, 10.0, 0.0);

    // Only update if it actually moved (prevents constant dirtying of geometry)
    if (parent_->classLabelUpper_->getPosition() != topLocalPos)
    {
      parent_->classLabelUpper_->setPosition(topLocalPos);
      parent_->classLabelLower_->setPosition(bottomLocalPos);
    }

    traverse(node, nv);
  }

private:
  ClassificationBanner* parent_ = nullptr;
};

//////////////////////////////////////////////

ClassificationBanner::ClassificationBanner(simData::DataStore* dataStore, unsigned int fontSize, const std::string& fontFile)
  : osg::Group(),
    classLabelUpper_(new ClassificationLabelNode),
    classLabelLower_(new ClassificationLabelNode)
{
  setFontFile(fontFile);
  setFontSize(fontSize);

  // Configure the upper label
  classLabelUpper_->setName("Classification Banner Upper");
  classLabelUpper_->bindTo(dataStore);
  classLabelUpper_->setAlignment(osgText::Text::CENTER_TOP);
  addChild(classLabelUpper_.get());

  // Configure the lower label
  classLabelLower_->setName("Classification Banner Lower");
  classLabelLower_->bindTo(dataStore);
  classLabelLower_->setAlignment(osgText::Text::CENTER_BOTTOM);
  addChild(classLabelLower_.get());

  resizeCallback_ = new FrameResizeCallback(this);
  addCullCallback(resizeCallback_.get());
}

ClassificationBanner::~ClassificationBanner()
{
  classLabelUpper_->bindTo(nullptr);
  classLabelLower_->bindTo(nullptr);
}

void ClassificationBanner::addToView(simVis::View* managedView)
{
  if (managedView)
    managedView->getOrCreateHUD()->addChild(this);
}

void ClassificationBanner::removeFromView(simVis::View* managedView)
{
  if (managedView)
    managedView->getOrCreateHUD()->removeChild(this);
}

void ClassificationBanner::setFontFile(const std::string& fontFile)
{
  osgText::Font* foundFont = simVis::Registry::instance()->getOrCreateFont(fontFile);
  if (foundFont)
  {
    classLabelUpper_->setFont(foundFont);
    classLabelLower_->setFont(foundFont);
  }
}

void ClassificationBanner::setFontSize(unsigned int fontSize)
{
  // Scale font resolution for crispness
  const double dpr = simVis::DevicePixelRatioUtils::getDpr();
  unsigned int res = static_cast<unsigned int>(simCore::sdkMax(32.0, static_cast<double>(fontSize) * dpr));

  classLabelUpper_->setFontResolution(res, res);
  classLabelLower_->setFontResolution(res, res);

  classLabelUpper_->setCharacterSize(simVis::osgFontSize(fontSize));
  classLabelLower_->setCharacterSize(simVis::osgFontSize(fontSize));
}

} // namespace simVis
