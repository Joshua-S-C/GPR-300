#version 450

out vec4 FragColor;

in Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
	vec3 Tangent;
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
layout(binding = 1) uniform sampler2D _NormalMap;

uniform Material _Material;
uniform Light _Light;
uniform DirectionalLight _DirectionalLight;

uniform vec3 _EyePos;
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

uniform bool _UseNormalMap;
uniform bool _UseDirectionalLight;
uniform int _RenderType;

// Use angent space Normal from Bump Map
vec3 calcNormalFromMap() {
    vec3 normal = normalize(fs_in.WorldNormal);
    vec3 tangent = normalize(fs_in.Tangent);
    tangent = normalize(tangent - dot(tangent, normal) * normal);

    vec3 biTangent = cross(tangent, normal);
    vec3 bumpMapNormal = texture(_NormalMap, fs_in.TexCoord).xyz;
    bumpMapNormal = 2.0 * bumpMapNormal - vec3(1.0, 1.0, 1.0);

    mat3 TBN = mat3(tangent, biTangent, normal);

    vec3 newNormal = normalize(TBN * bumpMapNormal);
    return newNormal;
}

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

	if (_UseNormalMap)
		normal = calcNormalFromMap();
	else
		normal = normalize(fs_in.WorldNormal);

	switch (_RenderType){
		// Use Texture
		default:
			break;

		// Unlit
		case 1:
			FragColor = vec4(texture(_MainTex,fs_in.TexCoord).rgb, 1.0);
			return;

		// Show World Normals
		case 2:
			FragColor = vec4(normal, 1.0);
			return;

		// Show Normal Map
		case 3:
			FragColor = vec4(texture(_NormalMap, fs_in.TexCoord).rgb, 1.0);
			return;
	}

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
