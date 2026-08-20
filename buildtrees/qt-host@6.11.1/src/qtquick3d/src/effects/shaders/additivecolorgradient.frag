void MAIN()
{
    vec4 origColor = texture(INPUT, INPUT_UV);
    vec4 gradient = vec4(mix(vec4(bottomColor.rgb, 1.0), vec4(topColor.rgb, 1.0), INPUT_UV.y));
    FRAGCOLOR = origColor + gradient;
}
