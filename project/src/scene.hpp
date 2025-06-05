#pragma once

#include "gravity.hpp"
#include "cgp/cgp.hpp"
#include "environment.hpp"
#include "heightgeneration.hpp"
// This definitions allow to use the structures: mesh, mesh_drawable, etc. without mentionning explicitly cgp::
using cgp::mesh;
using cgp::mesh_drawable;
using cgp::vec3;
using cgp::numarray;
using cgp::timer_basic;

// Variables associated to the GUI (buttons, etc)
struct gui_parameters {
	bool display_frame = true;
	bool display_wireframe = false;
	bool display_skybox = true; // Display the skybox
};

// The structure of the custom scene
struct scene_structure : cgp::scene_inputs_generic {
	
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //
	
	camera_projection_perspective camera_projection;
	window_structure window;

	mesh_drawable global_frame;          // The standard global frame
	environment_structure environment;   // Standard environment controler
	input_devices inputs;                // Storage for inputs status (mouse, keyboard, window dimension)
	gui_parameters gui;                  // Standard GUI element storage
	
	opengl_shader_structure shader_custom;

	camera_controller_2d_displacement camera_control;
	// ****************************** //
	// Elements and shapes of the scene
	// ****************************** //
	Gravity gravity;
	timer_basic timer;
	timer_basic lasttick;
	HeightsGenerator height_generator;
	MoveGenerator move_generator;

	const int NB_CHUNKS = 10; // Number of chunks in each direction (total number of chunks = NB_CHUNKS^2)
	int RENDER_DISTANCE = 1; // Number of chunks to render in each direction
	const float CHUNKS_LENGTH = 10.0f; // Length of a chunk side
	const int CHUNKS_DIV = 15; // Number of vertices per chunk each side
	const float ROLLLENGTH = NB_CHUNKS * CHUNKS_LENGTH; // Total length of the rolling terrain
	const float ROLLFLIPLENGTH = NB_CHUNKS * CHUNKS_LENGTH / 2;
	

	vec3 CAMERA_POS_START = { 1.0f, 10.0f, 10.0f };
	float CAMERA_HEIGHT = 1.0f;
	float FLOATING_HEIGHT = 10.0f;
	bool is_floating = false;
	bool noflat = false;


	mesh* chunks;
	mesh_drawable* chunks_draw;
	float* flatdecals;
	mesh_drawable water;
	
	mesh_drawable tree;
	opengl_texture_image_structure texture_terrain;
	opengl_texture_image_structure texture_water;
	opengl_texture_image_structure texture_earth;

	bool is_falling = false;
	float time_falling = 0.0f;
	//skybox shit
	cgp::skybox_drawable skybox;
	// ****************************** //
	// Functions
	// ****************************** //

	void initialize();    // Standard initialization to be called before the animation loop
	void display_frame(); // The frame display to be called within the animation loop
	void display_gui();   // The display of the GUI, also called within the animation loop

	int roll_chunk(int chunk);
	float roll_pos(float pos);
	float roll_flip_pos(float pos);



	void deform_chunk_init(mesh& chunk, int ichunk, int jchunk);
	void deform_chunk_dyna(mesh& chunk, int ichunk, int jchunk);

	void mouse_move_event();
	void mouse_click_event();
	void keyboard_event();
	void idle_frame();

	void display_info();

};





