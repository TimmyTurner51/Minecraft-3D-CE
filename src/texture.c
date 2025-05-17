#include <fileioc.h>
#include "texture.h"

uint8_t texture_colors[NUM_TILES][NUM_FACES][NUM_TRIANGLES];

bool load_texture_appvar(void) {
    ti_var_t var = ti_Open("BLOCKTILES", "r");
    if (!var) return false;

    ti_Seek(2, SEEK_SET, var); // Skip AppVar header

    const int faceRemap[6] = {
        3, // TOP
        2, // BOTTOM
        0, // LEFT
        1, // RIGHT
        4, // FRONT
        5  // BACK
    };

    for (int tile = 0; tile < NUM_TILES; tile++) {
        for (int face = 0; face < NUM_FACES; face++) {
            for (int tri = 0; tri < NUM_TRIANGLES; tri++) {
                uint8_t value = ti_GetC(var);
                texture_colors[tile][faceRemap[face]][tri] = value;
            }
        }
    }

    ti_Close(var);
    return true;
}
