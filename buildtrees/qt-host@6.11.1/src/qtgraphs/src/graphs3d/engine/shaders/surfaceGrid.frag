vec4 color;
VARYING vec3 pos;
VARYING float disc;
VARYING vec2 UV;

void MAIN()
{
    if (abs(pos.y) > graphHeight)
        discard;
    if (disc > 0)
        discard;
    if (texture(height, UV).a != 1.0)
        discard;

    color = gridColor;
}

void POST_PROCESS()
{
    COLOR_SUM = color;
}


