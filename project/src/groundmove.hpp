#pragma once

#include "cgp/cgp.hpp"

struct GroundMove
{
    vec3 position;
    float amplitude;
    float totaltime;
    float timeleft;

    void apply_frame_to_chunk(mesh& chunk);
};