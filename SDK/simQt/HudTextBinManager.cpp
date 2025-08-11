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
#include <map>
#include <QLabel>
#include <QString>
#include "osgEarth/NodeUtils"
#include "simVis/Utils.h"
#include "simQt/QWidgetNode.h"
#include "simQt/HudTextBinManager.h"

namespace simQt {

/** Sets the internal text margin around the label, between the background edge and label text. */
constexpr int DEFAULT_LABEL_BG_MARGIN_PX = 6;

/** Binds together a Bin identifier, the OSG node, the back-end data, and a dirty flag */
struct HudTextBinManager::TextBin
{
  BinId binId_ = BinId::ALIGN_LEFT_BOTTOM;
  osg::ref_ptr<TextBoxRenderer> node_;
  std::unique_ptr<TextBoxDataModel> data_;
  bool dataDirty_ = false;
};

/**
 * Single text box in the HUD, with multiple lines of text. Ties together the
 * QLabel and the underlying osg node along with the matrix transform, to
 * provide an easy to use and limited interface in to edit the displayed label.
 */
class TextBoxRenderer : public osg::MatrixTransform
{
public:
  TextBoxRenderer();

  /** Alignment on the text, which also impacts the screen positioning (anchor position). */
  void setAlignment(Qt::Alignment qtAlignment);
  void setRect(const QRect& rectPx);

  /** Change the text color */
  void setColor(const QColor& color);
  QColor color() const;

  /** Background color to make text easier to read. Alpha of 0 means no backdrop. Defaults to (0,0,0,0) */
  void setBackgroundColor(const QColor& color);
  QColor backgroundColor() const;

  /** Indicates distance for drop shadow offset; use 0 to not render a shadow. */
  void setShadowOffset(int shadowOffsetPx);
  int shadowOffset() const;

  /** Text font size in points */
  void setTextSize(double textSizePoints);
  double textSize() const;

  /** Changes the text string displayed; OK to be multi-line. */
  void setText(const QString& text);
  QString text() const;

  // From osg::Node:
  virtual const char* libraryName() const override { return "simQt"; }
  virtual const char* className() const override { return "TextBoxRenderer"; }

protected:
  /** Derived from osg::Referenced */
  virtual ~TextBoxRenderer();

private:
  QString buildStyleSheet_() const;
  void render_();
  QSize sizeForText_() const;

  QRect rectPx_ = QRect(10, 10, 400, 200);
  QColor color_ = Qt::white;
  QColor backgroundColor_ = QColor(0, 0, 0, 128);
  double textSizePoints_ = 13.5;
  bool dirty_ = true;

  std::unique_ptr<QLabel> label_;
  osg::ref_ptr<simQt::QLabelDropShadowNode> node_;
};

/**
 * Represents the data behind a single text bin. Users can add individual
 * text strings (that do not need to be unique), and are returned an
 * identifier. The identifier is used to refer to the text string for
 * future operations such as remove, updating text, or retrieving text.
 * The returned combined text is all current strings, separated by newlines.
 */
class TextBoxDataModel
{
public:
  TextBoxDataModel();
  virtual ~TextBoxDataModel();

  /** Adds a single text string, returning the unique identifier for it. */
  uint64_t addText(const std::string& text);
  /** Removes the text string referred to by the given UID, returning 0 on success. */
  int removeText(uint64_t uid);
  /** Updates the text string given */
  int setText(uint64_t uid, const std::string& text);
  /** Retrieves the text string associated with the ID */
  std::string textById(uint64_t uid) const;
  /** Retrieves a vector of all valid IDs with text strings. */
  std::vector<uint64_t> allTextIds() const;

  /** Returns the combined text string of all text values. */
  std::string combinedText() const;

private:
  /**
   * Combines all values in idToStringMap_. Called whenever it changes since
   * it is expected that the combined text will change infrequently vs how
   * frequently it is checked.
   */
  void createTextString_();

  std::map<uint64_t, std::string> idToStringMap_;
  std::string combinedText_;
  uint64_t nextId_ = 1;
};

///////////////////////////////////////////////////////////

TextBoxRenderer::TextBoxRenderer()
  : label_(std::make_unique<QLabel>()),
  node_(new simQt::QLabelDropShadowNode)
{
  addChild(node_.get());

  const int textSizePoints = label_->font().pointSize();
  textSizePoints_ = label_->font().pointSizeF();
  label_->setMargin(DEFAULT_LABEL_BG_MARGIN_PX);
  label_->setStyleSheet(buildStyleSheet_());
  label_->setFixedWidth(rectPx_.width());
  label_->setWordWrap(true);
  label_->setAlignment(Qt::AlignTop | Qt::AlignLeft);

  // On update, automatically re-render if dirty
  addUpdateCallback(new simVis::LambdaOsgCallback([this] { render_(); }));
}

TextBoxRenderer::~TextBoxRenderer()
{
}

void TextBoxRenderer::setAlignment(Qt::Alignment qtAlignment)
{
  label_->setAlignment(qtAlignment);
}

void TextBoxRenderer::setRect(const QRect& rectPx)
{
  if (rectPx_ == rectPx)
    return;

  // Any size changes might reflect on the label
  rectPx_ = rectPx;

  // Will need to regenerate the image based on size changes
  dirty_ = true;
}

void TextBoxRenderer::setColor(const QColor& color)
{
  if (color_ == color)
    return;
  color_ = color;
  label_->setStyleSheet(buildStyleSheet_());
  dirty_ = true;
}

QColor TextBoxRenderer::color() const
{
  return color_;
}

void TextBoxRenderer::setBackgroundColor(const QColor& color)
{
  if (backgroundColor_ == color)
    return;
  backgroundColor_ = color;
  label_->setStyleSheet(buildStyleSheet_());
  dirty_ = true;
}

QColor TextBoxRenderer::backgroundColor() const
{
  return backgroundColor_;
}

void TextBoxRenderer::setShadowOffset(int shadowOffsetPx)
{
  if (shadowOffsetPx == node_->shadowOffset())
    return;
  node_->setShadowOffset(shadowOffsetPx);
  dirty_ = true;
}

int TextBoxRenderer::shadowOffset() const
{
  return node_->shadowOffset();
}

void TextBoxRenderer::setTextSize(double textSizePoints)
{
  if (textSizePoints_ == textSizePoints)
    return;
  textSizePoints_ = textSizePoints;
  QFont font = label_->font();
  font.setPointSizeF(textSizePoints);
  label_->setFont(font);
  dirty_ = true;
}

double TextBoxRenderer::textSize() const
{
  return textSizePoints_;
}

void TextBoxRenderer::setText(const QString& text)
{
  if (label_->text() == text)
    return;
  label_->setText(text);
  dirty_ = true;
}

QString TextBoxRenderer::text() const
{
  return label_->text();
}

QSize TextBoxRenderer::sizeForText_() const
{
  // Available space is in rectPx_
  if (label_->text().isEmpty())
    return QSize(0, 0);

  const auto& oldPolicy = label_->sizePolicy();

  // Need to turn off word-wrap to get a default width that is accurate. Word wrap being
  // on makes the label's size policy guess roughly, rather than exactly. We want exact.
  label_->setWordWrap(false);
  label_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  label_->setMinimumSize(0, 0);
  label_->setMaximumSize(rectPx_.width(), rectPx_.height());

  // Force the label to a min of the preferred, and the rectangular box
  const int preferredWidth = label_->sizeHint().width();
  const int actualWidth = std::min(preferredWidth, rectPx_.width());

  // Tighten the "height" value now using word wrap
  label_->setWordWrap(true);
  const int preferredHeight = label_->heightForWidth(actualWidth);
  const int actualHeight = std::min(preferredHeight, rectPx_.height());

  // Restore changes that matter
  label_->setSizePolicy(oldPolicy);

  return QSize(actualWidth, actualHeight);
}

void TextBoxRenderer::render_()
{
  if (!dirty_)
    return;

  // Determine the size of the text box with tight wrapping, and set fixed size
  // on the label so that it doesn't stretch out.
  const auto& desireSize = sizeForText_();
  const bool validSize = (desireSize.width() > 0 && desireSize.height() > 0);
  if (validSize)
  {
    label_->setFixedSize(desireSize);
    node_->setNodeMask(~0);
  }
  else
  {
    // No need to render; can happen for empty strings
    node_->setNodeMask(0);
    return;
  }

  // Create the image of the label, which will then tell us the on-screen size
  node_->render(label_.get());

  // Image should match our desired size
  assert(node_->width() == desireSize.width());
  assert(node_->height() == desireSize.height());

  const auto alignment = label_->alignment();

  // Calculate the X position of the label
  int translateX = rectPx_.x();
  if (alignment.testFlag(Qt::AlignHCenter))
    translateX = rectPx_.center().x() - desireSize.width() / 2;
  else if (alignment.testFlag(Qt::AlignRight))
    translateX = rectPx_.right() - desireSize.width();

  // Calculate the Y position of the label
  int translateY = rectPx_.y();
  // Note that rectPx_.bottom is actually the TOP due to inversion of Qt/OSG coord systems
  if (alignment.testFlag(Qt::AlignVCenter))
    translateY = rectPx_.center().y() - desireSize.height() / 2;
  else if (alignment.testFlag(Qt::AlignTop))
    translateY = rectPx_.bottom() - desireSize.height();

  // Move to the expected image location based on alignment
  setMatrix(osg::Matrix::translate(translateX, translateY, 0.));
  dirty_ = false;
}

QString TextBoxRenderer::buildStyleSheet_() const
{
  const QString color = QString("color: rgba(%1, %2, %3, %4); ")
    .arg(color_.red())
    .arg(color_.green())
    .arg(color_.blue())
    .arg(color_.alpha());
  const QString bgColor = (backgroundColor_.alpha() == 0)
    ? ""
    : QString("background-color: rgba(%1, %2, %3, %4); ")
    .arg(backgroundColor_.red())
    .arg(backgroundColor_.green())
    .arg(backgroundColor_.blue())
    .arg(backgroundColor_.alpha());
  return color + bgColor;
}

///////////////////////////////////////////////////////////

TextBoxDataModel::TextBoxDataModel()
{
}

TextBoxDataModel::~TextBoxDataModel()
{
}

uint64_t TextBoxDataModel::addText(const std::string& text)
{
  const uint64_t newId = nextId_++;
  idToStringMap_[newId] = text;
  createTextString_();
  return newId;
}

int TextBoxDataModel::removeText(uint64_t uid)
{
  auto it = idToStringMap_.find(uid);
  if (it == idToStringMap_.end())
    return -1; // UID not found

  idToStringMap_.erase(it);
  createTextString_();
  return 0; // Success
}

int TextBoxDataModel::setText(uint64_t uid, const std::string& text)
{
  auto it = idToStringMap_.find(uid);
  if (it == idToStringMap_.end())
    return 1; // UID not found

  it->second = text;
  createTextString_();
  return 0; // Success
}

std::string TextBoxDataModel::textById(uint64_t uid) const
{
  const auto it = idToStringMap_.find(uid);
  if (it == idToStringMap_.end())
    return {};

  return it->second;
}

std::vector<uint64_t> TextBoxDataModel::allTextIds() const
{
  std::vector<uint64_t> ids;
  for (const auto& pair : idToStringMap_)
    ids.push_back(pair.first);

  return ids;
}

std::string TextBoxDataModel::combinedText() const
{
  return combinedText_;
}

void TextBoxDataModel::createTextString_()
{
  combinedText_.clear();
  for (const auto& pair : idToStringMap_)
  {
    if (!combinedText_.empty())
      combinedText_ += "\n"; // Newline separator
    combinedText_ += pair.second;
  }
}

///////////////////////////////////////////////////////////

HudTextBinManager::HudTextBinManager()
{
  // Assertion failure means we cannot use a simple vector for the bins,
  // or that the bin order needs to be updated internally.
  static_assert(static_cast<int>(BinId::ALIGN_LEFT_TOP) == 0);
  static_assert(static_cast<int>(BinId::ALIGN_RIGHT_BOTTOM) == 8);

  // Helper function to create a bin, and configure it in "this"
  const auto binFactory = [this](BinId binId, Qt::Alignment align) {
    auto bin = std::make_unique<TextBin>();
    bin->binId_ = binId;
    bin->node_ = new TextBoxRenderer;
    bin->data_ = std::make_unique<TextBoxDataModel>();
    bin->node_->setAlignment(align);
    addChild(bin->node_.get());
    bins_.push_back(std::move(bin));
    };

  // Create all bins
  binFactory(BinId::ALIGN_LEFT_TOP, Qt::AlignLeft | Qt::AlignTop);
  binFactory(BinId::ALIGN_LEFT_CENTER, Qt::AlignLeft | Qt::AlignVCenter);
  binFactory(BinId::ALIGN_LEFT_BOTTOM, Qt::AlignLeft | Qt::AlignBottom);
  binFactory(BinId::ALIGN_CENTER_TOP, Qt::AlignHCenter | Qt::AlignTop);
  binFactory(BinId::ALIGN_CENTER_CENTER, Qt::AlignHCenter | Qt::AlignVCenter);
  binFactory(BinId::ALIGN_CENTER_BOTTOM, Qt::AlignHCenter | Qt::AlignBottom);
  binFactory(BinId::ALIGN_RIGHT_TOP, Qt::AlignRight | Qt::AlignTop);
  binFactory(BinId::ALIGN_RIGHT_CENTER, Qt::AlignRight | Qt::AlignVCenter);
  binFactory(BinId::ALIGN_RIGHT_BOTTOM, Qt::AlignRight | Qt::AlignBottom);

  addUpdateCallback(new simVis::LambdaOsgCallback([this]() { checkViewportSize_(); }));
}

HudTextBinManager::~HudTextBinManager()
{
}

HudTextBinManager::TextBin* HudTextBinManager::textBinForBinId_(BinId binId) const
{
  const size_t asIndex = static_cast<size_t>(binId);
  // Invalid enumeration passed in or bins misconfigured
  assert(asIndex < bins_.size());
  if (asIndex >= bins_.size())
    return nullptr;
  return bins_[asIndex].get();
}

HudTextBinManager::TextId HudTextBinManager::addText(BinId binId, const std::string& text)
{
  auto* bin = textBinForBinId_(binId);
  if (!bin)
    return 0;
  // Add the text to the bin, with an early return on unexpected result
  const auto localId = bin->data_->addText(text);
  if (localId == 0)
    return 0;
  bin->dataDirty_ = true;
  const auto publicId = nextPublicId_++;
  publicIdToBinAndId_[publicId] = { binId, localId };
  return publicId;
}

int HudTextBinManager::removeText(TextId uid)
{
  auto iter = publicIdToBinAndId_.find(uid);
  if (iter == publicIdToBinAndId_.end())
    return 1;
  const auto& binAndId = iter->second;
  auto* bin = textBinForBinId_(binAndId.first);
  assert(bin); // Should be no realistic way to fail this; means our bin got removed
  if (!bin)
    return 1;

  const int rv = bin->data_->removeText(binAndId.second);
  publicIdToBinAndId_.erase(iter);
  bin->dataDirty_ = true;
  return rv;
}

std::vector<HudTextBinManager::TextId> HudTextBinManager::allTextIds() const
{
  std::vector<TextId> ids;
  for (const auto& pair : publicIdToBinAndId_)
    ids.push_back(pair.first);
  return ids;
}

HudTextBinManager::BinId HudTextBinManager::binId(TextId uid) const
{
  auto iter = publicIdToBinAndId_.find(uid);
  if (iter == publicIdToBinAndId_.end())
    return BinId::ALIGN_LEFT_TOP;
  const auto& binAndId = iter->second;
  return binAndId.first;
}

std::string HudTextBinManager::text(TextId uid) const
{
  auto iter = publicIdToBinAndId_.find(uid);
  if (iter == publicIdToBinAndId_.end())
    return {};
  const auto& binAndId = iter->second;
  auto* bin = textBinForBinId_(binAndId.first);
  assert(bin); // Should be no realistic way to fail this; means our bin got removed
  if (!bin)
    return {};
  return bin->data_->textById(binAndId.second);
}

void HudTextBinManager::setSize_(int width, int height)
{
  if (!sizeDirty_ && width_ == width && height_ == height) [[likely]]
    return;
  width_ = width;
  height_ = height;

  // Constants
  constexpr int NUM_ROWS = 3;
  constexpr int NUM_COLS = 3;

  // Calculate available width and height for the grid
  const int availableWidth = width - margins_.left() - margins_.right() - (NUM_COLS - 1) * padding_.width();
  const int availableHeight = height - margins_.top() - margins_.bottom() - (NUM_ROWS - 1) * padding_.height();

  // Calculate the width and height of each bin
  const int binWidth = availableWidth / NUM_COLS;
  const int binHeight = availableHeight / NUM_ROWS;

  // Calculate the lower-left corner -- (0,0) at lower left -- for each row/col
  const int left = margins_.left();
  const int xCenter = left + padding_.width() + binWidth;
  const int right = width - margins_.right() - binWidth;
  const int bottom = margins_.bottom();
  const int yCenter = bottom + padding_.height() + binHeight;
  const int top = height - margins_.top() - binHeight;

  // Helper function to assign a rect to a bin
  const auto setRect = [this](BinId binId, const QRect& rect) {
    const auto binIndex = static_cast<int>(binId);
    bins_[binIndex]->node_->setRect(rect);
    };

  setRect(BinId::ALIGN_LEFT_TOP, QRect{ left, top, binWidth, binHeight });
  setRect(BinId::ALIGN_LEFT_CENTER, QRect{ left, yCenter, binWidth, binHeight });
  setRect(BinId::ALIGN_LEFT_BOTTOM, QRect{ left, bottom, binWidth, binHeight });
  setRect(BinId::ALIGN_CENTER_TOP, QRect{ xCenter, top, binWidth, binHeight });
  setRect(BinId::ALIGN_CENTER_CENTER, QRect{ xCenter, yCenter, binWidth, binHeight });
  setRect(BinId::ALIGN_CENTER_BOTTOM, QRect{ xCenter, bottom, binWidth, binHeight });
  setRect(BinId::ALIGN_RIGHT_TOP, QRect{ right, top, binWidth, binHeight });
  setRect(BinId::ALIGN_RIGHT_CENTER, QRect{ right, yCenter, binWidth, binHeight });
  setRect(BinId::ALIGN_RIGHT_BOTTOM, QRect{ right, bottom, binWidth, binHeight });

  // Margins and padding applied, size is no longer dirty
  sizeDirty_ = false;
}

void HudTextBinManager::checkViewportSize_()
{
  // Lazily detect the camera
  if (!camera_.valid()) [[unlikely]]
  {
    camera_ = osgEarth::findFirstParentOfType<osg::Camera>(this);
    // Need a valid camera to continue
    if (!camera_.valid())
      return;
  }

  // Update text bin text, so that the renderer is correct
  refreshAllDirtyTextBins_();

  // Need a valid viewport to get its size; but don't care about setting
  // the size if there are no bins (performance optimization)
  const osg::Viewport* vp = camera_->getViewport();
  if (vp && !publicIdToBinAndId_.empty())
    setSize_(vp->width(), vp->height());
}

void HudTextBinManager::refreshAllDirtyTextBins_()
{
  for (const auto& binPtr : bins_)
  {
    if (binPtr->dataDirty_) [[unlikely]]
      refreshDirtyTextBin_(*binPtr);
  }
}

void HudTextBinManager::refreshDirtyTextBin_(TextBin& bin)
{
  // Precondition, handled by caller
  assert(bin.dataDirty_);
  const auto& combinedText = bin.data_->combinedText();
  bin.node_->setText(QString::fromStdString(combinedText).trimmed());
  bin.dataDirty_ = false;
}

void HudTextBinManager::setColor(BinId binId, const QColor& color)
{
  auto* bin = textBinForBinId_(binId);
  if (bin)
    bin->node_->setColor(color);
}

void HudTextBinManager::setColor(const QColor& color)
{
  for (const auto& binPtr : bins_)
    binPtr->node_->setColor(color);
}

QColor HudTextBinManager::color(BinId binId) const
{
  auto* bin = textBinForBinId_(binId);
  if (!bin)
    return Qt::white;
  return bin->node_->color();
}

void HudTextBinManager::setBackgroundColor(BinId binId, const QColor& color)
{
  auto* bin = textBinForBinId_(binId);
  if (bin)
    bin->node_->setBackgroundColor(color);
}

void HudTextBinManager::setBackgroundColor(const QColor& color)
{
  for (const auto& binPtr : bins_)
    binPtr->node_->setBackgroundColor(color);
}

QColor HudTextBinManager::backgroundColor(BinId binId) const
{
  auto* bin = textBinForBinId_(binId);
  if (!bin)
    return Qt::white;
  return bin->node_->backgroundColor();
}

void HudTextBinManager::setShadowOffset(BinId binId, int shadowOffsetPx)
{
  auto* bin = textBinForBinId_(binId);
  if (bin)
    bin->node_->setShadowOffset(shadowOffsetPx);
}

void HudTextBinManager::setShadowOffset(int shadowOffsetPx)
{
  for (const auto& binPtr : bins_)
    binPtr->node_->setShadowOffset(shadowOffsetPx);
}

int HudTextBinManager::shadowOffset(BinId binId) const
{
  auto* bin = textBinForBinId_(binId);
  if (!bin)
    return 0;
  return bin->node_->shadowOffset();
}

void HudTextBinManager::setTextSize(BinId binId, double textSizePoints)
{
  auto* bin = textBinForBinId_(binId);
  if (bin)
    bin->node_->setTextSize(textSizePoints);
}

void HudTextBinManager::setTextSize(double textSizePoints)
{
  for (const auto& binPtr : bins_)
    binPtr->node_->setTextSize(textSizePoints);
}

double HudTextBinManager::textSize(BinId binId) const
{
  auto* bin = textBinForBinId_(binId);
  if (!bin)
    return 0;
  return bin->node_->textSize();
}

void HudTextBinManager::setMargins(const QMargins& margins)
{
  if (margins_ == margins)
    return;
  margins_ = margins;
  sizeDirty_ = true;
}

QMargins HudTextBinManager::margins() const
{
  return margins_;
}

void HudTextBinManager::setPadding(const QSize& padding)
{
  if (padding_ == padding)
    return;
  padding_ = padding;
  sizeDirty_ = true;
}

QSize HudTextBinManager::padding() const
{
  return padding_;
}

}
