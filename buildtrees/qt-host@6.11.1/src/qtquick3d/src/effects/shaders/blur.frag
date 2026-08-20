void MAIN()
{
    vec4 c1 = texture(INPUT, vec2(INPUT_UV.x - amount, INPUT_UV.y - amount));
    vec4 c2 = texture(INPUT, vec2(INPUT_UV.x + amount, INPUT_UV.y - amount));
    vec4 c3 = texture(INPUT, vec2(INPUT_UV.x - amount, INPUT_UV.y + amount));
    vec4 c4 = texture(INPUT, vec2(INPUT_UV.x + amount, INPUT_UV.y + amount));
    vec4 c5 = texture(INPUT, vec2(INPUT_UV.x - (amount * 2.0), INPUT_UV.y));
    vec4 c6 = texture(INPUT, vec2(INPUT_UV.x, INPUT_UV.y - (amount * 2.0)));
    vec4 c7 = texture(INPUT, vec2(INPUT_UV.x, INPUT_UV.y + (amount * 2.0)));
    vec4 c8 = texture(INPUT, vec2(INPUT_UV.x + (amount * 2.0), INPUT_UV.y));

    FRAGCOLOR = (c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8) / 8.0;
}
