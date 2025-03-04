#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;

out Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
	vec3 Tangent;
	vec4 FragPosLightSpace;
}vs_out;

uniform mat4 _Model;			// Model->World Matrix
uniform mat4 _ViewProjection;	// Combined View->Projection Matrix
uniform mat4 _LightSpaceMatrix;	//

void main(){
	vs_out.WorldPos = vec3(_Model * vec4(vPos, 1.0));
	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;
	vs_out.TexCoord = vTexCoord;
	vs_out.Tangent = (_Model * vec4(vTangent, 0.0)).xyz;
	vs_out.FragPosLightSpace = _LightSpaceMatrix * vec4(vs_out.WorldPos, 1.0);

	//Transform vertex position to homogeneous clip space
	gl_Position = _ViewProjection * _Model * vec4(vPos,1.0);
}
