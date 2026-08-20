void MAIN()
{
    vec4 c1 = texture(INPUT, INPUT_UV);
    vec4 c2 = texture(pass1Result, INPUT_UV);
    FRAGCOLOR = mix(c1, c2, 0.9);
}
