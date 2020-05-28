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
 *               EW Modeling and Simulation, Code 5770
 *               4555 Overlook Ave.
 *               Washington, D.C. 20375-5339
 *
 * For more information please send email to simdis@enews.nrl.navy.mil
 *
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 ****************************************************************************
 *
 *
 */
#ifndef SIMQT_GL3FORMATGUESSER_H
#define SIMQT_GL3FORMATGUESSER_H

#include <QGLFormat>
#include "simCore/Common/Export.h"

namespace simQt {

/**
 * Different graphics drivers support different capabilities.  SIMDIS has a minimum
 * required OpenGL version of 3.3.  Some context configuration options, such as
 * multisample, have been known to change the OpenGL context version returned by
 * the driver when creating a context.  This class attempts to massage a QGLFormat
 * into one that will return a context that is SIMDIS-compatible without losing
 * configuration options.
 */
class SDKQT_EXPORT Gl3FormatGuesser
{
public:
  /** Default constructor initializes the internal format to QGLFormat::defaultFormat(). */
  Gl3FormatGuesser();

  /** Change the input format for the findCompatibleFormat() operation. */
  void setFormat(const QGLFormat& format);
  /** Retrieve the guessed format.  findCompatibleFormat() changes this value. */
  const QGLFormat& format() const;

  /**
   * Test the provided QGLFormat for compatibility with SIMDIS given the current
   * hardware configuration.  Values inside the QGLFormat will be altered if the
   * provided QGLFormat is insufficient for SIMDIS.  This method will change the
   * return value provided by format().
   * @return 0 on success; see the return value of format() for the resulting format.
   *   Returns non-zero on error.  This means the class was unable to find a QGLFormat
   *   on this hardware that meets the minimum system requirements for SIMDIS.
   *   Continuing to start the application may lead to software instability or crashes.
   */
  int findCompatibleFormat();

  /** Helper method to return a reasonable format from QGLFormat::defaultFormat(). */
  static QGLFormat getFormat();
  /** Helper method to return a reasonable format from the provided format. */
  static QGLFormat getFormat(const QGLFormat& format);

private:
  /** Tests a candidate format for suitability */
  bool testFormat_(const QGLFormat& format) const;

  QGLFormat format_;
};

}

#endif /* SIMQT_GL3FORMATGUESSER_H */
