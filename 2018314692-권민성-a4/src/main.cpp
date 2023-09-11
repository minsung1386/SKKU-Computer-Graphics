#include "cgmath.h"		// slee's simple math library
#define STB_IMAGE_IMPLEMENTATION
#include "cgut.h"		// slee's OpenGL utility
#include "stb_image.h"
#include "trackball.h"
#include "sphere.h"

//*************************************
// global constants
static const char* window_name = "Assignment 4: Solar System - Minsung Kwon 2018314692";
static const char* vert_shader_path = "shaders/transform.vert";
static const char* frag_shader_path = "shaders/transform.frag";
// planets
static const char* earth_image_path		= "shaders/textures/earth.jpg";
static const char* jupiter_image_path	= "shaders/textures/jupiter.jpg";
static const char* mars_image_path		= "shaders/textures/mars.jpg";
static const char* mercury_image_path	= "shaders/textures/mercury.jpg";
static const char* moon_image_path		= "shaders/textures/moon.jpg";
static const char* neptune_image_path	= "shaders/textures/neptune.jpg";
static const char* saturn_image_path	= "shaders/textures/saturn.jpg";
static const char* sun_image_path		= "shaders/textures/sun.jpg";
static const char* uranus_image_path	= "shaders/textures/uranus.jpg";
static const char* venus_image_path		= "shaders/textures/venus.jpg";
// rings
static const char* saturn_ring_image_path		= "shaders/textures/saturn-ring.jpg";
static const char* uranus_ring_image_path		= "shaders/textures/uranus-ring.jpg";
static const char* saturn_ring_alpha_image_path = "shaders/textures/saturn-ring-alpha.jpg";
static const char* uranus_ring_alpha_image_path = "shaders/textures/uranus-ring-alpha.jpg";
// bump
static const char* earth_bump_image_path	= "shaders/textures/earth-bump.jpg";
static const char* mars_bump_image_path		= "shaders/textures/mars-bump.jpg";
static const char* mercury_bump_image_path	= "shaders/textures/mercury-bump.jpg";
static const char* moon_bump_image_path		= "shaders/textures/moon-bump.jpg";
static const char* venus_bump_image_path	= "shaders/textures/venus-bump.jpg";
// normal
static const char* earth_normal_image_path		= "shaders/textures/earth-normal.jpg";
static const char* mars_normal_image_path		= "shaders/textures/mars-normal.jpg";
static const char* mercury_normal_image_path	= "shaders/textures/mercury-normal.jpg";
static const char* moon_normal_image_path		= "shaders/textures/moon-normal.jpg";
static const char* venus_normal_image_path		= "shaders/textures/venus-normal.jpg";

//*************************************
// common structures
struct camera
{
	vec3	eye = vec3(5, 0, 0);
	vec3	at = vec3(0, 0, 0);
	vec3	up = vec3(0, 0, 1);
	mat4	view_matrix = mat4::look_at(eye, at, up);

	float	fovy = PI / 4.0f; // must be in radian
	float	aspect;
	float	dnear = 1.0f;
	float	dfar = 1000.0f;
	mat4	projection_matrix;
};

struct light_t
{
	vec4	position = vec4(0.0f, 0.0f, 0.0f, 1.0f);   // non directional light
	vec4	ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
};

struct material_t
{
	vec4	ambient = vec4(0.2f, 0.2f, 0.2f, 1.0f);
	vec4	diffuse = vec4(0.8f, 0.8f, 0.8f, 1.0f);
	vec4	specular = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float	shininess = 1000.0f;
};

//*************************************
// window objects
GLFWwindow* window = nullptr;
ivec2		window_size = cg_default_window_size(); // initial window size

//*************************************
// OpenGL objects
GLuint	program = 0;	// ID holder for GPU program
GLuint	vertex_array = 0;	// ID holder for vertex array object (planet)
GLuint	ring_vertex_array = 0;	// ID holder for vertex array object (ring)
GLuint	PLANETS_TEX[13] = { 0 };
GLuint	RING_TEX[4] = { 0 };
GLuint	NORM_TEX_MERCURY;
GLuint	NORM_TEX_VENUS;
GLuint	NORM_TEX_EARTH;
GLuint	NORM_TEX_MARS;
GLuint	NORM_TEX_MOON;
//*************************************
// global variables
int		frame = 0;		// index of rendering frames
auto	spheres = std::move(create_spheres());

float	theta, pause_theta = 0.0f;
bool	b_wireframe = false;

vec2	prev_pos;
mat4	prev_view_matrix;

bool	b_left_control = false;
bool	b_left_shift = false;
bool	b_rotate = true;

GLint	fc = 0;
GLint	idx = 0;
//*************************************
// scene objects
camera		cam;
trackball	tb;
light_t		light;
material_t	material;
//*************************************
// holder of vertices and indices of a unit sphere
std::vector<vertex>	unit_sphere_vertices;	// host-side vertices
std::vector<vertex> unit_ring_vertices;
//*************************************
void update()
{
	float aspect = window_size.x / float(window_size.y);
	mat4 aspect_matrix = mat4::scale(std::min(1 / aspect, 1.0f), std::min(aspect, 1.0f), 1.0f);

	// update projection matrix
	cam.aspect = window_size.x / float(window_size.y);
	cam.projection_matrix = mat4::perspective(cam.fovy, cam.aspect, cam.dnear, cam.dfar);

	// update uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation(program, "view_matrix");			if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.view_matrix);
	uloc = glGetUniformLocation(program, "projection_matrix");	if (uloc > -1) glUniformMatrix4fv(uloc, 1, GL_TRUE, cam.projection_matrix);	

	// setup light properties
	glUniform4fv(glGetUniformLocation(program, "light_position"), 1, light.position);
	glUniform4fv(glGetUniformLocation(program, "Ia"), 1, light.ambient);
	glUniform4fv(glGetUniformLocation(program, "Id"), 1, light.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Is"), 1, light.specular);

	// setup material properties
	glUniform4fv(glGetUniformLocation(program, "Ka"), 1, material.ambient);
	glUniform4fv(glGetUniformLocation(program, "Kd"), 1, material.diffuse);
	glUniform4fv(glGetUniformLocation(program, "Ks"), 1, material.specular);
	glUniform1f(glGetUniformLocation(program, "shininess"), material.shininess);

}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// notify GL that we use our own program
	glUseProgram(program);

	// bind vertex array object
	//glDisable(GL_BLEND);
	glBindVertexArray(vertex_array);

	// render vertices: trigger shader programs to process vertex data
	theta = b_rotate ? float(glfwGetTime()) : theta;
	int index = 0;
	for (auto& s : spheres)
	{
		s.update(theta, spheres, index);
		glActiveTexture(GL_TEXTURE0+index);
		glBindTexture(GL_TEXTURE_2D, PLANETS_TEX[index]);
		glUniform1i(glGetUniformLocation(program, "TEX"), index);
		glUniform1i(glGetUniformLocation(program, "idx"), index);		// to index SUN for not shading
		
		// NORM TEXTURES
		if (index == 1) 
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, NORM_TEX_MERCURY);
			glUniform1i(glGetUniformLocation(program, "NORM"), 1);
		}
		else if (index == 2)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, NORM_TEX_VENUS);
			glUniform1i(glGetUniformLocation(program, "NORM"), 1);
		}
		else if (index == 3) 
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, NORM_TEX_EARTH);
			glUniform1i(glGetUniformLocation(program, "NORM"), 1);
		}
		else if (index == 4)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, NORM_TEX_MARS);
			glUniform1i(glGetUniformLocation(program, "NORM"), 1);
		}
		else if (index >= 9)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, NORM_TEX_MOON);
			glUniform1i(glGetUniformLocation(program, "NORM"), 1);
		}

		glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, s.model_matrix);
		glDrawElements(GL_TRIANGLES, 72 * 36 * 2 * 3, GL_UNSIGNED_INT, nullptr);
		index++;
	}

	//*************************************
	// Draw rings
	glDisable(GL_CULL_FACE);			// turn off backface culling
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Saturn ring
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, RING_TEX[0]);
	glUniform1i(glGetUniformLocation(program, "TEX1"), 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, RING_TEX[1]);
	glUniform1i(glGetUniformLocation(program, "TEX2"), 2);
	
	glUniform1i(glGetUniformLocation(program, "idx"), -1);

	glBindVertexArray(ring_vertex_array);

	mat4 ring_model_matrix, saturn_model_matrix;
	saturn_model_matrix = spheres[6].get_model_matrix();
	ring_model_matrix = saturn_model_matrix * mat4::scale(0.8f);
	glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, ring_model_matrix);
	glDrawElements(GL_TRIANGLES, 72 * 2 * 3, GL_UNSIGNED_INT, nullptr);
	
	// Uranus ring
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, RING_TEX[2]);
	glUniform1i(glGetUniformLocation(program, "TEX1"), 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, RING_TEX[3]);
	glUniform1i(glGetUniformLocation(program, "TEX2"), 2);

	mat4 uranus_model_matrix = spheres[7].get_model_matrix();
	ring_model_matrix = uranus_model_matrix * mat4::scale(0.6f);
	glUniformMatrix4fv(glGetUniformLocation(program, "model_matrix"), 1, GL_TRUE, ring_model_matrix);
	glDrawElements(GL_TRIANGLES, 72 * 2 * 3, GL_UNSIGNED_INT, nullptr);


	glEnable(GL_CULL_FACE);			// turn off backface culling
	glDisable(GL_BLEND);

	// swap front and back buffers, and display to screen
	glfwSwapBuffers(window);
}

void reshape(GLFWwindow* window, int width, int height)
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width, height);
	glViewport(0, 0, width, height);
}

void print_help()
{
	printf("[help]\n");
	printf("- press ESC or 'q' to terminate the program\n");
	printf("- press F1 or 'h' to see help\n");
	printf("- press 'w' to toggle wireframe\n");
	printf("- press Home to reset camera\n");
	printf("- press Pause to pause the simulation\n");
	printf("\n");
}

std::vector<vertex> create_sphere_vertices()
{
	// 72 edges in longitude & 36 edges in latitude
	std::vector<vertex> v;

	float rad = 1.0f;
	for (uint i = 0; i <= 36; i++)
	{
		for (uint j = 0; j <= 72; j++)
		{
			float theta = PI * i / float(36);
			float pi = PI * 2.0f * j / float(72);
			vec3 pos = vec3(rad * sin(theta) * cos(pi), rad * sin(theta) * sin(pi), rad * cos(theta));
			vec3 norm = pos;
			vec2 tc = vec2((float)j / 72.0f, 1-(float)i/36.0f);

			v.push_back({ pos, norm, tc });
		}
	}
	return v;
}

void update_vertex_buffer(const std::vector<vertex>& vertices)
{
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	std::vector<uint> indices;
	for (uint i = 0; i < 36; i++)
	{
		for (uint j = 0; j < 72; j++)
		{
			indices.push_back(i * 73 + j + 1);			//(i, j+1)
			indices.push_back(i * 73 + j);				//(i, j)
			indices.push_back((i + 1) * 73 + j);		//(i+1, j)

			indices.push_back(i * 73 + j + 1);			//(i, j+1)
			indices.push_back((i + 1) * 73 + j);		//(i+1, j)
			indices.push_back((i + 1) * 73 + j + 1);	//(i+1, j+1)
		}
	}
	// generation of vertex buffer: use vertices as it is
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// geneation of index buffer
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);

	if (vertex_array) glDeleteVertexArrays(1, &vertex_array);
	vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
	if (!vertex_array) { printf("%s(): failed to create vertex array\n", __func__); return; }
}

std::vector<vertex> create_ring_vertcies()
{
	std::vector<vertex> v;
	for (uint i = 0; i <= 72; i++)
	{
		float theta = (PI * 2.0f * i)/ float(72);
		vec3 pos = vec3(2.0f*cos(theta), 2.0f*sin(theta), 0.0f);
		vec3 norm =pos;
		vec2 tc = vec2(1.0f, (float)i / 72.0f);
		v.push_back({ pos, norm, tc });

		pos = vec3(4.0f*cos(theta), 4.0f*sin(theta), 0.0f);
		tc = vec2(0.0f, (float)i / 72.0f);
		v.push_back({ pos, norm, tc });
	}
	return v;
}

void update_ring_vertex_buffer(const std::vector<vertex>& vertices)
{
	static GLuint vertex_buffer = 0;	// ID holder for vertex buffer
	static GLuint index_buffer = 0;		// ID holder for index buffer

	std::vector<uint> indices;
	for (uint i = 0; i < 72; i++)
	{
		indices.push_back(2 * i);
		indices.push_back(2 * i + 3);
		indices.push_back(2 * i + 1);

		indices.push_back(2 * i);
		indices.push_back(2 * i + 2);
		indices.push_back(2 * i + 3);
	}
	// generation of vertex buffer: use vertices as it is
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// geneation of index buffer
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);

	if (ring_vertex_array) glDeleteVertexArrays(1, &ring_vertex_array);
	ring_vertex_array = cg_create_vertex_array(vertex_buffer, index_buffer);
	if (!ring_vertex_array) { printf("%s(): failed to create vertex array\n", __func__); return; }
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q)	glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_H || key == GLFW_KEY_F1)	print_help();
		else if (key == GLFW_KEY_PAUSE || key == GLFW_KEY_SPACE)
		{
			b_rotate = !b_rotate;
			if (b_rotate) glfwSetTime(pause_theta);
			else pause_theta = theta;
		}
		else if (key == GLFW_KEY_W)
		{
			b_wireframe = !b_wireframe;
			glPolygonMode(GL_FRONT_AND_BACK, b_wireframe ? GL_LINE : GL_FILL);
			printf("> using %s mode\n", b_wireframe ? "wireframe" : "solid");
		}
		else if (key == GLFW_KEY_D)
		{
			fc = (fc + 1) % 3;

			GLint uloc = glGetUniformLocation(program, "fc");	if (uloc > -1) glUniform1i(uloc, fc);

			if (fc == 0) printf("> using (texcoord.xy, 0) as color\n");
			else if (fc == 1) printf("> using (texcoord.xxx) as color\n");
			else if (fc == 2) printf("> using (texcoord.yyy) as color\n");
		}
		else if (key == GLFW_KEY_LEFT_CONTROL) b_left_control = true;
		else if (key == GLFW_KEY_LEFT_SHIFT) b_left_shift = true;
		else if (key == GLFW_KEY_HOME)
		{
			cam.view_matrix = mat4::look_at(cam.eye, cam.at, cam.up);
		}
	}
	else if (action == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_LEFT_CONTROL) b_left_control = false;
		else if (key == GLFW_KEY_LEFT_SHIFT) b_left_shift = false;
	}
}

void mouse(GLFWwindow* window, int button, int action, int mods)
{
	if (!b_left_control && !b_left_shift && button == GLFW_MOUSE_BUTTON_LEFT)
	{
		dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
		vec2 npos = cursor_to_ndc(pos, window_size);
		if (action == GLFW_PRESS)			tb.begin(cam.view_matrix, npos, 0);
		else if (action == GLFW_RELEASE)	tb.end(0);
	}
	//panning
	else if ((b_left_control && button == GLFW_MOUSE_BUTTON_LEFT) || button == GLFW_MOUSE_BUTTON_MIDDLE)	
	{
		dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
		vec2 npos = cursor_to_ndc(pos, window_size);
		if (action == GLFW_PRESS) tb.begin(cam.view_matrix, npos, 1);
		else if (action == GLFW_RELEASE) tb.end(1);
	}
	//zooming
	else if ((b_left_shift && button == GLFW_MOUSE_BUTTON_LEFT) || button == GLFW_MOUSE_BUTTON_RIGHT)	
	{
		dvec2 pos; glfwGetCursorPos(window, &pos.x, &pos.y);
		vec2 npos = cursor_to_ndc(pos, window_size);
		if (action == GLFW_PRESS) tb.begin(cam.view_matrix, npos, 2);
		else if (action == GLFW_RELEASE) tb.end(2);
	}
}

void motion(GLFWwindow* window, double x, double y)
{
	// trackball
	if (tb.is_tracking())
	{
		vec2 npos = cursor_to_ndc(dvec2(x, y), window_size);
		cam.view_matrix = tb.update(npos, 0);
	}

	// panning
	if (tb.is_panning())
	{
		vec2 npos = cursor_to_ndc(dvec2(x, y), window_size);
		cam.view_matrix = tb.update(npos, 1);
	}

	// zooming
	if (tb.is_zooming())
	{
		vec2 npos = cursor_to_ndc(dvec2(x, y), window_size);
		cam.view_matrix = tb.update(npos, 2);
	}
}

// this function will be avaialble as cg_create_texture() in other samples
GLuint create_texture(const char* image_path, bool mipmap = true, GLenum wrap = GL_CLAMP_TO_EDGE, GLenum filter = GL_LINEAR)
{
	// load image
	image* i = cg_load_image(image_path); if (!i) return 0; // return null texture; 0 is reserved as a null texture
	int		w = i->width, h = i->height, c = i->channels;

	// induce internal format and format from image
	GLint	internal_format = c == 1 ? GL_R8 : c == 2 ? GL_RG8 : c == 3 ? GL_RGB8 : GL_RGBA8;
	GLenum	format = c == 1 ? GL_RED : c == 2 ? GL_RG : c == 3 ? GL_RGB : GL_RGBA;
	
	// create a src texture (lena texture)
	GLuint texture;
	glGenTextures(1, &texture); if (texture == 0) { printf("%s(): failed in glGenTextures()\n", __func__); return 0; }
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format, GL_UNSIGNED_BYTE, i->ptr);
	if (i) { delete i; i = nullptr; } // release image

	// build mipmap
	if (mipmap)
	{
		int mip_levels = 0; for (int k = w > h ? w : h; k; k >>= 1) mip_levels++;
		for (int l = 1; l < mip_levels; l++)
			glTexImage2D(GL_TEXTURE_2D, l, internal_format, (w >> l) == 0 ? 1 : (w >> l), (h >> l) == 0 ? 1 : (h >> l), 0, format, GL_UNSIGNED_BYTE, nullptr);
		if (glGenerateMipmap) glGenerateMipmap(GL_TEXTURE_2D);
	}

	// set up texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, !mipmap ? filter : filter == GL_LINEAR ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);

	return texture;
}

bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glLineWidth(1.0f);
	glClearColor(39 / 255.0f, 40 / 255.0f, 34 / 255.0f, 1.0f);	// set clear color
	glEnable(GL_CULL_FACE);			// turn on backface culling
	glEnable(GL_DEPTH_TEST);		// turn on depth tests
	glEnable(GL_TEXTURE_2D);		// enable texturing
	glActiveTexture(GL_TEXTURE0);	// notify GL the current texture slot is 0

	unit_sphere_vertices = std::move(create_sphere_vertices());
	update_vertex_buffer(unit_sphere_vertices);

	unit_ring_vertices = std::move(create_ring_vertcies());
	update_ring_vertex_buffer(unit_ring_vertices);

	// load the image to a texture
	PLANETS_TEX[0] = create_texture(sun_image_path, true);		if (!PLANETS_TEX[0]) return false;
	PLANETS_TEX[1] = create_texture(mercury_image_path, true);	if (!PLANETS_TEX[1]) return false;
	PLANETS_TEX[2] = create_texture(venus_image_path, true);	if (!PLANETS_TEX[2]) return false;
	PLANETS_TEX[3] = create_texture(earth_image_path, true);	if (!PLANETS_TEX[3]) return false;
	PLANETS_TEX[4] = create_texture(mars_image_path, true);		if (!PLANETS_TEX[4]) return false;
	PLANETS_TEX[5] = create_texture(jupiter_image_path, true);	if (!PLANETS_TEX[5]) return false;
	PLANETS_TEX[6] = create_texture(saturn_image_path, true);	if (!PLANETS_TEX[6]) return false;
	PLANETS_TEX[7] = create_texture(uranus_image_path, true);	if (!PLANETS_TEX[7]) return false;
	PLANETS_TEX[8] = create_texture(neptune_image_path, true);	if (!PLANETS_TEX[8]) return false;
	PLANETS_TEX[9] = create_texture(moon_image_path, true);		if (!PLANETS_TEX[9]) return false;
	PLANETS_TEX[10] = create_texture(moon_image_path, true);		if (!PLANETS_TEX[10]) return false;
	PLANETS_TEX[11] = create_texture(moon_image_path, true);		if (!PLANETS_TEX[11]) return false;
	PLANETS_TEX[12] = create_texture(moon_image_path, true);		if (!PLANETS_TEX[12]) return false;

	RING_TEX[0] = create_texture(saturn_ring_image_path, true);			if (!RING_TEX[0]) return false;
	RING_TEX[1] = create_texture(saturn_ring_alpha_image_path, true);	if (!RING_TEX[1]) return false;
	RING_TEX[2] = create_texture(uranus_ring_image_path, true);			if (!RING_TEX[2]) return false;
	RING_TEX[3] = create_texture(uranus_ring_alpha_image_path, true);	if (!RING_TEX[3]) return false;

	NORM_TEX_MERCURY = create_texture(mercury_normal_image_path, true);		//index 1
	NORM_TEX_VENUS = create_texture(venus_normal_image_path, true);		//index 2
	NORM_TEX_EARTH = create_texture(earth_normal_image_path, true);		//index 3
	NORM_TEX_MARS = create_texture(mars_normal_image_path, true);			//index 4
	NORM_TEX_MOON = create_texture(moon_normal_image_path, true);			//index 9

	return true;
}

void user_finalize()
{
}

int main(int argc, char* argv[])
{
	// create window and initialize OpenGL extensions
	if (!(window = cg_create_window(window_name, window_size.x, window_size.y))) { glfwTerminate(); return 1; }
	if (!cg_init_extensions(window)) { glfwTerminate(); return 1; }	// version and extensions

	// initializations and validations
	if (!(program = cg_create_program(vert_shader_path, frag_shader_path))) { glfwTerminate(); return 1; }	// create and compile shaders/program
	if (!user_init()) { printf("Failed to user_init()\n"); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback(window, reshape);	// callback for window resizing events
	glfwSetKeyCallback(window, keyboard);			// callback for keyboard events
	glfwSetMouseButtonCallback(window, mouse);	// callback for mouse click inputs
	glfwSetCursorPosCallback(window, motion);		// callback for mouse movement

	// enters rendering/event loop
	for (frame = 0; !glfwWindowShouldClose(window); frame++)
	{
		glfwPollEvents();	// polling and processing of events
		update();			// per-frame update
		render();			// per-frame render
	}

	// normal termination
	user_finalize();
	cg_destroy_window(window);

	return 0;
}
