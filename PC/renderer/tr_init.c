/* Generated source skeleton from xSYM metadata. */
#ifndef __UNIMPLEMENTED__
#define __UNIMPLEMENTED__() do { } while (0)
#endif

#include "../qcommon/qcommon.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#include <GL/gl.h>

#define ri_Printf(level, fmt, ...) Com_Printf(fmt, ##__VA_ARGS__)

typedef unsigned char _BYTE;
typedef unsigned short _WORD;
typedef unsigned int _DWORD;

// Macro remappings for decompiled variables & functions
#define dword_16D8940 Cvar_Get
#define dword_16D8924 Com_Error
#define dword_16C3ED4 glGetString
#define dword_16C40E0 wglGetProcAddress
#define dword_16D8948 Cvar_Set

extern void Swap_Init(void);
extern void Com_NoiseInit(void);
extern void R_Register(void);
extern void R_InitAllocators(void);
extern void R_InitImages(void);
extern void R_InitVertexPrograms(void);
extern void R_InitShaders(const char *shaderPath);
extern void R_ModelInit(void);
extern void R_InitFreeType(void);
extern void R_SetHwLightGlobals(void);
extern void R_ClearLightVisCache(void);
extern void R_InitDebug(void);
extern void R_InitWater(void);

// Declarations of decompiled OpenGL globals
extern int vidWidth;
extern char *dword_16C3A60;
extern void dword_16C405C(int p1, int *p2);
extern int dword_16C3A74;
extern char dword_16C388C[];
extern int dword_16C3A7C;
extern void GfxInfo_f(void);
extern int dword_16C3980;
extern void GL_SetDefaultState(void);
extern int dword_16D8888;
extern char dword_16C3A40[];
extern int dword_16D8884;
extern char dword_16C3B0C[];
extern int dword_16D888C;
extern int dword_16C3890;
extern void sub_52E7B9(void *p);
extern unsigned int _security_cookie;

// GLimp_Init dependencies
extern int GLW_CheckOSVersion(void);
extern char String;
extern HINSTANCE hInstance;
extern int dword_19BFFE0;
extern cvar_t *dword_19BFFC8;
extern void sub_50A7A0(void);
extern int dword_16C3A64;
extern char *dword_16C3A68;
extern int dword_16C3A6C;
extern int dword_16C43F0;
extern int dword_16C45A0;
extern int dword_16C3A70;
extern void sub_50A870(void);
extern void GLW_InitExtensions(void);
extern int dword_16C3A8C;
extern char dword_16C3A14[];
extern char byte_1406CE8[];
extern char byte_1406EE7;
extern char byte_1406CE9;
extern char byte_14070E7;
extern char byte_1406EE9;
extern char byte_14072E7;
extern char byte_14070E9;
extern char byte_1406E53;
extern char byte_1406EE8[];
extern char byte_14070E8[];
extern int nullsub_7(void);

static int Q_stricmpn(int n, const char *s1, const char *s2) {
    return _strnicmp(s1, s2, n);
}

int GLimp_Init()
{
  int v0; // esi
  int v1; // eax
  int v2; // eax
  int (*v3)(void); // eax
  char *v4; // edx
  int v5; // esi
  HWND DesktopWindow; // eax
  HDC DC; // esi
  HWND v8; // eax
  int i; // eax

  v0 = (int)dword_16D8940("r_lastValidRenderer", "(uninitialized)", 1);
  ri_Printf(0, "Initializing OpenGL subsystem\n");
  if ( !GLW_CheckOSVersion() )
    dword_16D8924(0, "EXE_ERR_BAD_WINDOWS_VER");
  v1 = (int)dword_16D8940("win_hinstance", &String, 0);
  sscanf(*(const char *const *)(v1 + 4), "%i", (int *)&hInstance);
  v2 = (int)dword_16D8940("win_wndproc", &String, 0);
  sscanf(*(const char *const *)(v2 + 4), "%i", &dword_19BFFE0);
  dword_19BFFC8 = dword_16D8940("r_allowSoftwareGL", "0", 32);
  sub_50A7A0();
  dword_16C3A64 = (int)dword_16C3ED4(7936);
  dword_16C3A60 = (int)dword_16C3ED4(7937);
  dword_16C3A68 = (char *)dword_16C3ED4(7938);
  dword_16C3A6C = (int)dword_16C3ED4(7939);
  v3 = (int (*)(void))dword_16C40E0("wglGetExtensionsStringEXT");
  dword_16C43F0 = (int)v3;
  dword_16C45A0 = (int)v3;
  if ( v3 )
    dword_16C3A70 = v3();
  sub_50A870();
  v4 = *(char **)(v0 + 4);
  if ( !v4 || (v5 = (int)dword_16C3A60) == 0 || Q_stricmpn(99999, v4, (char *)dword_16C3A60) )
  {
    dword_16D8948("r_textureMode", "GL_LINEAR_MIPMAP_NEAREST");
    v5 = (int)dword_16C3A60;
  }
  dword_16D8948("r_lastValidRenderer", (const char *)v5);
  GLW_InitExtensions();
  dword_16C3A8C = 0;
  if ( !*(_DWORD *)(dword_16C3A14 + 32) )
  {
    DesktopWindow = GetDesktopWindow();
    DC = GetDC(DesktopWindow);
    dword_16C3A8C = GetDeviceGammaRamp(DC, byte_1406CE8);
    v8 = GetDesktopWindow();
    ReleaseDC(v8, DC);
    if ( dword_16C3A8C )
    {
      if ( (unsigned __int8)byte_1406EE7 <= (unsigned __int8)byte_1406CE9
        || (unsigned __int8)byte_14070E7 <= (unsigned __int8)byte_1406EE9
        || (unsigned __int8)byte_14072E7 <= (unsigned __int8)byte_14070E9 )
      {
        dword_16C3A8C = 0;
        ri_Printf(2, "WARNING: device has broken gamma support, generated gamma.dat\n");
      }
      if ( byte_1406E53 == -1 )
      {
        ri_Printf(2, "WARNING: suspicious gamma tables, using linear ramp for restoration\n");
        for ( i = 0; i < 255; ++i )
        {
          *(_WORD *)&byte_1406CE8[2 * i] = (_WORD)i << 8;
          *(_WORD *)&byte_1406EE8[2 * i] = (_WORD)i << 8;
          *(_WORD *)&byte_14070E8[2 * i] = (_WORD)i << 8;
        }
      }
    }
  }
  return nullsub_7();
}

void InitOpenGL()
{
  char *v0; // eax
  _BYTE *v1; // edx
  char v2; // cl
  _BYTE *v3; // esi
  int v5; // eax
  int v6; // ecx
  int v7; // [esp+10h] [ebp-408h] BYREF
  _BYTE v8[1024]; // [esp+14h] [ebp-404h] BYREF
  unsigned int v9; // [esp+414h] [ebp-4h]
  unsigned int retaddr = 0; // [esp+418h] [ebp+0h]

  // Stack cookie safety check mapping
  v9 = 0;

  if ( vidWidth )
    goto LABEL_18;
  GLimp_Init();
  v0 = (char *)dword_16C3A60;
  v1 = &v8[-(int)dword_16C3A60];
  do
  {
    v2 = *v0;
    v0[(int)v1] = *v0;
    ++v0;
  }
  while ( v2 );
  v3 = v8;
  if ( v8[0] )
  {
    do
      *v3 = tolower((char)*v3);
    while ( *++v3 );
  }
  dword_16C405C(3379, &v7);
  v5 = v7;
  dword_16C3A74 = v7;
  if ( v7 > 0 )
  {
    if ( v7 <= 0x2000 )
      goto LABEL_11;
    v5 = 0x2000;
  }
  else
  {
    v5 = 0;
  }
  dword_16C3A74 = v5;
LABEL_11:
  v6 = *(_DWORD *)(dword_16C388C + 32);
  if ( v6 >= 1024 && v5 > v6 )
  {
    do
    {
      v5 >>= 1;
      dword_16C3A74 = v5;
    }
    while ( v5 > *(_DWORD *)(dword_16C388C + 32) );
  }
  dword_16C405C(3377, &v7);
  dword_16C3A7C = v7;
  if ( v7 > 0 )
  {
    if ( v7 > 16 )
      dword_16C3A7C = 16;
  }
  else
  {
    dword_16C3A7C = 8;
  }
LABEL_18:
  GfxInfo_f();
  dword_16C3980 = 0;
  GL_SetDefaultState();
  dword_16D8888 = *(_DWORD *)(dword_16C3A40 + 32) != 0;
  dword_16D8884 = *(_DWORD *)(dword_16C3B0C + 32) != 0;
  dword_16D888C = 4 * (dword_16C3890 == 0) + 35044;
  sub_52E7B9((void *)(retaddr ^ v9));
}

int R_Init(const char *shaderPath)
{
    ri_Printf(0, "----- R_Init -----\n");

    Swap_Init();
    Com_NoiseInit();
    R_Register();
    
    InitOpenGL();
    R_InitAllocators();
    R_InitImages();
    R_InitVertexPrograms();
    R_InitShaders(shaderPath);
    R_ModelInit();
    R_InitFreeType();
    R_SetHwLightGlobals();
    R_ClearLightVisCache();
    
    R_InitDebug();
    R_InitWater();

    ri_Printf(0, "----- finished R_Init -----\n");
    return 0;
}
