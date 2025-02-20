#version 450

layout(location = 0) in vec3 vPos;
layout (location = 1) in vec2 vTexCoords;

out vec2 UV;

uniform mat4 _LightSpaceMatrix;
uniform mat4 _Model;

// From Eric. For dummy vao
vec4 verts[3] = {
    vec4(-1.0, -1.0, 0.0 ,0.0),	// Bottom left
	vec4(3.0, -1.0, 2.0, 0.0),		// Bottom right
	vec4(-1.0, 3.0, 0.0, 2.0)		// Top left
};

void main() {
    UV = vTexCoords;
    gl_Position = vec4(vPos, 1.0);
}