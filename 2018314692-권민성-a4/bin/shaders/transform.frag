#ifdef GL_ES
	#ifndef GL_FRAGMENT_PRECISION_HIGH	// highp may not be defined
		#define highp mediump
	#endif
	precision highp float; // default precision needs to be defined
#endif

// input from vertex shader
in vec4 epos;
in vec3 norm;
in vec2 tc;


// the only output variable
out vec4 fragColor;

// uniform variables
uniform mat4	model_matrix;
uniform mat4	view_matrix;
uniform float	shininess;
uniform vec4	light_position, Ia, Id, Is;	//light
uniform vec4	Ka, Kd, Ks;					// material properties

uniform sampler2D TEX;	// texture sampler object
uniform sampler2D TEX1;	// second texture sampler object (ring)
uniform sampler2D TEX2; // third texture sampler object (alpha)
uniform sampler2D NORM;	// normal map
uniform int idx;

vec4 phong( vec3 l, vec3 n, vec3 h, vec4 Kd )
{
	vec4 Ira = Ka*Ia;									// ambient reflection
	vec4 Ird = max(Kd*dot(l,n)*Id,0.0);					// diffuse reflection
	vec4 Irs = max(Ks*pow(dot(h,n),shininess)*Is,0.0);	// specular reflection
	return Ira + Ird + Irs;
}

void main()
{
	// light position in the eye space
	vec4 lpos = view_matrix*light_position;
	vec3 n = normalize(norm);	// norm interpolated via rasterizer should be normalized again here
	vec3 p = epos.xyz;			// 3D position of this fragment
	vec3 l = normalize(lpos.xyz-(lpos.a==0.0?vec3(0):p));	// lpos.a==0 means directional light
	vec3 v = normalize(-p);		// eye-epos = vec3(0)-epos
	vec3 h = normalize(l+v);	// the halfway vector
	vec4 iKd = texture( TEX, tc );	// Kd from image
	

	if (idx == 1 || idx == 2 || idx == 3 || idx == 4 || idx == 9)	// normal mapping
	{
		vec3 tnormal = texture( NORM, tc ).xyz;
		tnormal = normalize(tnormal-0.5);
		
		vec3 c1 = cross(norm,vec3(0,0,1));
		vec3 c2 = cross(norm,vec3(0,1,0));
		vec3 tangent = normalize(length(c1)>length(c2)?c1:c2);
		vec3 binormal = cross(norm, tangent);
		mat3 tbn = mat3( tangent, binormal, norm );
		vec3 world_space_bumped_normal = tbn * tnormal;

		n = normalize(world_space_bumped_normal);
		fragColor = phong( l, n, h, iKd );
	}
	else if(idx==-1){	// ring
		vec4 iKd_ring = texture(TEX1, tc);
		fragColor=phong(l, n, h, iKd_ring);
		fragColor.a=texture(TEX2, tc).x;
	}
	else if(idx==0)	fragColor = iKd;	// Sun
	else			fragColor = phong( l, n, h, iKd );
}