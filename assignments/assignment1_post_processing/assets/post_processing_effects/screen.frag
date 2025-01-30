#version 450

out vec4 FragColor;

in Surface{
	vec2 UV;
}fs_in;


uniform sampler2D _ScreenTexture;

void main()
{
    vec3 col = 1.0 - texture(_ScreenTexture, fs_in.UV).rgb;
    FragColor = vec4(col, 1.0);
} 