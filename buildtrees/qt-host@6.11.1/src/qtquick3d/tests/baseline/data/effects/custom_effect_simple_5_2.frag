void MAIN()
{
    vec4 c = texture(INPUT, INPUT_UV);
    vec4 c2 = texture(originalInput, INPUT_UV);
    c2.r *= uRed;
    FRAGCOLOR = c + c2;
}
