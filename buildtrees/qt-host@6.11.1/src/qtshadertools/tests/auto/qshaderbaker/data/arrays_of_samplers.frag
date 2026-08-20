#version 440

layout(location = 0) in vec2 v_texcoord;

layout(location = 0) out vec4 fragColor;

layout(binding = 4) uniform sampler2D tex1;
layout(binding = 8) uniform sampler2D shadowMaps[4]; // array of textures
layout(binding = 9) uniform samplerCube shadowCubes[4]; // array of cubemap textures
layout(binding = 10) uniform sampler2DArray texArr; // texture array

void main()
{
    fragColor = texture(tex1, v_texcoord) * texture(texArr, vec3(v_texcoord, 0.0))
    * texture(shadowMaps[0], v_texcoord)
    * texture(shadowMaps[1], v_texcoord)
    * texture(shadowMaps[2], v_texcoord)
    * texture(shadowMaps[3], v_texcoord)
    * texture(shadowCubes[0], vec3(v_texcoord, 0.0))
    * texture(shadowCubes[1], vec3(v_texcoord, 0.0))
    * texture(shadowCubes[2], vec3(v_texcoord, 0.0))
    * texture(shadowCubes[3], vec3(v_texcoord, 0.0));
}
