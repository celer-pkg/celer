void MAIN()
{
    vec4 c = texture(cubemap, NORMAL);
    BASE_COLOR = vec4(c.rgb, 1.0);
}
