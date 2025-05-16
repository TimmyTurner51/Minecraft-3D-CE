#ifndef RENDER_H
#define RENDER_H

#include <stdbool.h>

#define FACE_LEFT   0
#define FACE_RIGHT  1
#define FACE_BOTTOM 2
#define FACE_TOP    3
#define FACE_BACK   4
#define FACE_FRONT  5

#define COLOR_TOP    45
#define COLOR_SIDE   88
#define COLOR_BOTTOM 108

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define FAST_PROJECT(x, y, z, sx, sy)                             \
    do {                                                          \
        float iz = 1.0f / (z);                                     \
        sx = (int)(x * iz * playerFOV + SCREEN_WIDTH / 2);        \
        sy = (int)(-y * iz * playerFOV + SCREEN_HEIGHT / 2);      \
    } while (0)

void drawCubeFace(float cx, float cy, float cz, float size, int face, int tile_id, int blockX, int blockY, int blockZ);
void drawWorld(void);
bool cubeInView(float cx, float cy, float cz, float size);
void transform(float *x, float *y, float *z);
void drawSkybox(void);
void updateCursorTarget(void);

extern int cursorBlockX, cursorBlockY, cursorBlockZ;
extern bool cursorBlockValid;

#endif
