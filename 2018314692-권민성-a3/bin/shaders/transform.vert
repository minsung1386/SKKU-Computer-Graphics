// vertex attributes
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;

// matrices
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

//in vec2 texcoord;
out vec3 norm;
out vec2 tc;

void main()
{
	// transform the vertex position by model matrix
	vec4 wpos = model_matrix *vec4(position, 1.0);
	// transform the position to the eye-space position
	vec4 epos = view_matrix * wpos;
	// project the eye-space position to the canonical view volume
	gl_Position = projection_matrix * epos;

	// pass normal vector to fragment shader
	norm = normal;
	tc=texcoord;
}