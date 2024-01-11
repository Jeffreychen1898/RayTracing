#version 410 core

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_rayCoord;

out vec2 v_texCoord;
out vec2 v_rayCoord;

void main()
{
	gl_Position = vec4(a_position, 0.0, 1.0);
	v_texCoord = a_position * vec2(0.5) + vec2(0.5);
	v_rayCoord = a_rayCoord;
}
