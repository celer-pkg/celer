VARYING vec3 vNormal;

void MAIN()
{
    ivec2 iSize = textureSize(AO_TEXTURE, 0);
    vec2 smpUV = (gl_FragCoord.xy) / vec2(iSize);
    float aoFactor = texture(AO_TEXTURE, smpUV).x;

    vec3 c = vec3(0.1);
    c += vec3(0.0, 1.0, 0.0) * vec3(max(0.0, dot(normalize(vNormal), vec3(0.0, 0.3, 1.0))));
    FRAGCOLOR = vec4(aoFactor * c, 1.0);
}
