#include "scene.hpp"
#include "groundmove.hpp"
#include "gravity.hpp"
#include "heightgeneration.hpp"

void deform_terrain(mesh& m, HeightsGenerator& height_generator, float xdecal, float ydecal)
{
    for (int k = 0; k < m.position.size(); ++k)
    {
        vec3& p = m.position[k];
        // Compute world coordinates for seamless relief
        float world_x = p.x;
        float world_y = p.y;
		float new_z = height_generator.generateHeight(world_x+xdecal, world_y+ydecal,false);
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
	opengl_shader_structure shader_custom;
	shader_custom.load(
		project::path + "shaders/mesh/mesh.vert.glsl",
		project::path + "shaders/mesh/mesh.frag.glsl");
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
	//skybox shit
	image_structure image_skybox_template = image_load_file(project::path+"assets/stars_skybox.png");

	// Split the image into a grid of 4 x 3 sub-images
	std::vector<image_structure> image_grid = image_split_grid(image_skybox_template, 4, 3);


	skybox.initialize_data_on_gpu();
	skybox.texture.initialize_cubemap_on_gpu(image_grid[1], image_grid[7], image_grid[5], image_grid[3], image_grid[10], image_grid[4]);
	// Look at skybox_debug.png to see the correspondance of the image index

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

	height_generator = HeightsGenerator(NB_CHUNKS*CHUNKS_LENGTH); // Initialize the height generator with a roll parameter of 0.0f

	chunks = new mesh[NB_CHUNKS*NB_CHUNKS];
	chunks_draw = new mesh_drawable[NB_CHUNKS*NB_CHUNKS];

	texture_terrain.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sand.jpg", GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texture_earth.load_and_initialize_texture_2d_on_gpu(project::path + "assets/earth.jpg", GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texture_water.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sea.png", GL_REPEAT, GL_REPEAT);
	// TODO: Apply terrain deformation to each chunk after creation
	//TODO : change the number of triangles according to the distance from the camera
	for(int ichunk = 0 ; ichunk < NB_CHUNKS ; ichunk++)
	{
		for(int jchunk = 0 ; jchunk < NB_CHUNKS ; jchunk++)
		{
			chunks[ichunk*NB_CHUNKS+jchunk] = mesh_primitive_grid({ 0,0,0 }, { CHUNKS_LENGTH,0,0 }, { CHUNKS_LENGTH,CHUNKS_LENGTH,0 }, { 0,CHUNKS_LENGTH,0 }, CHUNKS_DIV, CHUNKS_DIV);
			float xdecal = ichunk*CHUNKS_LENGTH;
			float ydecal = jchunk*CHUNKS_LENGTH;
			deform_terrain(chunks[ichunk*NB_CHUNKS+jchunk], height_generator, xdecal, ydecal);
			chunks_draw[ichunk*NB_CHUNKS+jchunk].initialize_data_on_gpu(chunks[ichunk*NB_CHUNKS+jchunk]);
			chunks_draw[ichunk*NB_CHUNKS+jchunk].texture = texture_terrain;
			chunks_draw[ichunk*NB_CHUNKS+jchunk].shader= custom_shader;
			shader_custom.use();
			glUniform1i(glGetUniformLocation(shader_custom.id, "image_texture_ground"), 0); // texture unit 0
			glUniform1i(glGetUniformLocation(shader_custom.id, "image_texture_snow"), 1);   // texture unit 1
		}
		
	}
	float sea_w = 8.0;
	float sea_z = -0.8f;
	water.initialize_data_on_gpu(mesh_primitive_grid({ -sea_w,-sea_w,sea_z }, { sea_w,-sea_w,sea_z }, { sea_w,sea_w,sea_z }, { -sea_w,sea_w,sea_z }));
	water.texture = texture_water;

}


// This function is called permanently at every new frame
// Note that you should avoid having costly computation and large allocation defined there. This function is mostly used to call the draw() functions on pre-existing data.
void scene_structure::display_frame()
{

	// Set the light to the current position of the camera
	environment.light = camera_control.camera_model.position();
	glDepthMask(GL_FALSE); // disable depth-buffer writing
	draw(skybox, environment);
	glDepthMask(GL_TRUE);  // faut réactiver après avoir dessiné le skybox
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
	std::cout<<"starting chunk "<<icam<<" "<<jcam<<std::endl;
	// draw chunks in render distance for each direction
	for (int ichunk = icam-RENDER_DISTANCE; ichunk <= icam+RENDER_DISTANCE; ++ichunk)
	{
		for (int jchunk = jcam-RENDER_DISTANCE; jchunk <= jcam+RENDER_DISTANCE; ++jchunk)
		{
			int ichunk_pos = ((ichunk) % NB_CHUNKS + NB_CHUNKS) % NB_CHUNKS;
			int jchunk_pos = ((jchunk) % NB_CHUNKS + NB_CHUNKS) % NB_CHUNKS; // to avoid negative indices
			std::cout<<"chunk "<<ichunk_pos<<" "<<jchunk_pos<<" "<<ichunk<<" "<<jchunk<<std::endl;
			chunks_draw[ichunk_pos*NB_CHUNKS+jchunk_pos].model.translation = { ichunk*CHUNKS_LENGTH, jchunk*CHUNKS_LENGTH, 0 };
			draw(chunks_draw[ichunk_pos*NB_CHUNKS+jchunk_pos], environment);
			if(gui.display_wireframe)
			{
				draw_wireframe(chunks_draw[ichunk_pos*NB_CHUNKS+jchunk_pos], environment);
			}
		}
	}

	draw(water, environment);
	if (gui.display_wireframe) {
		draw_wireframe(water, environment);
	}
	







}

void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);
	ImGui::SliderInt("Render distance", &RENDER_DISTANCE, 1, NB_CHUNKS/2-1);

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
	float pos = CAMERA_HEIGHT + height_generator.generateHeight(camera_control.camera_model.position_camera.x, camera_control.camera_model.position_camera.y,false);
	if(is_floating)
	{
		pos+= FLOATING_HEIGHT;
	}
	if (camera_control.camera_model.position_camera.z <= pos){
		camera_control.camera_model.position_camera.z = pos;
		is_falling = false;
	}
	if (inputs.keyboard.is_pressed('e')) {
		is_floating = true;
	}
	else{
		is_floating = false;
		if (camera_control.camera_model.position_camera.z >pos) {
			is_falling = true;
			gravity.apply_gravity(camera_control, timer.t - time_falling);
		}
	}
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