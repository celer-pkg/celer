vec4 diffuse = vec4(0.0);
float ambientBrightness = 0.75; // 0...1.0
float directionalBrightness = 0.75; // 0...1.0

VARYING vec3 pos;
VARYING vec3 instanceColor;
VARYING float instanceAlpha;
VARYING float heightValue;

void MAIN()
{
    vec3 color;
    vec2 gradientUV;
    float alpha;
    switch(colorStyle) {
    case 0: //uniform
        gradientUV = vec2((pos.y + 1.0) / 2.0, 0.0); //for highlight only
        color = normalize(uniformColor.rgb);
        color *= instanceColor;
        alpha = instanceAlpha;
        break;
    case 1: // objectgradient
        gradientUV = vec2((pos.y + 1.0) / 2.0, 0.0);
        color = texture(custex, gradientUV).xyz;
        alpha = texture(custex, gradientUV).w;
        break;
    case 2: //rangegradient
        if (valueColoring)
            gradientUV = vec2(heightValue, 0);
        else
            gradientUV = vec2(((VAR_WORLD_POSITION.y + rootScale) / 2.0) / rootScale, 0.0);
        color = texture(custex, gradientUV).xyz;
        alpha = texture(custex, gradientUV).w;
        break;
    }
    if (isHighlight) {
        color = texture(custex, gradientUV).xyz;
        alpha = texture(custex, gradientUV).w;
    }

    diffuse = vec4(color, alpha);
    BASE_COLOR = diffuse;
}

void AMBIENT_LIGHT()
{
    if (colorStyle == 0)
        DIFFUSE += diffuse.rgb * ambientBrightness;
    else
        DIFFUSE += diffuse.rgb * TOTAL_AMBIENT_COLOR * ambientBrightness;
}

void DIRECTIONAL_LIGHT()
{
    DIFFUSE += diffuse.rgb * directionalBrightness * LIGHT_COLOR * SHADOW_CONTRIB * vec3(max(0.0, dot(normalize(NORMAL), TO_LIGHT_DIR)));
}

void SPECULAR_LIGHT()
{
    vec3 H = normalize(VIEW_VECTOR + TO_LIGHT_DIR);
    float cosAlpha = max(0.0, dot(H, normalize(NORMAL)));
    float shine = pow(cosAlpha, shininess);
    const vec3 specularColor = vec3(specularBrightness);
    SPECULAR += shine * specularColor;
}

void POST_PROCESS()
{
    if (shaded)
        COLOR_SUM = vec4(DIFFUSE.rgb + SPECULAR + EMISSIVE, DIFFUSE.a);
    else
        COLOR_SUM = diffuse;
}
