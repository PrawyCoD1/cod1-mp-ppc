#include "../qcommon/qcommon.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <GL/gl.h>

#define _byteswap_ushort(val) (val)

extern void FS_ReadFile(const char *qpath, void **buffer);

void R_ReadFile(const char *name, void **buffer) {
    *buffer = NULL;
    FS_ReadFile(name, buffer);
}

void* R_AllocTempMemory(int size, const char *tag) {
    return malloc(size);
}

void R_FreeTempMemory(void *p) {
    free(p);
}

void* R_AllocImage(const char *name, int p2, int p3, int p4, int p5, int p6) {
    static int image_data[1024];
    return image_data;
}

void R_FreeImage(void *img) {}

void GL_Bind(void *img) {}

int UploadImage(void *img, void *data, int p3, int p4, int p5, int p6, int p7, int p8, int p9, void *p10, void *p11, void *p12, int *v40, int *v41) {
    *v40 = 0;
    *v41 = 0;
    return 1;
}

int dword_6A7318 = 0;
int dword_6A0950 = 0;
float flt_1ECC34 = 1.0f;
float flt_1ECC68 = 1.0f;
double dbl_1ECC10 = 1.0;
int dword_6A0944 = 0;
extern int dword_6A0940;

const char aLightmap[] = "*lightmap";
const char aLoadtga[] = "loadTGA";

int LoadDDS(void) { return 0; }
int LoadJPG(const char *name, int *a2, unsigned short *a3, unsigned short *a4, int *a5, int a7) { return 0; }
extern int sub_1D94DC(int p1);

int LoadTGA(const char *a1, int *a2, unsigned short *a3, unsigned short *a4, int *a5, int a6)
{
  int result; // r3
  char *v12; // r31
  int v13; // r26
  int v14; // r27
  int v15; // r29
  int v16; // r30
  int v17; // r3
  unsigned char *v18; // r28
  int i; // r15
  int j; // r14
  char v21; // r0
  char v22; // r0
  char v23; // r4
  char v24; // r4
  char v25; // r0
  char v26; // r5
  char v27; // r16
  char v28; // r17
  char v29; // r18
  char v30; // r19
  int v31; // r21
  int v32; // r22
  int v33; // r20
  unsigned char k; // r15
  void *v36[2]; // [sp+58h] [-78h] BYREF
  unsigned char v37; // [sp+60h] [-70h] BYREF
  char v38; // [sp+61h] [-6Fh]
  unsigned char v39; // [sp+62h] [-6Eh]
  unsigned short v40; // [sp+63h] [-6Dh]
  unsigned short v41; // [sp+65h] [-6Bh]
  char v42; // [sp+67h] [-69h]
  unsigned short v43; // [sp+68h] [-68h]
  unsigned short v44; // [sp+6Ah] [-66h]
  unsigned short v45; // [sp+6Ch] [-64h]
  unsigned short v46; // [sp+6Eh] [-62h]
  unsigned char v47; // [sp+70h] [-60h]
  char v48; // [sp+71h] [-5Fh]

  if ( a6 )
  {
    R_ReadFile(a1, &v36[0]);
    result = (int)v36[0];
    if ( !v36[0] )
      return result;
    v37 = *(unsigned char *)v36[0];
    v38 = *(char *)(v36[0] + 1);
    v39 = *(unsigned char *)(v36[0] + 2);
    v40 = _byteswap_ushort(*(unsigned short *)(v36[0] + 3));
    v41 = _byteswap_ushort(*(unsigned short *)(v36[0] + 5));
    v42 = *(char *)(v36[0] + 7);
    v43 = _byteswap_ushort(*(unsigned short *)(v36[0] + 8));
    v44 = _byteswap_ushort(*(unsigned short *)(v36[0] + 10));
    v45 = _byteswap_ushort(*(unsigned short *)(v36[0] + 12));
    v12 = (char *)(v36[0] + 18);
    v46 = _byteswap_ushort(*(unsigned short *)(v36[0] + 14));
    v47 = *(unsigned char *)(v36[0] + 16);
    v48 = *(char *)(v36[0] + 17);
  }
  else
  {
    result = sub_1D94DC((int)a1);
    if ( result < 18 )
      return result;
    sub_1D94DC((int)&v37);
    sub_1D94DC((int)v36[1]);
    v40 = _byteswap_ushort(v40);
    v41 = _byteswap_ushort(v41);
    v43 = _byteswap_ushort(v43);
    v44 = _byteswap_ushort(v44);
    v45 = _byteswap_ushort(v45);
    v12 = NULL;
    v46 = _byteswap_ushort(v46);
  }
  if ( v39 != 2 && v39 != 10 && v39 != 3 )
    sub_1D94DC(1);
  if ( v38 )
    sub_1D94DC(1);
  if ( v47 != 32 && v47 != 24 && v39 != 3 )
    sub_1D94DC(1);
  v13 = v45;
  v14 = v46;
  *a3 = v45;
  result = v13 * v14;
  *a4 = v14;
  *a5 = 6408;
  if ( a6 )
  {
    v15 = 4;
    if ( (v48 & 0x20) != 0 )
      v16 = 0;
    else
      v16 = -((8 * v13) & 0x7FFF8);
    v17 = (int)R_AllocTempMemory(4 * result, aLoadtga);
    *a2 = v17;
    v18 = (unsigned char *)v17;
    if ( v16 < 0 )
      v18 = (unsigned char *)(v17 + v13 * 4 * (v14 - 1));
    if ( a6 == 2 && (unsigned short)*a3 < (unsigned int)(unsigned short)*a4 )
    {
      *a3 = v14;
      v18 = (unsigned char *)v17;
      *a4 = v13;
      v15 = (4 * v14) & 0x3FFFC;
      if ( (v48 & 0x20) != 0 )
      {
        v16 = 4 - v13 * v15;
      }
      else
      {
        v18 = (unsigned char *)(v17 + 4 * (v14 - 1));
        v16 = -4 - v13 * v15;
      }
    }
    if ( v37 )
      v12 += v37;
    result = v39;
    if ( (unsigned char)(v39 - 2) <= 1u )
    {
      for ( i = 0; ; ++i )
      {
        if ( i >= v14 )
          return result;
        for ( j = 0; j < v13; ++j )
        {
          if ( v47 == 24 )
          {
            v22 = *v12;
            result = (unsigned char)v12[1];
            v23 = v12[2];
            v12 += 3;
            *v18 = v23;
            v18[1] = result;
            v18[2] = v22;
            v18[3] = -1;
            goto LABEL_42;
          }
          if ( v47 >= 0x18u )
          {
            if ( v47 == 32 )
            {
              v24 = *v12;
              result = (unsigned char)v12[1];
              v25 = v12[2];
              v26 = v12[3];
              v12 += 4;
              *v18 = v25;
              v18[1] = result;
              v18[2] = v24;
              v18[3] = v26;
              goto LABEL_42;
            }
          }
          else if ( v47 == 8 )
          {
            v21 = *v12;
            *v18 = *v12++;
            v18[1] = v21;
            v18[2] = v21;
            v18[3] = -1;
            goto LABEL_42;
          }
          result = sub_1D94DC(1);
LABEL_42:
          v18 += v15;
        }
        v18 += v16;
      }
    }
    if ( v39 == 10 )
    {
      v27 = 0;
      v28 = 0;
      v29 = 0;
      v30 = -1;
      v31 = 0;
      while ( v31 < v14 )
      {
        v32 = 0;
        while ( v32 < v13 )
        {
          result = *v12 & 0x7F;
          v33 = result + 1;
          if ( (*v12++ & 0x80) == 0 )
          {
            for ( k = 0; k < v33; ++k )
            {
              if ( v47 == 32 )
              {
                v29 = *v12;
                v28 = v12[1];
                v27 = v12[2];
                v30 = v12[3];
                v12 += 4;
                *v18 = v27;
                v18[1] = v28;
                v18[2] = v29;
                v18[3] = v30;
              }
              else if ( v47 == 24 )
              {
                v29 = *v12;
                v28 = v12[1];
                v27 = v12[2];
                v12 += 3;
                *v18 = v27;
                v18[1] = v28;
                v18[2] = v29;
                v18[3] = -1;
              }
              else
              {
                result = sub_1D94DC(1);
              }
              ++v32;
              v18 += v15;
              if ( v32 == v13 )
              {
                ++v31;
                v32 = 0;
                if ( v31 == v14 )
                  return result;
                v18 += v16;
              }
            }
          }
          else
          {
            if ( v47 == 32 )
            {
              v29 = *v12;
              v28 = v12[1];
              v27 = v12[2];
              v30 = v12[3];
              v12 += 4;
            }
            else if ( v47 == 24 )
            {
              v29 = *v12;
              v30 = -1;
              v28 = v12[1];
              v27 = v12[2];
              v12 += 3;
            }
            else
            {
              sub_1D94DC(1);
            }
            for ( result = 0; (unsigned char)result < v33; ++result )
            {
              *v18 = v27;
              ++v32;
              v18[1] = v28;
              v18[2] = v29;
              v18[3] = v30;
              v18 += v15;
              if ( v32 == v13 )
              {
                ++v31;
                v32 = 0;
                if ( v31 == v14 )
                  return result;
                v18 += v16;
              }
            }
          }
        }
      }
    }
  }
  return result;
}

int __fastcall R_LoadImage(const char *a1, int *a2, unsigned short *a3, unsigned short *a4, int *a5, int *a6, int a7)
{
  int v14; // r6
  int result; // r3
  int v16; // r24
  char *v17; // r31
  char v18[76]; // [sp+40h] [-70h] BYREF

  *a2 = 0;
  *a3 = 0;
  *a4 = 0;
  *a5 = 0;
  *a6 = 0;
  result = strlen(a1);
  v16 = result;
  if ( result >= 5 )
  {
    if ( result >= 64 )
      sub_1D94DC(1);
    if ( *(unsigned char *)(v16 + (int)a1 - 4) == 46 )
      v16 -= 4;
    if ( v16 >= 60 )
      sub_1D94DC(1);
    memcpy(v18, a1, v16);
    v18[v16] = 46;
    v17 = &v18[v16];
    v17[4] = 0;
    v17[1] = 100;
    v17[2] = 100;
    v17[3] = 115;
    result = LoadDDS();
    if ( !*a5 )
    {
      *a6 = 1;
      v17[1] = 116;
      v17[2] = 103;
      v17[3] = 97;
      result = LoadTGA(v18, a2, a3, a4, a5, a7);
      if ( !*a5 )
      {
        v17[1] = 106;
        v17[2] = 112;
        v17[3] = 103;
        result = LoadJPG(v18, a2, a3, a4, a5, a7);
      }
    }
    if ( *a5 )
    {
      if ( !*a2 )
        return dword_6A0950++;
    }
  }
  return result;
}

int __fastcall R_CreateImageInternal(void *a1, void *a2, int a3, int a4, float *a5)
{
  BOOL v10; // r31
  double v11; // fp4
  int v12; // r7
  double v13; // fp6
  double v14; // fp10
  double v15; // fp8
  double v16; // fp0
  double v17; // fp9
  int v18; // r6
  double v19; // fp7
  double v20; // fp5
  double v21; // fp2
  int v22; // r5
  int v23; // r4
  double v24; // fp3
  int v25; // r3
  double v26; // fp3
  double v27; // fp1
  double v28; // fp3
  double v29; // fp3
  double v30; // fp1
  double v31; // fp3
  double v32; // fp3
  double v33; // fp1
  double v34; // fp3
  double v35; // fp3
  double v36; // fp1
  int v38; // r5
  int v39; // r5
  int v40; // [sp+50h] [-60h] BYREF
  int v41; // [sp+54h] [-5Ch] BYREF
  double v42; // [sp+58h] [-58h]
  double v43; // [sp+60h] [-50h]
  int v45; // [sp+6Ch] [-44h]
  int v47; // [sp+74h] [-3Ch]
  int v49; // [sp+7Ch] [-34h]
  int v51; // [sp+84h] [-2Ch]
  
  // Custom helper pointers for standard int representations of variables
  int* a1_int = (int*)a1;

  *(int*)&v42 = 0;
  *((int*)&v42 + 1) = 1127219200;
  *(int*)&v43 = 0;
  *((int*)&v43 + 1) = 1127219200;
  
  v10 = strncmp((char*)a1, aLightmap, 9) == 0;
  if ( a5 )
  {
    v11 = flt_1ECC34;
    v12 = 0;
    v13 = flt_1ECC34;
    v14 = flt_1ECC68;
    v15 = flt_1ECC34;
    v16 = dbl_1ECC10;
    v17 = flt_1ECC34;
    v18 = 0;
    v19 = flt_1ECC34;
    v20 = flt_1ECC34;
    v21 = flt_1ECC34;
    while ( v12 < *(unsigned short *)((int)a1 + 64) * *(unsigned short *)((int)a1 + 66) )
    {
      v22 = v18 + 1;
      v23 = v18 + 2;
      v24 = *a5;
      *(int*)&v42 = *(unsigned char *)((int)a2 + v18);
      v25 = v18 + 3;
      v26 = (float)((float)v24 * (float)((float)*(int*)&v42 - (float)v16));
      if ( v17 <= v26 )
        v27 = v26;
      else
        v27 = v17;
      if ( v14 >= v27 )
      {
        if ( v15 > v26 )
          v26 = v15;
      }
      else
      {
        v26 = flt_1ECC68;
      }
      v45 = (int)v26;
      *(unsigned char *)((int)a2 + v18) = (int)v26;
      v28 = a5[1];
      *(int*)&v43 = *(unsigned char *)((int)a2 + v22);
      v29 = (float)((float)v28 * (float)((float)*(int*)&v43 - (float)v16));
      if ( v19 <= v29 )
        v30 = v29;
      else
        v30 = v19;
      if ( v14 >= v30 )
      {
        if ( v13 > v29 )
          v29 = v13;
      }
      else
      {
        v29 = flt_1ECC68;
      }
      v47 = (int)v29;
      *(unsigned char *)((int)a2 + v22) = (int)v29;
      v31 = a5[2];
      *(int*)&v42 = *(unsigned char *)((int)a2 + v23);
      v32 = (float)((float)v31 * (float)((float)*(int*)&v42 - (float)v16));
      if ( v20 <= v32 )
        v33 = v32;
      else
        v33 = v20;
      if ( v14 >= v33 )
      {
        if ( v11 > v32 )
          v32 = v11;
      }
      else
      {
        v32 = flt_1ECC68;
      }
      v49 = (int)v32;
      *(unsigned char *)((int)a2 + v23) = (int)v32;
      v34 = a5[3];
      *(int*)&v43 = *(unsigned char *)((int)a2 + v25);
      v35 = (float)((float)v34 * (float)((float)*(int*)&v43 - (float)v16));
      if ( v21 <= v35 )
        v36 = v35;
      else
        v36 = v21;
      if ( v14 >= v36 )
      {
        if ( v11 > v35 )
          v35 = v11;
      }
      else
      {
        v35 = flt_1ECC68;
      }
      v18 += 4;
      ++v12;
      v51 = (int)v35;
      *(unsigned char *)((int)a2 + v25) = (int)v35;
    }
  }
  GL_Bind(a1);
  if ( !UploadImage(
          a1,
          a2,
          *(int *)((int)a1 + 84),
          a3,
          a4,
          *(unsigned short *)((int)a1 + 64),
          *(unsigned short *)((int)a1 + 66),
          *(int *)((int)a1 + 100),
          v10,
          (void*)((int)a1 + 96),
          (void*)((int)a1 + 68),
          (void*)((int)a1 + 70),
          &v40,
          &v41) )
    return 0;
  v38 = 10497;
  *(int *)((int)a1 + 72) += v40;
  *(int *)((int)a1 + 76) += v41;
  dword_6A7318 += v40;
  if ( (*(int *)((int)a1 + 100) & 0x10) != 0 )
    v38 = 33071;
  glTexParameteri(*(int *)((int)a1 + 84), 10242, v38);
  v39 = 10497;
  if ( (*(int *)((int)a1 + 100) & 0x20) )
    v39 = 33071;
  glTexParameteri(*(int *)((int)a1 + 84), 10243, v39);
  return 1;
}

int* __fastcall R_CreateImage(const char *a1, void *a2, int a3, int a4, int a5, int a6, int a7, int a8)
{
  char v8; // r30
  int* result; // r3
  int* v15; // r30
  int v16; // zf

  v8 = a6;
  result = (int *)R_AllocImage(a1, 3553, a3, a4, a6, a7);
  v16 = (v8 & 0x80) == 0;
  v15 = result;
  if ( !v16 )
  {
    if ( a5 == 6408 || a3 >= 4 && a4 >= 4 )
    {
      result[24] = a5;
      result[26] = 0;
      result[27] = dword_6A0940;
      result[28] = dword_6A0944;
      return result;
    }
    result[25] &= ~0x80u;
    --dword_6A0950;
  }
  if ( !R_CreateImageInternal(result, a2, 3553, a5, (float *)a8) )
  {
    R_FreeImage(v15);
    return NULL;
  }
  return v15;
}
