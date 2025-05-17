#include <tice.h>
#include <keypadc.h>
#include <graphx.h>
#include <stdbool.h>
#include "debug.h"
#include "player.h"
#include "world.h"
#include "render.h"
#include "texture.h"
#include "pauseMenu.h"

int main(void) {
    gfx_Begin();
    gfx_SetDrawScreen();

    if (!load_texture_appvar()) {
        gfx_End();
        return 0;
    }

    unsigned int seed = 0;
    int cursorPos = 0;
    bool prevDigitKeys[10] = {false};

    gfx_FillScreen(0);
    gfx_SetTextFGColor(254);
    gfx_SetTextXY(10, 50);
    gfx_PrintString("Enter Seed:");
    gfx_SetTextXY(10, 70);
    gfx_PrintUInt(seed, 10);

    while (true) {
        kb_Scan();
        if (kb_IsDown(kb_KeyClear)) return 0;
        if (kb_IsDown(kb_KeyEnter) || kb_IsDown(kb_Key2nd)) break;

        bool updated = false;

        for (int i = 0; i <= 9; i++) {
            if (kb_IsDown(kb_Key0 + i) && !prevDigitKeys[i] && cursorPos < 10) {
                seed = seed * 10 + i;
                cursorPos++;
                updated = true;
            }
        }

        if (kb_IsDown(kb_KeyDel) && cursorPos > 0) {
            seed /= 10;
            cursorPos--;
            updated = true;
            while (kb_IsDown(kb_KeyDel)) kb_Scan();
        }

        if (updated) {
            gfx_FillScreen(0);
            gfx_SetTextFGColor(254);
            gfx_SetTextXY(10, 50);
            gfx_PrintString("Enter Seed:");
            gfx_SetTextXY(10, 70);
            gfx_PrintUInt(seed, 10);
        }

        for (int i = 0; i <= 9; i++)
            prevDigitKeys[i] = kb_IsDown(kb_Key0 + i);
    }

    gfx_SetDrawBuffer();
    initWorld(seed);
    initPlayer();

    bool clearWasHeld = false;
    int frameCount = 0;
    while (true) {
        debug_Toggle();
        if (debugEnabled) {
            debug_Update();
            continue; // skip rendering
        }
        kb_Scan();
        if (kb_IsDown(kb_KeyClear)) {
            int result = runPauseMenu();
            if (result == PAUSE_QUIT) return 0;
        }


        updatePlayer();
        updateFPS();
        if (++frameCount % 3 == 0) updateCursorTarget();

        drawSkybox();
        drawWorld();
        gfx_SwapDraw();
    }

    gfx_End();
}
