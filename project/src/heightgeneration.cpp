#include "heightgeneration.hpp"
#include <cmath>
#include <limits>
#include <random>

const int OCTAVES = 5;
const float ROUGHNESS = 0.8f;
const float AMPLITUDE = 10.0f;

float pfmodfold(float x, float mod)
{
    float res = std::fmod(x, mod);
    if (res < 0) {
        res += mod;
    }
    if (res > mod/2) {
        res = mod - res;
    }
    return res;
}

HeightsGenerator::HeightsGenerator(float roll_parameter)
    : roll_parameter(roll_parameter){
    std::random_device rd;
    random = std::mt19937(rd());
    std::uniform_int_distribution<int> dist(0, 1000000000);
    seed = dist(random);
}

HeightsGenerator::HeightsGenerator(int gridX, int gridy, int vertexCount, int seed)
    : seed(seed)
{
    xOffset = gridX * (vertexCount - 1);
    yOffset = gridy * (vertexCount - 1);
    std::random_device rd;
    random = std::mt19937(rd());
}

float HeightsGenerator::generateHeight(float x, float y) {
    float total = 0.0f;
    float roll_factor = 1.0f * roll_parameter;
    x = pfmodfold(x,roll_factor);
    y = pfmodfold(y,roll_factor);
    
    float d = std::pow(2.0f, OCTAVES - 1);
    for (int i = 0; i < OCTAVES; ++i) {
        float freq = std::pow(2.0f, i) / d;
        float amp = std::pow(ROUGHNESS, i) * AMPLITUDE;
        total += getInterpolatedNoise((x + xOffset) * freq, (y + yOffset) * freq) * amp;
    }
    return total;
}

float HeightsGenerator::getInterpolatedNoise(float x, float y) {
    int intX = static_cast<int>(x);
    int inty = static_cast<int>(y);
    float fracX = x - intX;
    float fracy = y - inty;

    float v1 = getSmoothNoise(intX, inty);
    float v2 = getSmoothNoise(intX + 1, inty);
    float v3 = getSmoothNoise(intX, inty + 1);
    float v4 = getSmoothNoise(intX + 1, inty + 1);
    float i1 = interpolate(v1, v2, fracX);
    float i2 = interpolate(v3, v4, fracX);
    return interpolate(i1, i2, fracy);
}

float HeightsGenerator::interpolate(float a, float b, float blend) {
    double theta = blend * 3.1415927f;
    float f = static_cast<float>((1.0 - std::cos(theta)) * 0.5);
    return a * (1.0f - f) + b * f;
}

float HeightsGenerator::getSmoothNoise(int x, int y) {
    float corners = (getNoise(x - 1, y - 1) + getNoise(x + 1, y - 1) +
                     getNoise(x - 1, y + 1) + getNoise(x + 1, y + 1)) / 16.0f;
    float sides = (getNoise(x - 1, y) + getNoise(x + 1, y) +
                   getNoise(x, y - 1) + getNoise(x, y + 1)) / 8.0f;
    float center = getNoise(x, y) / 4.0f;
    return corners + sides + center;
}

float HeightsGenerator::getNoise(int x, int y) {
    // Use a deterministic seed for each (x, y) pair
    std::mt19937 localRandom(x * 49632 + y * 325176 + seed);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    return dist(localRandom);
}