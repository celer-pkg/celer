VARYING vec3 pos;
VARYING vec2 coord;

void MAIN()
{
    vec3 rgb = vec3(pos.x * 0.02, pos.y * 0.02, pos.z * 0.02);
    FRAGCOLOR = vec4(rgb, alpha);
}
