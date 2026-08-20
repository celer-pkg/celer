#version 440

layout(location = 0) in vec2 v_texcoord;

layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform texture2D sepTex[2];
layout(binding = 2) uniform sampler sepSampler;

void main()
{
    fragColor = texture(sampler2D(sepTex[0], sepSampler), v_texcoord);
    fragColor *= texture(sampler2D(sepTex[1], sepSampler), v_texcoord);
}
