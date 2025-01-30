#version 450

layout (location = 0) in vec2 vPos;
layout (location = 1) in vec2 vTexCoords;

out Surface{
	vec2 UV;
}vs_out;

// From Eric. For dummy vao
vec4 verts[3] = {
    vec4(-1.0, -1.0, 0.0 ,0.0 ),	// Bottom left
	vec4(3.0, -1.0, 2.0, 0.0),		// Bottom right
	vec4(-1.0, 3.0, 0.0, 2.0)		// Top left
};


void main()
{
    vs_out.UV = verts[gl_VertexID].zw;
    gl_Position = vec4(verts[gl_VertexID].xy, 0, 1);
}  