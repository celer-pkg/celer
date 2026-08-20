void MAIN()
{
    vec3 rgb = texture(tex, UV0).rgb;
    float C1 = 0.305306011;
    vec3 C2 = vec3(0.682171111, 0.682171111, 0.682171111);
    vec3 C3 = vec3(0.012522878, 0.012522878, 0.012522878);
    BASE_COLOR = vec4(rgb * (rgb * (rgb * C1 + C2) + C3), 1.0);
    ROUGHNESS = 0.4;

    // Important: undo the effect of doublesided support. (no culling when
    // generating the lightmap, but the inverted normal is not wanted)
    NORMAL = VAR_WORLD_NORMAL;
}
