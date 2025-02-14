#version 450

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D _DepthMap;

void main() {
	float depth = texture(_DepthMap, TexCoords).r;
	FragColor = vec4(vec3(depth), 1.0);
}