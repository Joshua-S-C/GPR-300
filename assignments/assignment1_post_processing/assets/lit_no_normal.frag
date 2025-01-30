#version 450

out vec4 FragColor;

in Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}fs_in;

struct Material{
	// K = coefficient (0-1 range)
	float aK;
	float dK;
	float sK;
	float shininess;
};

// I just threw this in here
struct Light {
	vec3 pos; // World Space
	vec3 clr; // RBG
};

struct DirectionalLight {
	vec3 dir;
};

layout(binding = 0) uniform sampler2D _MainTex;

uniform Material _Material;
uniform Light _Light;
uniform DirectionalLight _DirectionalLight;

uniform vec3 _EyePos;
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

uniform bool _UseNormalMap;
uniform bool _UseDirectionalLight;

// Change this is using other light types
vec3 calcLightDir() {
	if (_UseDirectionalLight)
		return _DirectionalLight.dir;
	else
		return normalize(_Light.pos - fs_in.WorldPos);
}

void main() {	
	// Normals
	vec3 normal;
	normal = normalize(fs_in.WorldNormal);

	// Light Setup
	vec3 lightDir = calcLightDir();
	vec3 toEye = normalize(_EyePos - fs_in.WorldPos);
	vec3 h = normalize(lightDir + toEye);

	// Phong
	vec3 ambientClr = _AmbientColor * _Material.aK;
	float diffuse = _Material.dK * max(dot(normal,lightDir),0.0);
	float specular = _Material.sK * pow(max(dot(normal,h),0.0), _Material.shininess);

	vec3 lightColor = (diffuse +  specular) * _Light.clr + ambientClr;
	vec3 objectColor = texture(_MainTex,fs_in.TexCoord).rgb;

	FragColor = vec4(objectColor * lightColor,1.0);
}
