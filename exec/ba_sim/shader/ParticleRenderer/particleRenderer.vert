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
flat out int isSink;

void main()
{
	gl_Position = model_view_projection * input_position;

    float size;
    if(mass > 20.0/16000)
    {
         size = render_size* 5;
        isSink =1;
    } else {
    isSink =0;
    size = render_size;
    }

#ifdef PARTICLES_PERSPECTIVE
	gl_PointSize = viewport_size.y * projection[1][1] * size / gl_Position.w;
#else
    gl_PointSize = size;
#endif

	center = (0.5 * gl_Position.xy/gl_Position.w +0.5) * viewport_size;
	radius = gl_PointSize / 2;
}