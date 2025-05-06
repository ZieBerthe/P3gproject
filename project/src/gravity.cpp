#include "gravity.hpp"
using namespace cgp;
void Gravity::apply(mesh_drawable& object, float dt)
{
    // Apply gravity to the object
    vec3& p = object;
    p += pesanteur * dt;
    
}