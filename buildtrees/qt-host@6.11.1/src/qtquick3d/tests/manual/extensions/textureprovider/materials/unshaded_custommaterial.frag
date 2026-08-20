VARYING vec2 uv;

void MAIN()
{
    FRAGCOLOR = texture(extensionTexture, apiFactor * uv + apiFactorFlip * vec2(uv.x, 1.0 - uv.y));
}
