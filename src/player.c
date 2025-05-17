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
    updateTransformCache();
}

void setCameraSpawnAboveTerrain(void) {
    int spawnX = WORLD_WIDTH / 2;
    int spawnZ = WORLD_DEPTH / 2;

    int topY = 0;
    for (int y = WORLD_HEIGHT - 1; y >= 0; y--) {
        if (world[spawnX][y][spawnZ] != 0) {
            topY = y;
            break;
        }
    }

    camX = spawnX * CUBE_SIZE + CUBE_SIZE / 2;
    camY = (topY + 2) * CUBE_SIZE + CUBE_SIZE / 2.0f;
    camZ = spawnZ * CUBE_SIZE + CUBE_SIZE / 2;

    rotX = 0.2f; // slight downward angle for debug
    
}