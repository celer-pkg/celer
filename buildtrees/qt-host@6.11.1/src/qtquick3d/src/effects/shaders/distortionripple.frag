VARYING vec2 center_vec;

void MAIN()
{
    float dist_to_center = length(center_vec) * 100.0 / radius;

    vec2 texc;
    if (dist_to_center > 1.0) {
        texc = INPUT_UV;
    } else {
        float r = radians(360.0) * (1.0 - dist_to_center);
        float distortion = sin(r * (100.0 - distortionWidth) + radians(distortionPhase));
        texc = INPUT_UV - (INPUT_UV - center) * distortion * distortionHeight / 800.0;
    }

    if (texc.x < 0.0 || texc.x > 1.0 || texc.y < 0.0 || texc.y > 1.0)
        FRAGCOLOR = vec4(0.0);
    else
        FRAGCOLOR = texture(INPUT, texc);
}
