#version 450

#include "common.glsl"

layout(location=RENDERER_POSITION_ARRAY) in vec4 input_position;
layout(location=RENDERER_SIZE_ARRAY) in float input_size;

uniform mat4 model_view_projection;
uniform mat4 projection;
uniform vec2 viewport_size;

out vec2 center;
out float radius;

void main()
{
	gl_Position = model_view_projection * input_position;
	gl_PointSize = viewport_size.y * projection[1][1] * input_size / gl_Position.w;
	center = (0.5 * gl_Position.xy/gl_Position.w +0.5) * viewport_size;
	radius = gl_PointSize / 2;
}