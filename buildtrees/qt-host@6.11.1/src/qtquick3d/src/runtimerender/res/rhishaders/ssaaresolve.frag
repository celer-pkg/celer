#version 440

layout(location = 0) out vec4 fragOutput;

layout(location = 0) in vec2 uv_coord;
#if QSHADER_VIEW_COUNT >= 2
layout(location = 1) flat in uint v_viewIndex;
#endif

#if QSHADER_VIEW_COUNT >= 2
layout(binding = 0) uniform sampler2DArray tex;
#else
layout(binding = 0) uniform sampler2D tex;
#endif

void main()
{
#if QSHADER_HLSL || QSHADER_MSL
    vec2 uv = vec2(uv_coord.x, 1.0 - uv_coord.y);
#if QSHADER_VIEW_COUNT >= 2
    fragOutput = texture(tex, vec3(uv, v_viewIndex));
#else
    fragOutput = texture(tex, uv);
#endif
#else
#if QSHADER_VIEW_COUNT >= 2
    fragOutput = texture(tex, vec3(uv_coord, v_viewIndex));
#else
    fragOutput = texture(tex, uv_coord);
#endif
#endif
}
