#include "../qcommon/qcommon.h"
#include <windows.h>
#include <GL/gl.h>

typedef struct {
    char name[64];
    GLuint textureId;
    int width;
    int height;
} shaderMap_t;

extern shaderMap_t g_shaderMap[1024];
extern int g_shaderMapCount;

void R_DrawStretchPic(float x, float y, float w, float h, 
                      float s1, float t1, float s2, float t2, 
                      int shaderHandle) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 640, 480, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    if (shaderHandle > 0 && shaderHandle < g_shaderMapCount) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, g_shaderMap[shaderHandle].textureId);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    } else {
        glDisable(GL_TEXTURE_2D);
        glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
    }

    glBegin(GL_QUADS);
        glTexCoord2f(s1, t1); glVertex2f(x, y);
        glTexCoord2f(s2, t1); glVertex2f(x + w, y);
        glTexCoord2f(s2, t2); glVertex2f(x + w, y + h);
        glTexCoord2f(s1, t2); glVertex2f(x, y + h);
    glEnd();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void RE_DrawStretchPic(float x, float y, float w, float h, 
                       float s1, float t1, float s2, float t2, 
                       int shaderHandle) {
    R_DrawStretchPic(x, y, w, h, s1, t1, s2, t2, shaderHandle);
}

int __fastcall RB_DrawStretchPic(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int *a10) {
    // Alias to fast hardware renderer to handle layout drawing
    R_DrawStretchPic(0, 0, 640, 480, 0, 0, 1, 1, a1);
    return 1;
}
