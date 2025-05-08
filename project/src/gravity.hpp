#pragma once

#include "cgp/cgp.hpp"
using namespace cgp;
struct Gravity
{
    vec3 position;
    vec3 pesanteur{0, 0, 9.81f};
    bool is_active = true;
    float k = 0.1f;
    float vitesse_limite = 0.9f;
    vec3 velocity{0, 0, 0};
    float mass = 1.0f;

    void apply_gravity(camera_controller_2d_displacement& object, float dt);
};