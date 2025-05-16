
#ifndef PLAYER_H
#define PLAYER_H

extern float camX;
extern float camY;
extern float camZ;
extern float rotX;
extern float rotY;
extern int renderDistance;
extern float playerFOV;            // used in projection
extern float cameraFOVDegrees;     // changeable by the slider or manually in code

void initPlayer(void);
void updatePlayer(void);
void setCameraSpawnAboveTerrain(void);

#endif
