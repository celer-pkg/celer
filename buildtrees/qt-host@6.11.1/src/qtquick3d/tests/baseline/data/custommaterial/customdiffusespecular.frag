VARYING vec3 vNormal;
VARYING vec3 vViewVec;

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

void SPECULAR_LIGHT()
{
    vec3 V = normalize(vViewVec);
    vec3 H = normalize(V + TO_LIGHT_DIR);
    float cosAlpha = max(0.0, dot(H, normalize(vNormal)));
    float shine = pow(cosAlpha, uShininess);
    const vec3 specularColor = vec3(1.0);
    SPECULAR += shine * specularColor;
}
