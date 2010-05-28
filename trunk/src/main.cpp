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

  lstrcat (exeName, " ");
  lstrcat (exeName, lpCmdLine);

  try
  {
    if (0 == CreateProcess (NULL,       // If No module name (then use command line).
                            exeName,    // Command line.
                            NULL,       // Process handle not inheritable.
                            NULL,       // Thread handle not inheritable.
                            FALSE,      // Set handle inheritance to FALSE.
                            CREATE_SUSPENDED | CREATE_DEFAULT_ERROR_MODE,       //CREATE_DEFAULT_ERROR_MODE,//DEBUG_ONLY_THIS_PROCESS,    // creation flags.
                            NULL,       // Use parent's environment block.
                            NULL,       // Use parent's starting directory.
                            &sif,       // Pointer to STARTUPINFO structure.
                            &pi))       // Pointer to PROCESS_INFORMATION structure.
      {
        throw "Create Process Failed";
      }

    char *dllName = DLL_NAME;
    int dllNameSize = strlen (dllName) + 1;
    char *dllNameP = (char *) VirtualAllocEx (pi.hProcess, NULL, dllNameSize, MEM_COMMIT,
                                              PAGE_READWRITE);
    if (dllNameP == NULL)
      throw "VirtualAllocEx Failed";

    if (0 == WriteProcessMemory (pi.hProcess, dllNameP, (void *) dllName, dllNameSize, NULL))
      throw "WriteProcessMemory Failed";

    PTHREAD_START_ROUTINE LoadLibAddress = (PTHREAD_START_ROUTINE)
      GetProcAddress (GetModuleHandle ("Kernel32"), "LoadLibraryA");
    if (LoadLibAddress == NULL)
      throw "GetProcAddress Failed";

    HANDLE thread = CreateRemoteThread (pi.hProcess, NULL, 0, LoadLibAddress, dllNameP, 0, NULL);
    if (thread == NULL)
      throw "CreateRemoteThread Failed";

    if (WaitForSingleObject (thread, INFINITE) == WAIT_FAILED)
      {
        throw "WaitForSingleObject Failed";
      }

    VirtualFreeEx (pi.hProcess, dllNameP, dllNameSize, MEM_DECOMMIT);
    ResumeThread (pi.hThread);
  }
  catch (char *err)
  {
    if (pi.hProcess != NULL)
      {
        TerminateProcess (pi.hProcess, 0);
      }
    char tmp[255];
    wsprintf (tmp, "%s \nError code: 0x%08x", err, GetLastError ());
    MessageBox (NULL, tmp, "Unexpected load error", MB_OK | MB_ICONERROR);
  }

  return 0;
}
