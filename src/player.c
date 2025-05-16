#include "player.h"
#include "render.h"
#include <math.h>
#include <keypadc.h>
#include "world.h"

float camX = 0;
float camY = 0;
float camZ = -5;
float rotX = 0.0f;
float rotY = 0.0f;

int renderDistance = 3;
float cameraFOVDegrees = 90.0f;  // Default Minecraft-style FOV
float playerFOV = 0.0f;

void setCameraSpawnAboveTerrain(void);


void updatePlayerFOV(void) {
    float fovRadians = cameraFOVDegrees * 3.14159265f / 180.0f;
    playerFOV = SCREEN_HEIGHT / (2.0f * tanf(fovRadians / 2.0f));
}

void initPlayer(void) {
    setCameraSpawnAboveTerrain();
    updatePlayerFOV();
}

void updatePlayer(void) {
    float moveSpeed = 2.5f;
    float rotSpeed = 0.05f;

    kb_Scan();

    // Camera rotation (left/right to look around)
    if (kb_IsDown(kb_KeyLeft))  rotY -= rotSpeed;
    if (kb_IsDown(kb_KeyRight)) rotY += rotSpeed;
    if (kb_IsDown(kb_KeyUp))    rotX -= rotSpeed;
    if (kb_IsDown(kb_KeyDown))  rotX += rotSpeed;

    // Clamp vertical rotation to avoid flipping
    if (rotX < -1.5f) rotX = -1.5f;
    if (rotX >  1.5f) rotX =  1.5f;

    // Forward/backward (Mode / Apps)
    if (kb_IsDown(kb_KeyMode)) {
        camX += moveSpeed * sinf(rotY);
        camZ += moveSpeed * cosf(rotY);
    }
    if (kb_IsDown(kb_KeyApps)) {
        camX -= moveSpeed * sinf(rotY);
        camZ -= moveSpeed * cosf(rotY);
    }

    // Strafe (Alpha / Stat)
    if (kb_IsDown(kb_KeyAlpha)) {
        camX -= moveSpeed * cosf(rotY);
        camZ += moveSpeed * sinf(rotY);
    }
    if (kb_IsDown(kb_KeyStat)) {
        camX += moveSpeed * cosf(rotY);
        camZ -= moveSpeed * sinf(rotY);
    }

    // Fly up (2nd) / down (Math)
    if (kb_IsDown(kb_Key2nd)) {
        camY += moveSpeed;
    }
    if (kb_IsDown(kb_KeyMath)) {
        camY -= moveSpeed;
    }
}

void setCameraSpawnAboveTerrain(void) {
    // Pick a center position
    int blockX = WORLD_WIDTH / 2;
    int blockZ = WORLD_DEPTH / 2;
    int blockY = 1; // default in case world is empty

    // Find the topmost solid block at (blockX, blockZ)
    for (int y = WORLD_HEIGHT - 1; y >= 0; y--) {
        if (world[blockX][y][blockZ] != 0) {
            blockY = y + 1; // one block above
            break;
        }
    }


    // Spawn camera at the center of the chosen block
    camX = blockX * CUBE_SIZE + CUBE_SIZE / 2.0f;
    camY = blockY * CUBE_SIZE + CUBE_SIZE / 2.0f;
    camZ = blockZ * CUBE_SIZE + CUBE_SIZE / 2.0f;
}