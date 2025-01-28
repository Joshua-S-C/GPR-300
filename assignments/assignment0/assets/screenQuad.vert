#version 450


void main() {
	vec2 pos = vec2(gl_VertexID % 2, gl_VertexID / 2) * 4.0 - 1;
	vec2 texCoord = (pos + 1) * 0.5;
}