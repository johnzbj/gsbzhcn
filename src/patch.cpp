#include <windows.h>
#include "patch.h"
#include "DxFont.h"

DWORD InitEntry = 0x00501bc9;
DWORD InitBack = 0x00501bd0;
#define InitRestoreCode __asm mov eax, dword ptr[esp + 0x9C8];

DWORD ClearEntry = 0x0050130c;
DWORD ClearBack = 0x00501311;
#define ClearRestoreCode  __asm mov eax, 0x00402F90 __asm call eax

DWORD ChooseFontEntry = 0x004ff170;
DWORD ChooseFontBack = 0x004ff179;
#define ChooseFontRestoreCode __asm push ebx __asm mov ebx, eax __asm fld dword ptr [ebx+0x21390]

DWORD DrawTextEntry = 0x004fff50;

DWORD DrawTextBoxEntry = 0x004ff630;
DWORD TextBoxSizeAddress = 0x005aea70;

DWORD GetTextBoxSizeEntry = 0x00448531;
DWORD GetTextBoxSizeBack = 0x00448536;
#define GetTextBoxSizeRestoreCode __asm mov eax, 0x00447f60 __asm call eax;

DWORD GetTextWidthEntry = 0x004ff810;

DWORD *IDirect3DDevice9Address = (DWORD *) 0x0058d554;

DxFont Font;

void
JmpPatch (void *pDest, int pSrc, int nNops = 0)
{
  DWORD OldProt = 0;
  VirtualProtect ((LPVOID) pSrc, 5 + nNops, PAGE_EXECUTE_READWRITE, &OldProt);
  unsigned char jmp = 0xE9;
  memcpy ((LPVOID) pSrc, &jmp, 1);
  DWORD address = (DWORD) pDest - (DWORD) pSrc - 5;
  memcpy ((LPVOID) (pSrc + 1), &address, 4);
  for (int i = 0; i < nNops; ++i)
    *(BYTE *) ((DWORD) pSrc + 5 + i) = 0x90;
  VirtualProtect ((LPVOID) pSrc, 5 + nNops, OldProt, &OldProt);
}

void __declspec(naked)
InitHook ()
{
  __asm pushad;
  Font.InitAll ((IDirect3DDevice9 *) * IDirect3DDevice9Address);
  __asm popad;
  InitRestoreCode;
  __asm jmp InitBack;
}

void __declspec(naked)
ClearHook ()
{
  __asm pushad;
  Font.ClearAll ();
  __asm popad;
  ClearRestoreCode;
  __asm jmp ClearBack;
}

void __declspec(naked)
ChooseFontHook ()
{
  static char *FontName;
  __asm pushad;
  __asm mov FontName, esi;
  Font.ChooseFont (FontName);
  __asm popad;
  ChooseFontRestoreCode;
  __asm jmp ChooseFontBack;
}

void __stdcall
DrawTextHook (char *Text, float X, float Y, D3DCOLOR Color, float Adjust)
{
  int flag;
  __asm mov flag, ecx;
  Font.DrawText (Text, X, Y, Color, flag, Adjust);
}

void __stdcall
DrawTextBoxHook (float X, float Y, float Width)
{
  char *Text;
  D3DCOLOR Color;
  __asm mov Text, ecx;
  __asm mov Color, ebx;
  Font.DrawTextBox (Text, X, Y, Width, Color, (float *) TextBoxSizeAddress);
}

void __declspec(naked)
GetTextBoxSizeHook ()
{
  static char *Text;
  static int *SizeP;
  GetTextBoxSizeRestoreCode;
  __asm pushad;
  __asm
  {
    lea eax, dword ptr[edi + 0x14] 
    mov SizeP, eax
    mov eax, dword ptr[edi + 0x3C]
    cmp dword ptr[eax + 0x18], 0x10
    jb G1
    mov eax, dword ptr[eax + 4]
    mov Text, eax
    jmp G2
G1:
    lea eax, dword ptr[eax + 4]
    mov Text, eax
G2:
  }
  Font.GetTextBoxSize (Text, SizeP);
  __asm popad;
  __asm jmp GetTextBoxSizeBack;
}

int __stdcall
GetTextWidthHook()
{
    char *Text;
    __asm mov Text, ebx;
    return Font.GetTextWidht(Text);
}

bool
ApplyPatch ()
{
  JmpPatch ((void *) InitHook, InitEntry);
  JmpPatch ((void *) ClearHook, ClearEntry);
  JmpPatch ((void *) DrawTextHook, DrawTextEntry);
  JmpPatch ((void *) ChooseFontHook, ChooseFontEntry);
  JmpPatch ((void *) DrawTextBoxHook, DrawTextBoxEntry);
  JmpPatch ((void *) GetTextBoxSizeHook, GetTextBoxSizeEntry);
  JmpPatch ((void *) GetTextWidthHook, GetTextWidthEntry);
  return true;
}
