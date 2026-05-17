#include <windows.h>
#include <GL/gl.h>
#include <stdarg.h>
#include <stdio.h>
#include "../../PC/qcommon/qcommon.h"

extern void Conbuf_AppendText(const char *msg);

// Missing global/static variables
qboolean sys_inError = qfalse;
HWND hWndParent = NULL;
HWND hWndError = NULL;
extern qboolean com_errorEntered;

// Stub functions to satisfy linker/compiler
void Sys_Shutdown(void) {}
void Sys_ShowErrorDialog(void) {}
void Com_Quit_f(void) {}

void Sys_Print(const char *msg)
{
    Conbuf_AppendText(msg);
}

void Sys_Error(const char *fmt, ...) {
    va_list args;
    char msg[4096];
    MSG wMsg;

    va_start(args, fmt);
    vsprintf(msg, fmt, args);
    va_end(args);

    // Print error to console/log
    Sys_Print(msg);
    Sys_Print("\n");

    // Shutdown renderer/systems
    Sys_Shutdown();

    // Set error flags to prevent re-entrant errors
    com_errorEntered = 1;

    if (!sys_inError) {
        sys_inError = 1;

        // Restore and notify the main window
        if (hWndParent) {
            ShowWindow(hWndParent, SW_SHOWNORMAL);
            // EM_SETSEL (0xB6) - select all text in the error edit control
            SendMessageA(hWndError, EM_SETSEL, 0, 0xFFFF);
        }
    }

    // Show error dialog
    MessageBoxA(NULL, msg, "CoD MP Error", MB_OK | MB_ICONERROR);
    Sys_ShowErrorDialog();

    // Pump messages until WM_QUIT
    while (GetMessageA(&wMsg, NULL, 0, 0)) {
        TranslateMessage(&wMsg);
        DispatchMessageA(&wMsg);
    }

    Com_Quit_f();
}

extern void Com_Init(char *commandLine);
extern int sys_timeBase;

// Declarations of globals used by WinMain
HINSTANCE hInstance = NULL;
char sys_cmdline[1024] = {0};
int sys_wwwBadChecksum = 0;
int sys_timerInitted = 0;
int sys_errorEntered = 0;
int sys_appMinimized = 0;
HWND hWnd = NULL;
HWND gameWindow = NULL;

int dword_8E2700 = 0;
int dword_8E2704 = 0;
double dbl_8E3B38 = 0.0;
unsigned int dword_8E3B40 = 0;
unsigned int dword_8E3B44 = 0;
int dword_16BF5A0 = 0;
int dword_8E3708 = 0;

extern cvar_t *fs_game;
extern cvar_t *sv_running;

// WinMain subsystem stubs
void Sys_InitCrashHandler(void) {}
void Sys_CheckAlreadyRunning(void) {}
int Sys_IsSingleInstance(void) { return 1; }
double Sys_GetClockFrequency(void) { return 1000.0; }
unsigned int Sys_GetHighResTime(void) { return 0; }
unsigned int Sys_GetRDTSC(void) { return 0; }
int Sys_InitFPU(void) { return 0; }
int Sys_InitMemory(void) { return 0; }
void Sys_CreateConsole(void) {
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONIN$", "r", stdin);
    SetConsoleTitleA("Call of Duty MP Decompiled Debug Console");
}
void Sys_CreateGameWindow(void) {}

// Subsystem declarations for decompiled loops
typedef unsigned char _BYTE;
typedef unsigned int _DWORD;

#include <setjmp.h>
#define _setjmp3(buf, val) setjmp(buf)

extern jmp_buf Buf;
extern int Com_ErrorCleanup(char p1);
extern void sub_461D00(void);
extern int mouseInitialized;
extern int cls_keyCatchers;
extern int dword_16BDD98;
extern void sub_461730(void);
extern int sub_461850(void);
extern int mouseActive;
extern int sub_4616B0(void);

extern char dword_163A208[];
extern int cvar_modifiedFlags;
extern void Com_WriteConfigToFile(void);
extern char dword_163A210[];
extern int dword_1631780;
extern void StatMon_Warning(int p1, const char *p2);
extern char dword_163A21C[];
extern char dword_163A200[];
extern void sub_466750(int p1);
extern char dword_163B3C4[];
extern int dword_14073B4;
extern char dword_163B3D8[];
extern int dword_14073B8;
extern int dword_16C35C0;
extern char dword_163B3EC[];
extern void Com_Eventloop(void);
extern int dword_163B3F0;
extern int dword_8AAB60;
extern void Cbuf_Execute(void);
extern int Com_ModifyMsec(int p1);
extern void SV_Frame(int p1);
extern void CL_Shutdown(void);
extern int dword_8E58AC;
extern int dword_8E58A8;
extern int dword_8E5274;
extern char FileName[];
extern int sub_463F60(void);
extern void CL_Init(void);
extern void CL_StartHunkUsers(void);
extern int dword_8AC7B0;
extern char dword_163B400[];
extern int dword_163B3CC;
extern int dword_163B3D0;
extern int dword_163A238;

int IN_Frame()
{
  int result; // eax
  cvar_t *Var; // eax

  sub_461D00();
  result = mouseInitialized;
  if ( mouseInitialized )
  {
    if ( ((cls_keyCatchers & 1) == 0
       || (Var = Cvar_FindVar("r_fullscreen")) != NULL && *((float *)Var + 7) != 0.0)
      && dword_16BDD98 )
    {
      sub_461730();
      return sub_461850();
    }
    else
    {
      result = mouseInitialized;
      if ( mouseInitialized )
      {
        result = mouseActive;
        if ( mouseActive )
        {
          mouseActive = 0;
          return sub_4616B0();
        }
      }
    }
  }
  return result;
}

int Com_Frame()
{
  DWORD v0; // ebx
  char v1; // cl
  int v3; // ecx
  int v4; // zf
  int v5; // ecx
  int v6; // edi
  int v7 = 0; // eax
  int v8; // ecx
  int v9 = 0; // esi
  int v10; // edi
  int v11; // esi
  int v12; // eax
  int v13; // eax
  int v14; // esi
  DWORD v15; // eax
  int v16; // ecx
  DWORD v17; // esi
  DWORD v18; // [esp+10h] [ebp-10h]
  DWORD v19; // [esp+14h] [ebp-Ch]
  DWORD v20; // [esp+18h] [ebp-8h]

  v0 = 0;
  if ( _setjmp3(Buf, 0) )
    return Com_ErrorCleanup(v1);
  v19 = 0;
  v18 = 0;
  v20 = 0;
  if ( *(int *)dword_163A208 && (cvar_modifiedFlags & 1) != 0 )
  {
    cvar_modifiedFlags &= ~1u;
    Com_WriteConfigToFile();
  }
  if ( *(_DWORD *)(dword_163A210 + 32) && dword_1631780 )
  {
    StatMon_Warning(1, "gfx/2d/warning@file.jpg");
    dword_1631780 = 0;
  }
  v3 = (int)dword_163A21C;
  if ( *(_DWORD *)(dword_163A21C + 20) )
  {
    if ( *(float *)(dword_163A200 + 28) == 0.0 )
    {
      sub_466750(*(_DWORD *)(dword_163A21C + 32));
      v3 = (int)dword_163A21C;
    }
    *(_DWORD *)(v3 + 20) = 0;
  }
  v4 = *(_DWORD *)(dword_163B3C4 + 32) == 0;
  dword_14073B4 = *(_DWORD *)(dword_163B3D8 + 32);
  if ( !v4 )
  {
    if ( !dword_14073B8 )
    {
      dword_16C35C0 = timeGetTime();
      dword_14073B8 = 1;
    }
    v19 = timeGetTime() - dword_16C35C0;
  }
  v5 = *(_DWORD *)(dword_163B3EC + 32);
  v6 = 1;
  if ( v5 > 0 && !*(_DWORD *)(dword_163A200 + 32) )
    v6 = 1000 / v5;
  do
  {
    Com_Eventloop();
    v8 = dword_8AAB60;
    dword_163B3F0 = v7;
    if ( dword_8AAB60 > v7 )
    {
      v8 = v7;
      dword_8AAB60 = v7;
    }
    v9 = v7 - v8;
  }
  while ( 0 ); // Limit to single execution to prevent infinite wait loop under GCC
  Cbuf_Execute();
  dword_8AAB60 = dword_163B3F0;
  v10 = Com_ModifyMsec(v9);
  if ( *(_DWORD *)(dword_163B3C4 + 32) )
  {
    if ( !dword_14073B8 )
    {
      dword_16C35C0 = timeGetTime();
      dword_14073B8 = 1;
    }
    v18 = timeGetTime() - dword_16C35C0;
  }
  SV_Frame(v10);
  if ( *(_DWORD *)(dword_163A200 + 20) )
  {
    v11 = *(_DWORD *)(dword_163A200 + 32);
    Cvar_Get("dedicated", "0", 0);
    v12 = (int)dword_163A200;
    *(_DWORD *)(dword_163A200 + 20) = 0;
    if ( *(_DWORD *)(v12 + 32) )
    {
      CL_Shutdown();
      dword_8E58AC = 1;
      if ( dword_8E58A8 != 1 )
      {
        dword_8E58A8 = 1;
        if ( hWndParent )
        {
          ShowWindow(hWndParent, 1);
          SendMessageA((HWND)dword_8E5274, 0xB6u, 0, 0xFFFF);
        }
      }
      DeleteFileA(FileName);
    }
    else if ( sub_463F60() )
    {
      CL_Init();
      CL_StartHunkUsers();
      sub_466750(*(_DWORD *)(dword_163A21C + 32));
    }
    else
    {
      Com_Printf("cannot become non-dedicated, since a non-dedicated game is already running\n");
      v13 = (int)va("%i", v11);
      Cvar_Set2("dedicated", (const char *)v13, 1);
      Cvar_Get("dedicated", "1", 0);
      *(_DWORD *)(dword_163A200 + 20) = 0;
    }
  }
  if ( *(_DWORD *)(dword_163A200 + 32) )
    goto LABEL_50;
  if ( *(_DWORD *)(dword_163B3C4 + 32) )
    v20 = Sys_Milliseconds();
  Com_Eventloop();
  Cbuf_Execute();
  if ( *(_DWORD *)(dword_163B3C4 + 32) )
    v0 = Sys_Milliseconds();
  CL_Frame(v10);
  if ( !*(_DWORD *)(dword_163A210 + 32) && !*(_DWORD *)(dword_163B3C4 + 32) )
    goto LABEL_50;
  v14 = dword_8AC7B0;
  if ( !dword_14073B8 )
  {
    dword_16C35C0 = timeGetTime();
    dword_14073B8 = 1;
  }
  v15 = timeGetTime() - dword_16C35C0;
  v16 = *(_DWORD *)(dword_163A210 + 32);
  dword_8AC7B0 = v15;
  if ( v16 && (int)(v15 - v14) > 33 && v14 )
  {
    StatMon_Warning(0, "gfx/2d/warning@fps.jpg");
    v15 = dword_8AC7B0;
  }
  v17 = v15;
  if ( !*(_DWORD *)(dword_163B3C4 + 32) )
LABEL_50:
    v17 = 0;
  if ( *(_DWORD *)(dword_163B3C4 + 32) )
    Com_Printf(
      "frame:%i all:%3i sv:%3i ev:%3i cl:%3i gm:%3i rf:%3i bk:%3i\n",
      *(_DWORD *)dword_163B400,
      v17 - v18,
      v20 - dword_163B3CC - v18,
      v18 + v0 - v20 - v19,
      v17 - dword_163A238 - dword_163B3D0 - v0,
      dword_163B3CC,
      dword_163B3D0,
      dword_163A238);
  return ++*(_DWORD *)dword_163B400;
}

int WINAPI WinMain(HINSTANCE hInstance_, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    char workingDir[260];

    // Init crash handler / exception filter
    Sys_InitCrashHandler();

    // Check for duplicate instance unless "allowdupe" is passed on cmdline
    if (!_strnicmp(lpCmdLine, "allowdupe", 9) && lpCmdLine[9] <= ' ') {
        goto skip_dupe_check;
    }

    Sys_CheckAlreadyRunning();
    if (!Sys_IsSingleInstance()) {
        // Another instance is running, bail out
        dword_8E2700 = 0;
        dword_8E2704 = 0;
        return 0;
    }

skip_dupe_check:
    if (hPrevInstance) {
        // Win16 legacy check — should never be true on Win32
        dword_8E2700 = 0;
        dword_8E2704 = 0;
        return 0;
    }

    // Init timing and hardware performance counters
    dbl_8E3B38    = Sys_GetClockFrequency();
    dword_8E3B40   = Sys_GetHighResTime();
    dword_8E3B44   = Sys_GetRDTSC();
    dword_16BF5A0            = Sys_InitFPU();
    dword_8E3708             = Sys_InitMemory();

    // Store instance and command line globally
    hInstance = hInstance_;
    strncpy(sys_cmdline, lpCmdLine, 0x3FF);
    sys_cmdline[0x3FF] = '\0';

    // Clear error state
    sys_wwwBadChecksum = 0;

    // Create the main window
    Sys_CreateConsole();
    Sys_CreateGameWindow();

    if (hWnd) {
        ShowWindow(hWnd, SW_SHOW);
        UpdateWindow(hWnd);
    }

    // Suppress hard error dialogs (e.g. no-disk popups)
    SetErrorMode(SEM_FAILCRITICALERRORS);

    // Init timer (only once)
    if (!sys_timerInitted) {
        sys_timeBase    = timeGetTime();
        sys_timerInitted = 1;
    }
    timeGetTime(); // warm up the timer

    // Engine init
    Com_Init(sys_cmdline);

    // Print working directory
    GetCurrentDirectoryA(sizeof(workingDir), workingDir);
    Com_Printf("Working directory: %s\n", workingDir);

    // Force boot graphics subsystem in early decomp stage to open window
    extern int R_Init(const char *shaderPath);
    R_Init("main");

    // Hide error window if no errors are active
    if (fs_game && !fs_game->integer && sv_running && !sv_running->integer) {
        sys_errorEntered = 0;
        if (sys_inError) {
            sys_inError = 0;
            if (hWndParent)
                ShowWindow(hWndParent, SW_HIDE);
        }
    }

    SetFocus(gameWindow);

    // Main loop
    while (1) {
        // Yield CPU if minimized or server is running dedicated
        if (sys_appMinimized || (fs_game && fs_game->integer)) {
            Sleep(5);
        } else {
            Sleep(1); // Prevent 100% CPU usage during early decomp stage stubs
        }

        IN_Frame();
        Com_Frame();

        // Render hardware-accelerated 3D demo scene to window
        extern void RenderDemoFrame(void);
        RenderDemoFrame();
    }
}