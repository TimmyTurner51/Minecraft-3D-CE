
#include <graphx.h>
#include <math.h>
#include <sys/rtc.h>
#include "player.h"
#include "world.h"
#include "render.h"
#include "texture.h"

#define FIXED_SHIFT 8
#define TO_FIXED(x) ((int)((x) * (1 << FIXED_SHIFT)))
#define FROM_FIXED(x) ((x) >> FIXED_SHIFT)
#define MUL_FIXED(a, b) (((a) * (b)) >> FIXED_SHIFT)

#define DEG2RAD (3.14159265f / 180.0f)
#define COS_THRESHOLD 0.92f
#define CURSOR_DOT_THRESHOLD 0.65f

bool cursorBlockValid = false;
int cursorBlockX, cursorBlockY, cursorBlockZ;
float closestCursorDot = -1.0f; // best match so far
// Cached trigonometric values per frame
static float sinRotX, cosRotX, sinRotY, cosRotY;


void updateTransformCache(void) {
    sinRotX = sinf(rotX);
    cosRotX = cosf(rotX);
    sinRotY = sinf(rotY);
    cosRotY = cosf(rotY);
}

void transform(float *x, float *y, float *z) {
    // Translate to camera space
    *x -= camX;
    *y -= camY;
    *z -= camZ;

    // Rotate around Y (horizontal)
    float tx = *x * cosRotY - *z * sinRotY;
    float tz = *x * sinRotY + *z * cosRotY;

    // Rotate around X (vertical)
    float ty = *y * cosRotX - tz * sinRotX;
    float newZ = *y * sinRotX + tz * cosRotX;

    *x = tx;
    *y = ty;
    *z = newZ;
}



bool cubeInView(float cx, float cy, float cz, float size) {
    float half = size / 2;

    for (int dx = -1; dx <= 1; dx += 2)
    for (int dy = -1; dy <= 1; dy += 2)
    for (int dz = -1; dz <= 1; dz += 2) {
        float x = cx + dx * half;
        float y = cy + dy * half;
        float z = cz + dz * half;

        transform(&x, &y, &z);

        // Looser near/far and screen bounds
        if (z > -100 && z < 1200) {
            int sx, sy;
            FAST_PROJECT(x, y, z, sx, sy);
            if (sx >= -80 && sx <= 400 && sy >= -80 && sy <= 320)
                return true;
        }
    }

    return false;
}


void drawCubeFace(float cx, float cy, float cz, float size, int face, int tile_id, int blockX, int blockY, int blockZ) {
    float half = size * 0.5f;

    // Set face normal and face center offset
    float fx = cx, fy = cy, fz = cz;
    float nx = 0, ny = 0, nz = 0;

    switch (face) {
        case FACE_LEFT:   fx -= half; nx = -1; break;
        case FACE_RIGHT:  fx += half; nx =  1; break;
        case FACE_BOTTOM: fy -= half; ny = -1; break;
        case FACE_TOP:    fy += half; ny =  1; break;
        case FACE_BACK:   fz -= half; nz = -1; break;
        case FACE_FRONT:  fz += half; nz =  1; break;
    }

    // Vector from camera to face center
    float tx = fx - camX;
    float ty = fy - camY;
    float tz = fz - camZ;

    float vx = tx * cosRotY - tz * sinRotY;
    float vz = tx * sinRotY + tz * cosRotY;
    float vy = ty * cosRotX - vz * sinRotX;
    float viewZ = ty * sinRotX + vz * cosRotX;

    float dot = nx * vx + ny * vy + nz * viewZ;
    if (dot > -0.1f) return; // Backface culling

    const int indices[6][4] = {
        {0, 3, 7, 4}, // LEFT
        {1, 2, 6, 5}, // RIGHT
        {0, 1, 5, 4}, // BOTTOM
        {3, 2, 6, 7}, // TOP
        {0, 1, 2, 3}, // BACK
        {4, 5, 6, 7}  // FRONT
    };

    float vertices[8][3] = {
        {cx - half, cy - half, cz - half}, // 0
        {cx + half, cy - half, cz - half}, // 1
        {cx + half, cy + half, cz - half}, // 2
        {cx - half, cy + half, cz - half}, // 3
        {cx - half, cy - half, cz + half}, // 4
        {cx + half, cy - half, cz + half}, // 5
        {cx + half, cy + half, cz + half}, // 6
        {cx - half, cy + half, cz + half}  // 7
    };

    int xPoints[4], yPoints[4];

    for (int i = 0; i < 4; i++) {
        float x = vertices[indices[face][i]][0] - camX;
        float y = vertices[indices[face][i]][1] - camY;
        float z = vertices[indices[face][i]][2] - camZ;

        // Inline transform
        float tx = x * cosRotY - z * sinRotY;
        float tz = x * sinRotY + z * cosRotY;
        float ty = y * cosRotX - tz * sinRotX;
        float finalZ = y * sinRotX + tz * cosRotX;

        if (finalZ < 5.0f) return;

        FAST_PROJECT(tx, ty, finalZ, xPoints[i], yPoints[i]);
    }

    // Cursor highlight (blinking)
    bool isCursor = cursorBlockValid &&
        blockX == cursorBlockX &&
        blockY == cursorBlockY &&
        blockZ == cursorBlockZ &&
        (rtc_Seconds % 2 == 0);

    // First triangle
    gfx_SetColor(isCursor ? 254 : texture_colors[tile_id][face][0]);
    gfx_FillTriangle(xPoints[0], yPoints[0], xPoints[1], yPoints[1], xPoints[2], yPoints[2]);

    // Second triangle
    gfx_SetColor(isCursor ? 254 : texture_colors[tile_id][face][1]);
    gfx_FillTriangle(xPoints[2], yPoints[2], xPoints[3], yPoints[3], xPoints[0], yPoints[0]);
}



void drawSkybox(void) {
    // Lightest sky layer (top third)
    gfx_SetColor(159);  // 0x1B
    gfx_FillRectangle(0, 0, 320, 80);

    // Mid sky layer
    gfx_SetColor(127);  // 0x2B
    gfx_FillRectangle(0, 80, 320, 80);

    // Deepest blue near horizon
    gfx_SetColor(95);  // 0x3B
    gfx_FillRectangle(0, 160, 320, 80);
}


void updateCursorTarget(void) {
    cursorBlockValid = false;
    closestCursorDot = -1.0f;

    int camBlockX = (int)(camX / CUBE_SIZE);
    int camBlockY = (int)(camY / CUBE_SIZE);
    int camBlockZ = (int)(camZ / CUBE_SIZE);

    float viewX = sinRotY * cosf(rotX);
    float viewY = -sinf(rotX);
    float viewZ = cosRotY * cosf(rotX);

    for (int x = camBlockX - renderDistance; x <= camBlockX + renderDistance; x++) {
        if (x < 0 || x >= WORLD_WIDTH) continue;

        for (int z = camBlockZ - renderDistance; z <= camBlockZ + renderDistance; z++) {
            if (z < 0 || z >= WORLD_DEPTH) continue;

            for (int y = WORLD_HEIGHT - 1; y >= 0; y--) {
                if (y < 0 || y >= WORLD_HEIGHT) continue;
                if (world[x][y][z] == 0) continue;

                float bx = x * CUBE_SIZE + CUBE_SIZE * 0.5f;
                float by = y * CUBE_SIZE + CUBE_SIZE * 0.5f;
                float bz = z * CUBE_SIZE + CUBE_SIZE * 0.5f;

                float dx = bx - camX;
                float dy = by - camY;
                float dz = bz - camZ;

                // Early skip for bad angles
                float dotEstimate = dx * viewX + dy * viewY + dz * viewZ;
                if (dotEstimate < 0.5f) continue;

                float lenSq = dx * dx + dy * dy + dz * dz;
                if (lenSq < 0.0001f) continue;

                float invLen = 1.0f / sqrtf(lenSq);
                dx *= invLen;
                dy *= invLen;
                dz *= invLen;

                float dot = dx * viewX + dy * viewY + dz * viewZ;

                if (dot > closestCursorDot && dot > CURSOR_DOT_THRESHOLD) {
                    closestCursorDot = dot;
                    cursorBlockX = x;
                    cursorBlockY = y;
                    cursorBlockZ = z;
                    cursorBlockValid = true;
                }
            }
        }
    }

    // Optional smoothing (3 missed frames before clearing)
    static int cursorMissFrames = 0;
    if (!cursorBlockValid) {
        if (++cursorMissFrames > 3) {
            cursorBlockX = -1;
            cursorBlockY = -1;
            cursorBlockZ = -1;
        }
    } else {
        cursorMissFrames = 0;
    }
}



void drawWorld(void) {
    updateTransformCache();
    int cubesDrawn = 0;

    int camBlockX = (int)(camX / CUBE_SIZE);
    int camBlockY = (int)(camY / CUBE_SIZE);
    int camBlockZ = (int)(camZ / CUBE_SIZE);

    int minY = camBlockY - renderDistance;
    int maxY = camBlockY + renderDistance;
    if (minY < 0) minY = 0;
    if (maxY >= WORLD_HEIGHT) maxY = WORLD_HEIGHT - 1;

    float maxXZDistSq = (renderDistance + 0.5f) * CUBE_SIZE;
    maxXZDistSq *= maxXZDistSq;

    for (int x = camBlockX - renderDistance; x <= camBlockX + renderDistance; x++) {
        if (x < 0 || x >= WORLD_WIDTH) continue;

        for (int z = camBlockZ - renderDistance; z <= camBlockZ + renderDistance; z++) {
            if (z < 0 || z >= WORLD_DEPTH) continue;

            int dx = x - camBlockX;
            int dz = z - camBlockZ;
            if ((dx * dx + dz * dz) > (renderDistance * renderDistance)) continue;

            bool columnRendered = false;

            for (int y = maxY; y >= minY; y--) {
                if (y < 0 || y >= WORLD_HEIGHT) continue;

                int block = world[x][y][z];
                if (block == 0 || !anyFaceVisible[x][y][z]) continue;

                float bx = x * CUBE_SIZE + CUBE_SIZE * 0.5f;
                float by = y * CUBE_SIZE + CUBE_SIZE * 0.5f;
                float bz = z * CUBE_SIZE + CUBE_SIZE * 0.5f;

                float ddx = camX - bx;
                float ddz = camZ - bz;
                float dist2 = ddx * ddx + ddz * ddz;
                if (dist2 > maxXZDistSq) break;

                // 👇 Move cubeInView into face loop
                bool anyFaceDrawn = false;

                for (int face = 0; face < 6; face++) {
                    if (!faceVisible[x][y][z][face]) continue;

                    if (!anyFaceDrawn && !cubeInView(bx, by, bz, CUBE_SIZE)) break;

                    drawCubeFace(bx, by, bz, CUBE_SIZE, face, block - 1, x, y, z);
                    anyFaceDrawn = true;
                }

                if (anyFaceDrawn) {
                    cubesDrawn++;
                    columnRendered = true;
                    break;
                }
            }

            if (columnRendered) continue;
        }
    }

    gfx_SetTextXY(2, 220);
    gfx_SetTextFGColor(0);
    gfx_PrintString("Cubes:");
    gfx_PrintInt(cubesDrawn, 3);

    static uint32_t lastTime = 0;
    static uint8_t frameCounter = 0;
    static uint8_t fps = 0;

    frameCounter++;
    uint32_t now = rtc_Time();

    if (now != lastTime) {
        lastTime = now;
        fps = frameCounter;
        frameCounter = 0;
    }

    gfx_SetTextXY(2, 2);
    gfx_PrintString("FPS:");
    gfx_PrintInt(fps, 2);
}
