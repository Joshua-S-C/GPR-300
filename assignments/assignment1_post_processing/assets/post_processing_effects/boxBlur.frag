#version 450

out vec4 FragColor;

in Surface{
	vec2 UV;
}fs_in;

uniform sampler2D _ScreenTexture;

uniform bool _Active;
uniform int _KernelSize;

void main()
{
    // Returning early doesn't work
    if(_Active) {
        vec2 texelSize = 1.0 / textureSize(_ScreenTexture, 0).xy;
        vec3 totalCol = vec3(0,0,0);
        int offset = _KernelSize / 2;

        for(int y = -offset; y <= offset; y++){
            for(int x = -offset; x <= offset; x++){
                vec2 offset = vec2(x,y) * texelSize;
                totalCol += texture(_ScreenTexture, fs_in.UV + offset).rgb;
            }
        }

        totalCol /= (_KernelSize * _KernelSize);
        FragColor = vec4(totalCol, 1.0);
    } else {
        vec3 col = texture(_ScreenTexture, fs_in.UV).rgb;
        FragColor = vec4(col, 1.0);
    }
}