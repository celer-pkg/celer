#version 440

layout(location = 0) in vec2 v_texcoord;
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform sampler2D tex;

void main()
{
    vec2 size = vec2(textureSize(tex, 0));
    vec2 d = vec2(1.0 / size.x, 1.0 / size.y);
    fragColor = texture(tex, v_texcoord + d);
}
