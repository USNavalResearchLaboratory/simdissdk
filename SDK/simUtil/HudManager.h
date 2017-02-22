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
#ifndef SIMUTIL_HUD_MANAGER_H
#define SIMUTIL_HUD_MANAGER_H

#include <map>
#include <string>
#include <vector>

#include <osg/Geode>
#include <osgText/Text>
#include "simCore/Common/Common.h"
#include "simVis/View.h"

namespace osgGA { class GUIEventHandler; }

namespace simUtil {

class HudManager;

/// Alignment for the overlay text
enum Alignment
{
  ALIGN_LEFT,
  ALIGN_CENTER_X,
  ALIGN_RIGHT,
  ALIGN_TOP,
  ALIGN_CENTER_Y,
  ALIGN_BOTTOM
};

/// Render bin number to set for displaying items at different levels in the HUD
enum HudRenderLevel
{
  HUD_BASE_LEVEL = 0,
  HUD_MID_LEVEL = 10,
  HUD_TOP_LEVEL = 20
};

/**
 * Interface for managing overlay text.  Handles both fixed and percentage based location.
 * Unlike the Control library the Alignment is relative to the specified point and not the parent
 */
class SDKUTIL_EXPORT HudText : public osg::Geode
{
public:
  /** Constructor */
  HudText();

  /// Called by the HUD Manager when a window re-size occurs so that relative text can repositioned
  virtual void resize(int width, int height) = 0;

  /// Getters and Setters
  /**
   * Single method to set all parameters of the HudText at once.  Percentages are from 0 to 100.
   * @param text The text to display; may contain \n
   * @param x The X position in either pixels or percentage as per the percentage_ variable  with respect to the lower left hand corner
   * @param y The Y position in either pixels or percentage as per the percentage_ variable  with respect to the lower left hand corner
   * @param percentageX True means X is percentage value; false means X is pixel value
   * @param percentageY True means Y is percentage value; false means Y is pixel value
   * @param hAlign Horizontal alignment
   * @param vAlign Vertical alignment
   * @param color of the text
   * @param font The font family of the text
   * @param fontSize The font size in points
   */
  virtual void update(const std::string& text, double x=0.0, double y=0.0,
        bool percentageX=true, bool percentageY=true,
        Alignment hAlign=ALIGN_LEFT, Alignment vAlign=ALIGN_BOTTOM,
        const osg::Vec4& color=osg::Vec4(1.0, 1.0, 1.0, 1.0),
        const std::string& font="arial.ttf", double fontSize=10) = 0;

  // Functions to set/get smaller parts

  /// Returns the text that will be displayed
  virtual std::string text() const = 0;
  /// Sets the text to display
  virtual void setText(const std::string& text) = 0;

  /// Returns the X position in either pixels or percentage as per the percentage_ variable with respect to the lower left hand corner
  virtual double x() const = 0;
  /// Returns the X position in either pixels or percentage as per the percentage_ variable  with respect to the lower left hand corner
  virtual double y() const = 0;
  /// Returns the percentage;  true means X is percentage value; false means X is pixel value
  virtual bool isPercentageX() const = 0;
  /// Returns the percentage;  true means Y is percentage value; false means Y is pixel value
  virtual bool isPercentageY() const = 0;
  /// Sets the text position
  virtual void setPosition(double x, double y, bool percentageX, bool percentageY) = 0;

  /// Returns the font family of the text
  virtual std::string font() const = 0;
  /// Returns the font size in points
  virtual double fontSize() const = 0;
  /// Sets the font family and the font size in points
  virtual void setFont(const std::string& font, double size) = 0;

  /// Returns the horizontal alignment
  virtual Alignment hAlignment() const = 0;
  /// Return the vertical alignment
  virtual Alignment vAlignment() const = 0;
  /// Sets the alignment of the text with respect to the specified point
  virtual void setAlignment(Alignment hAlign, Alignment vAlign) = 0;

  /// Returns the color of the text
  virtual osg::Vec4 color() const = 0;
  /// Sets the color of the text
  virtual void setColor(const osg::Vec4& color) = 0;

  /// Sets the backdrop type, including offset; see documentation of osgText::Text::setBackdropType() and setBackdropOffset()
  virtual void setBackdrop(osgText::Text::BackdropType backdrop, float backdropOffset) = 0;
  /// Sets the backdrop type; see documentation of osgText::Text::setBackdropType()
  virtual void setBackdropType(osgText::Text::BackdropType backdrop) = 0;
  /// Retrieves the backdrop type; see documentation of osgText::Text::setBackdropType()
  virtual osgText::Text::BackdropType backdropType() const = 0;
  /// Sets the backdrop offset; see documentation of osgText::Text::setBackdropOffset()
  virtual void setBackdropOffset(float backdropOffset) = 0;
  /// Retrieves the backdrop offset; see documentation of osgText::Text::setBackdropOffset()
  virtual float backdropOffset() const = 0;

  /// Returns the text width and height in pixels of the last rendered text
  virtual void textSize(int& width, int& height) const = 0;

  /// Returns true if the text is visible
  virtual bool visible() const = 0;
  /// Sets if the text is visible
  virtual void setVisible(bool value) = 0;

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }

  /** Return the class name */
  virtual const char* className() const { return "HudText"; }

protected:
  virtual ~HudText() {}
};


/**
 * Adds basic functionality with 3 protected over-rides to specialize
 * tokenization, initialization, and placement.
 */
class SDKUTIL_EXPORT HudTextAdapter : public HudText
{
public:
  /** Constructor */
  HudTextAdapter(int width, int height);

  /// Called by the HUD Manager when a window re-size occurs so that relative text can repositioned
  virtual void resize(int width, int height);

  /// Getters and Setters
  /**
   * Single method to set all parameters of the HudText at once.  Percentages are from 0 to 100.
   * @param text The text to display; may contain \n
   * @param x The X position in either pixels or percentage as per the percentage_ variable  with respect to the lower left hand corner
   * @param y The Y position in either pixels or percentage as per the percentage_ variable  with respect to the lower left hand corner
   * @param percentageX True means X is percentage value; false means X is pixel value
   * @param percentageY True means Y is percentage value; false means Y is pixel value
   * @param hAlign Horizontal alignment
   * @param vAlign Vertical alignment
   * @param color of the text
   * @param font The font family of the text
   * @param fontSize The font size in points
   */
  virtual void update(const std::string& text, double x=0.0, double y=0.0,
        bool percentageX=true, bool percentageY=true,
        Alignment hAlign=ALIGN_LEFT, Alignment vAlign=ALIGN_BOTTOM,
        const osg::Vec4& color=osg::Vec4(1.0, 1.0, 1.0, 1.0),
        const std::string& font="arial.ttf", double fontSize=10);

  // Functions to set/get smaller parts

  /// Returns the text that will be displayed
  virtual std::string text() const;
  /// Sets the text to display
  virtual void setText(const std::string& text);

  /// Returns the X position in either pixels or percentage as per the percentage_ variable with respect to the lower left hand corner
  virtual double x() const;
  /// Returns the X position in either pixels or percentage as per the percentage_ variable  with respect to the lower left hand corner
  virtual double y() const;
  /// Returns the percentage;  true means X is percentage value; false means X is pixel value
  virtual bool isPercentageX() const;
  /// Returns the percentage;  true means Y is percentage value; false means Y is pixel value
  virtual bool isPercentageY() const;
  /// Sets the text position
  virtual void setPosition(double x, double y, bool percentageX, bool percentageY);

  /// Returns the font family of the text
  virtual std::string font() const;
  /// Returns the font size in points
  virtual double fontSize() const;
  /// Sets the font family and the font size in points
  virtual void setFont(const std::string& font, double size);

  /// Returns the horizontal alignment
  virtual Alignment hAlignment() const;
  /// Return the vertical alignment
  virtual Alignment vAlignment() const;
  /// Sets the alignment of the text with respect to the specified point
  virtual void setAlignment(Alignment hAlign, Alignment vAlign);

  /// Returns the color of the text
  virtual osg::Vec4 color() const;
  /// Sets the color of the text
  virtual void setColor(const osg::Vec4& color);

  /// Sets the backdrop type, including offset; see documentation of osgText::Text::setBackdropType() and setBackdropOffset()
  virtual void setBackdrop(osgText::Text::BackdropType backdrop, float backdropOffset);
  /// Sets the backdrop type; see documentation of osgText::Text::setBackdropType()
  virtual void setBackdropType(osgText::Text::BackdropType backdrop);
  /// Retrieves the backdrop type; see documentation of osgText::Text::setBackdropType()
  virtual osgText::Text::BackdropType backdropType() const;
  /// Sets the backdrop offset; see documentation of osgText::Text::setBackdropOffset()
  virtual void setBackdropOffset(float backdropOffset);
  /// Retrieves the backdrop offset; see documentation of osgText::Text::setBackdropOffset()
  virtual float backdropOffset() const;

  /// Returns the text width and height in pixels of the last rendered text
  virtual void textSize(int& width, int& height) const;

  /// Returns true if the text is visible
  virtual bool visible() const;
  /// Sets if the text is visible
  virtual void setVisible(bool value);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }

  /** Return the class name */
  virtual const char* className() const { return "HudTextAdapter"; }

protected:
  class TextExtent;

  virtual ~HudTextAdapter();

  /// Does the actual screen update
  void update_();

  /// Routine for tokenizing the given line into tokens
  virtual void tokenize_(const std::string& line, std::vector<std::string>& tokens) const = 0;

  /// Called after the creation of text to allow for specialized initialization
  virtual void initializeText_(osgText::Text* text) = 0;

  /// Routine for positioning in the display the given text.  Index is the token number of text.
  virtual void positionText_(int index, osgText::Text* text) = 0;

  std::vector< osg::ref_ptr<osgText::Text> > osgTextVector_;  ///< A vector of osg::Text to handle text with \n
  int windowWidth_; ///< Save a copy of the window width
  int windowHeight_;  ///< Save a copy of the window height
  std::string text_;  ///< The text to display; may contain \n
  double x_;  ///< The X position in either pixels or percentage as per the percentage_ variable with respect to the lower left hand corner
  double y_;  ///< The Y position in either pixels or percentage as per the percentage_ variable with respect to the lower left hand corner
  bool percentageX_;  ///< True means X is percentage value; false means X is pixel value
  bool percentageY_;  ///< True means Y is percentage value; false means Y is pixel value
  Alignment hAlign_;  ///< Horizontal alignment
  Alignment vAlign_;  ///< Vertical alignment
  osg::Vec4 color_;  ///< Color of the text
  std::string requestedFont_;  ///< The requested font family of the text
  double requestedFontSize_;  ///< The requested font size in points
  std::string currentFont_;  ///< Used to optimize font calls which are expensive
  double currentFontSize_;  ///< Used to optimize font calls which are expensive
  bool visible_; ///< If the text is visible
  TextExtent* extent_;  ///< The display area covered by the text
  osgText::Text::BackdropType backdrop_; ///< Text backdrop type
  float backdropOffset_; ///< Backdrop offset in osgText backdrop offset units
};


/**
 * A row based implementation modeled after SIMDIS 9
 */
class SDKUTIL_EXPORT HudRowText : public HudTextAdapter
{
public:
  /** Constructor */
  HudRowText(int width, int height);

protected:
  virtual ~HudRowText();

  virtual void tokenize_(const std::string& line, std::vector<std::string>& tokens) const;
  virtual void initializeText_(osgText::Text* text);
  virtual void positionText_(int index, osgText::Text* text);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }

  /** Return the class name */
  virtual const char* className() const { return "HudRowText"; }

private:
  float initialX_;  ///< The lower left start location in pixels
  float initialY_;  ///< The lower left start location in pixels
  float stepY_;   ///< The vertical step in pixels
};

/**
 * A column based implementation used for the lower left hand status information
 */
class SDKUTIL_EXPORT HudColumnText : public HudTextAdapter
{
public:
  /** Constructor */
  HudColumnText(int width, int height);

protected:
  virtual ~HudColumnText();

  virtual void tokenize_(const std::string& line, std::vector<std::string>& tokens) const;
  virtual void initializeText_(osgText::Text* text);
  virtual void positionText_(int index, osgText::Text* text);

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }

  /** Return the class name */
  virtual const char* className() const { return "HudColumnText"; }

private:
  float initialX_;  ///< The lower left start location in pixels
  float initialY_;  ///< The lower left start location in pixels
  float deltaY_;   ///< The vertical step in pixels, based on the first column but applied to all columns
};


/**
 * Class for managing overlay images.  Handles both fixed and percentage based location.
 */
class SDKUTIL_EXPORT HudImage : public osg::Geode
{
public:
  /** Constructor */
  HudImage(int width, int height);

  /// Called by the HUD Manager when a window re-size occurs so that relative images can repositioned
  void resize(int width, int height);

  // Getters and Setters

  /**
   * Single method to set all parameters of the HudImage at once.  Percentages are from 0 to 100.
   * @param image Image to draw on the overlay
   * @param x The X position in either pixels or percentage as per the percentage_ variable  with respect to the lower left hand corner
   * @param y The Y position in either pixels or percentage as per the percentage_ variable  with respect to the lower left hand corner
   * @param w The width in either pixels or percentage as per the percentage_ variable  with respect to the lower left side
   * @param h The height in either pixels or percentage as per the percentage_ variable  with respect to the lower left side
   * @param percentageX True means X is percentage value; false means X is pixel value
   * @param percentageY True means Y is percentage value; false means Y is pixel value
   * @param percentageW True means Width is percentage value; false means Width is pixel value
   * @param percentageH True means Height is percentage value; false means Height is pixel value
   */
  void update(osg::Image* image, double x=0.0, double y=0.0,
        double w=10.0, double h=10.0,
        bool percentageX=true, bool percentageY=true,
        bool percentageW=true, bool percentageH=true);

  // Functions to set/get smaller parts

  /// Returns the image that will be displayed
  osg::Image* image() const;
  /// Sets the image to display
  virtual void setImage(osg::Image* image);

  /// Returns the X position in either pixels or percentage as per the percentage_ variable with respect to the lower left hand corner
  double x() const;
  /// Returns the X position in either pixels or percentage as per the percentage_ variable  with respect to the lower left hand corner
  double y() const;
  /// Returns the Width in either pixels or percentage as per the percentage_ variable with respect to the lower left hand corner
  double width() const;
  /// Returns the Height in either pixels or percentage as per the percentage_ variable  with respect to the lower left hand corner
  double height() const;
  /// Returns the percentage;  true means X is percentage value; false means X is pixel value
  bool isPercentageX() const;
  /// Returns the percentage;  true means Y is percentage value; false means Y is pixel value
  bool isPercentageY() const;
  /// Returns the percentage;  true means Width is percentage value; false means Width is pixel value
  bool isPercentageWidth() const;
  /// Returns the percentage;  true means Height is percentage value; false means Height is pixel value
  bool isPercentageHeight() const;

  /// Sets the image position
  void setPosition(double x, double y, bool percentageX, bool percentageY);
  /// Sets the image size
  void setSize(double w, double h, bool percentageWidth, bool percentageHeight);

  /// Changes the modulation color for the image
  void setColor(const osg::Vec4f& color);
  /// Retrieves the modulation color for the image
  osg::Vec4f color() const;

  /** Return the proper library name */
  virtual const char* libraryName() const { return "simUtil"; }

  /** Return the class name */
  virtual const char* className() const { return "HudImage"; }

protected:
  virtual ~HudImage();

private:
  /// Does the actual screen update
  virtual void update_();

protected:
  int windowWidth_;   ///< Save a copy of the window width
  int windowHeight_;  ///< Save a copy of the window height

private:
  osg::ref_ptr<osg::Image> image_;  ///< Reference to the image to draw
  double x_;  ///< The X position in either pixels or percentage as per the percentage_ variable with respect to the lower left hand corner
  double y_;  ///< The Y position in either pixels or percentage as per the percentage_ variable with respect to the lower left hand corner
  double width_;   ///< The width in either pixels or percentage as per the percentage_ variable with respect to the lower left hand corner
  double height_;  ///< The height in either pixels or percentage as per the percentage_ variable with respect to the lower left hand corner
  bool percentageX_;  ///< True means X is percentage value; false means X is pixel value
  bool percentageY_;  ///< True means Y is percentage value; false means Y is pixel value
  bool percentageWidth_;   ///< True means width is percentage value; false means width is pixel value
  bool percentageHeight_;  ///< True means height is percentage value; false means height is pixel value
  osg::Vec4f color_; ///< Modulation color applied to image
};


/// Class for managing HUD elements, currently text and image elements
class SDKUTIL_EXPORT HudManager
{
public:
  /** Constructor */
  HudManager(simVis::View* view);
  virtual ~HudManager();

  /**
   * Creates and returns a HudText for display overlay text.  Percentages are from 0 to 100.
   * @param text The text to display; may contain \n
   * @param x The X position in either pixels or percentage as per the percentage_ variable
   * @param y The Y position in either pixels or percentage as per the percentage_ variable
   * @param percentage True means X and Y are percentage values; false means X and Y are pixel values
   * @param hAlign Horizontal alignment
   * @param vAlign Vertical alignment
   * @param color of the text
   * @param font The font family of the text
   * @param fontSize The font size in points
   * @return Always returns a HudText
   */
  HudText* createText(const std::string& text, double x, double y, bool percentage,
    Alignment hAlign, Alignment vAlign, const osg::Vec4& color, const std::string& font,
    double fontSize);

  /**
   * Creates and returns a HudText for display overlay text.  Percentages are from 0 to 100.
   * @param text The text to display; may contain \n
   * @param x The X position in either pixels or percentage as per the percentage_ variable
   * @param y The Y position in either pixels or percentage as per the percentage_ variable
   * @param percentageX True means X is percentage value; false means X is pixel value
   * @param percentageY True means Y is percentage value; false means Y is pixel value
   * @param hAlign Horizontal alignment
   * @param vAlign Vertical alignment
   * @param color of the text
   * @param font The font family of the text
   * @param fontSize The font size in points
   * @return Always returns a HudText
   */
  HudText* createText(const std::string& text, double x=0.0, double y=0.0,
    bool percentageX=true, bool percentageY=true,
    Alignment hAlign=ALIGN_LEFT, Alignment vAlign=ALIGN_BOTTOM,
    const osg::Vec4& color=osg::Vec4(1.0, 1.0, 1.0, 1.0),
    const std::string& font="arial.ttf", double fontSize=10);

  /**
   * Creates and returns a HudColumnText for displaying overlay text in columns.  Percentages are from 0 to 100.
   * @param text The text to display; may contain \\t (raw tab) to mark end of one column and beginning of next column
   *   and \n to mark end of line/beginning of new line; each line must have same number of columns
   * @param x The X position in either pixels or percentage as per the percentage_ variable
   * @param y The Y position in either pixels or percentage as per the percentage_ variable
   * @param percentage True means X and Y are percentage values; false means X and Y or pixel values
   * @param vAlign Vertical alignment
   * @param color of the text
   * @param font The font family of the text
   * @param fontSize The font size in points
   * @return Always returns a HudText
   */
  HudColumnText* createColumnText(const std::string& text, double x=0.0, double y=0.0,
                                  bool percentage=true, Alignment vAlign=ALIGN_BOTTOM,
                                  const osg::Vec4& color=osg::Vec4(1.0, 1.0, 1.0, 1.0),
                                  const std::string& font="arial.ttf", double fontSize=20);

  /**
   * Creates and returns a HudImage for displaying images on the HUD.  Percentages are from 0 to 100.
   * @param image Image to draw on the overlay
   * @param x The X position in either pixels or percentage as per the percentage_ variable  with respect to the lower left hand corner
   * @param y The Y position in either pixels or percentage as per the percentage_ variable  with respect to the lower left hand corner
   * @param w The width in either pixels or percentage as per the percentage_ variable  with respect to the lower left side
   * @param h The height in either pixels or percentage as per the percentage_ variable  with respect to the lower left side
   * @param percentageX True means X is percentage value; false means X is pixel value
   * @param percentageY True means Y is percentage value; false means Y is pixel value
   * @param percentageW True means Width is percentage value; false means Width is pixel value
   * @param percentageH True means Height is percentage value; false means Height is pixel value
   * @return Always returns a new HudImage, which acts as a handle to the displayed HUD item
   */
  HudImage* createImage(osg::Image* image, double x=0.0, double y=0.0, double w=10.0, double h=10.0,
    bool percentageX=true, bool percentageY=true, bool percentageW=true, bool percentageH=true);

  /// Removes the specified text
  void removeText(HudText* hudText);
  /// Removes the specified image
  void removeImage(HudImage* hudImage);

  /// Returns the current HUD
  osg::Camera* hud() const;

  /// Set the render level this HudManager should apply to all its hud items
  void setRenderLevel(HudRenderLevel renderLevel);

  /// Returns the current windows size
  void getWindowSize(int& width, int& height) const;

private:
  class ResizeHandler;
  /// Called by osgGA::GUIEventHandler when the window re-sizes
  void resize_(int width, int height);

  /// Declare to keep cppCheck warning free, but not implemented since it is not needed
  HudManager(HudManager& noCopyConstructor);

  HudRenderLevel renderLevel_; ///< render level to draw this hud manager and its hud items
  osg::ref_ptr<osg::Group> group_;  ///< group node that holds all the hud text and images
  std::vector< osg::ref_ptr<HudText> > textVector_;    ///< The current overlay text
  std::vector< osg::ref_ptr<HudImage> > imageVector_;  ///< The current overlay images
  osg::observer_ptr<simVis::View> view_;  ///< The view for this manager
  osg::observer_ptr<osg::Camera> hud_;    ///< The HUD for this manager
  osg::ref_ptr<osgGA::GUIEventHandler> handler_;  ///< The callback for window re-size
  int windowWidth_;   ///< Save a copy of the window width
  int windowHeight_;  ///< Save a copy of the window height
};

}

#endif
