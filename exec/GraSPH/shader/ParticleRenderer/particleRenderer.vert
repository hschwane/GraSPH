#version 450

#include "common.glsl"

layout(location=RENDERER_POSITION_ARRAY) in vec4 input_position;

uniform mat4 model_view_projection;
uniform mat4 projection;
uniform vec2 viewport_size;
uniform float render_size;
uniform vec4 color;

out vec2 center;
out float radius;
flat out vec4 particle_color;

void main()
{
	gl_Position = model_view_projection * input_position;

    float size;
    size = render_size;

#ifdef PARTICLES_PERSPECTIVE
	gl_PointSize = viewport_size.y * projection[1][1] * size / gl_Position.w;
#else
    gl_PointSize = size;
#endif

    if(gl_VertexID >= 8192)
        particle_color = vec4(0.2,0.4,0.9,1);
    else
        particle_color = color;

	center = (0.5 * gl_Position.xy/gl_Position.w +0.5) * viewport_size;
	radius = gl_PointSize / 2;
}