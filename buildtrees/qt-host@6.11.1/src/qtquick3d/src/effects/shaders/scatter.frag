void MAIN()
{
    float size = 15.0;
    float strength = amount / 127.0 * 0.4;

    vec2 uv = INPUT_UV * size;
    if (randomize)
        uv = fract(uv + 0.031 * FRAME);

    uv = texture(noiseSample, fract(uv)).xy - 0.5;

    if (direction == 0)
        uv *= (vec2(1.5, 0.15) * strength);
    else if (direction == 1)
        uv *= (vec2(1.3, 0.0) * strength);
    else
        uv *= (vec2(0.0, 0.29) * strength);

    uv += INPUT_UV;

    vec2 halfPixelSize = 0.5 / INPUT_SIZE;
    FRAGCOLOR = texture(INPUT, clamp(uv, halfPixelSize, 1.0 - halfPixelSize));
}
