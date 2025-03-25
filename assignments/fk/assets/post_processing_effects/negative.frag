#version 450

out vec4 FragColor;

in Surface{
	vec2 UV;
}fs_in;


uniform sampler2D _ScreenTexture;

uniform bool _Active;

void main()
{
    // TODO Does this make sense to even exist (prolly not?)

    vec3 col = texture(_ScreenTexture, fs_in.UV).rgb;

    if (_Active)
        col = 1.0 - col;

    FragColor = vec4(col, 1.0);
} 