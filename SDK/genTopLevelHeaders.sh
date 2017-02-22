#!/usr/bin/env bash

writeHeader()
{
  echo "/* -*- mode: c++ -*- */" > $1
  echo "/****************************************************************************" >> $1
  echo " *****                                                                  *****" >> $1
  echo " *****                   Classification: UNCLASSIFIED                   *****" >> $1
  echo " *****                    Classified By:                                *****" >> $1
  echo " *****                    Declassify On:                                *****" >> $1
  echo " *****                                                                  *****" >> $1
  echo " ****************************************************************************" >> $1
  echo " *" >> $1
  echo " *" >> $1
  echo " * Developed by: Naval Research Laboratory, Tactical Electronic Warfare Div." >> $1
  echo " *               EW Modeling & Simulation, Code 5773" >> $1
  echo " *               4555 Overlook Ave." >> $1
  echo " *               Washington, D.C. 20375-5339" >> $1
  echo " *" >> $1
  echo " * License for source code at https://simdis.nrl.navy.mil/License.aspx" >> $1
  echo " *" >> $1
  echo " * The U.S. Government retains all rights to use, duplicate, distribute," >> $1
  echo " * disclose, or release this software." >> $1
  echo " *" >> $1
  echo " */" >> $1
}

# simVis
writeHeader simVis.h
echo "#ifndef SIMDISSDK_SIMVIS_H" >> simVis.h
echo "#define SIMDISSDK_SIMVIS_H" >> simVis.h
echo "" >> simVis.h

find simVis -name '*.h' | sort -f | grep -v "simVis/Shaders.h" | sed 's/^/#include "/' | sed 's/$/"/' >> simVis.h

echo "" >> simVis.h
echo "#endif /* SIMDISSDK_SIMVIS_H */" >> simVis.h
echo "" >> simVis.h

# simUtil
writeHeader simUtil.h
echo "#ifndef SIMDISSDK_SIMUTIL_H" >> simUtil.h
echo "#define SIMDISSDK_SIMUTIL_H" >> simUtil.h
echo "" >> simUtil.h

find simUtil -name '*.h' | sort -f | sed 's/^/#include "/' | sed 's/$/"/' >> simUtil.h

echo "" >> simUtil.h
echo "#endif /* SIMDISSDK_SIMUTIL_H */" >> simUtil.h
echo "" >> simUtil.h

# simCore
writeHeader simCore.h
echo "#ifndef SIMDISSDK_SIMCORE_H" >> simCore.h
echo "#define SIMDISSDK_SIMCORE_H" >> simCore.h
echo "" >> simCore.h

find simCore -name '*.h' | sort -f | grep -v "inttypes.h" | grep -v "stdint.h" | grep -v "HighPerformanceGraphics.h" | sed 's/^/#include "/' | sed 's/$/"/' >> simCore.h

echo "" >> simCore.h
echo "#endif /* SIMDISSDK_SIMCORE_H */" >> simCore.h
echo "" >> simCore.h

# simNotify
writeHeader simNotify.h
echo "#ifndef SIMDISSDK_SIMNOTIFY_H" >> simNotify.h
echo "#define SIMDISSDK_SIMNOTIFY_H" >> simNotify.h
echo "" >> simNotify.h

find simNotify -name '*.h' | sort -f | sed 's/^/#include "/' | sed 's/$/"/' >> simNotify.h

echo "" >> simNotify.h
echo "#endif /* SIMDISSDK_SIMNOTIFY_H */" >> simNotify.h
echo "" >> simNotify.h

# simData
writeHeader simData.h
echo "#ifndef SIMDISSDK_SIMDATA_H" >> simData.h
echo "#define SIMDISSDK_SIMDATA_H" >> simData.h
echo "" >> simData.h

find simData -name '*.h' | sort -f | grep -v "simData.pb.h" | grep -v "\-inl.h" | sed 's/^/#include "/' | sed 's/$/"/' >> simData.h

echo "" >> simData.h
echo "#endif /* SIMDISSDK_SIMDATA_H */" >> simData.h
echo "" >> simData.h

# simQt
writeHeader simQt.h
echo "#ifndef SIMDISSDK_SIMQT_H" >> simQt.h
echo "#define SIMDISSDK_SIMQT_H" >> simQt.h
echo "" >> simQt.h

find simQt -name '*.h' | sort -f | sed 's/^/#include "/' | sed 's/$/"/' >> simQt.h

echo "" >> simQt.h
echo "#endif /* SIMDISSDK_SIMQT_H */" >> simQt.h
echo "" >> simQt.h

