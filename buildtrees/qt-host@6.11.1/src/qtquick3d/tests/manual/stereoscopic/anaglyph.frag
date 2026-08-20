#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D leftEye;
layout(binding = 2) uniform sampler2D rightEye;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    int stereoMode;
} ubuf;

void main()
{
    vec4 tex1 = texture(leftEye, qt_TexCoord0);
    vec4 tex2 = texture(rightEye, qt_TexCoord0);
    if (ubuf.stereoMode == 3)
        fragColor = vec4(tex1.r, tex2.gb, 0.0) * ubuf.qt_Opacity;
    else
        fragColor = vec4(tex2.r, tex1.g, tex2.b, 0.0) * ubuf.qt_Opacity;
}
