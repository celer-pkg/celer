void MAIN()
{
    vec2 size = vec2(textureSize(SCREEN_MIP_TEXTURE, 0));
    vec2 uv = FRAGCOORD.xy / size;

    vec4 c;
    if (mipLevel == 0.0)
        c = texture(SCREEN_TEXTURE, uv);
    else
        c = textureLod(SCREEN_MIP_TEXTURE, uv, mipLevel);

    c.g = c.r;
    c.b = c.r;

    BASE_COLOR = c;
}
