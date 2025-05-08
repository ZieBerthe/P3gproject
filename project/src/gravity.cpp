#include "gravity.hpp"
#include <cmath>
using namespace cgp;
void Gravity::apply_gravity(camera_controller_2d_displacement& object, float dt)
{
    // Apply gravity to the object
    velocity =  mass*pesanteur.z/k* (1- exp(-dt*k/mass))* vec3{0.0f, 0.0f, -1.0f};
    object.camera_model.position_camera += velocity * dt;
}