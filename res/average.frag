#version 410

in vec2 v_texCoord;

uniform sampler2D u_accumTexture;

layout(location = 0) out vec4 FragColor;

void main()
{
	vec4 texel = texture(u_accumTexture, v_texCoord);
	FragColor = vec4(texel.rgb / texel.a, 1.0);
}
