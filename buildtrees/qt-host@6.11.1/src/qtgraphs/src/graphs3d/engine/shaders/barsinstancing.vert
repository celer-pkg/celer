VARYING vec3 pos;
VARYING vec3 instanceColor;
VARYING float instanceAlpha;
VARYING float heightValue;

void MAIN()
{
    pos = VERTEX;
    instanceColor = INSTANCE_COLOR.rgb;
    instanceAlpha = INSTANCE_COLOR.a;
    heightValue = INSTANCE_DATA.x;
    POSITION = INSTANCE_MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
}
