#ifdef GL_ES
	#ifndef GL_FRAGMENT_PRECISION_HIGH	// highp may not be defined
		#define highp mediump
	#endif
	precision highp float; // default precision needs to be defined
#endif


// input from vertex shader
in vec2 tc;

uniform int fc;

// the only output variable
out vec4 fragColor;

void main()
{
	if(fc==0) fragColor = vec4(tc.xy, 0, 1);
	else if(fc==1) fragColor = vec4(tc.xxx, 1);
	else if(fc==2) fragColor = vec4(tc.yyy, 1);
}
