void MAIN()
{
#if QSHADER_VIEW_COUNT >= 2
    vec4 c = texture(INPUT, vec3(INPUT_UV, VIEW_INDEX));
#else
    vec4 c = texture(INPUT, INPUT_UV);
#endif
    c.r = uRed;
    FRAGCOLOR = c;
}
