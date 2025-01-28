#version 450

out vec4 FragColor;

in Surface{
	vec2 TexCoord;
}fs_in;

uniform sampler2D _ScreenTexture;

void main()
{ 
    FragColor = texture(_ScreenTexture, fs_in.TexCoord);
}