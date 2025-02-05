#version 450

out vec4 FragColor;

in Surface{
	vec2 UV;
}fs_in;


uniform sampler2D _ScreenTexture;

uniform bool _Active;
uniform int _KernelSize;
uniform float _Sigma;

// The equation
// https://en.wikipedia.org/wiki/Gaussian_blur
float gaussian(float sigma, float pos) {
    float PI = 3.141592;
    return (1.0f / sqrt(2.0f * PI * sigma * sigma)) * exp(-(pos * pos) / (2.0f * sigma * sigma));
}


void main()
{
    if(_Active) {
        vec2 texelSize = 1.0 / textureSize(_ScreenTexture, 0).xy;
        vec3 total = vec3(0,0,0);
        float sum = 0;

        // Horizontal
        for (int i = -_KernelSize; i <= _KernelSize; i++) {
            vec4 col = texture(_ScreenTexture, fs_in.UV + vec2(i,0)  * texelSize);
            float gauss = gaussian(_Sigma, i);

            total += col.rbg * gauss;
            sum += gauss;
        }
        total / sum;
    
        sum = 0;
        // Vertical
        for (int i = -_KernelSize; i <= _KernelSize; i++) {
            vec4 col = texture(_ScreenTexture, fs_in.UV + vec2(0,i)  * texelSize);
            float gauss = gaussian(_Sigma, i);

            total += col.rbg * gauss;
            sum += gauss;
        }
        total / sum;

        total /= (_KernelSize * _KernelSize);

        FragColor = vec4(total, 1.0);
    } else {
        vec3 col = texture(_ScreenTexture, fs_in.UV).rgb;
        FragColor = vec4(col, 1.0);
    }
} 