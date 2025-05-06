#pragma once

#include "../camera_controller_first_person_euler/camera_controller_first_person_euler.hpp"

namespace cgp {

struct camera_controller_2d_displacement : camera_controller_first_person_euler
{
	void action_mouse_move(mat4& camera_matrix_view);
	void idle_frame(mat4& camera_matrix_view);
	float n_speed= 1.5f; // speed of the camera
	float run_speed= 3.0f; // speed of the camera when shift is pressed
	void configure_speed(float n_speed, float run_speed);
	std::string doc_usage() const;
};


}