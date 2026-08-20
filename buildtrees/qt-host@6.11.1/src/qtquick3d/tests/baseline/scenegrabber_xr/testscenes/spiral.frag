VARYING vec2 center_vec;

void MAIN()
{
    float radius = 0.25;
    float dist_to_center = length(center_vec) / radius;
    vec2 texcoord = INPUT_UV;
    if (dist_to_center <= 1.0) {
        float rotation_amount = (1.0 - dist_to_center) * (1.0 - dist_to_center);
        float r = radians(360.0) * rotation_amount / 4.0;
        mat2 rotation = mat2(cos(r), sin(r), -sin(r), cos(r));
        texcoord = vec2(0.5, 0.5) + rotation * (INPUT_UV - vec2(0.5, 0.5));
    }
#if QSHADER_VIEW_COUNT >= 2
    FRAGCOLOR = texture(INPUT, vec3(texcoord, VIEW_INDEX));
#else
    FRAGCOLOR = texture(INPUT, texcoord);
#endif
}
