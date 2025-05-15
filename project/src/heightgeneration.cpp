#include "heightgeneration.hpp"
#include <cmath>
#include <limits>
#include <random>

const int OCTAVES = 4;
const float ROUGHNESS = 0.3f;
const float AMPLITUDE = 10.0f;

HeightsGenerator::HeightsGenerator() {
    std::random_device rd;
    random = std::mt19937(rd());
    std::uniform_int_distribution<int> dist(0, 1000000000);
    seed = dist(random);
}

HeightsGenerator::HeightsGenerator(int gridX, int gridZ, int vertexCount, int seed)
    : seed(seed)
{
    xOffset = gridX * (vertexCount - 1);
    zOffset = gridZ * (vertexCount - 1);
    std::random_device rd;
    random = std::mt19937(rd());
}

float HeightsGenerator::generateHeight(float x, float z) {
    float total = 0.0f;
    float d = std::pow(2.0f, OCTAVES - 1);
    for (int i = 0; i < OCTAVES; ++i) {
        float freq = std::pow(2.0f, i) / d;
        float amp = std::pow(ROUGHNESS, i) * AMPLITUDE;
        total += getInterpolatedNoise((x + xOffset) * freq, (z + zOffset) * freq) * amp;
    }
    return total;
}

float HeightsGenerator::getInterpolatedNoise(float x, float z) {
    int intX = static_cast<int>(x);
    int intZ = static_cast<int>(z);
    float fracX = x - intX;
    float fracZ = z - intZ;

    float v1 = getSmoothNoise(intX, intZ);
    float v2 = getSmoothNoise(intX + 1, intZ);
    float v3 = getSmoothNoise(intX, intZ + 1);
    float v4 = getSmoothNoise(intX + 1, intZ + 1);
    float i1 = interpolate(v1, v2, fracX);
    float i2 = interpolate(v3, v4, fracX);
    return interpolate(i1, i2, fracZ);
}

float HeightsGenerator::interpolate(float a, float b, float blend) {
    double theta = blend * 3.1415927f;
    float f = static_cast<float>((1.0 - std::cos(theta)) * 0.5);
    return a * (1.0f - f) + b * f;
}

float HeightsGenerator::getSmoothNoise(int x, int z) {
    float corners = (getNoise(x - 1, z - 1) + getNoise(x + 1, z - 1) +
                     getNoise(x - 1, z + 1) + getNoise(x + 1, z + 1)) / 16.0f;
    float sides = (getNoise(x - 1, z) + getNoise(x + 1, z) +
                   getNoise(x, z - 1) + getNoise(x, z + 1)) / 8.0f;
    float center = getNoise(x, z) / 4.0f;
    return corners + sides + center;
}

float HeightsGenerator::getNoise(int x, int z) {
    // Use a deterministic seed for each (x, z) pair
    std::mt19937 localRandom(x * 49632 + z * 325176 + seed);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    return dist(localRandom);
}