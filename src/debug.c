#include <graphx.h>
#include <keypadc.h>
#include "debug.h"
#include "noise.h"
#include "player.h"
#include "render.h"
#include "texture.h"
#include "world.h"

bool debugEnabled = false;
int debugPage = 0;
float debug_tx[4];
float debug_ty[4];
float debug_tz[4];
float debug_dot;
float debug_nz;
float debug_min_tz;
float debug_max_tz;
float debug_mag;
float debug_area;
int debugTileIndex = 0;

void debug_Toggle() {
    kb_Scan();
    if (kb_IsDown(kb_KeyVars)) {
        debugEnabled = !debugEnabled;
        while (kb_IsDown(kb_KeyVars)) kb_Scan(); // debounce
    }

    if (!debugEnabled) return;

    if (kb_IsDown(kb_KeyRight)) {
        debugPage = (debugPage + 1) % 3;
        while (kb_IsDown(kb_KeyRight)) kb_Scan();
    }
    if (kb_IsDown(kb_KeyLeft)) {
        debugPage = (debugPage + 2) % 3; // underflow-safe
        while (kb_IsDown(kb_KeyLeft)) kb_Scan();
    }
    if (kb_IsDown(kb_KeyUp)) {
        debugTileIndex = (debugTileIndex + 1) % NUM_TILES;
        while (kb_IsDown(kb_KeyUp)) kb_Scan();
    }
    if (kb_IsDown(kb_KeyDown)) {
        debugTileIndex = (debugTileIndex - 1 + NUM_TILES) % NUM_TILES;
        while (kb_IsDown(kb_KeyDown)) kb_Scan();
    }
}

void debug_Update() {
    if (!debugEnabled) return;

    gfx_FillScreen(0);
    gfx_SetTextFGColor(254);

    switch (debugPage) {
        case 0:
            gfx_SetTextXY(2, 2);
            gfx_PrintString("Page 0: Player/Camera");
            gfx_SetTextXY(2, 20);
            gfx_PrintString("cam: ");
            gfx_PrintInt((int)camX, 3); gfx_PrintString(",");
            gfx_PrintInt((int)camY, 3); gfx_PrintString(",");
            gfx_PrintInt((int)camZ, 3);

            gfx_SetTextXY(2, 30);
            gfx_PrintString("rot: ");
            gfx_PrintInt((int)rotX, 3); gfx_PrintString(",");
            gfx_PrintInt((int)rotY, 3);

            gfx_SetTextXY(2, 40);
            gfx_PrintString("Cursor: ");
            gfx_PrintInt(cursorBlockX, 3); gfx_PrintString(",");
            gfx_PrintInt(cursorBlockY, 3); gfx_PrintString(",");
            gfx_PrintInt(cursorBlockZ, 3);

            gfx_SetTextXY(2, 50);
            gfx_PrintString("Dot: ");
            gfx_PrintInt((int)(closestCursorDot * 100), 3);

            gfx_SetTextXY(2, 60);
            gfx_PrintString("Valid: ");
            gfx_PrintInt(cursorBlockValid, 1);

            gfx_SetTextXY(2, 70);
            gfx_PrintString("Face: ");
            gfx_PrintInt(cursorBlockFace, 1);
            break;

        case 1:
            gfx_SetTextXY(2, 2);
            gfx_PrintString("Page 1: Transform + Depth");

            for (int i = 0; i < 4; i++) {
                gfx_SetTextXY(2, 20 + i * 10);
                gfx_PrintString("t["); gfx_PrintInt(i, 1); gfx_PrintString("]: ");
                gfx_PrintInt((int)debug_tx[i], 3); gfx_PrintString(",");
                gfx_PrintInt((int)debug_ty[i], 3); gfx_PrintString(",");
                gfx_PrintInt((int)debug_tz[i], 3);
            }

            gfx_SetTextXY(2, 70);
            gfx_PrintString("nz: ");
            gfx_PrintInt((int)(debug_nz * 100), 5);

            gfx_SetTextXY(2, 80);
            gfx_PrintString("dot: ");
            gfx_PrintInt((int)(debug_dot * 100), 5);

            gfx_SetTextXY(2, 90);
            gfx_PrintString("minTz: ");
            gfx_PrintInt((int)debug_min_tz, 4);

            gfx_SetTextXY(2, 100);
            gfx_PrintString("maxTz: ");
            gfx_PrintInt((int)debug_max_tz, 4);

            gfx_SetTextXY(2, 110);
            gfx_PrintString("mag: ");
            gfx_PrintInt((int)(debug_mag * 100), 5);

            gfx_SetTextXY(2, 120);
            gfx_PrintString("area: ");
            gfx_PrintInt(debug_area, 5);
            break;
        
        case 2:
            gfx_SetTextXY(2, 2);
            gfx_PrintString("Page 2: Texturing");

            gfx_SetTextXY(2, 12);
            gfx_PrintString("Tile: ");
            gfx_PrintInt(debugTileIndex, 2);

            // Layout setup
            const int cx = 160, cy = 40;
            const int s = 30;

            const int fx[6] = {
                cx - s,       // 0 = LEFT
                cx + s,       // 1 = RIGHT
                cx,           // 2 = BOTTOM
                cx,           // 3 = TOP
                cx + 2 * s,   // 4 = BACK
                cx            // 5 = FRONT
            };
            const int fy[6] = {
                cy,       // LEFT
                cy,       // RIGHT
                cy + s,   // BOTTOM
                cy - s,   // TOP
                cy,       // BACK
                cy        // FRONT
            };

            const char *labels[6] = {"L", "R", "B", "T", "K", "F"}; // K = back

            // Draw each face: colored triangles + label
            for (int face = 0; face < 6; face++) {
                int x = fx[face];
                int y = fy[face];

                // Triangle 1: top-left to bottom-right
                gfx_SetColor(texture_colors[debugTileIndex][face][0]);
                gfx_FillTriangle_NoClip(x, y, x + s, y, x + s, y + s);

                // Triangle 2: bottom-left to top-right
                gfx_SetColor(texture_colors[debugTileIndex][face][1]);
                gfx_FillTriangle_NoClip(x, y, x, y + s, x + s, y + s);

                // Outline
                gfx_SetColor(254); // white
                gfx_Rectangle(x, y, s, s);

                // Face label
                gfx_SetTextXY(x + 2, y + 2);
                gfx_PrintString(labels[face]);
            }

            // Show raw triangle values
            for (int face = 0; face < 6; face++) {
                gfx_SetTextXY(2, 30 + (face * 10));
                gfx_PrintInt(texture_colors[debugTileIndex][face][0], 3); 
                gfx_PrintChar(' ');
                gfx_PrintInt(texture_colors[debugTileIndex][face][1], 3); 
            }
            break;


        default:
            gfx_SetTextXY(2, 2);
            gfx_PrintString("Page 3: Reserved");
            break;
    }

    gfx_SwapDraw();
}

