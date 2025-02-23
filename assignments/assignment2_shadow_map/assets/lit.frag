#version 450

out vec4 FragColor;

in Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
	vec3 Tangent;
	vec4 FragPosLightSpace;
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
layout(binding = 2) uniform sampler2D _ShadowMap;

uniform Material _Material;
uniform Light _Light;
uniform DirectionalLight _DirectionalLight;

uniform vec3 _EyePos;

uniform bool _UseAmbientClr = false;
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

uniform bool _UseNormalMap;
uniform bool _UseDirectionalLight;
uniform int _RenderType;

uniform float _MinShadowBiasK;
uniform float _MaxShadowBiasK;

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

// Not In Shadow [0 - 1] In Shadow
float calcShadow(vec4 FragPosLightSpace, float shadowBias) {
	//return 0;

	// Frag Light Space pos in [-1,1]
    vec3 projCoords = FragPosLightSpace.xyz / FragPosLightSpace.w;

	// Remap to [0,1]
	projCoords = projCoords * 0.5 + 0.5; 

	float closestDepth = texture(_ShadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	//float shadow = currentDepth - shadowBias > closestDepth  ? 1.0 : 0.0;

	// PCF
	float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(_ShadowMap, 0);

    for (int x = -1; x <= 1; x++)
    {
        for(int y = -1; y <= 1; y++)
        {
            float pcfDepth = texture(_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - shadowBias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }

    shadow /= 9.0;

	// Check this
	if(projCoords.z > 1.0)
        shadow = 0.0;
	
	return 1.0 - shadow;
}

float calcShadowBias(vec3 normal, vec3 lightDir) {
	return max(_MaxShadowBiasK * (1.0 - dot(normal, lightDir)), _MinShadowBiasK);
}

void main() {	
	// Normals
	vec3 normal;

	if (_UseNormalMap)
		normal = calcNormalFromMap();
	else
		normal = normalize(fs_in.WorldNormal);

	switch (_RenderType) {
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
	vec3 ambientClr = _Material.aK * (_UseAmbientClr ? _AmbientColor : _Light.clr);
	float diffuse = _Material.dK * max(dot(normal,lightDir),0.0);
	float specular = _Material.sK * pow(max(dot(normal,h),0.0), _Material.shininess);

	// Shadow
	float shadow = calcShadow(fs_in.FragPosLightSpace, calcShadowBias(normal, lightDir));

	// Combine Lighting
	//vec3 lightColor = 
	//	((diffuse + specular) * _Light.clr) +
	//	(ambientClr + (-shadow));

	//ambientClr += -shadow;
	//diffuse += -shadow;
	//specular += -shadow;

	vec3 lightColor = ((diffuse +  specular) * _Light.clr) * shadow + ambientClr;
	//lightColor = lightColor * (1.0 - shadow);

	vec3 objectColor = texture(_MainTex,fs_in.TexCoord).rgb;

	FragColor = vec4(objectColor * lightColor,1.0);
}
