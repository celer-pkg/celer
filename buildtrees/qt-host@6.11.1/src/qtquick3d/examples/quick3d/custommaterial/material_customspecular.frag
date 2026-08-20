void MAIN()
{
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

void SPECULAR_LIGHT()
{
    vec3 H = normalize(VIEW_VECTOR + TO_LIGHT_DIR);
    float cosAlpha = max(0.0, dot(H, normalize(NORMAL)));
    float shine = pow(cosAlpha, uShininess);
    const vec3 specularColor = vec3(1.0);
    SPECULAR += shine * specularColor;
}
