#version 440

QC_INCLUDE "customfrag.glsl"

void main()
{
    float a = 0.3 * sin(0.025 * fragCoord.x + 0.8 * iTime);
    vec4 color1 = vec4(1.0, 0.6 + a, 0.0, 1.0);
    vec4 color2 = data1;
    float a1 = 0.2 + 0.2 * sin(0.1 * fragCoord.y + 0.01 * fragCoord.x - 2.5 * iTime);
    float fontAlpha = sdfFontAlphaRaw();
    vec4 f1 = color1 * smoothstep(a1, fontAlphaMin + a1, fontAlpha);
    vec4 f2 = color2 * smoothstep(fontAlphaMin, fontAlphaMax, fontAlpha);
    fragColor = mix(f1, f2, f2.a) * globalAlpha;
    // Note: Disable this if you don't want color effects to affect.
    applyColorEffects(fragColor);
}
