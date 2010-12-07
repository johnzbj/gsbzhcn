#ifndef __DXFONT_H__
#define __DXFONT_H__
#include <d3d9.h>
#include <d3dx9core.h>
#include "config.h"

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif
#ifndef SAFE_LOSTDEVICE
#define SAFE_LOSTDEVICE(p) { if(p) { (p)->OnLostDevice();} }
#endif

struct FontStruct
{
  ID3DXFont *handle;
  char name[80];
  int size;
  int weight;
  int outline;
};

class DxFont
{
private:
  IDirect3DDevice9 *D3dDevice;
  ID3DXSprite *TextSprite;
  int VAdjust;
  FontStruct Font16, Font20, *CurrentFont;

  void ReadIni ();
  void RenderText (char *Text, RECT * Rc, D3DCOLOR Color, DWORD Flag);

public:
  DxFont ();
  ~DxFont();

  void InitAll (IDirect3DDevice9 * pd3dDevice);
  void ClearAll ();
  void DrawText (char *Text, float X, float Y, D3DCOLOR Color, int Flag, float Adjust);
  void ChooseFont (char *FontName);
  void DrawTextBox (char *Text, float X, float Y, float Width, D3DCOLOR Color, float *Size);
  void GetTextBoxSize(char *Text, int *Size);
  int GetTextWidth(char *Text);
  void Reset ();
};

#endif
