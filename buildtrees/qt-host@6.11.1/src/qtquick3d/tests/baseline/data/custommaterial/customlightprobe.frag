VARYING vec3 vNormal;
VARYING vec3 vViewVec;

void MAIN()
{
    METALNESS = 1.0;
    ROUGHNESS = 0.25;
    FRESNEL_POWER = 5.0;
}

void AMBIENT_LIGHT()
{
    DIFFUSE += TOTAL_AMBIENT_COLOR;
}
