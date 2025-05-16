#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <stdint.h>


#define WORLD_WIDTH 16
#define WORLD_HEIGHT 16
#define WORLD_DEPTH 16
#define FACE_LEFT   0
#define FACE_RIGHT  1
#define FACE_BOTTOM 2
#define FACE_TOP    3
#define FACE_BACK   4
#define FACE_FRONT  5

#define CUBE_SIZE 18.0f

extern unsigned int worldSeed;
extern bool faceVisible[WORLD_WIDTH][WORLD_HEIGHT][WORLD_DEPTH][6];
extern bool anyFaceVisible[WORLD_WIDTH][WORLD_HEIGHT][WORLD_DEPTH];
void generateFaceVisibility(void);

//extern int world[WORLD_WIDTH][WORLD_HEIGHT][WORLD_DEPTH];
extern uint8_t world[WORLD_WIDTH][WORLD_HEIGHT][WORLD_DEPTH];  

void initWorld(unsigned int seed);
void generateFaceVisibility(void);

#endif
