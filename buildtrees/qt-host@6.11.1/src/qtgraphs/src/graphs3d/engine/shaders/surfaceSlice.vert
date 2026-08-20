void MAIN()
{
    NORMAL = vec3(0.0, 0.0, 1.0);
    if (isColumn)
        NORMAL.z = 1.0;
    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
}
