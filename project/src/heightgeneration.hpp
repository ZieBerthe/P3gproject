#pragma once
#include <random>

class HeightsGenerator {
public:
    HeightsGenerator(float roll_parameter = 0.0f);
    HeightsGenerator(int gridX, int gridy, int vertexCount, int seed);

    float generateHeight(float x, float y,bool use_liss);

private:
    static constexpr float AMPLITUDE = 18.0f;
    static constexpr int OCTAVES = 5;
    static constexpr float ROUGHNESS = 0.5f;

    float roll_parameter = 0.0f;
    float lissdiameter = 0.2f;

    std::mt19937 random;
    int seed;
    int xOffset = 0;
    int yOffset = 0;

    float getInterpolatedNoise(float x, float y);
    float interpolate(float a, float b, float blend);
    float getSmoothNoise(int x, int y);
    float getNoise(int x, int y);
};