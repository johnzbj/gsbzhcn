#include "DxFont.h"

DxFont::DxFont ()
{
  D3dDevice = NULL;
  TextSprite = NULL;
  VAdjust = 0;
}

DxFont::~DxFont()
{
    ClearAll();
}

void
DxFont::ReadIni ()
{
  char ini_name[MAX_PATH];
  GetModuleFileName (NULL, ini_name, MAX_PATH);
  *(strrchr (ini_name, '\\') + 1) = '\0';
  lstrcat (ini_name, INI_NAME);
  GetPrivateProfileString ("FONT16", "Name", "ÎÄÈªæäÎ¢Ã×ºÚ", Font16.name, 80, ini_name);
  Font16.size = GetPrivateProfileInt ("FONT16", "Size", 20, ini_name);
  Font16.weight = GetPrivateProfileInt ("FONT16", "Weight", 400, ini_name);
  Font16.outline = GetPrivateProfileInt ("Font16", "Outline", 1, ini_name);

  GetPrivateProfileString ("FONT20", "Name", "ÎÄÈªæäÎ¢Ã×ºÚ", Font20.name, 80, ini_name);
  Font20.size = GetPrivateProfileInt ("FONT20", "Size", 26, ini_name);
  Font20.weight = GetPrivateProfileInt ("FONT20", "Weight", 700, ini_name);
  Font20.outline = GetPrivateProfileInt ("Font20", "Outline", 2, ini_name);

  VAdjust = GetPrivateProfileInt ("Main", "VAdjust", 2, ini_name);
}

void
DxFont::ClearAll ()
{
  SAFE_RELEASE (Font16.handle);
  SAFE_RELEASE (Font20.handle);
  SAFE_RELEASE (TextSprite);
  return;
}

void
DxFont::InitAll (IDirect3DDevice9 * pd3dDevice)
{
  ClearAll ();
  ReadIni ();

  D3dDevice = pd3dDevice;

  HDC hdc = GetDC (NULL);
  if (Font16.size > 0)
    {
      Font16.size = -MulDiv (Font16.size, GetDeviceCaps (hdc, LOGPIXELSY), 72);
    }
  if (Font20.size > 0)
    {
      Font20.size = -MulDiv (Font20.size, GetDeviceCaps (hdc, LOGPIXELSY), 72);
    }
  ReleaseDC (NULL, hdc);

  AddFontResourceEx ("wqy-microhei.ttc", FR_PRIVATE, 0);

  D3DXCreateFont (D3dDevice,    // D3D device
                  Font16.size,  // Height
                  0,            // Width
                  Font16.weight,        // Weight
                  1,            // MipLevels, 0 = autogen mipmaps
                  FALSE,        // Italic
                  DEFAULT_CHARSET,      // CharSet
                  OUT_DEFAULT_PRECIS,   // OutputPrecision
                  ANTIALIASED_QUALITY,  // Quality
                  VARIABLE_PITCH | FF_DONTCARE, // PitchAndFamily
                  Font16.name,  // pFaceName
                  &Font16.handle);      // ppFont

  D3DXCreateFont (D3dDevice,    // D3D device
                  Font20.size,  // Height
                  0,            // Width
                  Font20.weight,        // Weight
                  1,            // MipLevels, 0 = autogen mipmaps
                  FALSE,        // Italic
                  DEFAULT_CHARSET,      // CharSet
                  OUT_DEFAULT_PRECIS,   // OutputPrecision
                  ANTIALIASED_QUALITY,  // Quality
                  VARIABLE_PITCH | FF_DONTCARE, // PitchAndFamily
                  Font20.name,  // pFaceName
                  &Font20.handle);      // ppFont

  CurrentFont = &Font16;

  D3DXCreateSprite (D3dDevice, &TextSprite);

  return;
}



void
DxFont::RenderText (char *Text, RECT * Rc, D3DCOLOR Color, DWORD Flag)
{

  RECT outRc;

  OffsetRect (Rc, 0, VAdjust);

  TextSprite->Begin (D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);
  D3dDevice->SetSamplerState (0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
  D3dDevice->SetSamplerState (0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
  D3dDevice->SetSamplerState (0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

  if (CurrentFont->outline > 0)
    {
      //ÂÖÀªµÄÑÕÉ«³ý4£¬alphaÖµ³ý2
      D3DCOLOR outlineColor = (0x003f3f3f & (Color >> 2)) | (0x7f000000 & (Color >> 1));

      SetRect (&outRc, Rc->left, Rc->top, Rc->right, Rc->bottom);

      OffsetRect (&outRc, -CurrentFont->outline, -CurrentFont->outline);
      CurrentFont->handle->DrawText (TextSprite, Text, -1, &outRc, Flag, outlineColor);

      OffsetRect (&outRc, CurrentFont->outline * 2, 0);
      CurrentFont->handle->DrawText (TextSprite, Text, -1, &outRc, Flag, outlineColor);

      OffsetRect (&outRc, 0, CurrentFont->outline * 2);
      CurrentFont->handle->DrawText (TextSprite, Text, -1, &outRc, Flag, outlineColor);

      OffsetRect (&outRc, -CurrentFont->outline * 2, 0);
      CurrentFont->handle->DrawText (TextSprite, Text, -1, &outRc, Flag, outlineColor);
    }

  CurrentFont->handle->DrawText (TextSprite, Text, -1, Rc, Flag, Color);

  TextSprite->End ();
  return;
}

void
DxFont::DrawText (char *Text, float X, float Y, D3DCOLOR Color, int Flag, float Adjust)
{
  RECT rc;
  switch (Flag)
    {
    case 0:
      //left align
      SetRect (&rc, X, Y, 0, 0);
      RenderText (Text, &rc, Color, DT_NOCLIP);
      break;

    case 1:
      //right align
      SetRect (&rc, X, Y, X + Adjust, Y);
      RenderText (Text, &rc, Color, DT_NOCLIP | DT_RIGHT);
      break;

    case 2:
    default:
      //center
      SetRect (&rc, X, Y, X + Adjust, Y);
      RenderText (Text, &rc, Color, DT_NOCLIP | DT_CENTER);
      break;
    }

  return;
}

void
DxFont::ChooseFont (char *FontName)
{
  if (strcmp (FontName, "cwfont20.dds") == 0)
    {
      CurrentFont = &Font20;
    }
  else
    {
      CurrentFont = &Font16;
    }
  return;
}

void
DxFont::DrawTextBox (char *Text, float X, float Y, float Width, D3DCOLOR Color, float *Size)
{
  RECT rc;
  SetRect (&rc, X, Y, X + Width, 0);
  RenderText (Text, &rc, Color, DT_NOCLIP | DT_WORDBREAK);
  
  CurrentFont->handle->DrawText (NULL, Text, -1, &rc, DT_CALCRECT | DT_WORDBREAK, 0);
  Size[0] = rc.right;
  Size[1] = rc.top;
  
  return;
}

void DxFont::GetTextBoxSize(char *Text, int *Size)
{
	RECT rc = {0, 0, 240, 0};
	CurrentFont->handle->DrawText (NULL, Text, -1, &rc, DT_CALCRECT | DT_WORDBREAK, 0);
	Size[0] = rc.right;
	Size[1] = rc.bottom;
}

int DxFont::GetTextWidht(char *Text)
{
	RECT rc = {0, 0, 0, 0};
	CurrentFont->handle->DrawText (NULL, Text, -1, &rc, DT_CALCRECT, 0);
	return rc.right;
}