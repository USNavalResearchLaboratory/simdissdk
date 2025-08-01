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
#ifndef SIMQT_HUDTEXTBINMANAGER_H
#define SIMQT_HUDTEXTBINMANAGER_H

#include <string>
#include <vector>
#include <QColor>
#include <QMargins>
#include <QSize>
#include "osg/observer_ptr"
#include "osg/Group"
#include "simCore/Common/Common.h"
#include "simData/CommonPreferences.h"

namespace osg { class Camera; }

namespace simQt {

class TextBoxRenderer;
class TextBoxDataModel;

/**
 * Manages 9 text bins at each of the sides, corners, and screen center. Each
 * text bin is associated with a bin ID relating to its position, and is
 * independently managed in the sense that each one has its own text. This
 * class unifies the nine into one single text ID scheme.
 */
class SDKQT_EXPORT HudTextBinManager : public osg::Group
{
public:
  // Aliases for readability
  using TextId = uint64_t;
  using BinId = simData::TextAlignment;

  HudTextBinManager();
  SDK_DISABLE_COPY_MOVE(HudTextBinManager);

  /** Adds the given text string to the bin, returning a unique identifier for the text. */
  TextId addText(BinId binId, const std::string& text);
  /** Removes the text given, from the bin it was created in. Returns 0 on success. */
  int removeText(TextId uid);

  /** Changes the color for a bin */
  void setColor(BinId binId, const QColor& color);
  /** Changes the color for all bins */
  void setColor(const QColor& color);
  /** Retrieves the bin's color */
  QColor color(BinId binId) const;

  /** Changes the text size (in pixels) */
  void setTextSize(BinId binId, int textSizePx);
  /** Changes the text size (in pixels) for all bins */
  void setTextSize(int textSizePx);
  /** Retrieves the text size (in pixels) */
  int textSize(BinId binId) const;

  /** Returns all registered text IDs */
  std::vector<TextId> allTextIds() const;
  /** Returns the bin associated with a text ID. */
  BinId binId(TextId uid) const;
  /** Returns the text for a given Text ID. */
  std::string text(TextId uid) const;

  /** Sets the margins from the edge of the screen. */
  void setMargins(const QMargins& margins);
  /** Retrieves the current margins */
  QMargins margins() const;

  /** Sets the padding (spacing between boxes) in pixels: width, height. */
  void setPadding(const QSize& padding);
  /** Retrieves the padding (spacing between boxes) in pixels: width, height. */
  QSize padding() const;

  // From osg::Node:
  virtual const char* libraryName() const override{ return "simQt"; }
  virtual const char* className() const override { return "HudTextBinManager"; }

protected:
  /** Derived from osg::Referenced */
  virtual ~HudTextBinManager();

private:
  using BinAndTextId = std::pair<BinId, uint64_t>;
  struct TextBin;

  /** Called during the update traversals to reposition text boxes around new screen position */
  void checkViewportSize_();
  /** Called to reposition text boxes based on the given screen width/height */
  void setSize_(int width, int height);
  /** Refreshes all text strings for any text bin that is dirty; called during update traversal */
  void refreshAllDirtyTextBins_();
  /** Refreshes a single text bin that is dirty */
  void refreshDirtyTextBin_(TextBin& textBin);

  /** Retrieves the bin, for the given alignment identifier */
  TextBin* textBinForBinId_(BinId binId) const;

  osg::observer_ptr<osg::Camera> camera_;
  int width_ = 100;
  int height_ = 100;
  TextId nextPublicId_ = 1;
  std::map<TextId, BinAndTextId> publicIdToBinAndId_;
  std::vector<std::unique_ptr<TextBin>> bins_;
  QMargins margins_ = { 8, 8, 8, 8 }; // left, top, right, bottom
  QSize padding_ = { 0, 0 }; // horizontal, vertical

  /**
   * Set to true when the size calculation needs to happen, even when
   * the size doesn't change, due to other changes that can impact size
   * such as the margins or padding changing.
   */
  bool sizeDirty_ = false;
};

}

#endif /* SIMQT_HUDTEXTBINMANAGER_H */
