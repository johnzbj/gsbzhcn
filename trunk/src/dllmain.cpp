#include <windows.h>
#include "patch.h"

BOOL APIENTRY
DllMain (HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        ApplyPatch();
    }
  return true;
}
