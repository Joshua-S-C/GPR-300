#version 450

out vec4 FragColor;

in vec2 UV;

uniform sampler2D _DepthMap;
uniform float _NearPlane;
uniform float _FarPlane;

// From LearnOpenGL
// required when using a perspective projection matrix
// TODO When using this make sure to fix it
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * _NearPlane * _FarPlane) / (_FarPlane + _NearPlane - z * (_FarPlane - _NearPlane));	
}

void main() {
	float depth = texture(_DepthMap, UV).r;
	FragColor = vec4(vec3(depth), 1.0);
}