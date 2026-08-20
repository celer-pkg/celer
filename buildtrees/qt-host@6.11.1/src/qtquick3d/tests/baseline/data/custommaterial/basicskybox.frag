VARYING vec3 coord;

void MAIN()
{
    vec3 c = texture(skybox, coord).xyz;
    FRAGCOLOR = vec4(c, 1.0);
}
