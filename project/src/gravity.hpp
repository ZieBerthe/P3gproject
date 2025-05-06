#pragma once

#include "cgp/cgp.hpp"
using namespace cgp;
struct Gravity
{
    vec3 position;
    vec3 pesanteur{0, 0, -9.81f};
    bool is_active = true;
    
    

    void apply(mesh_drawable& object, float dt);
};