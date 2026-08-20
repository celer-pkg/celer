#version 440

layout(location = 0) in vec2 v_texcoord;
layout(location = 0) out vec4 fragColor;

// must be compiled with -DMAKE_IT_WORK -DYES_REALLY=99

#ifdef MAKE_IT_WORK
#if YES_REALLY > 98
layout(binding = 1) uniform sampler2D tex;
#endif
#endif

void main()
{
    fragColor = texture(tex, v_texcoord);
}
