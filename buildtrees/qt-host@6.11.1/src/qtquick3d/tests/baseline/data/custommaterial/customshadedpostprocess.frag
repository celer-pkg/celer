
void MAIN()
{
    BASE_COLOR = vec4(1.0, 0.0, 0.0, 1.0);
    METALNESS = 0.5;
    EMISSIVE_COLOR = vec3(0.0, 1.0, 0.0);
}

void POST_PROCESS()
{
    if (gl_FragCoord.x > screen_width * 0.5)
        COLOR_SUM = DIFFUSE;
    else
        COLOR_SUM = vec4(EMISSIVE, DIFFUSE.a);
}
