void MAIN()
{
    vec4 c = texture(notReallyCubemap, UV0);
    BASE_COLOR = vec4(c.rgb, 1.0);
}
