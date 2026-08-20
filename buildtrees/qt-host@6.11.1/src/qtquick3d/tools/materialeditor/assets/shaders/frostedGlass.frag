#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    // qt_Matrix and qt_Opacity must always be both present
    // if the built-in vertex shader is used.
    mat4 qt_Matrix;
    float qt_Opacity;
    float range;
    float blur;
    vec4 color;
};

layout(binding = 1) uniform sampler2D sourceTex;
layout(binding = 2) uniform sampler2D noiseTex;

void main()
{
    vec2 frostUV = texture(noiseTex, qt_TexCoord0).rg;

    frostUV -= 0.5;
    frostUV *= range;
    frostUV += qt_TexCoord0;

    vec4 frost = texture(sourceTex, frostUV);
    frost += texture(sourceTex, frostUV + vec2(blur, blur));
    frost += texture(sourceTex, frostUV + vec2(blur, -blur));
    frost += texture(sourceTex, frostUV + vec2(-blur, blur));
    frost += texture(sourceTex, frostUV + vec2(-blur, -blur));
    frost *= 0.2;
    frost.rgb *= color.rgb;

    fragColor = vec4(frost.rgb, qt_Opacity);
}
