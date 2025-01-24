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

uniform Material _Material;

uniform sampler2D _MainTex;
uniform sampler2D _NormalMap;

uniform vec3 _EyePos;
uniform vec3 _LightDirection /* = vec3(-0.5,-1.0,0.0)*/;
uniform vec3 _LightColor = vec3(1.0);
uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

// Use Tangent space Normal from Bump Map
vec3 calcNormalFromMap()
{
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

void main(){
	//vec3 normal = normalize(fs_in.WorldNormal);
	//vec3 normal = texture(_NormalMap, fs_in.TexCoord).rgb;
	//normal = normalize(normal * 2.0 - 1.0);
	
	vec3 normal = calcNormalFromMap();
	//FragColor = vec4(normal, 1.0);
	//return;

	vec3 toLight = -_LightDirection;
	vec3 toEye = normalize(_EyePos - fs_in.WorldPos);
	vec3 h = normalize(toLight + toEye);

	vec3 ambientClr = _AmbientColor * _Material.aK;
	float diffuse = _Material.dK * max(dot(normal,toLight),0.0);
	float specular = _Material.sK * pow(max(dot(normal,h),0.0), _Material.shininess);

	vec3 lightColor = (diffuse +  specular) * _LightColor + ambientClr;
	vec3 objectColor = texture(_MainTex,fs_in.TexCoord).rgb;

	FragColor = vec4(objectColor * lightColor,1.0);
}
