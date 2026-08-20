VARYING vec2 center_vec;

void MAIN()
{
    float dist_to_center = length(center_vec) / radius;

    vec2 texc;
    if (dist_to_center > 1.0) {
        texc = INPUT_UV;
    } else {
        float rotation_amount = (1.0 - dist_to_center) * (1.0 - dist_to_center);
        float r = radians(360.0) * rotation_amount * distortionStrength / 4.0;
        float cos_r = cos(r);
        float sin_r = sin(r);
        mat2 rotation = mat2(cos_r, sin_r, -sin_r, cos_r);
        texc = center + rotation * (INPUT_UV - center);
    }

    if (texc.x < 0.0 || texc.x > 1.0 || texc.y < 0.0 || texc.y > 1.0)
        FRAGCOLOR = vec4(0.0);
    else
        FRAGCOLOR = texture(INPUT, texc);
}
