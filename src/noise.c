#include "noise.h"

float valueNoise2D(int x, int z) {
    int n = x + z * 57;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float smoothNoise2D(int x, int z) {
    float corners = (valueNoise2D(x - 1, z - 1) + valueNoise2D(x + 1, z - 1) +
                     valueNoise2D(x - 1, z + 1) + valueNoise2D(x + 1, z + 1)) / 16.0f;
    float sides   = (valueNoise2D(x - 1, z) + valueNoise2D(x + 1, z) +
                     valueNoise2D(x, z - 1) + valueNoise2D(x, z + 1)) / 8.0f;
    float center  = valueNoise2D(x, z) / 4.0f;
    return corners + sides + center;
}

float interpolatedNoise(float x, float z) {
    int intX = (int)x;
    int intZ = (int)z;
    float fracX = x - intX;
    float fracZ = z - intZ;

    float v1 = smoothNoise2D(intX,     intZ);
    float v2 = smoothNoise2D(intX + 1, intZ);
    float v3 = smoothNoise2D(intX,     intZ + 1);
    float v4 = smoothNoise2D(intX + 1, intZ + 1);

    float i1 = v1 + fracX * (v2 - v1);
    float i2 = v3 + fracX * (v4 - v3);
    return i1 + fracZ * (i2 - i1);
}

float fbmNoise2D(float x, float z, int octaves, float persistence) {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; i++) {
        total += interpolatedNoise(x * frequency, z * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }

    return total / maxValue;
}