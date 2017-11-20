#version 450

#include <mvp.glsl>

layout(binding=0) uniform MVP
{
	ModelViewProjection mvp;
};

layout(location=0) in vec4 input_position;

void main()
{
	gl_Position = mvp.model_view_projection * input_position;
}