#version 440

layout(location = 0) out vec4 fragOutput;

layout(location = 0) in vec2 uv_coord;

layout(std140, binding = 0) uniform buf {
    int hemisphere;
} ubuf;

layout(binding = 1) uniform samplerCube shadowMapCube;

#define FORWARD_HEMISPHERE 0
#define REVERSE_HEMISPHERE 1

void main()
{
    // Convert [0..1] to [-1..1]
    vec2 uv = 2.0 * uv_coord - 1.0;
    float r2 = dot(uv, uv);
    float z = 0;

    if (ubuf.hemisphere == FORWARD_HEMISPHERE) {
        // Compute direction for the forward paraboloid
        z = (1.0 - r2) / (1.0 + r2);
    } else {
        z = (r2 - 1.0) / (1.0 + r2);
    }

    float x = (2.0 * uv.x) / (1.0 + r2);
    float y = (2.0 * uv.y) / (1.0 + r2);

    // Sample radial depth from the cubemap
    float depthVal = texture(shadowMapCube, vec3(x, y, z)).r;

    fragOutput = vec4(depthVal);
}
