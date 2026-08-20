void MAIN()
{
    vec2 size = vec2(textureSize(SCREEN_TEXTURE, 0));
    vec2 uv = FRAGCOORD.xy / size;

    vec2 d = vec2(1.0 / size.x, 1.0 / size.y);
    vec4 diff = texture(SCREEN_TEXTURE, uv + d) - texture(SCREEN_TEXTURE, uv - d);
    float c = (diff.x + diff.y + diff.z) + 0.5;

    float alpha = 1.0;
    if (uKeepAlpha)
        alpha = texture(SCREEN_TEXTURE, uv).a;

    BASE_COLOR = vec4(vec3(c), alpha);
}
