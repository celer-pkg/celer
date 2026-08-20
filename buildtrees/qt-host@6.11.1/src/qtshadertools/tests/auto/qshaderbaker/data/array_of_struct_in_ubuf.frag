#version 440

layout(location = 0) in vec3 vECVertNormal;
layout(location = 1) in vec3 vECVertPos;
layout(location = 2) flat in vec3 vDiffuseAdjust;

#define MAX_LIGHTS 10

struct Light {
    // padding1, 2, and 3 ensures it stays translatable to HLSL with packoffset in the top-level block

    vec3 ECLightPosition;
    float padding1;
    vec3 attenuation;
    float padding2;
    vec3 color;
    float padding3;
    float intensity;
    float specularExp;
};

layout(std140, binding = 1) uniform buf {
    vec3 ECCameraPosition;
    vec3 ka;
    vec3 kd;
    vec3 ks;
    Light lights[MAX_LIGHTS];
    int numLights;
    layout(row_major) mat3 mm;
} ubuf;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 unnormL = ubuf.lights[0].ECLightPosition - vECVertPos;
    float dist = length(unnormL);
    float att = 1.0 / (ubuf.lights[0].attenuation.x + ubuf.lights[0].attenuation.y * dist + ubuf.lights[0].attenuation.z * dist * dist);

    vec3 N = normalize(vECVertNormal);
    vec3 L = normalize(unnormL);
    float NL = max(0.0, dot(N, L));
    vec3 dColor = att * ubuf.lights[0].intensity * ubuf.lights[0].color * NL;

    vec3 R = reflect(-L, N);
    vec3 V = normalize(ubuf.ECCameraPosition - vECVertPos);
    float RV = max(0.0, dot(R, V));
    vec3 sColor = att * ubuf.lights[0].intensity * ubuf.lights[0].color * pow(RV, ubuf.lights[0].specularExp);

    fragColor = vec4(ubuf.ka + (ubuf.kd + vDiffuseAdjust) * dColor + ubuf.ks * sColor, 1.0);
}
