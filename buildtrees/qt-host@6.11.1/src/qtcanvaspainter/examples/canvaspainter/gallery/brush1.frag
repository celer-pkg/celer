#version 440

QC_INCLUDE "customfrag.glsl"

// Shader snippets taken from QQEM NoiseHelper and ElectricClounds nodes.

// Hash from uvec2 to float
float _hash21( uvec2 x )
{
    uvec2 q = 1103515245U * ( (x>>1U) ^ (x.yx   ) );
    uint  n = 1103515245U * ( (q.x  ) ^ (q.y>>3U) );
    return float(n) * (1.0/float(0xffffffffU));
}

// Hash from vec2 to float
// Can be used directly with fragCoord
float hash21(vec2 n)
{
    uvec2 uin = uvec2(n);
    return _hash21(uin);
}

vec2 pseudo3dNoiseLevel(vec2 intPos, float t) {
    float rand = hash21(intPos);
    float angle = TAU * rand + t * rand;
    return vec2(cos(angle), sin(angle));
}

// Generates noise which resembles 3D perlin-noise
float pseudo3dNoise(vec3 pos) {
    const vec2 i = floor(pos.xy);
    const vec2 f = fract(pos.xy);
    const vec2 blend = f * f * (3.0 - 2.0 * f);
    float noiseVal = mix(
        mix(
            dot(pseudo3dNoiseLevel(i + vec2(0.0, 0.0), pos.z), f - vec2(0.0, 0.0)),
            dot(pseudo3dNoiseLevel(i + vec2(1.0, 0.0), pos.z), f - vec2(1.0, 0.0)),
            blend.x),
        mix(
            dot(pseudo3dNoiseLevel(i + vec2(0.0, 1.0), pos.z), f - vec2(0.0, 1.0)),
            dot(pseudo3dNoiseLevel(i + vec2(1.0, 1.0), pos.z), f - vec2(1.0, 1.0)),
            blend.x),
        blend.y);
    return noiseVal * SQRT2;
}

void main()
{
    vec3 iResolution = vec3(500, 500, 1);
    int electricCloudLevels = 5;
    vec4 electricCloudColor = data1;
    vec2 uv = fragCoord.xy / iResolution.y;
    float cloudVal = 0.0;
    float sum = 0.0;
    float multiplier = 1.0;
    float time = iTime * 0.2;
    for (int i = 0; i < electricCloudLevels; i++) {
        vec3 noisePos = vec3(uv, time / multiplier);
        cloudVal += multiplier * abs(pseudo3dNoise(noisePos));
        sum += multiplier;
        multiplier *= 0.5;
        uv = 2.0 * uv;
    }
    cloudVal /= sum;
    fragColor.a = 1.0;
    fragColor.rgb = mix(fragColor.rgb, vec3(cos(TAU * cloudVal)) * electricCloudColor.rgb * fragColor.a, electricCloudColor.a);

    // Note: If you don't need clipping support, remove clipMask().
    // Note: If you don't need antialiasing support, remove antialiasingAlpha().
    //fragColor *= globalAlpha * antialiasingAlpha() * clipMask();
    fragColor *= globalAlpha * antialiasingAlpha();

    // Note: Disable this if you don't want color effects to affect.
    applyColorEffects(fragColor);
}
