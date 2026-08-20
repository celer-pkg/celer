void MAIN()
{
    vec2 uv = TEXTURE_UV;
    FRAGCOLOR = texture(extensionTexture, vec2(uv.x, uv.y * apiFactor + (1.0 - uv.y) * apiFactorFlip));
}
