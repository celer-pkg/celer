#version 440
#extension GL_GOOGLE_include_directive : enable

#include "../effectlib/tonemapping.glsllib"

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
    vec4 color = texture(tex, vec3(uv, v_viewIndex));
#else
    vec4 color = texture(tex, uv);
#endif
#else
#if QSHADER_VIEW_COUNT >= 2
    vec4 color = texture(tex, vec3(uv_coord, v_viewIndex));
#else
    vec4 color = texture(tex, uv_coord);
#endif
#endif

    fragOutput = vec4(qt_tonemap(qt_exposure(color.rgb, 1.0)), 1.0);
}
