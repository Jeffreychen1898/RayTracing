#version 410 core

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_coord;

uniform mat4 u_projection;

out vec2 v_coord;
out vec2 v_screenPos;

void main()
{
	gl_Position = u_projection * vec4(a_position, 0.0, 1.0);
	v_coord = a_coord;
	v_screenPos = vec2(a_position.x, 600.f - a_position.y);
}
