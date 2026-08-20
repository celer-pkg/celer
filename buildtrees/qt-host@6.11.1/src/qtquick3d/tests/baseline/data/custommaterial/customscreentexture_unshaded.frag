void MAIN()
{
    vec2 size = vec2(textureSize(SCREEN_TEXTURE, 0));
    vec2 uv = gl_FragCoord.xy / size;

    vec4 c = texture(SCREEN_TEXTURE, uv);
    c.r = 1.0;

    FRAGCOLOR = c;
}
