#include "pauseMenu.h"
#include <graphx.h>
#include <keypadc.h>
#include <string.h>
#include <stdbool.h>

int pause_selected = 0;

const char *pause_labels[PAUSE_TOTAL] = {
    "Continue",
    "Achievements",
    "Options",
    "Open to USB",
    "Save and Quit"
};

void drawPauseMenu(void) {
    gfx_FillScreen(0); // Black background
    gfx_SetTextFGColor(255); // White text

    gfx_SetTextXY(130, 15);
    gfx_PrintString("Paused");

    int startY = 50;
    for (int i = 0; i < PAUSE_TOTAL; i++) {
        int y = startY + i * 20;

        if (i == pause_selected) {
            gfx_SetColor(255); // Bright white for selected
        } else {
            gfx_SetColor(115);  // Medium-dark gray (xlibc index 96)
        }

        gfx_FillRectangle(90, y, 140, 18);
        gfx_SetTextFGColor(0); // Black text
        gfx_SetTextXY(100, y + 5);
        gfx_PrintString(pause_labels[i]);
    }
}

int runPauseMenu(void) {
    pause_selected = 0;

    // Step 1: Wait until CLEAR and 2nd are both released
    do {
        kb_Scan();
    } while (kb_Data[6] & kb_Clear || kb_Data[1] & kb_2nd);

    // Step 2: Menu loop
    uint8_t keyDelay = 0;
    uint8_t keyRepeatDelay = 8; // Lower = faster repeat

    while (true) {
        kb_Scan();
        drawPauseMenu();
        gfx_SwapDraw();

        if (keyDelay > 0) keyDelay--;

        // Navigation with delay
        if ((kb_Data[7] & kb_Up) && keyDelay == 0 && pause_selected > 0) {
            pause_selected--;
            keyDelay = keyRepeatDelay;
        } else if ((kb_Data[7] & kb_Down) && keyDelay == 0 && pause_selected < PAUSE_TOTAL - 1) {
            pause_selected++;
            keyDelay = keyRepeatDelay;
        }

        // Exit options
        if (kb_Data[1] & kb_2nd) {
            while (kb_Data[1] & kb_2nd) kb_Scan();
            return pause_selected;
        }
        if (kb_Data[6] & kb_Clear) {
            while (kb_Data[6] & kb_Clear) kb_Scan();
            return PAUSE_CONTINUE;
        }
    }
}
