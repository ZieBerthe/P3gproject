#include "scene.hpp"
#include "groundmove.hpp"
#include "gravity.hpp"
#include "heightgeneration.hpp"


bool is_chunk_visible(int ichunk, int jchunk, int icamchunk, int jcamchunk, int RENDER_DISTANCE) {
    return std::abs(ichunk - icamchunk) <= RENDER_DISTANCE &&
           std::abs(jchunk - jcamchunk) <= RENDER_DISTANCE;
}
int scene_structure::roll_chunk(int chunk)
{
	// Roll the chunk index to ensure it stays within the bounds of the array
	return ((chunk % NB_CHUNKS) + NB_CHUNKS) % NB_CHUNKS;
}
float scene_structure::roll_pos(float pos)
{
	// Roll the position index to ensure it stays within the bounds of the array
	return fmod((fmod(pos, ROLLLENGTH) + ROLLLENGTH), ROLLLENGTH);
}

void scene_structure::deform_chunk_dyna(mesh& chunk, int ichunk, int jchunk)
{
	int chunkid = CHUNKS_DIV * CHUNKS_DIV * (ichunk * NB_CHUNKS + jchunk);
    for (int k = 0; k < chunk.position.size(); ++k)
    {
        vec3& p = chunk.position[k];
        // Compute world coordinates for seamless relief
		float zbase = flatdecals[chunkid + k];
		if(noflat)
		{
			zbase = 0;
		}
		float new_z = zbase + move_generator.calculateHeight(p.x, p.y);
		// Update the vertex position
		p = { p.x, p.y, new_z };
    }
    chunk.normal_update();
}


void scene_structure::deform_chunk_init(mesh& chunk, int ichunk, int jchunk)
{
	// init chunk deformation with perlin noise
	int chunkid = CHUNKS_DIV * CHUNKS_DIV * (ichunk * NB_CHUNKS + jchunk);
	for (int k = 0; k < chunk.position.size(); ++k)
	{
		// position of the vertex in the chunk
		vec3& p = chunk.position[k];
		float usex = roll_pos(p.x);
		float usey = roll_pos(p.y);
		float z = height_generator.generateHeight(usex, usey);
		if (noflat)
		{
			z = 0; // if no flat, set the height to 0
		}
		flatdecals[chunkid + k] = z;
		p = { p.x, p.y, z};
	}
	chunk.normal_update();
}

// This function is called only once at the beginning of the program
// This function can contain any complex operation that can be pre-computed once
void scene_structure::initialize()
{
	std::cout << "Start function scene_structure::initialize()" << std::endl;
	opengl_shader_structure shader_custom;
	shader_custom.load(
		project::path + "shaders/mesh/mesh.vert.glsl",
		project::path + "shaders/mesh/mesh_multi_texture.frag.glsl");
	// Set the behavior of the camera and its initial position
	// ********************************************** //
	camera_control.initialize(inputs, window); 
	// mettre la vitesse de marche et celle de course

	camera_control.configure_speed(6.0f, 1.5f); // speed of the camera
	camera_control.set_rotation_axis_z(); // camera rotates around z-axis
	camera_control.look_at(
		CAMERA_POS_START /* position of the camera in the 3D scene */,
		{0,0,0} /* targeted point in 3D scene */,
		{0,0,1} /* direction of the "up" vector */);
	
	// SKYBOX SETUP
	image_structure image_skybox_template = image_load_file(project::path+"assets/nskybox.png");
	std::vector<image_structure> image_grid = image_split_grid(image_skybox_template, 4, 3); // Split the image into a grid of 4 x 3 sub-images
	skybox.initialize_data_on_gpu();
	skybox.texture.initialize_cubemap_on_gpu(image_grid[1], image_grid[7], image_grid[5], image_grid[3], image_grid[10], image_grid[4]);

	// INFO DISPLAY
	display_info();
	global_frame.initialize_data_on_gpu(mesh_primitive_frame());

	// TEXTURE SETUP
	texture_terrain.load_and_initialize_texture_2d_on_gpu(project::path + "assets/earth.jpg", GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	//texture_earth.load_and_initialize_texture_2d_on_gpu(project::path + "assets/earth.jpg", GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texture_water.load_and_initialize_texture_2d_on_gpu(project::path + "assets/sea.png", GL_REPEAT, GL_REPEAT);

	// CHUNK SETUP ----------------------------------------------

	// GENERATORS SETUP
	height_generator = HeightsGenerator(ROLLLENGTH);
	move_generator = MoveGenerator(ROLLLENGTH, 8*NB_CHUNKS, 0); // Initialize the move generator with a roll parameter of 0.0f // Initialize the height generator with a roll parameter of 0.0f
	const auto& pts = move_generator.getPoints();
	for (const auto& pt : pts) {
		std::cout << "les points sont Point: (" << pt.x0 << ", " << pt.y0 << ", " << pt.speed << ", " << pt.amplitudemax << ", " << pt.flateningfactor << ", " << pt.rollparameter << ", " << pt.seed << ")" << std::endl;
	}
	// --- Send points to the vertex shader as a uniform array ---
	// Assuming shader_custom is still in scope and pts is a vector of a struct with float fields
	// We'll send x0, y0, speed, amplitudemax as a vec4 array (adjust as needed)
	std::vector<float> points_flat;
	for (const auto& pt : pts) {
		points_flat.push_back(pt.x0);
		points_flat.push_back(pt.y0);
		points_flat.push_back(pt.speed);
		points_flat.push_back(pt.amplitudemax);
		
	}
	GLuint shader_id = shader_custom.id;
	glUseProgram(shader_id);
	int max_points = NB_CHUNKS * NB_CHUNKS; 
	int n_points = std::min(int(pts.size()), max_points);
	GLint loc = glGetUniformLocation(shader_id, "points");
	if (loc != -1) {
		glUniform4fv(loc, n_points, points_flat.data());
	}
	GLint loc_count = glGetUniformLocation(shader_id, "points_count");
	if (loc_count != -1) {
		glUniform1i(loc_count, n_points);
	}
	glUseProgram(0);
	chunks = new mesh[NB_CHUNKS*NB_CHUNKS];
	chunks_draw = new mesh_drawable[NB_CHUNKS*NB_CHUNKS];
	flatdecals = new float[NB_CHUNKS*NB_CHUNKS* CHUNKS_DIV * CHUNKS_DIV];


	
	for(int ichunk = 0 ; ichunk < NB_CHUNKS ; ichunk++)
	{
		for(int jchunk = 0 ; jchunk < NB_CHUNKS ; jchunk++)
		{
			mesh& cur_chunk = chunks[ichunk*NB_CHUNKS+jchunk];
			float xpos = ichunk*CHUNKS_LENGTH;
			float ypos = jchunk*CHUNKS_LENGTH;
			cur_chunk = mesh_primitive_grid(
				{ xpos, ypos, 0 }, 
				{ xpos+CHUNKS_LENGTH, ypos, 0 }, 
				{ xpos+CHUNKS_LENGTH, ypos+CHUNKS_LENGTH, 0 }, 
				{ xpos, ypos+CHUNKS_LENGTH, 0 }, 
				CHUNKS_DIV, CHUNKS_DIV);
			deform_chunk_init(cur_chunk, ichunk, jchunk);

			//TEXTURE AND SHADER SETUP
			chunks_draw[ichunk*NB_CHUNKS+jchunk].initialize_data_on_gpu(cur_chunk);
			chunks_draw[ichunk*NB_CHUNKS+jchunk].shader = shader_custom;
			chunks_draw[ichunk*NB_CHUNKS+jchunk].texture = texture_terrain;
			chunks_draw[ichunk*NB_CHUNKS+jchunk].supplementary_texture["image_texture_2"].load_and_initialize_texture_2d_on_gpu(project::path + "assets/snowtexture.jpg", GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
			//chunks_draw[ichunk*NB_CHUNKS+jchunk].shader= shader_custom;
			// glUniform1i(glGetUniformLocation(shader_custom.id, "image_texture_ground"), 0); // texture unit 0
			// glUniform1i(glGetUniformLocation(shader_custom.id, "image_texture_snow"), 1);   // texture unit 1
		}
		
	}

	// WATER SETUP
	float SEA_Z = -0.8f; // Height of the sea
	float sea_w = NB_CHUNKS*CHUNKS_LENGTH/2.0f; // Width of the sea
	water.initialize_data_on_gpu(mesh_primitive_grid({ -sea_w,-sea_w,SEA_Z }, { sea_w,-sea_w,SEA_Z }, { sea_w,sea_w,SEA_Z }, { -sea_w,sea_w,SEA_Z }));
	water.texture = texture_water;


}



void scene_structure::display_frame()
{
	environment.uniform_generic.uniform_float["time"] = timer.t; // send the time to the shader

	// CAMERA AND SKYBOX DISPLAYING
	// Set the light to the current position of the camera
	environment.light = camera_control.camera_model.position();
	glDepthMask(GL_FALSE); // disable depth-buffer writing
	draw(skybox, environment);
	glDepthMask(GL_TRUE);  // faut réactiver après avoir dessiné le skybox

	// TIMER UPDATE AND TERRAIN DEFORMATION
	timer.update();
	if (is_falling == false) {
		// std::cout << "nottt t thsahfdahFalling" << std::endl;
		// std::cout << "Time " << time_falling << std::endl;
		time_falling = timer.t;
	}
	std::cout << "tick" << timer.t - lasttick.t << std::endl;
	move_generator.updatesituation(timer.t - lasttick.t);
	lasttick.update();
	
	// conditional display of the global frame (set via the GUI)
	if (gui.display_frame)
		draw(global_frame, environment);
	

	// CHUNKS DRAWING

	// CALCULATING CHUNK POSITION
	int icamchunk = int(camera_control.camera_model.position().x / CHUNKS_LENGTH);
	int jcamchunk = int(camera_control.camera_model.position().y / CHUNKS_LENGTH);
	//std::cout<<"Camera in chunk : ("<<icamchunk<<" ; "<<jcamchunk<<")"<<std::endl;
	// print current camera position
	std::cout << "Camera position: (" << camera_control.camera_model.position().x << ", " 
		<< camera_control.camera_model.position().y << ", " 
		<< camera_control.camera_model.position().z << ")" << std::endl;
	for (int ichunk = icamchunk-RENDER_DISTANCE; ichunk <= icamchunk+RENDER_DISTANCE; ++ichunk)
	{

		for (int jchunk = jcamchunk-RENDER_DISTANCE; jchunk <= jcamchunk+RENDER_DISTANCE; ++jchunk)
		{
			if((ichunk -icamchunk) * (ichunk -icamchunk) + (jchunk -jcamchunk) * (jchunk -jcamchunk) > RENDER_DISTANCE * (RENDER_DISTANCE+2))
			{
				continue; // skip chunks that are too far away
			}
			int ichunk_pos = roll_chunk(ichunk);
			int jchunk_pos = roll_chunk(jchunk);
			int ichunk_dec = ichunk - ichunk_pos; // calculate the difference between the chunk index and the rolled chunk index
			int jchunk_dec = jchunk - jchunk_pos; // calculate the difference between the chunk index and the rolled chunk index
			//std::cout<<"Chunk : "<<ichunk<<" ; "<<jchunk<<" -- Assigned chunk : "<<ichunk_pos<<" ; "<<jchunk_pos<<std::endl;
			
			deform_chunk_dyna(chunks[ichunk_pos*NB_CHUNKS+jchunk_pos], ichunk_pos, jchunk_pos); // deform with moves
			chunks_draw[ichunk_pos*NB_CHUNKS+jchunk_pos].initialize_data_on_gpu(chunks[ichunk_pos*NB_CHUNKS+jchunk_pos]);
			chunks_draw[ichunk_pos*NB_CHUNKS+jchunk_pos].model.translation = { ichunk_dec*CHUNKS_LENGTH, jchunk_dec*CHUNKS_LENGTH, 0 }; // move the chunk to the correct place
			draw(chunks_draw[ichunk_pos*NB_CHUNKS+jchunk_pos], environment); // draw the chunk
			if(gui.display_wireframe)
			{
				draw_wireframe(chunks_draw[ichunk_pos*NB_CHUNKS+jchunk_pos], environment);
			}
		}
	}

	//WATER DRAWING
	if (gui.display_wireframe) {
		draw_wireframe(water, environment);
	}
}

void scene_structure::display_gui()
{
	ImGui::Checkbox("Frame", &gui.display_frame);
	ImGui::Checkbox("Wireframe", &gui.display_wireframe);
	ImGui::SliderInt("Render distance", &RENDER_DISTANCE, 1, NB_CHUNKS/2);

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
	float camx = camera_control.camera_model.position_camera.x;
	float camy = camera_control.camera_model.position_camera.y;
	//float camz = camera_control.camera_model.position_camera.z;

	float camx_rollflipped = roll_pos(camx);
	float camy_rollflipped = roll_pos(camy);
	float decalbase = height_generator.generateHeight(camx_rollflipped, camy_rollflipped);
	if (noflat) {
		decalbase = 0.0f; // if no flat, set the height to 0
	}
	float decal = decalbase + move_generator.calculateHeight(camx_rollflipped, camy_rollflipped);

	float newz = CAMERA_HEIGHT + decal;
	if (camera_control.camera_model.position_camera.z <= newz){
		camera_control.camera_model.position_camera.z = newz;
		is_falling = false;
	}
	if (inputs.keyboard.is_pressed('x')) {
		camera_control.camera_model.position_camera.z += 0.5f;
		if (camera_control.camera_model.position_camera.z > newz+10.0f) {
			camera_control.camera_model.position_camera.z = newz+10.0f;
		}
	}
	else{
	if (camera_control.camera_model.position_camera.z >newz) {
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