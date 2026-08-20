void MAIN()
{
    BASE_COLOR = vec4(1.0, 0.0, 0.0, 1.0);
    ROUGHNESS = 0.4;

    // Important: undo the effect of doublesided support. (no culling when
    // generating the lightmap, but the inverted normal is not wanted)
    NORMAL = VAR_WORLD_NORMAL;
}
