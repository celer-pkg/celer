void MAIN()
{
    if (uSel > 1.0)
        BASE_COLOR = vec4(normalize(VIEW_VECTOR), 1.0);
    else if (uSel > 0.0)
        BASE_COLOR = vec4(VAR_WORLD_POSITION * VAR_WORLD_NORMAL, 1.0);
    else if (uSel == 0.0)
        BASE_COLOR = vec4(normalize(-CAMERA_DIRECTION), 1.0);
    else if (uSel < 0.0)
        BASE_COLOR = vec4(normalize(CAMERA_POSITION), 1.0);
}
