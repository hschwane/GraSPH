#pragma once

struct ModelViewProjection
{
	mat4 model;
	mat4 view;
	mat4 projection;

	mat4 normal_normal;
	mat4 model_view;
	mat4 model_view_normal;
	mat4 view_projection;
	mat4 model_view_projection;
};