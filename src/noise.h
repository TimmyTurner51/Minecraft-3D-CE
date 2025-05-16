#ifndef NOISE_H
#define NOISE_H

#ifdef __cplusplus
extern "C" {
#endif

// Base noise: deterministic pseudorandom
float valueNoise2D(int x, int z);

// Smooths noise using neighbor averaging
float smoothNoise2D(int x, int z);

// Interpolated smooth noise, returns float between ~-1.0 and 1.0
float interpolatedNoise(float x, float z);

float fbmNoise2D(float x, float z, int octaves, float persistence);

#ifdef __cplusplus
}
#endif

#endif // NOISE_H
