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
#ifndef SIMQT_QTFORMATTING_H
#define SIMQT_QTFORMATTING_H

#include <QString>

namespace simQt {

/**
 * Create a rich text tooltip from a title and description
 * @param title  Title (emboldened) of tooltip
 * @param desc  Description of the tooltip
 * @return Formatted rich-text tooltip
 */
inline QString formatTooltip(const QString& title, const QString& desc)
{
  if (desc.isEmpty())
    return QString("<strong>%1</strong>").arg(title);
  const QString formatBlock("<strong>%1</strong><div style=\"margin-left: 1em; margin-right: 1em;\"><p>%2</p></div>");
  return formatBlock.arg(title, desc);
}

}

#endif
