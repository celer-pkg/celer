void MAIN()
{
    BASE_COLOR = texture(extensionTexture, apiFactor * UV0 + apiFactorFlip * vec2(UV0.x, 1.0 - UV0.y));
}
