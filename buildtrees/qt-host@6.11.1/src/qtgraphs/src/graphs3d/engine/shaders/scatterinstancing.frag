vec4 diffuse = vec4(0.0);
float ambientBrightness = 0.75;
float directionalBrightness = 0.75;
float pointSize = 0.75f;
VARYING vec3 pos;
VARYING vec4 vColor;

void MAIN()
{
    if (usePoint) {
        float distanceToCenter = distance(vec2(0.0), pos.xy);
        if (distanceToCenter > pointSize)
            discard;
    }

    vec4 color;
    vec2 gradientUV;
    switch (colorStyle) {
    case 0: // Uniform
        color = uColor.rgba;
        break;
    case 1: // Object gradient
        gradientUV = vec2((pos.y + 1.0) / 2.0, 0.0);
        color = texture(custex, gradientUV);
        break;
    case 2: //  Range gradient
        vec2 gradientUV = vec2(((VAR_WORLD_POSITION.y + rootScale) / 2.0) / rootScale, 0.0);
        color = texture(custex, gradientUV);
        if (!usePoint)
            color = vColor;
        break;
    }
    diffuse = color;
    BASE_COLOR = diffuse;
}

void AMBIENT_LIGHT()
{
    DIFFUSE += diffuse.rgb * TOTAL_AMBIENT_COLOR * ambientBrightness;
}

void DIRECTIONAL_LIGHT()
{
    DIFFUSE += diffuse.rgb * directionalBrightness * LIGHT_COLOR * SHADOW_CONTRIB
            * vec3(max(0.0, dot(normalize(NORMAL), TO_LIGHT_DIR)));
}

void POST_PROCESS()
{
    if (shaded)
        COLOR_SUM = vec4(DIFFUSE.rgb + SPECULAR + EMISSIVE, DIFFUSE.a);
    else
        COLOR_SUM = diffuse;
}
