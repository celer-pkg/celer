VARYING vec2 center_vec;

void MAIN()
{
    float dist_to_center = length(center_vec) / radius;

    vec2 texc;
    if (dist_to_center > 1.0) {
        texc = INPUT_UV;
    } else {
        float distortion = 1.0 - dist_to_center * dist_to_center;
        texc = INPUT_UV - (INPUT_UV - center) * distortion * distortionHeight;
    }

    if (texc.x < 0.0 || texc.x > 1.0 || texc.y < 0.0 || texc.y > 1.0)
        FRAGCOLOR = vec4(0.0);
    else
        FRAGCOLOR = texture(INPUT, texc);
}
