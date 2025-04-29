#include "groundmove.hpp"

void GroundMove::apply_frame_to_chunk(mesh& chunk)
{
    for (int k = 0; k < chunk.position.size(); ++k)
    {
        vec3& p = chunk.position[k];
        float d2 = p.x * p.x + p.y * p.y;
        float z = amplitude * exp(-d2 / 4) - 1;
        z += 0.05f * noise_perlin({ p.x, p.y });
        p.z = z;
    }
    chunk.normal_update();
}