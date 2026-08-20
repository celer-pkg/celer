void MAIN()
{
    SPECULAR_AMOUNT = 1.0;
    ROUGHNESS = 0.5;
    BASE_COLOR = uDiffuse;
}

void AMBIENT_LIGHT()
{
    DIFFUSE += uDiffuse.rgb * TOTAL_AMBIENT_COLOR;
}

void DIRECTIONAL_LIGHT()
{
    DIFFUSE += uDiffuse.rgb * LIGHT_COLOR * SHADOW_CONTRIB * vec3(max(0.0, dot(normalize(NORMAL), TO_LIGHT_DIR)));
}

void POINT_LIGHT()
{
    DIFFUSE += uDiffuse.rgb * LIGHT_COLOR * LIGHT_ATTENUATION * SHADOW_CONTRIB * vec3(max(0.0, dot(normalize(NORMAL), TO_LIGHT_DIR)));
}

void SPOT_LIGHT()
{
     DIFFUSE += uDiffuse.rgb * LIGHT_COLOR * LIGHT_ATTENUATION * SPOT_FACTOR * SHADOW_CONTRIB * vec3(max(0.0, dot(normalize(VAR_WORLD_NORMAL), TO_LIGHT_DIR)));
}
