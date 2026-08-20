VARYING vec2 texcoord;
VARYING vec3 normal;

void MAIN()
{
    vec4 c = texture(tex, texcoord);
    vec3 diffuse = c.rgb * vec3(max(0.0, dot(normalize(normal), vec3(0.5, 0.8, -0.3))));
    FRAGCOLOR = vec4(diffuse, 1.0);
}
