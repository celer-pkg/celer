VARYING vec2 uv1;

void MAIN()
{
    vec2 size = vec2(textureSize(tex, 0));
    vec2 pix = vec2(1.0 / size.x, 1.0 / size.y);
    vec4 c = texture(tex, uv1);
    if (c.a == 0.0)
        c = texture(tex, uv1 - vec2(0.0, pix.y));
    if (c.a == 0.0)
        c = texture(tex, uv1 + vec2(0.0, pix.y));
    if (c.a == 0.0)
        c = texture(tex, uv1 - vec2(pix.x, 0.0));
    if (c.a == 0.0)
        c = texture(tex, uv1 + vec2(pix.x, 0.0));
    if (c.a == 0.0)
        c = texture(tex, uv1 - pix);
    if (c.a == 0.0)
        c = texture(tex, uv1 + pix);
    if (c.a == 0.0)
        c = texture(tex, uv1 + vec2(pix.x, -pix.y));
    if (c.a == 0.0)
        c = texture(tex, uv1 + vec2(-pix.x, pix.y));
    FRAGCOLOR = c;
}
