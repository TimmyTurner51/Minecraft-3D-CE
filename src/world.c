#include "world.h"
#include "noise.h"
#include <stdbool.h>
#include <math.h>
#include <graphx.h>

#include <keypadc.h>
#include <tice.h>


uint8_t world[WORLD_WIDTH][WORLD_HEIGHT][WORLD_DEPTH];
bool faceVisible[WORLD_WIDTH][WORLD_HEIGHT][WORLD_DEPTH][6];
unsigned int worldSeed = 42069;  // ✅ actual definition
// Apply frequency and seed-based variation
#define TERRAIN_FREQ 8.0f

bool anyFaceVisible[WORLD_WIDTH][WORLD_HEIGHT][WORLD_DEPTH];

void initWorld(unsigned int seed) {
    worldSeed = seed;  // or anything ≤ 4,294,967,295

    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int z = 0; z < WORLD_DEPTH; z++) {
            float rawNoise = fbmNoise2D(
                (float)(x + worldSeed * 131) * TERRAIN_FREQ,
                (float)(z + worldSeed * 17) * TERRAIN_FREQ,
                4, 0.5f
            );
            float noise = (rawNoise + 1.0f) / 2.0f;
            noise = fmaxf(0.0f, fminf(1.0f, noise));
            int height = (int)(noise * (WORLD_HEIGHT - 4)) + 2;
            if (height >= WORLD_HEIGHT) height = WORLD_HEIGHT - 1;

            for (int y = 0; y < WORLD_HEIGHT; y++) {
                if (y < height) {
                    if (y == height - 1) {
                        world[x][y][z] = 1; // Grass
                    } else if (y >= height - 4) {
                        world[x][y][z] = 2; // Dirt
                    } else {
                        world[x][y][z] = 3; // Stone
                    }
                } else {
                    world[x][y][z] = 0; // Air
                }
            }
        }

        // === Progress bar frame ===
        gfx_FillScreen(65);  // Brown background
        gfx_SetTextFGColor(116);

        const char *text = "Generating world...";
        int textWidth = gfx_GetStringWidth(text);
        gfx_SetTextXY((LCD_WIDTH - textWidth) / 2, 90);
        gfx_PrintString(text);

        gfx_SetColor(116);
        gfx_FillRectangle(49, 119, 222, 12);  // bar outline

        gfx_SetColor(70);
        int progressWidth = (x * 220) / WORLD_WIDTH;
        gfx_FillRectangle(50, 120, progressWidth, 10);

        gfx_SwapDraw(); // Show update
    }

    // === Face visibility screen ===
    gfx_FillScreen(65);  // Reuse brown
    gfx_SetTextFGColor(116);

    const char *fvText = "Generating Face Visibility...";
    int fvTextWidth = gfx_GetStringWidth(fvText);
    gfx_SetTextXY((LCD_WIDTH - fvTextWidth) / 2, 90);
    gfx_PrintString(fvText);

    gfx_SetColor(116);
    gfx_FillRectangle(49, 119, 222, 12);

    gfx_SetColor(70);
    gfx_FillRectangle(50, 120, 220, 10);
    gfx_SwapDraw();

    generateFaceVisibility();
}


void generateFaceVisibility(void) {
    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            for (int z = 0; z < WORLD_DEPTH; z++) {
                if (!world[x][y][z]) {
                    anyFaceVisible[x][y][z] = false;
                    for (int f = 0; f < 6; f++) faceVisible[x][y][z][f] = false;
                    continue;
                }

                faceVisible[x][y][z][FACE_LEFT]   = (x == 0 || world[x - 1][y][z] == 0);
                faceVisible[x][y][z][FACE_RIGHT]  = (x == WORLD_WIDTH - 1 || world[x + 1][y][z] == 0);
                faceVisible[x][y][z][FACE_BOTTOM] = (y == 0 || world[x][y - 1][z] == 0);
                faceVisible[x][y][z][FACE_TOP]    = (y == WORLD_HEIGHT - 1 || world[x][y + 1][z] == 0);
                faceVisible[x][y][z][FACE_BACK]   = (z == 0 || world[x][y][z - 1] == 0);
                faceVisible[x][y][z][FACE_FRONT]  = (z == WORLD_DEPTH - 1 || world[x][y][z + 1] == 0);

                anyFaceVisible[x][y][z] =
                    faceVisible[x][y][z][FACE_LEFT]   ||
                    faceVisible[x][y][z][FACE_RIGHT]  ||
                    faceVisible[x][y][z][FACE_BOTTOM] ||
                    faceVisible[x][y][z][FACE_TOP]    ||
                    faceVisible[x][y][z][FACE_BACK]   ||
                    faceVisible[x][y][z][FACE_FRONT];
            }
        }
    }
}