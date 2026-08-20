#version 440

layout(location = 0) in vec3 localPos;
layout(location = 0) out vec4 FragColor;

layout(binding = 1) uniform sampler2D environmentMap;
layout(std140, binding = 2) uniform buf {
    int colorSpace;
} ubuf2;

vec2 sampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= vec2(0.1591, 0.3183);
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = sampleSphericalMap(normalize(localPos)); // make sure to normalize localPos

    vec4 textureColor = texture(environmentMap, uv);
    // sRGB to Linear
    if (ubuf2.colorSpace == 1)
        textureColor.rgb = textureColor.rgb * (textureColor.rgb * (textureColor.rgb * 0.305306011 + 0.682171111) + 0.012522878);

    FragColor = textureColor;
}
