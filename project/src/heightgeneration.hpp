#pragma once
#include <random>

class HeightsGenerator {
public:
    HeightsGenerator(float roll_parameter = 0.0f, bool use_liss = false);
    HeightsGenerator(int gridX, int gridy, int vertexCount, int seed);

    float generateHeight(float x, float y);

private:
    static constexpr float AMPLITUDE = 10.0f;
    static constexpr int OCTAVES = 5;
    static constexpr float ROUGHNESS = 0.5f;

    float roll_parameter = 0.0f;
    float lissdiameter = 0.2f;
    bool use_liss;

    std::mt19937 random;
    int seed;
    int xOffset = 0;
    int yOffset = 0;

    float getInterpolatedNoise(float x, float y);
    float interpolate(float a, float b, float blend);
    float getSmoothNoise(int x, int y);
    float getNoise(int x, int y);
};

class MovePoint{
    public:
        MovePoint(float x0, float y0, float amplitudemax, float flateningfactor,float prochangestate,float speed,float rollparameter, int seed);
        float calculateHeight(float x, float y);
        void updatesituation(float dt);

    private:
        float x0;
        float y0;
        float amplitudemax;
        float flateningfactor;
        float prochangestate;
        float speed;
        float rollparameter;
        int state = 0; // 0 : stopped 1 moving up 2 stopping 3 moving down
        float current_amplitude;
        std::mt19937 random;
        int seed;
};

class MoveGenerator {
    public:
        MoveGenerator();
        MoveGenerator(float roll_parameter, int movedpoints,int seed);

        float calculateHeight(float x, float y);
        void updatesituation(float dt);

    private:
        float AMPLITUDE = 5.0f;
        float FLATTENING = 0.05f;
        float PROBABILITY_CHANGE_STATE = 0.5f;
        float SPEED = 0.1f;

        std::vector<MovePoint> points;
        float roll_parameter;
        int movedpoints;
        std::mt19937 random;
        int seed;
};
