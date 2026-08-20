void MAIN()
{
    vec4 c = texture(tex, TEXTURE_UV);
    FRAGCOLOR = c * texture(INPUT, INPUT_UV);
}
