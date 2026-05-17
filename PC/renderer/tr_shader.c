#include "../qcommon/qcommon.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <GL/gl.h>

typedef unsigned char _BYTE;
typedef unsigned short _WORD;
typedef unsigned int _DWORD;

#define Q_stricmp _stricmp

typedef struct {
    char name[64];
    GLuint textureId;
    int width;
    int height;
} shaderMap_t;

shaderMap_t g_shaderMap[1024];
int g_shaderMapCount = 1;

int dword_6A0818[1] = {1};
int dword_6A08E0 = 0;
int dword_6A095C = 1024;
int dword_8D4160[1024] = {0};
int dword_6A08C4 = 1;
int dword_6A0940 = 0;

int dword_4A0514_data[32] = {0};
int *dword_4A0514 = dword_4A0514_data;
int dword_4A0450_data[32] = {0};
int *dword_4A0450 = dword_4A0450_data;

static char shader_scratch_buffer[32768] = {0};
int dword_8CC788[13376 / 4] = {0};

#define unk_8CFBC8 (&shader_scratch_buffer[0])
#define dword_8CFC08 (*(int*)&shader_scratch_buffer[64])
#define dword_8CFC14 (*(int*)&shader_scratch_buffer[76])
#define dword_8CFC18 (*(int*)&shader_scratch_buffer[80])
#define unk_8CFD60 (&shader_scratch_buffer[408])

const char aTga_0[] = ".tga";
const char aShaderNameExce[] = "Shader name exceeds 64 chars\n";

void Com_StripExtension(const char *in, char *out) {
    strcpy(out, in);
    char *p = strrchr(out, '.');
    if (p) *p = 0;
}

void Com_DefaultExtension(char *path, int maxSize, const char *extension) {
    if (!strchr(path, '.')) {
        strncat(path, extension, maxSize - strlen(path) - 1);
    }
}

int generateHashValue_1(const char *name) {
    int hash = 0;
    for (int i = 0; name[i]; i++) {
        hash += name[i];
    }
    return hash & 1023;
}

void UpdateDelayLoadImagesForShader(int shader, int p2) {}
void R_SyncRenderThread(void) {}

int ShaderFromShaderType(const char *name, int p2, int p3) { return 0; }
int FindShaderInShaderText(const char *name) { return 0; }
int ParseShader(int *p1, int p2, int p3) { return 0; }
int FinishShader(int p1) { return p1; }
int R_FindImageFile(const char *name, int p2, int p3, int p4, int p5) { return 1; }
int sub_1D94DC(int p1) { return 0; }
int R_BuildShaderFromImage(int img, int p2) { return img; }

int __fastcall R_FindShader(const char *a1, int a2, int a3, int a4)
{
  int *v5; // r28
  int v6; // r25
  int result; // r3
  int i; // r23
  int v11; // r0
  char *v12; // r9
  int v13; // ctr
  char *v14; // r8
  char *v15; // r7
  char *v16; // r6
  char *v17; // r5
  char *v18; // r4
  char *v19; // r3
  char *v20; // r0
  char *v21; // r9
  int v22; // r3
  int ImageFile; // r3
  int v24; // r3
  unsigned int v25; // r26
  int v26; // r3
  int v27; // r3
  int ShaderInShaderText; // [sp+40h] [-C0h] BYREF
  char v29[64]; // [sp+48h] [-B8h] BYREF
  char v30[84]; // [sp+88h] [-78h] BYREF

  v5 = &dword_8CC788[0];
  v6 = a2;
  if ( !*a1 )
    return dword_6A08E0;
  if ( a2 >= 0 && a2 >= dword_6A095C )
    v6 = -2;
  Com_StripExtension(a1, v29);
  for ( i = dword_8D4160[generateHashValue_1(v29)]; i; i = *(_DWORD *)(i + 404) )
  {
    v11 = *(_DWORD *)(i + 64);
    if ( (v11 == v6 || v11 == -1 && v6 >= 0 && a4 == 9) && !Q_stricmp((char*)i, v29) )
    {
      UpdateDelayLoadImagesForShader(i, 0);
      return i;
    }
  }
  R_SyncRenderThread();
  memset(unk_8CFBC8, 0, 408);
  memset(&dword_8CC788[0], 0, 13376);
  Q_strncpyz(unk_8CFBC8, v29, 64);
  dword_8CFC08 = v6;
  if ( v6 == -1 )
    dword_8CFC14 = 512;
  v12 = (char *)unk_8CFD60;
  v13 = 4;
  do
  {
    v5[48] = (int)v12;
    v5[98] = (int)(v12 + 272);
    v5[148] = (int)(v12 + 544);
    v14 = v12 + 2448;
    v15 = v12 + 2720;
    v5[198] = (int)(v12 + 816);
    v16 = v12 + 2992;
    v5[248] = (int)(v12 + 1088);
    v17 = v12 + 3264;
    v5[298] = (int)(v12 + 1360);
    v18 = v12 + 3536;
    v5[348] = (int)(v12 + 1632);
    v19 = v12 + 3808;
    v5[398] = (int)(v12 + 1904);
    v20 = v12 + 4080;
    v21 = v12 + 2176;
    v5[466] = (int)v21;
    v12 = v21 + 2176;
    v5[516] = (int)v14;
    v5[566] = (int)v15;
    v5[616] = (int)v16;
    v5[666] = (int)v17;
    v5[716] = (int)v18;
    v5[766] = (int)v19;
    v5[816] = (int)v20;
    v5 += 836;
    --v13;
  }
  while ( v13 );
  if ( strrchr(v29, 64) )
  {
    result = ShaderFromShaderType(v29, v6, a4);
    if ( !result )
      return dword_6A08E0;
  }
  else
  {
    ShaderInShaderText = FindShaderInShaderText(v29);
    if ( ShaderInShaderText )
    {
      if ( dword_4A0514[8] ) // offset 32 / sizeof(int)
        sub_1D94DC(0);
      v22 = ParseShader(&ShaderInShaderText, 0, a4);
      if ( !v22 )
      {
        v22 = 3;
        dword_8CFC14 = 3;
        dword_8CFC18 = 16;
      }
      return FinishShader(v22);
    }
    else
    {
      if ( !dword_4A0450[8] || dword_8CFC08 == -4 || dword_8CFC08 == -2 )
      {
        v24 = 48;
        if ( a3 )
          v24 = 3;
        v25 = v24;
        if ( dword_6A0940 && dword_8CFC08 == -1 )
          v25 = v24 & 0xFFFFFF7F;
        Q_strncpyz(v30, a1, 64);
        Com_DefaultExtension(v30, 64, aTga_0);
        ImageFile = R_FindImageFile(v30, 3553, v25, a4, 0);
        if ( !ImageFile )
        {
          v26 = sub_1D94DC(1);
          dword_8CFC14 |= 1u;
          return FinishShader(v26);
        }
      }
      else
      {
        ImageFile = dword_6A08C4;
      }
      v27 = R_BuildShaderFromImage(ImageFile, v6);
      return FinishShader(v27);
    }
  }
  return result;
}

int RE_RegisterShader(const char *a1, int a2)
{
  for (int i = 1; i < g_shaderMapCount; i++) {
      if (!_stricmp(g_shaderMap[i].name, a1)) {
          return i;
      }
  }

  int buffer = 0;
  unsigned short width = 0;
  unsigned short height = 0;
  int format = 0;
  
  extern int LoadTGA(const char *a1, int *a2, unsigned short *a3, unsigned short *a4, int *a5, int a6);
  LoadTGA(a1, &buffer, &width, &height, &format, 1);
  
  if (buffer) {
      GLuint tex;
      glGenTextures(1, &tex);
      glBindTexture(GL_TEXTURE_2D, tex);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)buffer);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      free((void*)buffer);

      int idx = g_shaderMapCount++;
      strcpy(g_shaderMap[idx].name, a1);
      g_shaderMap[idx].textureId = tex;
      g_shaderMap[idx].width = width;
      g_shaderMap[idx].height = height;
      return idx;
  }

  return 0;
}
