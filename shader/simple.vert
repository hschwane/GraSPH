#version 450

#include <mvp.glsl>

uniform mat4 model_view_projection;

layout(location=0) in vec4 input_position;

void main()
{
	gl_Position = model_view_projection * input_position;
}