#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D source;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float multiplier;
};

vec3 SRGBToLinear(vec3 srgbIn)
{
    return srgbIn * (srgbIn * (srgbIn * 0.305306011 + 0.682171111) + 0.012522878);
}

void main()
{
    vec4 c = texture(source, qt_TexCoord0);

    // so much for Qt Quick is always linear, has no concept of sRGB, etc. The
    // content is effectively sRGB so it all needs to be linearized (but
    // nothing else, assuming scRGB).

    c.rgb = SRGBToLinear(c.rgb);

    c.rgb *= multiplier;

    c *= qt_Opacity;
    fragColor = c;
}
