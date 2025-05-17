#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>

void debug_Toggle();
void debug_Update();

extern bool debugEnabled;
extern int debugPage;

// Used by render.c or drawCubeFace
extern float debug_tx[4], debug_ty[4], debug_tz[4];
extern float debug_dot, debug_nz;
extern float debug_min_tz, debug_max_tz, debug_mag, debug_area;


#endif