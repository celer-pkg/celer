#version 440

layout(location = 0) in vec2 v_uv;

layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform sampler2D tex;

void main()
{
    vec2 uv = v_uv;
#if QSHADER_HLSL || QSHADER_MSL
    uv.y = 1.0 - uv.y;
#endif

    vec2 sz = vec2(textureSize(tex, 0));
    vec2 pix = vec2(1.0 / sz.x, 1.0 / sz.y);
    vec4 c = texture(tex, uv);

    // Dilate, to avoid seams: take a neighbor with non-zero alpha, if possible.
    // Requires the sampler uses Nearest filtering, it is meaningless with Linear.

    c = c.a > 0.5 ? c : texture(tex, uv + vec2(-pix.x, 0.0));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(0.0, pix.y));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(pix.x, 0.0));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(0.0, -pix.y));

    c = c.a > 0.5 ? c : texture(tex, uv + vec2(-pix.x, -pix.y));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(-pix.x, pix.y));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(pix.x, -pix.y));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(pix.x, pix.y));

    c = c.a > 0.5 ? c : texture(tex, uv + vec2(-2.0 * pix.x, 0.0));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(0.0, 2.0 * pix.y));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(2.0 * pix.x, 0.0));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(0.0, -2.0 * pix.y));

    c = c.a > 0.5 ? c : texture(tex, uv + vec2(-2.0 * pix.x, -pix.y));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(-2.0 * pix.x, pix.y));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(2.0 * pix.x, -pix.y));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(2.0 * pix.x, pix.y));

    c = c.a > 0.5 ? c : texture(tex, uv + vec2(-pix.x, -2.0 * pix.y));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(-pix.x, 2.0 * pix.y));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(pix.x, -2.0 * pix.y));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(pix.x, 2.0 * pix.y));

    c = c.a > 0.5 ? c : texture(tex, uv + vec2(-2.0 * pix.x, -2.0 * pix.y));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(-2.0 * pix.x, 2.0 * pix.y));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(2.0 * pix.x, -2.0 * pix.y));
    c = c.a > 0.5 ? c : texture(tex, uv + vec2(2.0 * pix.x, 2.0 * pix.y));

    fragColor = c;
}
