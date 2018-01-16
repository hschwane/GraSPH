#version 450

#include "common.glsl"

layout(location=RENDERER_POSITION_ARRAY) in vec4 input_position;
layout(location=RENDERER_MASS_ARRAY) in float mass;

uniform mat4 model_view_projection;
uniform mat4 projection;
uniform vec2 viewport_size;
uniform float render_size;

out vec2 center;
out float radius;

void main()
{
	gl_Position = model_view_projection * input_position;

#ifdef PARTICLES_PERSPECTIVE
	gl_PointSize = viewport_size.y * projection[1][1] * render_size / gl_Position.w;
#else
    gl_PointSize = render_size;
#endif

    if(mass > 30.0/16000)
         gl_PointSize = 8;

	center = (0.5 * gl_Position.xy/gl_Position.w +0.5) * viewport_size;
	radius = gl_PointSize / 2;
}