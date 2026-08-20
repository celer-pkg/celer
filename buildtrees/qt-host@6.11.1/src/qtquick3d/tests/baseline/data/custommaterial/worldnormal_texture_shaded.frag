void MAIN()
{
    vec2 uv = (gl_FragCoord.xy) / vec2(textureSize(NORMAL_ROUGHNESS_TEXTURE, 0));
    BASE_COLOR = vec4(normalize(texture(NORMAL_ROUGHNESS_TEXTURE, uv).rgb), 1.0);
}
