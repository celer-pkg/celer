VARYING vec3 worldNormal;

void MAIN()
{
#if QSSG_ENABLE_NORMAL_PASS
    FRAGCOLOR = vec4(worldNormal, 1.0);
#else
    vec2 uv = (gl_FragCoord.xy) / vec2(textureSize(NORMAL_ROUGHNESS_TEXTURE, 0));
    FRAGCOLOR = vec4(normalize(texture(NORMAL_ROUGHNESS_TEXTURE, uv).rgb), 1.0);
#endif
}
