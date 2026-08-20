#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float spread;
    vec4 color;
};

layout(binding = 1) uniform sampler2D original;
layout(binding = 2) uniform sampler2D shadow;

float linearstep(float e0, float e1, float x) {
    return clamp((x - e0) / (e1 - e0), 0.0, 1.0);
}

void main(void) {
    vec4 originalColor = texture(original, qt_TexCoord0);
    vec4 shadowColor = texture(shadow, qt_TexCoord0);
    shadowColor.rgb = mix(originalColor.rgb, color.rgb * originalColor.a, linearstep(0.0, spread, shadowColor.a));
    fragColor = vec4(shadowColor.rgb, originalColor.a) * originalColor.a * qt_Opacity;
}
