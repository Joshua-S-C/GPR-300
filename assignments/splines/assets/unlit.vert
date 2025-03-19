#version 450

layout(location = 0) in vec3 vPos;

//uniform mat4 _Model;			// Model->World Matrix
uniform mat4 _ViewProjection;	// Combined View->Projection Matrix

void main(){
	gl_Position = _ViewProjection * vec4(vPos,1.0);
}
