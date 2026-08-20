void MAIN()
{
    vec4 c = texture(tex, TEXTURE_UV);
    c.r *= uRed;
    FRAGCOLOR = c;
}
