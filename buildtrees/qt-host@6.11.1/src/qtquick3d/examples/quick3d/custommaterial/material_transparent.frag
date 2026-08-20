void MAIN()
{
    vec2 size = vec2(textureSize(SCREEN_TEXTURE, 0));
    vec2 uv = FRAGCOORD.xy / size;

    vec3 view = normalize(VIEW_VECTOR);
    vec3 projection = view - view * normalize(NORMAL);
    vec3 refraction = projection * projection;
    uv += refraction.xy * 0.5;

    vec3 col = texture(SCREEN_TEXTURE, uv).rgb;
    col = col * 0.8 + vec3(0.2);

    BASE_COLOR = vec4(col, 1.0);
}
