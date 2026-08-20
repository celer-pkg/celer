VARYING vec2 TexCoord0;
VARYING vec2 TexCoord1;
VARYING vec2 TexCoord2;
VARYING vec2 TexCoord3;
VARYING vec2 TexCoord4;
VARYING vec2 TexCoord5;
VARYING vec2 TexCoord6;
VARYING vec2 TexCoord7;

float cutoff(float color)
{
    return color >= bloomThreshold ? color : 0.0;
}

vec4 cutoffColor(sampler2D inSampler, vec2 texCoord)
{
    vec4 color = texture(inSampler, texCoord);
    vec3 exposed_color = color.xyz * exposureExp2;
    vec3 cutoff_color = vec3(cutoff(color.x), cutoff(color.y), cutoff(color.z));
    float pixelMult = dot(cutoff_color, cutoff_color) > 0.0 ? 1.0 : 0.0;
    return vec4(exposed_color.xyz, color.a) * pixelMult;
}

vec4 smear(sampler2D inSampler)
{
    vec4 outColor = cutoffColor(inSampler, TexCoord0) / 8.0;
    outColor += cutoffColor(inSampler, TexCoord1) / 8.0;
    outColor += cutoffColor(inSampler, TexCoord2) / 8.0;
    outColor += cutoffColor(inSampler, TexCoord3) / 8.0;
    outColor += cutoffColor(inSampler, TexCoord4) / 8.0;
    outColor += cutoffColor(inSampler, TexCoord5) / 8.0;
    outColor += cutoffColor(inSampler, TexCoord6) / 8.0;
    outColor += cutoffColor(inSampler, TexCoord7) / 8.0;
    return outColor;
}

void MAIN()
{
    FRAGCOLOR = smear(INPUT);
}
