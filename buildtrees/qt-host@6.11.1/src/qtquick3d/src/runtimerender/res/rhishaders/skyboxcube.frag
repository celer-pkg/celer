#version 440
#extension GL_GOOGLE_include_directive : enable

#ifdef QSSG_RUNTIME_TONEMAPPING

#ifndef QSSG_ENABLE_ACES_TONEMAPPING
#define QSSG_ENABLE_ACES_TONEMAPPING 1
#endif

#ifndef QSSG_ENABLE_HEJLDAWSON_TONEMAPPING
#define QSSG_ENABLE_HEJLDAWSON_TONEMAPPING 1
#endif

#ifndef QSSG_ENABLE_FILMIC_TONEMAPPING
#define QSSG_ENABLE_FILMIC_TONEMAPPING 1
#endif

#ifndef QSSG_ENABLE_LINEAR_TONEMAPPING
#define QSSG_ENABLE_LINEAR_TONEMAPPING 1
#endif

#include "../effectlib/tonemapping.glsllib"

#endif

layout(location = 0) out vec4 fragOutput;

layout(std140, binding = 0) uniform buf {
    vec4 skyboxProperties;
    mat3 orientation;
#if QSHADER_VIEW_COUNT >= 2
    mat4 viewProjection[QSHADER_VIEW_COUNT];
    mat4 inverseProjection[QSHADER_VIEW_COUNT];
    mat3 viewMatrix[QSHADER_VIEW_COUNT];
#else
    mat4 viewProjection;
    mat4 inverseProjection;
    mat3 viewMatrix;
#endif
} ubuf;

layout(location = 0) in vec3 tex_coords;

layout(binding = 1) uniform samplerCube tex;

void main()
{
    vec4 color = texture(tex, tex_coords);

#ifdef QSSG_RUNTIME_TONEMAPPING
    uint tonemapMode = int(ubuf.skyboxProperties.w);
    bool sRrgb = bool(ubuf.skyboxProperties.z);

    fragOutput = qt_tonemap(color, sRrgb, tonemapMode);
#else
    fragOutput = color;
#endif
}
