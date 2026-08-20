void MAIN()
{
    vec2 tc = vec2(UV0.x + 1.5, UV0.y - 2.5);
    BASE_COLOR = texture(tex, tc);
}
