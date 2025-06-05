#pragma once
#include <random>

class HeightsGenerator {
public:
    HeightsGenerator(float roll_parameter = 0.0f, bool use_liss = false);
    HeightsGenerator(int gridX, int gridy, int vertexCount, int seed);

    float generateHeight(float x, float y);

private:
    static constexpr float AMPLITUDE = 10.0f;
    static constexpr int OCTAVES = 3;
    static constexpr float ROUGHNESS = 0.03f;

    float roll_parameter = 0.0f;
    float lissdiameter = 0.2f;
    bool use_liss;

    std::mt19937 random;
    int seed;

    float getInterpolatedNoise(float x, float y,float roll = 1.0f);
    float interpolate(float a, float b, float blend);
    float getSmoothNoise(int x, int y);
    float getNoise(int x, int y);
};

class MovePoint{
    public:
        MovePoint(float x0, float y0, float amplitudemax, float flateningfactor,float speed,float rollparameter, int seed);
        float calculateHeight(float x, float y);
        void updatesituation(float dt);
        
        float x0;
        float y0;
        float amplitudemax;
        float flateningfactor;
        float speed;
        float rollparameter;
        bool upwards; 
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
        const std::vector<MovePoint>& getPoints() const;
    private:
        float AMPLITUDE = 2.0f;
        float FLATTENING = 4.0f;
        float SPEEDMIN = 0.05f;
        float SPEEDMAX = 0.6f;

        std::vector<MovePoint> points;
        float roll_parameter;
        int movedpoints;
        std::mt19937 random;
        int seed;
};
