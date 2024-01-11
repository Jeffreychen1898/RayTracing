#version 410

in vec2 v_texCoord;

uniform sampler2D u_accumTexture;
uniform float u_frameCount;

out vec4 FragColor;

void main()
{
	vec4 texel = texture(u_accumTexture, v_texCoord);
	FragColor = vec4(texel.rgb / vec3(u_frameCount), 1.0);
}
