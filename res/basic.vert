#version 410 core

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec3 a_color;

uniform mat4 u_projection;

out vec3 v_color;

void main()
{
	gl_Position = u_projection * vec4(a_position, 0.0, 1.0);
	v_color = a_color;
}
