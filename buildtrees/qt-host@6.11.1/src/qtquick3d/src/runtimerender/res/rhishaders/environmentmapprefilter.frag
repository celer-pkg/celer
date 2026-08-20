#version 440

#ifndef QSSG_ENABLE_RGBE_LIGHT_PROBE
#define QSSG_ENABLE_RGBE_LIGHT_PROBE 0
#endif

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec3 localPos;

layout(binding = 1) uniform samplerCube environmentMap;
layout(std140, binding = 2) uniform buf {
    float roughness;
    float resolution;
    float lodBias;
    int sampleCount;
    int distribution;
} ubuf2;

const int DistributionLambertian = 0;
const int DistributionGGX = 1;

const float M_PI = 3.14159265359;

#if QSSG_ENABLE_RGBE_LIGHT_PROBE
vec4 decodeRGBE(in vec4 rgbe)
{
    float f = pow(2.0, 255.0 * rgbe.a - 128.0);
    return vec4(rgbe.rgb * f, 1.0);
}

vec4 encodeRGBE(in vec4 rgba)
{
    float maxMan = max(rgba.r, max(rgba.g, rgba.b));
    float maxExp = 1.0 + floor(log2(maxMan));
    return vec4(rgba.rgb / pow(2.0, maxExp), (maxExp + 128.0) / 255.0);
}
#endif

float radicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), radicalInverse_VdC(i));
}

mat3 generateTBN(vec3 normal)
{
    vec3 bitangent = vec3(0.0, 1.0, 0.0);

    float NdotUp = dot(normal, vec3(0.0, 1.0, 0.0));
    float epsilon = 0.0000001;
    if (1.0 - abs(NdotUp) <= epsilon) {
        // Sampling +Y or -Y, so we need a more robust bitangent.
        if (NdotUp > 0.0)
            bitangent = vec3(0.0, 0.0, 1.0);
        else
            bitangent = vec3(0.0, 0.0, -1.0);
    }

    vec3 tangent = normalize(cross(bitangent, normal));
    bitangent = cross(normal, tangent);

    return mat3(tangent, bitangent, normal);
}

struct DistributionSample
{
    float pdf;
    float cosTheta;
    float sinTheta;
    float phi;
};

float D_GGX(float NdotH, float roughness) {
    float a = NdotH * roughness;
    float k = roughness / (1.0 - NdotH * NdotH + a * a);
    return k * k * (1.0 / M_PI);
}

float saturate(float v)
{
    return clamp(v, 0.0f, 1.0f);
}

DistributionSample GGX(vec2 xi, float roughness)
{
    DistributionSample ggx;

    // evaluate sampling equations
    float alpha = roughness * roughness;
    ggx.cosTheta = saturate(sqrt((1.0 - xi.y) / (1.0 + (alpha * alpha - 1.0) * xi.y)));
    ggx.sinTheta = sqrt(1.0 - ggx.cosTheta * ggx.cosTheta);
    ggx.phi = 2.0 * M_PI * xi.x;

    // evaluate GGX pdf (for half vector)
    ggx.pdf = D_GGX(ggx.cosTheta, alpha);

    ggx.pdf /= 4.0;

    return ggx;
}

DistributionSample Lambertian(vec2 xi)
{
    DistributionSample lambertian;

    // Cosine weighted hemisphere sampling
    // http://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations.html#Cosine-WeightedHemisphereSampling
    lambertian.cosTheta = sqrt(1.0 - xi.y);
    lambertian.sinTheta = sqrt(xi.y);
    lambertian.phi = 2.0 * M_PI * xi.x;

    lambertian.pdf = lambertian.cosTheta / M_PI;

    return lambertian;
}

vec4 getImportanceSample(int sampleIndex, vec3 N, float roughness)
{
    // generate a quasi monte carlo point in the unit square [0.1)^2
    vec2 xi = hammersley(sampleIndex, ubuf2.sampleCount);

    DistributionSample importanceSample;

    // generate the points on the hemisphere with a fitting mapping for
    // the distribution (e.g. lambertian uses a cosine importance)
    if (ubuf2.distribution == DistributionLambertian) {
        importanceSample = Lambertian(xi);
    } else if (ubuf2.distribution == DistributionGGX) {
        // Trowbridge-Reitz / GGX microfacet model (Walter et al)
        // https://www.cs.cornell.edu/~srm/publications/EGSR07-btdf.html
        importanceSample = GGX(xi, roughness);
    }

    // transform the hemisphere sample to the normal coordinate frame
    // i.e. rotate the hemisphere to the normal direction
    vec3 localSpaceDirection = normalize(vec3(
        importanceSample.sinTheta * cos(importanceSample.phi),
        importanceSample.sinTheta * sin(importanceSample.phi),
        importanceSample.cosTheta
    ));
    mat3 TBN = generateTBN(N);
    vec3 direction = TBN * localSpaceDirection;

    return vec4(direction, importanceSample.pdf);
}

float computeLod(float pdf)
{
    float lod = 0.5 * log2( 6.0 * ubuf2.resolution * ubuf2.resolution / (float(ubuf2.sampleCount) * pdf));
    return lod;
}

vec3 filterColor(vec3 N)
{
    vec3 color = vec3(0.f);
    float weight = 0.0f;

    for (int i = 0; i < ubuf2.sampleCount; ++i) {
        vec4 importanceSample = getImportanceSample(i, N, ubuf2.roughness);

        vec3 H = vec3(importanceSample.xyz);
        float pdf = importanceSample.w;

        // mipmap filtered samples (GPU Gems 3, 20.4)
        float lod = computeLod(pdf);

        // apply the bias to the lod
        lod += ubuf2.lodBias;

        if (ubuf2.distribution == DistributionLambertian) {
            // sample lambertian at a lower resolution to avoid fireflies

#if QSSG_ENABLE_RGBE_LIGHT_PROBE
            vec3 lambertian = decodeRGBE(texture(environmentMap, H)).rgb;
#else
            vec3 lambertian = textureLod(environmentMap, H, lod).rgb;
#endif
            color += lambertian;
        } else if (ubuf2.distribution == DistributionGGX) {
            // Note: reflect takes incident vector.
            vec3 V = N;
            vec3 L = normalize(reflect(-V, H));
            float NdotL = dot(N, L);

            if (NdotL > 0.0) {
                if (ubuf2.roughness == 0.0)
                    lod = ubuf2.lodBias;

#if QSSG_ENABLE_RGBE_LIGHT_PROBE
                vec3 sampleColor = decodeRGBE(texture(environmentMap, L)).rgb;
#else
                vec3 sampleColor = textureLod(environmentMap, L, lod).rgb;
#endif
                color += sampleColor * NdotL;
                weight += NdotL;
            }
        }
    }

    if (weight != 0.0f)
        color /= weight;
    else
        color /= float(ubuf2.sampleCount);

    return color.rgb ;
}

void main()
{
    vec3 direction = normalize(localPos);
    vec3 color = filterColor(direction);

#if QSSG_ENABLE_RGBE_LIGHT_PROBE
    FragColor = encodeRGBE(vec4(color, 1.0));
#else
    FragColor = vec4(color, 1.0);
#endif
}
