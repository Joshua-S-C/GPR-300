#version 450

out vec4 FragColor;

in Surface{
	vec2 UV;
}fs_in;

uniform sampler2D _ScreenTexture;

void main()
{
    FragColor = vec4(texture(_ScreenTexture, fs_in.UV).rgb, 1.0);
} 