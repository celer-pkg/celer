void MAIN()
{
    vec4 origColor = texture(INPUT, INPUT_UV);
    vec2 uv = INPUT_UV;

    vec4 bg = origColor;

    uv *= 1.0 - uv.yx;
    float vig = uv.x * uv.y * vignetteStrength;
    vig = pow(vig, vignetteRadius);

    vec4 vigColor = vec4(vignetteColor.rgb, vig) * vig;
    FRAGCOLOR = mix(vigColor, origColor, vig);
}
