#include "heightgeneration.hpp"
#include <cmath>
#include <limits>
#include <random>
#include <iostream>

const int OCTAVES = 1;
const float ROUGHNESS = 0.2f;
const float AMPLITUDE = 15.0f;

float pfmodfold(float x, float mod)
{
    float res = std::fmod(x, mod);
    if (res < 0) {
        res += mod;
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
        total += getInterpolatedNoise(x * freq, y * freq,roll_parameter*freq) * amp;
    }
    return total;
}

float HeightsGenerator::getInterpolatedNoise(float x, float y,float roll) {
    x = pfmodfold(x, roll);
    y = pfmodfold(y, roll);
    int intX = static_cast<int>(x);
    int inty = static_cast<int>(y);
    int nextIntX = (intX + 1)% static_cast<int>(roll);
    int nextInty = (inty + 1)% static_cast<int>(roll);
    float fracX = x - intX;
    float fracy = y - inty;

    float v1 = getSmoothNoise(intX, inty);
    float v2 = getSmoothNoise(nextIntX, inty);
    float v3 = getSmoothNoise(intX, nextInty);
    float v4 = getSmoothNoise(nextIntX, nextInty);
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
    int xp1 = (x + 1) % static_cast<int>(roll_parameter);
    int yp1 = (y + 1) % static_cast<int>(roll_parameter);
    int xm1 = (x - 1 + static_cast<int>(roll_parameter)) % static_cast<int>(roll_parameter);
    int ym1 = (y - 1 + static_cast<int>(roll_parameter)) % static_cast<int>(roll_parameter);
    float corners = (getNoise(xm1, ym1) + getNoise(xp1, ym1) +
                     getNoise(xm1, yp1) + getNoise(xp1, yp1)) / 16.0f;
    float sides = (getNoise(xm1, y) + getNoise(xp1, y) +
                     getNoise(x, ym1) + getNoise(x, yp1)) / 8.0f;
    float center = getNoise(x, y) / 4.0f;
    return corners + sides + center;
}

float HeightsGenerator::getNoise(int x, int y) {
    // Using a pseudo-random number generator based on the seed
    uint64_t hash = static_cast<uint64_t>(x) * 49632 + static_cast<uint64_t>(y) * 325176 + static_cast<uint64_t>(seed);
    std::seed_seq seq{static_cast<uint32_t>(hash & 0xFFFFFFFF), static_cast<uint32_t>(hash >> 32)};
    std::mt19937 localRandom(seq);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    return dist(localRandom);
}



MovePoint::MovePoint(float x0, float y0, float speed, float amplitudemax, float flateningfactor,  float rollparameter,int seed=0)
    : x0(x0), y0(y0), amplitudemax(amplitudemax), flateningfactor(flateningfactor), speed(speed), rollparameter(rollparameter),
      seed(seed){
    current_amplitude = 0.0f;
    std::mt19937 random(seed);
    std::uniform_int_distribution<> stategen(0,1);
    upwards = stategen(random);
}

void MovePoint::updatesituation(float dt) {
    // std::cout << "Updating situation for point at (" << x0 << ", " << y0 << ") with state " << upwards << " Amplitude :" << current_amplitude << std::endl;
    if(upwards)
    {
        current_amplitude = std::min(current_amplitude + speed * dt * amplitudemax,amplitudemax);
        if (current_amplitude == amplitudemax) {
            upwards = false; // switch to moving down
        }
    }
    else 
    {
        current_amplitude = std::max(current_amplitude - speed * dt * amplitudemax,-amplitudemax);
        if (current_amplitude == -amplitudemax) {
            upwards = true; // switch to moving up
        }
    }
}

float MovePoint::calculateHeight(float x, float y) {
    float dxb = std::abs(x - x0);
    float dyb = std::abs(y - y0);
    float dx = std::min(dxb, rollparameter - dxb);
    float dy = std::min(dyb, rollparameter - dyb);
    float distancesq = dx * dx + dy * dy;
    float height = current_amplitude * std::exp(-distancesq / flateningfactor);
    return height;
}

MoveGenerator::MoveGenerator(float roll_parameter = 10.0f, int movedpoints = 0, int seed = 431)
    : roll_parameter(roll_parameter), movedpoints(movedpoints),seed(seed) {
    std::mt19937 random(seed);
    std::uniform_real_distribution<float> coord(0, roll_parameter);
    std::uniform_int_distribution<int> seeding(0, 1000000000);
    std::uniform_real_distribution<float> speedgen(SPEEDMIN, SPEEDMAX);
    for (int i = 0; i < movedpoints; ++i) {
        float x0 = static_cast<float>(coord(random)) ;
        float y0 = static_cast<float>(coord(random)) ;
        float speed = speedgen(random);
        points.emplace_back(x0, y0, speed, AMPLITUDE, FLATTENING, roll_parameter, seeding(random)*i*i);
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
const std::vector<MovePoint>& MoveGenerator::getPoints() const {
    return points;
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