//============================================================================
// gltext.cpp : bitmap text drawing routine; works like "printf"
//============================================================================

#include <GL/glut.h>
#include <stdio.h>
#include <stdarg.h>
#include "gltext.h"

//-------------------------------------------------------------------------------
// Storage for the current font needed by the Text() routine. Global state for
// the text module. Access functions are provided. The following fonts are available
// and are defined in glut.h (fontids are on the left):
//  0: GLUT_BITMAP_8_BY_13        : 13 pixels high
//  1: GLUT_BITMAP_9_BY_15        : 15 pixels high
//  2: GLUT_BITMAP_TIMES_ROMAN_10 : 10 pixels high
//  3: GLUT_BITMAP_TIMES_ROMAN_24 : 24 pixels high
//  4: GLUT_BITMAP_HELVETICA_10   : 10 pixels high
//  5: GLUT_BITMAP_HELVETICA_12   : 12 pixels high
//  6: GLUT_BITMAP_HELVETICA_18   : 18 pixels high
//-------------------------------------------------------------------------------
void *CurrentFont = GLUT_BITMAP_9_BY_15;

void *Fonts[] = { GLUT_BITMAP_8_BY_13, 
                  GLUT_BITMAP_9_BY_15,
                  GLUT_BITMAP_TIMES_ROMAN_10, 
                  GLUT_BITMAP_TIMES_ROMAN_24,
                  GLUT_BITMAP_HELVETICA_10, 
                  GLUT_BITMAP_HELVETICA_12,
                  GLUT_BITMAP_HELVETICA_18 };
int FontHeights[] = { 13, 15, 10, 24, 10, 12, 18 };
int NumFonts = 7;

void* GetCurrentFont() { return(CurrentFont); }
void SetCurrentFont(void *font) { CurrentFont=font; }
void SetCurrentFont(int fontid) { CurrentFont=Fonts[fontid]; }


//-------------------------------------------------------------------------------
// Get the height in pixels for a specified font. Needed for calculating the
// Yoffset to go to the next line. There are several different ways:
// use the current font, specify font by id or explicitly.
//-------------------------------------------------------------------------------
int GetFontHeight(void *font)
{
       if (font==GLUT_BITMAP_8_BY_13) return(13);
  else if (font==GLUT_BITMAP_9_BY_15) return(15);
  else if (font==GLUT_BITMAP_TIMES_ROMAN_10) return(10);
  else if (font==GLUT_BITMAP_TIMES_ROMAN_24) return(24);
  else if (font==GLUT_BITMAP_HELVETICA_10) return(10);
  else if (font==GLUT_BITMAP_HELVETICA_12) return(12);
  else if (font==GLUT_BITMAP_HELVETICA_18) return(18);
  printf("ERROR (Text): Not a valid font!\n"); 
  return(0);
}

int GetFontHeight() 
{ 
  return( GetFontHeight(CurrentFont) ); 
}

int GetFontHeight(int fontid)
{
  return( FontHeights[fontid] );
}


//-------------------------------------------------------------------------------
// BITMAP TEXT ROUTINE: draws the printf style format string (with args) with
// the bottom-left corner of the string at pixel position (x,y) in the current 
// window with the current color. The bottom-left corner of the window is at 
// (0,0). Many different fonts are available (see CurrentFont above):
// The current raster position is updated to the end of the string; check with:
//  float CurrentRasterPos[4];
//  glGetFloatv(GL_CURRENT_RASTER_POSITION,CurrentRasterPos);
//  x=CurrentRasterPos[0]+0.5;
//  y=CurrentRasterPos[1]+0.5;
// Setting negative values for x or y will avoid the update of the raster pos;
// this is useful for continuing more text on the same line after a previous
// call to Text(). Newline chars ('\n') in the string advance the raster position
// to the next line down (Newline chars are ignored in string continuing on same line).
//-------------------------------------------------------------------------------
void gltext(int x, int y, char *format, ...)
{
  va_list args;
  char buffer[256];
  va_start(args,format);
  vsprintf(buffer,format,args);
  va_end(args);

  int WW = glutGet((GLenum)GLUT_WINDOW_WIDTH);
  int WH = glutGet((GLenum)GLUT_WINDOW_HEIGHT);

  glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
  glPushAttrib(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DITHER);     
  glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

  glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0, 0, WW, WH);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WW-1, 0, WH-1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
    glLoadIdentity();

  int FontHeight = GetFontHeight(CurrentFont);

  if (x>=0 && y>=0)
    glRasterPos2i(x,y);

  for (char *p=buffer; *p; p++)
    if (*p=='\n')
      { if (x>=0 && y>=0) { y-=FontHeight; glRasterPos2i(x,y); } }
    else
      { glutBitmapCharacter(CurrentFont,*p); }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glPopAttrib();

  glPopAttrib();
  glPopAttrib();
  glPopAttrib();
}
