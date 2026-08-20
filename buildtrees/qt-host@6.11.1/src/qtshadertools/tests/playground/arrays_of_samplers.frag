#version 440

layout(location = 0) in vec2 v_texcoord;

layout(location = 0) out vec4 fragColor;

layout(binding = 4) uniform sampler2D tex1;
layout(binding = 8) uniform sampler2D shadowMaps[4];
layout(binding = 9) uniform samplerCube shadowCubes[4];
layout(binding = 10) uniform sampler2D tex2;

void main()
{
    fragColor = texture(tex1, v_texcoord) * texture(tex2, v_texcoord)
    * texture(shadowMaps[0], v_texcoord)
    * texture(shadowMaps[1], v_texcoord)
    * texture(shadowMaps[2], v_texcoord)
    * texture(shadowMaps[3], v_texcoord)
    * texture(shadowCubes[0], vec3(v_texcoord, 0.0))
    * texture(shadowCubes[1], vec3(v_texcoord, 0.0))
    * texture(shadowCubes[2], vec3(v_texcoord, 0.0))
    * texture(shadowCubes[3], vec3(v_texcoord, 0.0));
}
