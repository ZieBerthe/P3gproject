#pragma once

#include "cgp/cgp.hpp"
using namespace cgp;
struct GroundMove
{
    vec3 position;
    float amplitude;
    float totaltime;
    float timeleft;

    void apply_frame_to_chunk(mesh& chunk);
};