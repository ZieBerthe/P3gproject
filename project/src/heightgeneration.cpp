#include "heightgeneration.hpp"
#include <cmath>
#include <limits>
#include <random>
#include <iostream>

const int OCTAVES = 5;
const float ROUGHNESS = 0.6f;
const float AMPLITUDE = 18.0f;

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

HeightsGenerator::HeightsGenerator(float roll_parameter, bool use_liss)
    : roll_parameter(roll_parameter), use_liss(use_liss)
    {
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
    if (use_liss) {
        // Liss on many points ?
    }
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

MovePoint::MovePoint(float x0, float y0, float amplitudemax, float flateningfactor, float prochangestate, float speed, float rollparameter,int seed=0)
    : x0(x0), y0(y0), amplitudemax(amplitudemax), flateningfactor(flateningfactor), prochangestate(prochangestate), speed(speed), rollparameter(rollparameter),
      seed(seed){
    current_amplitude = 0.0f;
    std::mt19937 random(seed);
    std::uniform_int_distribution<> stategen(0,3);
    state = stategen(random);
}

void MovePoint::updatesituation(float dt) {
    std::cout << "Updating situation for point at (" << x0 << ", " << y0 << ") with state " << state << " Amplitude :" << current_amplitude << std::endl;
    std::mt19937 random(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);


    float proba = dist(random);
    if(proba<prochangestate*dt && state%2 == 0)
    {
        state = (state + 1) % 4;
    }
    
    switch (state) {
        case 0: 
            break;
        case 1: // moving up
            current_amplitude = std::min(current_amplitude + speed * dt * amplitudemax,amplitudemax);
            if (current_amplitude == amplitudemax) {
                state = 2; // stopping
            }
            break;
        case 2: 
            break;
        case 3: // moving down
            current_amplitude = std::max(current_amplitude - speed * dt * amplitudemax,-amplitudemax);
            if (current_amplitude == -amplitudemax) {
                state = 0; // stopping
            }
            break;
    }
}

float MovePoint::calculateHeight(float x, float y) {
    float dx = x - x0;
    float dy = y - y0;
    float distancesq = dx * dx + dy * dy;
    float height = current_amplitude * std::exp(-distancesq * flateningfactor);
    return height;
}

MoveGenerator::MoveGenerator(float roll_parameter = 10.0f, int movedpoints = 0, int seed = 431)
    : roll_parameter(roll_parameter), movedpoints(movedpoints),seed(seed) {
    std::mt19937 random(seed);
    std::uniform_real_distribution<float> coord(0, roll_parameter);
    std::uniform_int_distribution<int> seeding(0, 1000000000);
    for (int i = 0; i < movedpoints; ++i) {
        float x0 = static_cast<float>(coord(random)) ;
        float y0 = static_cast<float>(coord(random)) ;
        points.emplace_back(x0, y0, AMPLITUDE, FLATTENING, PROBABILITY_CHANGE_STATE, SPEED, roll_parameter, seeding(random)*i*i);
    }
}
MoveGenerator::MoveGenerator() : MoveGenerator(10.0f, 0, 0) {
    // Default constructor initializes with default parameters
}

void MoveGenerator::updatesituation(float dt) {
    for (auto& point : points) {
        point.updatesituation(dt);
    }
}

float MoveGenerator::calculateHeight(float x, float y) {
    x = pfmodfold(x, roll_parameter);
    y = pfmodfold(y, roll_parameter);
    float total_height = 0.0f;
    for (auto& point : points) {
        total_height += point.calculateHeight(x, y);
    }
    //std::cout << "Total height at (" << x << ", " << y << ") is " << total_height << std::endl;
    return total_height;
}