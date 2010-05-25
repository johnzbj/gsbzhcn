#include <windows.h>
#include "config.h"

int APIENTRY
WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  STARTUPINFO sif;
  PROCESS_INFORMATION pi;
  ZeroMemory (&sif, sizeof (STARTUPINFO));
  ZeroMemory (&pi, sizeof (PROCESS_INFORMATION));
  sif.cb = sizeof (STARTUPINFO);

  char exeName[MAX_PATH];
  char iniName[MAX_PATH];
  GetModuleFileName (NULL, iniName, MAX_PATH);
  *(strrchr (iniName, '\\') + 1) = '\0';
  lstrcat (iniName, INI_NAME);
  GetPrivateProfileString ("MAIN", "FileName", "gsb.exe", exeName, MAX_PATH, iniName);

  if (!CreateProcess (NULL,     // If No module name (then use command line).
                      exeName,  // Command line.
                      NULL,     // Process handle not inheritable.
                      NULL,     // Thread handle not inheritable.
                      FALSE,    // Set handle inheritance to FALSE.
                      CREATE_SUSPENDED | CREATE_DEFAULT_ERROR_MODE,     //CREATE_DEFAULT_ERROR_MODE,//DEBUG_ONLY_THIS_PROCESS,    // creation flags.
                      NULL,     // Use parent's environment block.
                      NULL,     // Use parent's starting directory.
                      &sif,     // Pointer to STARTUPINFO structure.
                      &pi))     // Pointer to PROCESS_INFORMATION structure.
    {
      MessageBox (0, "Unexpected load error", "Create Process Failed", MB_OK + MB_ICONERROR);
      return -1;
    }

  char *dllName = DLL_NAME;
  int dllNameSize = strlen (dllName) + 1;
  char *dllNameP = (char *) VirtualAllocEx (pi.hProcess, NULL, dllNameSize, MEM_COMMIT,
                                            PAGE_READWRITE);
  WriteProcessMemory (pi.hProcess, dllNameP, (void *) dllName, dllNameSize, NULL);

  PTHREAD_START_ROUTINE LoadLibAddress = (PTHREAD_START_ROUTINE)
    GetProcAddress (GetModuleHandle ("Kernel32"), "LoadLibraryA");
  
  HANDLE thread = CreateRemoteThread (pi.hProcess, NULL, 0, LoadLibAddress, dllNameP, 0, NULL);

  WaitForSingleObject (thread, INFINITE);
  
  VirtualFreeEx (pi.hProcess, dllNameP, dllNameSize, MEM_DECOMMIT);
  
  ResumeThread (pi.hThread);
  return 0;
}
