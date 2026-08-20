VARYING vec3 pos;
VARYING float disc;
VARYING vec2 UV;

void MAIN()
{
    UV = UV0 * (vertices / range);

    vec3 v = texture(height, UV).xyz;
    pos = v;
    disc = 0;
    if (fill && v.y > graphHeight)
        disc = 1;

    VERTEX = v;
    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
}
