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
 * License for source code can be found at:
 * https://github.com/USNavalResearchLaboratory/simdissdk/blob/master/LICENSE.txt
 *
 * The U.S. Government retains all rights to use, duplicate, distribute,
 * disclose, or release this software.
 *
 */
#include "osg/Version"
#include "simVis/Text.h"

#undef LC
#define LC "[Text] "

#define SCREEN_COORDS_BUGFIX 1

namespace simVis {

#if OSG_VERSION_LESS_THAN(3,5,0)
/**
* This is code is copied exactly from osgText::Text::computePositions.
* The change is enclosed in the "SCREEN_COORDS_BUGFIX" define block.
*/
void simVis::Text::computePositions(unsigned int contextID) const
{
  switch(_alignment)
  {
  case LEFT_TOP:      _offset.set(_textBB.xMin(),_textBB.yMax(),_textBB.zMin()); break;
  case LEFT_CENTER:   _offset.set(_textBB.xMin(),(_textBB.yMax()+_textBB.yMin())*0.5f,_textBB.zMin()); break;
  case LEFT_BOTTOM:   _offset.set(_textBB.xMin(),_textBB.yMin(),_textBB.zMin()); break;

  case CENTER_TOP:    _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,_textBB.yMax(),_textBB.zMin()); break;
  case CENTER_CENTER: _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,(_textBB.yMax()+_textBB.yMin())*0.5f,_textBB.zMin()); break;
  case CENTER_BOTTOM: _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,_textBB.yMin(),_textBB.zMin()); break;

  case RIGHT_TOP:     _offset.set(_textBB.xMax(),_textBB.yMax(),_textBB.zMin()); break;
  case RIGHT_CENTER:  _offset.set(_textBB.xMax(),(_textBB.yMax()+_textBB.yMin())*0.5f,_textBB.zMin()); break;
  case RIGHT_BOTTOM:  _offset.set(_textBB.xMax(),_textBB.yMin(),_textBB.zMin()); break;

  case LEFT_BASE_LINE:  _offset.set(0.0f,0.0f,0.0f); break;
  case CENTER_BASE_LINE:  _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,0.0f,0.0f); break;
  case RIGHT_BASE_LINE:  _offset.set(_textBB.xMax(),0.0f,0.0f); break;

  case LEFT_BOTTOM_BASE_LINE:  _offset.set(0.0f,-_characterHeight*(1.0 + _lineSpacing)*(_lineCount-1),0.0f); break;
  case CENTER_BOTTOM_BASE_LINE:  _offset.set((_textBB.xMax()+_textBB.xMin())*0.5f,-_characterHeight*(1.0 + _lineSpacing)*(_lineCount-1),0.0f); break;
  case RIGHT_BOTTOM_BASE_LINE:  _offset.set(_textBB.xMax(),-_characterHeight*(1.0 + _lineSpacing)*(_lineCount-1),0.0f); break;
  }

  _offset.set(_offset.x() - x_, _offset.y() - y_, _offset.z());

  AutoTransformCache& atc = _autoTransformCache[contextID];
  osg::Matrix& matrix = atc._matrix;

  if (_characterSizeMode!=OBJECT_COORDS || _autoRotateToScreen)
  {

      matrix.makeTranslate(-_offset);

      osg::Matrix rotate_matrix;
      if (_autoRotateToScreen)
      {
          osg::Vec3d trans(atc._modelview.getTrans());
          atc._modelview.setTrans(0.0f,0.0f,0.0f);

          rotate_matrix.invert(atc._modelview);

          atc._modelview.setTrans(trans);
      }

      matrix.postMultRotate(_rotation);

      if (_characterSizeMode!=OBJECT_COORDS)
      {

          osg::Matrix M(rotate_matrix);
          M.postMultTranslate(_position);
          M.postMult(atc._modelview);
          osg::Matrix& P = atc._projection;

          // compute the pixel size vector.

          // pre adjust P00,P20,P23,P33 by multiplying them by the viewport window matrix.
          // here we do it in short hand with the knowledge of how the window matrix is formed
          // note P23,P33 are multiplied by an implicit 1 which would come from the window matrix.
          // Robert Osfield, June 2002.

          // scaling for horizontal pixels
          float P00 = P(0,0)*atc._width*0.5f;
          float P20_00 = P(2,0)*atc._width*0.5f + P(2,3)*atc._width*0.5f;
          osg::Vec3 scale_00(M(0,0)*P00 + M(0,2)*P20_00,
              M(1,0)*P00 + M(1,2)*P20_00,
              M(2,0)*P00 + M(2,2)*P20_00);

          // scaling for vertical pixels
          float P10 = P(1,1)*atc._height*0.5f;
          float P20_10 = P(2,1)*atc._height*0.5f + P(2,3)*atc._height*0.5f;
          osg::Vec3 scale_10(M(0,1)*P10 + M(0,2)*P20_10,
              M(1,1)*P10 + M(1,2)*P20_10,
              M(2,1)*P10 + M(2,2)*P20_10);

          float P23 = P(2,3);
          float P33 = P(3,3);

          float pixelSizeVector_w = M(3,2)*P23 + M(3,3)*P33;

          float pixelSizeVert=(_characterHeight*sqrtf(scale_10.length2()))/(pixelSizeVector_w*0.701f);

          // avoid nasty math by preventing a divide by zero
          if (pixelSizeVert == 0.0f)
              pixelSizeVert= 1.0f;

          if (_characterSizeMode==SCREEN_COORDS)
          {

#ifdef SCREEN_COORDS_BUGFIX
              float scale_font_vert=_characterHeight/pixelSizeVert;

              if (P10<0)
                  scale_font_vert=-scale_font_vert;
              matrix.postMultScale(osg::Vec3f(scale_font_vert, scale_font_vert,1.0f));

#else // original code:
              float pixelSizeHori=(_characterHeight/getCharacterAspectRatio()*sqrtf(scale_00.length2()))/(pixelSizeVector_w*0.701f);

              if (pixelSizeHori == 0.0f)
                  pixelSizeHori= 1.0f;

              float scale_font_vert=_characterHeight/pixelSizeVert;
              float scale_font_hori=_characterHeight/getCharacterAspectRatio()/pixelSizeHori;

              if (P10<0)
                  scale_font_vert=-scale_font_vert;
              matrix.postMultScale(osg::Vec3f(scale_font_hori, scale_font_vert,1.0f));
#endif
          }
          else if (pixelSizeVert>getFontHeight())
          {
              float scale_font = getFontHeight()/pixelSizeVert;
              matrix.postMultScale(osg::Vec3f(scale_font, scale_font,1.0f));
          }

      }

      if (_autoRotateToScreen)
      {
          matrix.postMult(rotate_matrix);
      }

      matrix.postMultTranslate(_position);
  }
  else if (!_rotation.zeroRotation())
  {
      matrix.makeRotate(_rotation);
      matrix.preMultTranslate(-_offset);
      matrix.postMultTranslate(_position);
  }
  else
  {
      matrix.makeTranslate(_position-_offset);
  }

  // now apply matrix to the glyphs.
  for(TextureGlyphQuadMap::iterator titr=_textureGlyphQuadMap.begin();
      titr!=_textureGlyphQuadMap.end();
      ++titr)
  {
      GlyphQuads& glyphquad = titr->second;
      //OSG_NOTICE<<"Text::computePositions("<<contextID<<") glyphquad= "<<&glyphquad<<std::endl;
      GlyphQuads::Coords2& coords2 = glyphquad._coords;

      if (contextID>=glyphquad._transformedCoords.size())
      {
          // contextID exceeds one setup for glyphquad._transformedCoords, ignore this request.
          continue;
      }

      GlyphQuads::Coords3& transformedCoords = glyphquad._transformedCoords[contextID];
      if (!transformedCoords) transformedCoords = new osg::Vec3Array;

      unsigned int numCoords = coords2->size();
      if (numCoords != transformedCoords->size())
      {
          transformedCoords->resize(numCoords);
      }

      for(unsigned int i = 0; i < numCoords; ++i)
      {
          (*transformedCoords)[i] = osg::Vec3((*coords2)[i].x(), (*coords2)[i].y(), 0.0f)*matrix;
      }
      transformedCoords->dirty();
  }

  computeBackdropPositions(contextID);

  _normal = osg::Matrix::transform3x3(osg::Vec3(0.0f,0.0f,1.0f),matrix);
  _normal.normalize();

  const_cast<Text*>(this)->dirtyBound();
}
#endif

void Text::setScreenOffset(float x, float y)
{
  x_ = x;
  y_ = y;
}

}
