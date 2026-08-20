VARYING vec3 vNormal;
VARYING vec3 vViewVec;

void MAIN()
{
    SPECULAR_AMOUNT = 1.0;
    ROUGHNESS = 0.5;
}

void AMBIENT_LIGHT()
{
    DIFFUSE += TOTAL_AMBIENT_COLOR;
}

void DIRECTIONAL_LIGHT()
{
    DIFFUSE += uDiffuse.rgb * LIGHT_COLOR * SHADOW_CONTRIB * vec3(max(0.0, dot(normalize(vNormal), TO_LIGHT_DIR)));
}

void POINT_LIGHT()
{
    DIFFUSE += uDiffuse.rgb * LIGHT_COLOR * LIGHT_ATTENUATION * SHADOW_CONTRIB * vec3(max(0.0, dot(normalize(vNormal), TO_LIGHT_DIR)));
}
