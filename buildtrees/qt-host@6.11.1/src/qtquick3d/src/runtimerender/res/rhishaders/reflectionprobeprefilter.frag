#version 440
layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec3 localPos;

layout(binding = 1) uniform samplerCube environmentMap;
layout(std140, binding = 2) uniform buf {
    vec4 sampleDirections[16];
    float invTotalWeight;
    uint sampleCount;
} ubuf2;

// This implements the pre-filtering technique described here:
// https://placeholderart.wordpress.com/2015/07/28/implementation-notes-runtime-environment-map-filtering-for-image-based-lighting/
void main()
{
    vec3 N = normalize(localPos);
    vec3 R = N;
    vec3 V = R;

    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    mat3 tangentToWorld = mat3(tangent, bitangent, N);

    vec3 prefilteredColor = vec3(0.0);
    for(uint i = 0u; i < ubuf2.sampleCount; ++i)
    {
        vec3 L = tangentToWorld * ubuf2.sampleDirections[i].xyz;
        prefilteredColor += textureLod(environmentMap, L, ubuf2.sampleDirections[i].w).rgb * ubuf2.sampleDirections[i].z;
    }
    prefilteredColor = prefilteredColor * ubuf2.invTotalWeight;
    FragColor = vec4(prefilteredColor, 1.0);
}
