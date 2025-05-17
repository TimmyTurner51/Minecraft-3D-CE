
#include <graphx.h>
#include <math.h>
#include <tice.h>
#include <keypadc.h>
#include <sys/rtc.h>
#include "debug.h"
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
#define CURSOR_DOT_THRESHOLD 0.5f

bool cursorBlockValid = false;
int cursorBlockX, cursorBlockY, cursorBlockZ;
float closestCursorDot = -1.0f; // best match so far
// Cached trigonometric values per frame
static float sinRotX, cosRotX, sinRotY, cosRotY;
int currentFPS = 0;
int cursorBlockFace = -1;


void updateFPS(void) {
    static uint8_t lastSecond = 0;
    static int framesThisSecond = 0;

    framesThisSecond++;

    uint8_t currentSecond = rtc_Seconds;
    if (currentSecond != lastSecond) {
        currentFPS = framesThisSecond;
        framesThisSecond = 0;
        lastSecond = currentSecond;
    }
}

void updateTransformCache(void) {
    sinRotX = sinf(rotX);
    cosRotX = cosf(rotX);
    sinRotY = sinf(rotY);
    cosRotY = cosf(rotY);
}


bool cubeInView(float cx, float cy, float cz, float size) {
    
    // debug
    return true;

    float half = size / 2;

    for (int dx = -1; dx <= 1; dx += 2)
    for (int dy = -1; dy <= 1; dy += 2)
    for (int dz = -1; dz <= 1; dz += 2) {
        float x = cx + dx * half;
        float y = cy + dy * half;
        float z = cz + dz * half;

        float tx, ty, tz;
        transform(x, y, z, &tx, &ty, &tz);

        if (tz > -100 && tz < 1200) {
            int sx, sy;
            FAST_PROJECT(tx, ty, tz, sx, sy);
            if (sx >= -80 && sx <= 400 && sy >= -80 && sy <= 320)
                return true;
        }
    }

    return false;
}

void transform(float x, float y, float z, float *outX, float *outY, float *outZ) {
    float tx = x - camX;
    float ty = y - camY;
    float tz = z - camZ;

    float rotY_tx = tx * cosRotY - tz * sinRotY;
    float rotY_tz = tx * sinRotY + tz * cosRotY;

    float rotX_ty = ty * cosRotX - rotY_tz * sinRotX;
    float rotX_tz = ty * sinRotX + rotY_tz * cosRotX;

    *outX = rotY_tx;
    *outY = rotX_ty;
    *outZ = rotX_tz;
}



void drawCubeFace(float cx, float cy, float cz, float size, int face, int tile_id, int bx, int by, int bz) {
    float half = size * 0.5f;

    const int indices[6][4] = {
        {0, 3, 7, 4}, // LEFT
        {1, 2, 6, 5}, // RIGHT
        {0, 1, 5, 4}, // BOTTOM
        {3, 2, 6, 7}, // TOP
        {4, 5, 6, 7}, // FRONT
        {0, 1, 2, 3}  // BACK
    };

    float vertices[8][3] = {
        {cx - half, cy - half, cz - half},
        {cx + half, cy - half, cz - half},
        {cx + half, cy + half, cz - half},
        {cx - half, cy + half, cz - half},
        {cx - half, cy - half, cz + half},
        {cx + half, cy - half, cz + half},
        {cx + half, cy + half, cz + half},
        {cx - half, cy + half, cz + half}
    };

    float tx[4], ty[4], tz[4];
    int xPoints[4], yPoints[4];

    // Step 1: Transform all points
    for (int i = 0; i < 4; i++) {
        float x = vertices[indices[face][i]][0];
        float y = vertices[indices[face][i]][1];
        float z = vertices[indices[face][i]][2];

        transform(x, y, z, &tx[i], &ty[i], &tz[i]);

        debug_tx[i] = tx[i];
        debug_ty[i] = ty[i];
        debug_tz[i] = tz[i];

        // Discard triangle if any vertex is behind the camera or warped
        if (tz[i] < 8.0f || isnan(tz[i]) || tz[i] > 1000.0f || fabsf(ty[i]) > 120.0f)
        return;
    }

    // Reject triangle if tz values vary too much (prevents warping)
    float min_tz = tz[0], max_tz = tz[0];
    for (int i = 1; i < 4; i++) {
        if (tz[i] < min_tz) min_tz = tz[i];
        if (tz[i] > max_tz) max_tz = tz[i];
    }
    //if (max_tz / min_tz > 2.0f) return;

    // Step 2: Backface culling using dot = nz
    float vx1 = tx[1] - tx[0];
    float vy1 = ty[1] - ty[0];
    float vz1 = tz[1] - tz[0];
    float vx2 = tx[2] - tx[0];
    float vy2 = ty[2] - ty[0];
    float vz2 = tz[2] - tz[0];
    float nx = vy1 * vz2 - vz1 * vy2;
    float ny = vz1 * vx2 - vx1 * vz2;
    float nz = vx1 * vy2 - vy1 * vx2;

    float mag = sqrtf(nx * nx + ny * ny + nz * nz);
    if (mag == 0) return; // degenerate triangle
    float dot = -nz / mag; // camera faces -Z

    debug_nz = nz;
    debug_dot = dot;
    debug_min_tz = min_tz;
    debug_max_tz = max_tz;
    debug_mag = mag;
    debug_area = abs(vx1 * vy2 - vx2 * vy1); // 2D projected area (rough)

    // Refined rejection: large in 3D, edge-on, and barely visible on screen
    if (dot < 0.01f && mag > 10000.0f && debug_area < 100) return;

    // Step 3: Project (fallback to 2D alignment if nearly edge-on)
    // Step 3: Always project using perspective
    for (int i = 0; i < 4; i++) {
        if (tz[i] < 2.0f || tz[i] > 1000.0f || isnan(tz[i])) return;

        FAST_PROJECT(tx[i], ty[i], tz[i], xPoints[i], yPoints[i]);

        if (xPoints[i] < -16 || xPoints[i] > SCREEN_WIDTH + 16 ||
            yPoints[i] < -16 || yPoints[i] > SCREEN_HEIGHT + 16)
            return;
    }

//    for (int i = 0; i < 4; i++) {
//        if (isnan(tx[i]) || isnan(ty[i]) || isnan(tz[i]))
//            return;
//
//        if (tz[i] < 0.01f || tz[i] > 1000.0f)
//            return;
//
//        if (fabsf(ty[i] / tz[i]) > 200.0f) {
//            gfx_PrintStringXY("warp", 10, 10);
//            return;
//        }
//    }

    // Step 4: Cursor highlight
    bool highlight = cursorBlockValid &&
        bx == cursorBlockX && by == cursorBlockY && bz == cursorBlockZ &&
        (rtc_Seconds % 2 == 0);

    int dx1 = xPoints[1] - xPoints[0];
    int dy1 = yPoints[1] - yPoints[0];
    int dx2 = xPoints[2] - xPoints[0];
    int dy2 = yPoints[2] - yPoints[0];
    int screen_area = abs(dx1 * dy2 - dx2 * dy1);
    if (screen_area > 30000) return; // safety net for triangle that's way too big
    gfx_SetColor(highlight ? 254 : texture_colors[tile_id][face][0]);
    gfx_FillTriangle_NoClip(xPoints[0], yPoints[0], xPoints[1], yPoints[1], xPoints[2], yPoints[2]);

    gfx_SetColor(highlight ? 254 : texture_colors[tile_id][face][1]);
    gfx_FillTriangle_NoClip(xPoints[2], yPoints[2], xPoints[3], yPoints[3], xPoints[0], yPoints[0]);
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
    cursorBlockFace = -1;

    float vx = sinRotY * cosf(rotX);
    float vy = sinf(rotX);
    float vz = cosRotY * cosf(rotX);

    float px = camX;
    float py = camY;
    float pz = camZ;

    float maxDist = renderDistance * CUBE_SIZE;
    for (float t = 0.0f; t <= maxDist; t += CUBE_SIZE * 0.5f) {
        int bx = (int)(px / CUBE_SIZE);
        int by = (int)(py / CUBE_SIZE);
        int bz = (int)(pz / CUBE_SIZE);

        // Draw debug point
        float wx = bx * CUBE_SIZE + CUBE_SIZE * 0.5f;
        float wy = by * CUBE_SIZE + CUBE_SIZE * 0.5f;
        float wz = bz * CUBE_SIZE + CUBE_SIZE * 0.5f;

        float tx, ty, tz;
        transform(wx, wy, wz, &tx, &ty, &tz);
        if (tz > 0) {
            int sx, sy;
            FAST_PROJECT(tx, ty, tz, sx, sy);
        }

        if (bx < 0 || bx >= WORLD_WIDTH ||
            by < 0 || by >= WORLD_HEIGHT ||
            bz < 0 || bz >= WORLD_DEPTH)
            break;

        if (world[bx][by][bz] != 0) {
            cursorBlockX = bx;
            cursorBlockY = by;
            cursorBlockZ = bz;
            cursorBlockValid = true;

            // Find the hit face (approximate by largest ray component)
            float absX = fabsf(vx);
            float absY = fabsf(vy);
            float absZ = fabsf(vz);

            if (absX > absY && absX > absZ)
                cursorBlockFace = (vx > 0) ? FACE_LEFT : FACE_RIGHT;
            else if (absY > absX && absY > absZ)
                cursorBlockFace = (vy > 0) ? FACE_BOTTOM : FACE_TOP;
            else
                cursorBlockFace = (vz > 0) ? FACE_BACK : FACE_FRONT;

            return;
        }

        px += vx * CUBE_SIZE * 0.5f;
        py += vy * CUBE_SIZE * 0.5f;
        pz += vz * CUBE_SIZE * 0.5f;
    }
}




void drawWorld(void) {
    updateTransformCache();
    cursorBlockValid = false;
    closestCursorDot = -100.0f;

    // debug
    gfx_SetTextFGColor(0);
    gfx_SetTextXY(80, 2);
    gfx_PrintString("cam:");
    gfx_PrintInt((int)camX, 3);
    gfx_PrintString(" ");
    gfx_PrintInt((int)camY, 3);
    gfx_PrintString(" ");
    gfx_PrintInt((int)camZ, 3);

    gfx_SetTextXY(220, 2);
    gfx_PrintString("rot:");
    gfx_PrintInt((int)(rotX * 100), 3);
    gfx_PrintString(" ");
    gfx_PrintInt((int)(rotY * 100), 3);


    updateCursorTarget();
    
    gfx_SetClipRegion(-SCREEN_WIDTH * 20, -SCREEN_HEIGHT * 20, SCREEN_WIDTH * 20, SCREEN_HEIGHT * 20);

    int cubesDrawn = 0;

    int camBlockX = (int)(camX / CUBE_SIZE);
    int camBlockY = (int)(camY / CUBE_SIZE);
    int camBlockZ = (int)(camZ / CUBE_SIZE);

    int minY = camBlockY - renderDistance;
    int maxY = camBlockY + renderDistance;
    if (minY < 0) minY = 0;
    if (maxY >= WORLD_HEIGHT) maxY = WORLD_HEIGHT - 1;

    for (int x = camBlockX - renderDistance; x <= camBlockX + renderDistance; x++) {
        if (x < 0 || x >= WORLD_WIDTH) continue;

        for (int z = camBlockZ - renderDistance; z <= camBlockZ + renderDistance; z++) {
            if (z < 0 || z >= WORLD_DEPTH) continue;

            int dx = x - camBlockX;
            int dz = z - camBlockZ;
            if ((dx * dx + dz * dz) > (renderDistance * renderDistance)) continue;

            for (int y = maxY; y >= minY; y--) {
                if (y < 0 || y >= WORLD_HEIGHT) continue;

                int block = world[x][y][z];
                if (block == 0 || !anyFaceVisible[x][y][z]) continue;

                float bx = x * CUBE_SIZE + CUBE_SIZE * 0.5f;
                float by = y * CUBE_SIZE + CUBE_SIZE * 0.5f;
                float bz = z * CUBE_SIZE + CUBE_SIZE * 0.5f;

                if (!cubeInView(bx, by, bz, CUBE_SIZE)) continue;

                for (int face = 0; face < 6; face++) {
                    if (!faceVisible[x][y][z][face]) continue;
                    drawCubeFace(bx, by, bz, CUBE_SIZE, face, block - 1, x, y, z);
                }

                cubesDrawn++;
                break;
            }
        }
    }

    gfx_SetTextXY(2, 220);
    gfx_SetTextFGColor(0);
    gfx_PrintString("Cubes:");
    gfx_PrintInt(cubesDrawn, 3);
    gfx_SetTextXY(2, 2);
    gfx_SetTextFGColor(0);
    gfx_PrintString("FPS:");
    gfx_PrintInt(currentFPS, 2);
}