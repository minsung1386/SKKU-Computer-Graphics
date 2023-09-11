#include "cgmath.h"		// slee's simple math library
#include "cgut.h"		// slee's OpenGL utility

//*************************************
// global constants
static const char*	window_name = "Assignment 2: Planet in Space - Minsung Kwon 2018314692";
static const char*	vert_shader_path = "shaders/transform.vert";
static const char*	frag_shader_path = "shaders/transform.frag";

//*************************************
// common structures
struct camera
{
	vec3	eye = vec3( 5, 0, 0 );
	vec3	at = vec3( 0, 0, 0 );
	vec3	up = vec3( 0, 0, 1 );
	mat4	view_matrix = mat4::look_at( eye, at, up );
		
	float	fovy = PI/4.0f; // must be in radian
	float	aspect;
	float	dnear = 1.0f;
	float	dfar = 1000.0f;
	mat4	projection_matrix;
};

//*************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = cg_default_window_size(); // initial window size

//*************************************
// OpenGL objects
GLuint	program	= 0;	// ID holder for GPU program
GLuint	vertex_array = 0;	// ID holder for vertex array object

//*************************************
// global variables
int		frame = 0;		// index of rendering frames
float	t, theta, pause_theta=0.0f;
bool	rotate_flag = true;
bool	b_wireframe = false;
GLint fc = 0;

//*************************************
// scene objects
camera	cam;

std::vector<vertex> sphere_vertices;
//*************************************
// holder of vertices and indices of a unit sphere
std::vector<vertex>	unit_sphere_vertices;	// host-side vertices

//*************************************
void update()
{
	float aspect = window_size.x / float(window_size.y);
	mat4 aspect_matrix = mat4::scale(std::min(1 / aspect, 1.0f), std::min(aspect, 1.0f), 1.0f);
	mat4 view_projection_matrix = aspect_matrix * mat4{ 0,1,0,0,0,0,1,0,-1,0,0,1,0,0,0,1 };
	
	// update projection matrix
	cam.aspect = window_size.x/float(window_size.y);
	cam.projection_matrix = mat4::perspective( cam.fovy, cam.aspect, cam.dnear, cam.dfar );

	// update uniform variables in vertex/fragment shaders
	GLint uloc;
	uloc = glGetUniformLocation( program, "view_matrix" );			if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, cam.view_matrix );		// update the view matrix (covered later in viewing lecture)
	uloc = glGetUniformLocation( program, "projection_matrix" );	if(uloc>-1) glUniformMatrix4fv( uloc, 1, GL_TRUE, cam.projection_matrix );	// update the projection matrix (covered later in viewing lecture)
	uloc = glGetUniformLocation(program, "view_projection_matrix");
	glUniformMatrix4fv(uloc, 1, GL_TRUE, view_projection_matrix);

}

void render()
{
	// clear screen (with background color) and clear depth buffer
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	// notify GL that we use our own program
	glUseProgram( program );
	
	// bind vertex array object
	glBindVertexArray( vertex_array );

	// render vertices: trigger shader programs to process vertex data

	// configure transformation parameters
	t = rotate_flag?float(glfwGetTime()):t;
	theta = t * 0.5f;

	// build the model matrix
	mat4 model_matrix = /*mat4::translate(-1.0f, 0.0f, 0.0f)**/
						mat4::translate( cam.at ) *
						mat4::rotate( vec3(0,0,1), theta ) *
						mat4::translate( -cam.at );

	// update the uniform model matrix and render
	GLint uloc = glGetUniformLocation(program, "model_matrix");
	if(uloc> -1) glUniformMatrix4fv( uloc, 1, GL_TRUE, model_matrix );

	glDrawElements( GL_TRIANGLES, 72*35*2*3, GL_UNSIGNED_INT, nullptr );
	

	// swap front and back buffers, and display to screen
	glfwSwapBuffers( window );
}

void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	glViewport( 0, 0, width, height );
}

void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
	printf("- press 'w' to toggle wireframe\n");
	printf("- press 'd' to toggle (tc.xy,0) > (tc.xxx) > (tc.yyy)\n");
	printf("- press 'r' to rotate the sphere\n");
	printf( "\n" );
}

std::vector<vertex> create_sphere_vertices(uint rad=1)
{
	// 72 edges in longitude & 36 edges in latitude
	std::vector<vertex> v;
	for (uint i = 0; i < 36; i++)
	{
		for (uint j = 0; j <= 72; j++)
		{
			float theta = PI * i / float(36);
			float pi = PI * 2.0f * j / float(72);
			vec3 pos = vec3(float(rad) * sin(theta) * cos(pi), float(rad) * sin(theta) * sin(pi), float(rad) * cos(theta));
			vec3 norm = vec3(sin(theta) * cos(pi), sin(theta) * sin(pi), cos(theta));
			vec2 tc = vec2(pi / PI * 2.0f, 1.0f - theta / PI);
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
	for (uint i = 0; i < 35; i++) 
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
	if (!vertex_array) { printf("%s(): failed to create vertex aray\n", __func__); return; }

}
void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		else if (key == GLFW_KEY_R)
		{
			rotate_flag = !rotate_flag;
			if (rotate_flag) glfwSetTime(pause_theta);
			else pause_theta = t;
			
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
	}
}

void mouse( GLFWwindow* window, int button, int action, int mods )
{
	if(button==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS )
	{
		dvec2 pos; glfwGetCursorPos(window,&pos.x,&pos.y);
		printf( "> Left mouse button pressed at (%d, %d)\n", int(pos.x), int(pos.y) );
	}
}

void motion( GLFWwindow* window, double x, double y )
{
}

bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glLineWidth(1.0f);
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// turn on backface culling
	glEnable( GL_DEPTH_TEST );								// turn on depth tests

	GLint uloc = glGetUniformLocation(program, "fc");
	//glUseProgram(program);
	glUniform1i(uloc, fc);

	unit_sphere_vertices = std::move(create_sphere_vertices(1));

	update_vertex_buffer(unit_sphere_vertices);
	
	return true;
}

void user_finalize()
{
}

int main( int argc, char* argv[] )
{
	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return 1; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return 1; }	// version and extensions

	// initializations and validations
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return 1; }	// create and compile shaders/program
	if(!user_init()){ printf( "Failed to user_init()\n" ); glfwTerminate(); return 1; }					// user initialization

	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movement

	// enters rendering/event loop
	for( frame=0; !glfwWindowShouldClose(window); frame++ )
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
