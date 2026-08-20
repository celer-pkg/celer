void MAIN()
{
    vec4 view1Sample = texture(view1Input, TEXTURE_UV);
    vec4 inputSample = texture(INPUT, INPUT_UV);
    FRAGCOLOR = vec4(inputSample.rgb * (1 - view1Sample.a) + view1Sample.rgb * view1Sample.a, 1.0);
}
