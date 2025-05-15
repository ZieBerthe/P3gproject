#pragma once
#include <random>

class HeightsGenerator {
public:
    HeightsGenerator();
    HeightsGenerator(int gridX, int gridZ, int vertexCount, int seed);

    float generateHeight(float x, float z);

private:
    static constexpr float AMPLITUDE = 10.0f;
    static constexpr int OCTAVES = 3;
    static constexpr float ROUGHNESS = 0.3f;

    std::mt19937 random;
    int seed;
    int xOffset = 0;
    int zOffset = 0;

    float getInterpolatedNoise(float x, float z);
    float interpolate(float a, float b, float blend);
    float getSmoothNoise(int x, int z);
    float getNoise(int x, int z);
};