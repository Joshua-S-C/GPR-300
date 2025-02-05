#version 450

out vec4 FragColor;

in Surface{
	vec2 UV;
}fs_in;


uniform sampler2D _ScreenTexture;

uniform vec3 _TintColour;
uniform float _TintStrength;

void main()
{
    vec3 col = texture(_ScreenTexture, fs_in.UV).rgb;
    col = col * (1 - _TintStrength);
    vec3 tint = _TintColour * _TintStrength;

    FragColor = vec4(col + tint , 1.0);
} 