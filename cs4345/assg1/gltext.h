//============================================================================
// gltext.h : bitmap text drawing routine; works like "printf"
//============================================================================
#ifndef _GLTEXT_H_
#define _GLTEXT_H_

void* GetCurrentFont();
void SetCurrentFont(void *font);
void SetCurrentFont(int fontid);

int GetFontHeight(void *font);
int GetFontHeight();
int GetFontHeight(int fontid);

void gltext(int x, int y, char *format, ...);

#endif
