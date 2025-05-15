#include "scene.hpp"
#include "groundmove.hpp"
#include "gravity.hpp"
#include "heightgeneration.hpp"
#include "heightgeneration.cpp"

void deform_terrain(mesh& m, HeightsGenerator& height_generator)
{
    for (int k = 0; k < m.position.size(); ++k)
    {
        vec3& p = m.position[k];
        // Compute world coordinates for seamless relief
        float world_x = p.x;
        float world_y = p.y;
		float new_z = height_generator.generateHeight(world_x, world_y);
        // float z = 0.2f * exp(-( (world_x)*(world_x) + (world_y )*(world_y) ) / 40.0f);
        p = { p.x, p.y, new_z};
    }
    m.normal_update();
}

// This function is called only once at the beginning of the program
// This function can contain any complex operation that can be pre-computed once
void scene_structure::initialize()
{
	std::cout << "Start function scene_structure::initialize()" << std::endl;
	
	// Set the behavior of the camera and its initial position
	// ********************************************** //
	camera_control.initialize(inputs, window); 
	// mettre la vitesse de marche et celle de course

	camera_control.configure_speed(2.0f, 15.0f); // speed of the camera
	camera_control.set_rotation_axis_z(); // camera rotates around z-axis
	//   look_at(camera_position, targeted_point, up_direction)
	camera_control.look_at(
		CAMERA_POS_START /* position of the camera in the 3D scene */,
		{0,0,0} /* targeted point in 3D scene */,
		{0,0,1} /* direction of the "up" vector */);

	// Display general information
	display_info();
	// Create the global (x,y,z) frame
	global_frame.initialize_data_on_gpu(mesh_primitive_frame());
	//gravity shit
	// gravity.mass = 1.0f; // Set the mass of the object
    // gravity.pesanteur = {0, 0, 1.0f}; // Set the gravity vector
    // gravity.k = 0.1f; // Set the resistance coefficient

	// Create the shapes seen in the 3D scene
	// ********************************************** //
	chunks = new mesh[NB_CHUNSKS*NB_CHUNSKS];
	chunks_draw = new mesh_drawable[NB_CHUNSKS*NB_CHUNSKS];
	// TODO: Apply terrain deformation to each chunk after creation
	//TODO : change the number of triangles according to the distance from the camera
	for(int ichunk = 0 ; ichunk < NB_CHUNSKS ; ichunk++)
	{
		for(int jchunk = 0 ; jchunk < NB_CHUNSKS ; jchunk++)
		{
			float x = ichunk*CHUNKS_LENGTH;
			float y = jchunk*CHUNKS_LENGTH;
			chunks[ichunk*NB_CHUNSKS+jchunk] = mesh_primitive_grid({ x,y,0 }, { x+CHUNKS_LENGTH,y,0 }, { x+CHUNKS_LENGTH,y+CHUNKS_LENGTH,0 }, { x,y+CHUNKS_LENGTH,0 }, 50, 50);
			deform_terrain(chunks[ichunk*NB_CHUNSKS+jchunk], height_generator);
			chunks_draw[ichunk*NB_CHUNSKS+jchunk].initialize_data_on_gpu(chunks[ichunk*NB_CHUNSKS+jchunk]);
			chunks_draw[ichunk*NB_CHUNSKS+jchunk].texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sand.jpg", GL_REPEAT, GL_REPEAT);
		}
		
	}
	float sea_w = 8.0;
	float sea_z = -0.8f;
	water.initialize_data_on_gpu(mesh_primitive_grid({ -sea_w,-sea_w,sea_z }, { sea_w,-sea_w,sea_z }, { sea_w,sea_w,sea_z }, { -sea_w,sea_w,sea_z }));
	water.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sea.png");

	tree.initialize_data_on_gpu(mesh_load_file_obj(project::path + "assets/palm_tree/palm_tree.obj"));
	tree.model.rotation = rotation_transform::from_axis_angle({ 1,0,0 }, Pi / 2.0f);
	tree.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/palm_tree/palm_tree.jpg", GL_REPEAT, GL_REPEAT);

	cube1.initialize_data_on_gpu(mesh_primitive_cube({ 0,0,0 }, 0.5f));
	cube1.model.rotation = rotation_transform::from_axis_angle({ -1,1,0 }, Pi / 7.0f);
	cube1.model.translation = { 1.0f,1.0f,-0.1f };
	cube1.texture.load_and_initialize_texture_2d_on_gpu(project::path + "assets/wood.jpg");

	cube2 = cube1;

}


// This function is called permanently at every new frame
// Note that you should avoid having costly computation and large allocation defined there. This function is mostly used to call the draw() functions on pre-existing data.
void scene_structure::display_frame()
{

	// Set the light to the current position of the camera
	environment.light = camera_control.camera_model.position();

	// Update time
	timer.update();
	if (is_falling == false) {
		std::cout << "nottt t thsahfdahFalling" << std::endl;
		//std::cout << "Time passing " << timer.t << std::endl;
		std::cout << "Time " << time_falling << std::endl;
		time_falling = timer.t;
	}
	
	// conditional display of the global frame (set via the GUI)
	if (gui.display_frame)
		draw(global_frame, environment);
	
	
	// Draw all the shapes
	// calculate chunk in which the cam is located
	int icam = int(camera_control.camera_model.position().x / CHUNKS_LENGTH);
	int jcam = int(camera_control.camera_model.position().y / CHUNKS_LENGTH);
	// draw chunks in render distance for each direction
	for (int ichunk = -RENDER_DISTANCE; ichunk <= RENDER_DISTANCE; ++ichunk)
	{
		for (int jchunk = -RENDER_DISTANCE; jchunk <= RENDER_DISTANCE; ++jchunk)
		{
			int ichunk_pos = icam + ichunk;
			int jchunk_pos = jcam + jchunk;
			if (ichunk_pos >= 0 && ichunk_pos < NB_CHUNSKS && jchunk_pos >= 0 && jchunk_pos < NB_CHUNSKS)
			{
				draw(chunks_draw[ichunk_pos*NB_CHUNSKS+jchunk_pos], environment);
				if(gui.display_wireframe)
				{
					draw_wireframe(chunks_draw[ichunk_pos*NB_CHUNSKS+jchunk_pos], environment);
				}
			}
		}
	}

	draw(water, environment);
	draw(tree, environment);
	draw(cube1, environment);

	// Animate the second cube in the water
	cube2.model.translation = { -1.0f, 6.0f+0.1*sin(0.5f*timer.t), -0.8f + 0.1f * cos(0.5f * timer.t)};
	cube2.model.rotation = rotation_transform::from_axis_angle({1,-0.2,0},Pi/12.0f*sin(0.5f*timer.t));
	draw(cube2, environment);

	if (gui.display_wireframe) {
		draw_wireframe(water, environment);
		draw_wireframe(tree, environment);
		draw_wireframe(cube1, environment);
		draw_wireframe(cube2, environment);
	}
	







}

void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);

}

void scene_structure::mouse_move_event()
{
	if (!inputs.keyboard.shift)
		camera_control.action_mouse_move(environment.camera_view);
}
void scene_structure::mouse_click_event()
{
	camera_control.action_mouse_click(environment.camera_view);
}
void scene_structure::keyboard_event()
{
	camera_control.action_keyboard(environment.camera_view);
}
void scene_structure::idle_frame()
{
	camera_control.idle_frame(environment.camera_view);
	float pos = 0.5f+ height_generator.generateHeight(camera_control.camera_model.position_camera.x, camera_control.camera_model.position_camera.y);
	if (camera_control.camera_model.position_camera.z <= pos){
		camera_control.camera_model.position_camera.z = pos;
		is_falling = false;
	}
	if (inputs.keyboard.is_pressed('e')) {
		camera_control.camera_model.position_camera.z += 1.0f;
	}
	else{
	if (camera_control.camera_model.position_camera.z >pos) {
		if (is_falling == false) {
			
			std::cout << "Falling" << std::endl;
			std::cout << "Time passing " << timer.t << std::endl;
			std::cout << "Time " << std::endl;
			}
		is_falling = true;
		std::cout<< "Falling" << std::endl;
		std::cout << "Time " << timer.t << std::endl;
		std::cout << "Time fdfadafeg " << time_falling << std::endl;
		std::cout << "Time falling: " << timer.t - time_falling << std::endl;
		gravity.apply_gravity(camera_control, timer.t - time_falling);
	}
	}

	std::cout << "Camera position:" << camera_control.camera_model.position_camera << std::endl;
}


void scene_structure::display_info()
{
	std::cout << "\nCAMERA CONTROL:" << std::endl;
	std::cout << "-----------------------------------------------" << std::endl;
	std::cout << camera_control.doc_usage() << std::endl;
	std::cout << "-----------------------------------------------\n" << std::endl;


	std::cout << "\nSCENE INFO:" << std::endl;
	std::cout << "-----------------------------------------------" << std::endl;
	std::cout << "press shift +c for mouse control, shift +f for full screen mode" << std::endl;
	std::cout << "Display here the information you would like to appear at the start of the program on the command line (file scene.cpp, function display_info())." << std::endl;
	std::cout << "-----------------------------------------------\n" << std::endl;
}