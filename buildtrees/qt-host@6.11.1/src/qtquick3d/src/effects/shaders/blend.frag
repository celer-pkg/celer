void MAIN()
{
    vec4 src = texture(INPUT, INPUT_UV);
    vec4 dst = texture(sprite, INPUT_UV);
    FRAGCOLOR = src * (1.0 - dst.a) + dst;
}
