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
#ifndef SIMQT_QTFORMATTING_H
#define SIMQT_QTFORMATTING_H

#include <QString>

namespace simQt {

/** Tag used to mark location of hot key in tool tip */
static const QString HOT_KEY_TAG = "<hotkey />";

/**
 * Create a rich text tooltip from a title and description
 * @param title  Title (emboldened) of tooltip
 * @param desc  Description of the tooltip
 * @prarm textColor Normally not set to get the default color. Only set if the default text color and the foreground color match
 * @return Formatted rich-text tooltip
 */
inline QString formatTooltip(const QString& title, const QString& desc, const QString& textColor="")
{
  if (desc.isEmpty())
    return QString("<strong>%1%2</strong>").arg(title, HOT_KEY_TAG);

  if (!textColor.isEmpty())
  {
    const QString formatBlock("<strong><font color=\"%4\">%1</font>%2</strong><div style=\"margin-left: 1em; margin-right: 1em;\"><p><font color=\"%4\">%3</font></p></div>");
    return formatBlock.arg(title, HOT_KEY_TAG, desc, textColor);
  }

  const QString formatBlock("<strong>%1%2</strong><div style=\"margin-left: 1em; margin-right: 1em;\"><p>%3</p></div>");
  return formatBlock.arg(title, HOT_KEY_TAG, desc);
}

}

#endif
