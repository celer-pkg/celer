#version 440

layout(location = 0) in vec2 v_texcoord;

layout(location = 0) out vec4 fragColor;

layout(binding = 4) uniform sampler2D tex1;
layout(binding = 8) uniform sampler2DArray shadowMaps;
layout(binding = 9) uniform samplerCubeArray shadowCubes;
layout(binding = 10) uniform sampler2D tex2;

void main()
{
    fragColor = texture(tex1, v_texcoord) * texture(tex2, v_texcoord)
    * texture(shadowMaps, vec3(v_texcoord, 0.0))
    * texture(shadowMaps, vec3(v_texcoord, 1.0))
    * texture(shadowMaps, vec3(v_texcoord, 2.0))
    * texture(shadowMaps, vec3(v_texcoord, 3.0))
    * texture(shadowCubes, vec4(v_texcoord, 0.0, 0.0))
    * texture(shadowCubes, vec4(v_texcoord, 0.0, 1.0))
    * texture(shadowCubes, vec4(v_texcoord, 0.0, 2.0))
    * texture(shadowCubes, vec4(v_texcoord, 0.0, 3.0));
}
