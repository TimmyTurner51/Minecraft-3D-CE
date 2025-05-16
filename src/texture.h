#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_TILES 4
#define NUM_FACES 6
#define NUM_TRIANGLES 2

extern uint8_t texture_colors[NUM_TILES][NUM_FACES][NUM_TRIANGLES];

bool load_texture_appvar(void);

#endif