#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;

uniform mat4 _Model; //Model->World Matrix
uniform mat4 _ViewProjection; //Combined View->Projection Matrix

out vec3 Normal;

void main(){
	Normal = vNormal;
	//Transform vertex position to homogeneous clip space
	gl_Position = _ViewProjection * _Model * vec4(vPos,1.0);
}
