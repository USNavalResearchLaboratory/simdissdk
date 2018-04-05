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
#include "osg/Image"
#include "osgDB/FileUtils"

#include "simVis/View.h"
#include "simVis/Utils.h"
#include "simVis/Registry.h"
#include "simCore/String/Tokenizer.h"
#include "simCore/Calc/Math.h"
#include "simUtil/HudManager.h"

namespace simUtil {

/// Helper class to calculate the extent of multiple text fields
class HudTextAdapter::TextExtent
{
public:
  TextExtent()
    : minX_(std::numeric_limits<float>::max()),
      minY_(std::numeric_limits<float>::max()),
      maxX_(-std::numeric_limits<float>::max()),
      maxY_(-std::numeric_limits<float>::max())
  {
  }

  virtual ~TextExtent()
  {
  }

  /// Clears the values and gets ready for the next calculation
  void clear()
  {
    minX_ = std::numeric_limits<float>::max();
    minY_ = std::numeric_limits<float>::max();
    maxX_ = -std::numeric_limits<float>::max();
    maxY_ = -std::numeric_limits<float>::max();
  }

  /// Adds a text area to the calculation
  void add(const osg::BoundingBox& box)
  {
    if (box.xMin() < minX_)
      minX_ = box.xMin();

    if (box.yMin() < minY_)
      minY_ = box.yMin();

    if (box.xMax() > maxX_)
      maxX_ = box.xMax();

    if (box.yMax() > maxY_)
      maxY_ = box.yMax();
  }

  /// Returns the size in pixels.
  void getSize(int& width, int& height)
  {
    if (minX_ == std::numeric_limits<float>::max())
    {
      width = 0;
      height = 0;
    }
    else
    {
      width = static_cast<int>(simCore::round(maxX_ - minX_));
      height = static_cast<int>(simCore::round(maxY_ - minY_));
    }
  }

private:
  float minX_;  ///< Lower Left corner in pixels, it is a float to match osg
  float minY_;  ///< Lower Left corner in pixels, it is a float to match osg
  float maxX_;  ///< Upper Right corner in pixels, it is a float to match osg
  float maxY_;  ///< Upper Right corner in pixels, it is a float to match osg
};

HudText::HudText()
  : osg::Geode()
{
  setName("simUtil::HudText");
}

//----------------------------------------------------------------------------
HudTextAdapter::HudTextAdapter(int width, int height)
  : HudText(),
    windowWidth_(width),
    windowHeight_(height),
    x_(0),
    y_(0),
    percentageX_(true),
    percentageY_(true),
    hAlign_(ALIGN_LEFT),
    vAlign_(ALIGN_BOTTOM),
    color_(1.0, 1.0, 1.0, 1.0),
    requestedFont_("arial.ttf"),
    requestedFontSize_(10.0),
    currentFont_("arial.ttf"),
    currentFontSize_(10.0),
    visible_(true),
    extent_(new HudTextAdapter::TextExtent()),
    backdrop_(osgText::Text::DROP_SHADOW_BOTTOM_RIGHT),
    backdropOffset_(0.07)
{
  setName("simUtil::HudTextAdapter");
}

HudTextAdapter::~HudTextAdapter()
{
  for (std::vector< osg::ref_ptr<osgText::Text> >::const_iterator it = osgTextVector_.begin(); it != osgTextVector_.end(); ++it)
    removeDrawable((*it).get());

  delete extent_;
}

void HudTextAdapter::update_()
{
  if (!visible_)
    return;

  extent_->clear();

  std::vector<std::string> tokens;
  // Empty out the tokens if alpha is blank, to avoid shadow-without-text issue
  if (color_.a() != 0)
    tokenize_(text_, tokens);

  for (size_t ii = 0; ii != tokens.size(); ++ii)
  {
    osg::ref_ptr<osgText::Text> osgText;
    if (osgTextVector_.size() <= ii)
    {
      osgText = new osgText::Text();
      osgTextVector_.push_back(osgText);
      addDrawable(osgText.get());

      osgText->setFont(simVis::Registry::instance()->getOrCreateFont(requestedFont_));
      // Set a minimum font size so that large text doesn't invoke magnification filtering
      if (requestedFontSize_ > 32)
        osgText->setFontResolution(static_cast<unsigned int>(requestedFontSize_), static_cast<unsigned int>(requestedFontSize_));
      osgText->setCharacterSize(requestedFontSize_);
      osgText->setPosition(osg::Vec3(0.0f, 0.0f, 0.0f));
      osgText->setColor(color_);
      osgText->setEnableDepthWrites(false); // No depth buffering needed
      // Set up the Halo
      osgText->setBackdropType(backdrop_);
      osgText->setBackdropOffset(backdropOffset_);
      osgText->setBackdropColor(osg::Vec4(0.f, 0.f, 0.f, 1.f));
      osgText->setBackdropImplementation(osgText::Text::DELAYED_DEPTH_WRITES);
      initializeText_(osgText.get());
    }
    else
    {
      osgText = osgTextVector_[ii];

      // It only sets the font if it has changed.  Constantly setting the font to the same value shows up as a hot spot.
      if (requestedFont_ != currentFont_)
      {
        osgText->setFont(simVis::Registry::instance()->getOrCreateFont(requestedFont_));
      }

      if (requestedFontSize_ != currentFontSize_)
      {
        osgText->setCharacterSize(requestedFontSize_);
        // Set a minimum font size so that large text doesn't invoke magnification filtering
        if (requestedFontSize_ > 32)
          osgText->setFontResolution(static_cast<unsigned int>(requestedFontSize_), static_cast<unsigned int>(requestedFontSize_));
      }

      // Update the color of the text
      if (osgText->getColor() != color_)
        osgText->setColor(color_);

      // Set up the backdrop; use a temporary so we can use NONE as needed
      osgText::Text::BackdropType backdropType = backdrop_;
      if (backdropOffset_ <= 0.f)
        backdropType = osgText::Text::NONE;
      if (osgText->getBackdropType() != backdropType)
        osgText->setBackdropType(backdropType);
      if (!simCore::areEqual(osgText->getBackdropHorizontalOffset(), backdropOffset_))
        osgText->setBackdropOffset(backdropOffset_);
    }

    osgText->setText(tokens[ii]);
    positionText_(ii, osgText.get());
#if OSG_VERSION_GREATER_OR_EQUAL(3,3,2)
    extent_->add(osgText->getBoundingBox());
#else
    extent_->add(osgText->getBound());
#endif
  }

  // After processing all the tokens it is safe to update the font information
  currentFont_ = requestedFont_;
  currentFontSize_ = requestedFontSize_;

  /// Remove any extras; vector should be small so should not be a performance concern
  while (osgTextVector_.size() > tokens.size())
  {
    osg::ref_ptr<osgText::Text> osgText = osgTextVector_.back();
    removeDrawable(osgText.get());
    osgTextVector_.pop_back();
  }
}

void HudTextAdapter::update(const std::string& text, double x, double y, bool percentageX, bool percentageY, Alignment hAlign, Alignment vAlign, const osg::Vec4& color, const std::string& font, double fontSize)
{
  if ((text_ == text) &&
      (x_ == x) &&
      (y_ == y) &&
      (percentageX_ == percentageX) &&
      (percentageY_ == percentageY) &&
      (hAlign_ == hAlign) &&
      (vAlign_ == vAlign) &&
      (color_ == color) &&
      (requestedFont_ == font) &&
      (requestedFontSize_ == fontSize))
    return;

  text_ = text;
  x_ = x;
  y_ = y;
  percentageX_ = percentageX;
  percentageY_ = percentageY;
  hAlign_ = hAlign;
  vAlign_ = vAlign;
  color_ = color;
  requestedFont_ = font;
  requestedFontSize_ = fontSize;

  update_();
}

void HudTextAdapter::resize(int width, int height)
{
  if ((windowWidth_ == width) && (windowHeight_ == height))
    return;

  windowWidth_ = width;
  windowHeight_ = height;
  update_();
}

std::string HudTextAdapter::text() const
{
  return text_;
}

void HudTextAdapter::setText(const std::string& text)
{
  if (text != text_)
  {
    text_ = text;
    update_();
  }
}

double HudTextAdapter::x() const
{
  return x_;
}

double HudTextAdapter::y() const
{
  return y_;
}

bool HudTextAdapter::isPercentageX() const
{
  return percentageX_;
}

bool HudTextAdapter::isPercentageY() const
{
  return percentageY_;
}

void HudTextAdapter::setPosition(double x, double y, bool percentageX, bool percentageY)
{
  if ((x != x_) || (y != y_) || (percentageX != percentageX_) || (percentageY != percentageY_))
  {
    x_ = x;
    y_ = y;
    percentageX_ = percentageX;
    percentageY_ = percentageY;
    update_();
  }
}

std::string HudTextAdapter::font() const
{
  return currentFont_;
}

double HudTextAdapter::fontSize() const
{
  return currentFontSize_;
}

void HudTextAdapter::setFont(const std::string& font, double size)
{
  if ((font != currentFont_) || (size != currentFontSize_))
  {
    requestedFont_ = font;
    requestedFontSize_ = size;
    update_();
  }
}

Alignment HudTextAdapter::hAlignment() const
{
  return hAlign_;
}

Alignment HudTextAdapter::vAlignment() const
{
  return vAlign_;
}

void HudTextAdapter::setAlignment(Alignment hAlign, Alignment vAlign)
{
  if ((hAlign != hAlign_) || (vAlign != vAlign_))
  {
    hAlign_ = hAlign;
    vAlign_ = vAlign;
    update_();
  }
}

osg::Vec4 HudTextAdapter::color() const
{
  return color_;
}

void HudTextAdapter::setColor(const osg::Vec4& color)
{
  if (color != color_)
  {
    color_ = color;
    update_();
  }
}

void HudTextAdapter::textSize(int& width, int& height) const
{
  extent_->getSize(width, height);
}

bool HudTextAdapter::visible() const
{
  return visible_;
}

void HudTextAdapter::setVisible(bool value)
{
  if (value == visible_)
    return;

  visible_ = value;
  if (visible_ == true)
  {
    // switching from invisible to visible
    update_();
  }
  else
  {
    // switching from visible to invisible
    extent_->clear();
    // Should be small so should not be a performance concern
    while (!osgTextVector_.empty())
    {
      osg::ref_ptr<osgText::Text> osgText = osgTextVector_.back();
      removeDrawable(osgText.get());
      osgTextVector_.pop_back();
    }
  }
}

void HudTextAdapter::setBackdrop(osgText::Text::BackdropType backdrop, float backdropOffset)
{
  if (backdrop != backdrop_ || !simCore::areEqual(backdropOffset, backdropOffset_))
  {
    backdrop_ = backdrop;
    backdropOffset_ = backdropOffset;
    update_();
  }
}

void HudTextAdapter::setBackdropType(osgText::Text::BackdropType backdrop)
{
  setBackdrop(backdrop, backdropOffset());
}

void HudTextAdapter::setBackdropOffset(float backdropOffset)
{
  setBackdrop(backdropType(), backdropOffset);
}

osgText::Text::BackdropType HudTextAdapter::backdropType() const
{
  return backdrop_;
}

float HudTextAdapter::backdropOffset() const
{
  return backdropOffset_;
}

//-------------------------------------------------------------------------------------------------------

/** Watches for display resize events */
class HudManager::ResizeHandler : public osgGA::GUIEventHandler
{
public:
  /** Constructor */
  explicit ResizeHandler(HudManager* manager)
   : manager_(manager),
     width_(0),
     height_(0)
  { }

  /** Checks for resize events */
  bool virtual handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*)
  {
    if (manager_ == NULL)
      return false;

    // this handler does not (always?) receive RESIZE events, so check it manually.
    if (ea.getEventType() == osgGA::GUIEventAdapter::FRAME)
    {
      const osg::View* view = aa.asView();
      const osg::Camera* camera = (view) ? view->getCamera() : NULL;
      const osg::Viewport* viewport = (camera) ? camera->getViewport() : NULL;
      if (viewport)
      {
        int width = static_cast<int>(viewport->width());
        int height = static_cast<int>(viewport->height());
        if (width != width_ || height != height_)
        {
          width_  = width;
          height_ = height;
          manager_->resize_(width_, height_);
        }
      }
    }
    return false;
  }

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }

  /** Return the class name */
  virtual const char* className() const { return "HudManager::ResizeHandler"; }

protected:
  virtual ~ResizeHandler() {}
private:
  HudManager* manager_;
  int width_;
  int height_;
};

//-------------------------------------------------------------------------------------------------------

HudManager::HudManager(simVis::View* view)
  : renderLevel_(HUD_BASE_LEVEL),
    view_(view)
{
  group_ = new osg::Group();
  hud_ = view_->getOrCreateHUD();
  osg::StateSet* stateset = hud_->getOrCreateStateSet();
  simVis::setLighting(stateset, osg::StateAttribute::OFF);

  const osg::Viewport* vp = view->getCamera()->getViewport();
  windowWidth_ = static_cast<int>(vp->width());
  windowHeight_ = static_cast<int>(vp->height());

  handler_ = new ResizeHandler(this);
  view_->addEventHandler(handler_);
  hud_->addChild(group_);
}

HudManager::~HudManager()
{
  hud_->removeChild(group_);
  view_->removeEventHandler(handler_);
}

HudText* HudManager::createText(const std::string& text, double x, double y, bool percentage, Alignment hAlign, Alignment vAlign, const osg::Vec4& color, const std::string& font, double fontSize)
{
  HudText* hudText = new HudRowText(windowWidth_, windowHeight_);
  hudText->update(text, x, y, percentage, percentage, hAlign, vAlign, color, font, fontSize);

  textVector_.push_back(hudText);
  group_->addChild(hudText);
  return hudText;
}

HudText* HudManager::createText(const std::string& text, double x, double y, bool percentageX,
                                bool percentageY, Alignment hAlign, Alignment vAlign,
                                const osg::Vec4& color, const std::string& font, double fontSize)
{
  HudText* hudText = new HudRowText(windowWidth_, windowHeight_);
  hudText->update(text, x, y, percentageX, percentageY, hAlign, vAlign, color, font, fontSize);
  textVector_.push_back(hudText);
  group_->addChild(hudText);
  return hudText;
}

HudColumnText* HudManager::createColumnText(const std::string& text, double x, double y,
                                            bool percentage, Alignment vAlign, const osg::Vec4& color,
                                            const std::string& font, double fontSize)
{
  HudColumnText* hudColumnText = new HudColumnText(windowWidth_, windowHeight_);
  HudText *hudText = dynamic_cast<HudText*>(hudColumnText);
  // HudColumnText currently only implements ALIGN_LEFT
  hudText->update(text, x, y, percentage, percentage, simUtil::ALIGN_LEFT, vAlign, color, font, fontSize);

  textVector_.push_back(hudColumnText);
  group_->addChild(hudColumnText);
  return hudColumnText;
}

HudImage* HudManager::createImage(osg::Image* image, double x, double y, double w, double h,
                                  bool percentageX, bool percentageY, bool percentageW, bool percentageH,
                                  Alignment hAlign, Alignment vAlign)
{
  HudImage* hudImage = new HudImage(windowWidth_, windowHeight_);
  hudImage->update(image, x, y, w, h, percentageX, percentageY, percentageW, percentageH, hAlign, vAlign);
  imageVector_.push_back(hudImage);
  group_->addChild(hudImage);
  return hudImage;
}

void HudManager::removeText(HudText* hudText)
{
  std::vector< osg::ref_ptr<HudText> >::iterator it = std::find(textVector_.begin(), textVector_.end(), hudText);
  if (it != textVector_.end())
  {
    group_->removeChild(*it);
    textVector_.erase(it);
  }
}

void HudManager::removeImage(HudImage* hudImage)
{
  std::vector< osg::ref_ptr<HudImage> >::iterator it = std::find(imageVector_.begin(), imageVector_.end(), hudImage);
  if (it != imageVector_.end())
  {
    group_->removeChild(*it);
    imageVector_.erase(it);
  }
}

void HudManager::resize_(int width, int height)
{
  if (windowWidth_ == width && windowHeight_ == height)
    return;

  windowWidth_ = width;
  windowHeight_ = height;
  for (std::vector< osg::ref_ptr<HudText> >::const_iterator it = textVector_.begin(); it != textVector_.end(); ++it)
    (*it)->resize(windowWidth_, windowHeight_);
  for (std::vector< osg::ref_ptr<HudImage> >::const_iterator it = imageVector_.begin(); it != imageVector_.end(); ++it)
    (*it)->resize(windowWidth_, windowHeight_);
}

osg::Camera* HudManager::hud() const
{
  return hud_.get();
}

void HudManager::setRenderLevel(HudRenderLevel renderLevel)
{
  group_->getOrCreateStateSet()->setRenderBinDetails(renderLevel, "RenderBin");
  renderLevel_ = renderLevel;
}

void HudManager::getWindowSize(int& width, int& height) const
{
  width = windowWidth_;
  height = windowHeight_;
}

//-------------------------------------------------------------------------------------------------------

HudColumnText::HudColumnText(int width, int height)
  : HudTextAdapter(width, height),
    initialX_(0.0f),
    initialY_(0.0f),
    deltaY_(0.0f)
{
  setName("simUtil::HudColumnText");
}

HudColumnText::~HudColumnText()
{
}

void HudColumnText::tokenize_(const std::string& line, std::vector<std::string>& tokens) const
{
  std::vector<std::string> lines;
  simCore::stringTokenizer(lines, text(), "\n");
  size_t lineCount = lines.size();

  for (size_t line = 0; line != lineCount; ++line)
  {
    std::vector<std::string> columns;
    simCore::stringTokenizer(columns, lines[line], "\t");

    // assumes that each line will have same number of columns
    for (size_t column=0; column < columns.size(); ++column)
    {
      if (line == 0)
        tokens.push_back(columns[column]);
      else
        tokens[column] += '\n' + columns[column];
    }
  }
}

void HudColumnText::initializeText_(osgText::Text* text)
{
  text->setAlignment(osgText::TextBase::LEFT_BOTTOM_BASE_LINE);
}

void HudColumnText::positionText_(int index, osgText::Text* text)
{
  const float MIN_COL_SEP = 30.0f;

  // get bounds to determine horz and vert alignments
#if OSG_VERSION_GREATER_OR_EQUAL(3,3,2)
  const osg::BoundingBox& box = text->getBoundingBox();
#else
  const osg::BoundingBox& box = text->getBound();
#endif

  if (index == 0)
  {
    if (percentageX_)
      initialX_ = static_cast<float>(windowWidth_ * x_ / 100.0);
    else
      initialX_ = static_cast<float>(x_);

    if (percentageY_)
      initialY_ = static_cast<float>(windowHeight_ * y_ / 100.0);
    else
      initialY_ = static_cast<float>(y_);

    if (vAlignment() == ALIGN_TOP)
    {
      // drop entire text box below specified vertical position
      deltaY_ = (box.yMin() - box.yMax());
    }
    else if (vAlignment() == ALIGN_CENTER_Y)
    {
      // center entire box at specified vertical position
      deltaY_ = (box.yMin() - box.yMax()) * 0.5f;
    }
    else if (vAlignment() == ALIGN_BOTTOM)
    {
      // this positions text at absolute bottom of screen, with last line descenders off screen
      deltaY_ = 0.0f;
    }
  }

  text->setPosition(osg::Vec3(initialX_, initialY_ + deltaY_, 0.0f));

  // set horizontal alignment - position next column based on this column's horizontal bounds
  initialX_ += (box.xMax() - box.xMin() + MIN_COL_SEP);
}

//-------------------------------------------------------------------------------------------------------

HudRowText::HudRowText(int width, int height)
  : HudTextAdapter(width, height),
    initialX_(0.0f),
    initialY_(0.0f),
    stepY_(0.0f)
{
  setName("simUtil::HudRowText");
}

HudRowText::~HudRowText()
{
}

void HudRowText::tokenize_(const std::string& line, std::vector<std::string>& tokens) const
{
  simCore::stringTokenizer(tokens, line, "\n", true, false);
}

void HudRowText::initializeText_(osgText::Text* text)
{
  // Nothing to do
}

void HudRowText::positionText_(int index, osgText::Text* text)
{
  if (index == 0)
  {
    if (percentageX_)
      initialX_ = static_cast<float>(windowWidth_ * x_ / 100.0);
    else
      initialX_ = static_cast<float>(x_);

    if (percentageY_)
      initialY_ = static_cast<float>(windowHeight_ * y_ / 100.0);
    else
      initialY_ = static_cast<float>(y_);

    stepY_ = 0.0f;
  }

  // Calculate the X offset for text
  float deltaX = 0.0f;
  if (hAlign_ != ALIGN_LEFT)
  {
    // Sum up the width of the text
    float width = 0.f;
    osgText::Font* font = const_cast<osgText::Font*>(text->getFont());
    // Assertion failure means we don't have a font yet
    assert(font != NULL);
    if (!font)
      return;
    for (osgText::String::const_iterator i = text->getText().begin(); i != text->getText().end(); ++i)
    {
      osgText::Glyph* glyph = font->getGlyph(osgText::FontResolution(text->getFontWidth(), text->getFontHeight()), *i);
      if (glyph != NULL)
        width += glyph->getHorizontalAdvance();
    }

    // Scale up to the width ratio
    const float aspectRatio = text->getCharacterAspectRatio();
    const float widthRatio = (aspectRatio == 0.f) ? 1.0 : text->getCharacterHeight() / aspectRatio;
    width *= widthRatio;

    // Calculate the width adjustment based on the calculated width
    if (hAlign_ == ALIGN_RIGHT)
      deltaX = -width;
    else if (hAlign_ == ALIGN_CENTER_X)
      deltaX = width * -0.5f;
  }

  // Calculate the Y offset for text
  float deltaY = 0.0f;
  if (vAlign_ == ALIGN_TOP)
    deltaY = -text->getCharacterHeight();
  else if (vAlign_ == ALIGN_CENTER_Y)
    deltaY = -text->getCharacterHeight() / 2.0f;

  text->setPosition(osg::Vec3(initialX_ + deltaX, initialY_ + deltaY + stepY_, 0.0f));

  stepY_ -= text->getCharacterHeight();
}

//-------------------------------------------------------------------------------------------------------

HudImage::HudImage(int width, int height)
  : osg::Geode(),
    windowWidth_(width),
    windowHeight_(height),
    x_(0),
    y_(0),
    width_(10.0),
    height_(10.0),
    percentageX_(true),
    percentageY_(true),
    percentageWidth_(true),
    percentageHeight_(true),
    color_(1.f, 1.f, 1.f, 1.f)
{
  setName("simUtil::HudImage");
  getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
  getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
}

HudImage::~HudImage()
{
}

void HudImage::update_()
{
  float initialX;
  float initialY;
  float initialWidth;
  float initialHeight;

  if (percentageX_)
    initialX = static_cast<float>(windowWidth_ * x_ / 100.0);
  else
    initialX = static_cast<float>(x_);

  if (percentageWidth_)
    initialWidth = static_cast<float>(windowWidth_ * width_ / 100.0);
  else
    initialWidth = static_cast<float>(width_);

  if (percentageY_)
    initialY = static_cast<float>(windowHeight_ * y_ / 100.0);
  else
    initialY = static_cast<float>(y_);

  if (percentageHeight_)
    initialHeight = static_cast<float>(windowHeight_ * height_ / 100.0);
  else
    initialHeight = static_cast<float>(height_);

  // Remove any previously existing geometry
  removeDrawables(0, getNumDrawables());

  // Allocate the geometry and the screen vertices
  osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
  geometry->setName("simVis::Hud");
  geometry->setUseVertexBufferObjects(true);
  geometry->setUseDisplayList(false);
  geometry->setDataVariance(osg::Object::DYNAMIC);
  osg::ref_ptr<osg::Vec3Array> verts = new osg::Vec3Array(4);
  geometry->setVertexArray(verts.get());

  // update x values based on alignment
  switch (hAlign_)
  {
  case ALIGN_LEFT:
    break;
  case ALIGN_RIGHT:
    initialX -= initialWidth;
    break;
  case ALIGN_CENTER_X:
    initialX -= (initialWidth / 2);
    break;
  default:
    break;
  }

  // update y values based on alignment
  switch (vAlign_)
  {
  case ALIGN_BOTTOM:
    break;
  case ALIGN_TOP:
    initialY -= initialHeight;
    break;
  case ALIGN_CENTER_Y:
    initialY -= (initialHeight / 2);
    break;
  default:
    break;
  }

  // Assign the screen coordinates
  (*verts)[0].set(initialX, initialY, 0);
  (*verts)[1].set(initialX + initialWidth, initialY, 0);
  (*verts)[2].set(initialX, initialY + initialHeight, 0);
  (*verts)[3].set(initialX + initialWidth, initialY + initialHeight, 0);
  geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, 4));

  // Set up the color
  osg::ref_ptr<osg::Vec4Array> colorArray = new osg::Vec4Array(osg::Array::BIND_OVERALL, 1);
  (*colorArray)[0] = color_;
  geometry->setColorArray(colorArray.get());

  // Map texture coordinates to the corners
  osg::ref_ptr<osg::Vec2Array> texCoords = new osg::Vec2Array(4);
  (*texCoords)[0].set(0, 0);
  (*texCoords)[1].set(1, 0);
  (*texCoords)[2].set(0, 1);
  (*texCoords)[3].set(1, 1);
  geometry->setTexCoordArray(0, texCoords.get());

  // Set up the Texture2D
  osg::ref_ptr<osg::Texture2D> tex2d = new osg::Texture2D(image_);
  tex2d->setResizeNonPowerOfTwoHint(true);
  tex2d->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
  tex2d->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
  geometry->getOrCreateStateSet()->setTextureAttributeAndModes(0, tex2d, osg::StateAttribute::ON);

  // Add to the geode
  addDrawable(geometry);

  // Run shader generator to get texturing parameters correct
  osgEarth::Registry::shaderGenerator().run(this);
}

void HudImage::update(osg::Image* image, double x, double y, double w, double h,
                      bool percentageX, bool percentageY, bool percentageW, bool percentageH,
                      Alignment hAlign, Alignment vAlign)
{
  image_ = image;
  x_ = x;
  y_ = y;
  width_ = w;
  height_ = h;
  percentageX_ = percentageX;
  percentageY_ = percentageY;
  percentageWidth_ = percentageW;
  percentageHeight_ = percentageH;
  hAlign_ = hAlign;
  vAlign_ = vAlign;

  update_();
}

void HudImage::resize(int width, int height)
{
  if (windowWidth_ == width && windowHeight_ == height)
    return;
  windowWidth_ = width;
  windowHeight_ = height;
  update_();
}

osg::Image* HudImage::image() const
{
  return image_.get();
}

void HudImage::setImage(osg::Image* image)
{
  if (image_ != image)
  {
    image_ = image;
    update_();
  }
}

double HudImage::x() const
{
  return x_;
}

double HudImage::y() const
{
  return y_;
}

double HudImage::width() const
{
  return width_;
}

double HudImage::height() const
{
  return height_;
}

bool HudImage::isPercentageX() const
{
  return percentageX_;
}

bool HudImage::isPercentageY() const
{
  return percentageY_;
}

bool HudImage::isPercentageWidth() const
{
  return percentageWidth_;
}

bool HudImage::isPercentageHeight() const
{
  return percentageHeight_;
}

void HudImage::setAlignment(Alignment hAlign, Alignment vAlign)
{
  hAlign_ = hAlign;
  vAlign_ = vAlign;
  update_();
}

void HudImage::setPosition(double x, double y, bool percentageX, bool percentageY)
{
  if ((x != x_) || (y != y_) || (percentageX != percentageX_) || (percentageY != percentageY_))
  {
    x_ = x;
    y_ = y;
    percentageX_ = percentageX;
    percentageY_ = percentageY;
    update_();
  }
}

void HudImage::setSize(double w, double h, bool percentageWidth, bool percentageHeight)
{
  if ((w != width_) || (h != height_) || (percentageWidth != percentageX_) ||
    (percentageHeight != percentageY_))
  {
    width_ = w;
    height_ = h;
    percentageWidth_ = percentageWidth;
    percentageHeight_ = percentageHeight;
    update_();
  }
}

void HudImage::setColor(const osg::Vec4f& color)
{
  if (color_ == color)
    return;
  color_ = color;
  update_();
}

osg::Vec4f HudImage::color() const
{
  return color_;
}

}
