#version 450

out vec4 FragColor;

//layout(location = 0) out float FragDepth;

in Surface{
	vec2 UV;
}fs_in;

in float FragDepth;

uniform sampler2D _ScreenTexture;
//uniform sampler2D _DepthTexture;



void main()
{
    vec3 col = texture(_ScreenTexture, fs_in.UV).rgb;
    //vec3 col = texture(_DepthTexture, fs_in.UV).rgb;

    //float FragDepth = gl_FragCoord.z;

    //FragColor = vec4(col, 1.0);
    FragColor = vec4(FragDepth * col.x, FragDepth, FragDepth, 1.0);
} 