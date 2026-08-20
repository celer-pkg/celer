void MAIN()
{
    vec4 origColor = texture(INPUT, INPUT_UV);

    float step_w = amount;
    float step_h = amount;

    vec4 t1 = texture(INPUT, vec2(INPUT_UV.x - step_w, INPUT_UV.y - step_h));
    vec4 t2 = texture(INPUT, vec2(INPUT_UV.x, INPUT_UV.y - step_h));
    vec4 t3 = texture(INPUT, vec2(INPUT_UV.x - step_w, INPUT_UV.y));
    vec4 t4 = texture(INPUT, INPUT_UV);

    vec3 rr = -4.0 * t1.rgb - 4.0 * t2.rgb - 4.0 * t3.rgb + 12.0 * t4.rgb;
    float y = (rr.r + rr.g + rr.b) / 3.0;

    vec4 result = vec4(vec3(y, y, y) + 0.3, origColor.a);

    FRAGCOLOR = result;
}
