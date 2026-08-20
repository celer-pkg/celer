vec4 diffuse = vec4(0.0);
float ambientBrightness = 0.75; // 0...1.0
float directionalBrightness = 0.50; // 0...1.0
VARYING vec3 pos;
VARYING vec2 UV;
VARYING float overhigh;

void MAIN()
{
    if ((any(greaterThan(UV, vec2(1.01)))) && (!any(greaterThan(UV0, vec2(1.0)))))
        discard;

    if (abs(pos.y) > graphHeight)
        discard;

    if (texture(height, UV).a != 1.0)
        discard;

    bool sides = (any(greaterThan(UV0, vec2(1.0))) || any(lessThan(UV0, vec2(0.0))));
    if (sides && overhigh > 0)
        discard;

    vec4 color;
    vec2 gradientUV;
    switch (colorStyle) {
    case 0: //Object gradient
        gradientUV = vec2(gradientMin + pos.y * gradientHeight, 0.0);
        color = texture(custex, gradientUV);
        break;
    case 1: //Range gradient
        gradientUV = vec2(((VAR_WORLD_POSITION.y + rootScale) / 2.0) / rootScale, 0.0);
        color = texture(custex, gradientUV);
        break;
    case 2: // Uniform color
        color = uniformColor;
        break;
    }

    if (textured && !sides) {
        vec2 offsetNormalized = uvOffset * (1 / (vertCount - 1));
        vec2 texUV = UV0 + offsetNormalized;
        if (flipU)
            texUV.x = 1 - texUV.x;
        if (flipV)
            texUV.y = 1 - texUV.y;
        color = texture(baseColor, texUV);
    }

    // keep sides flat shaded always
    if (flatShading || sides || overhigh > 0) {
        if (lineData && !sides) {
            NORMAL = -CAMERA_DIRECTION;
        } else {
            vec3 dpdx = dFdx(VAR_WORLD_POSITION);
            vec3 dpdy = dFdy(VAR_WORLD_POSITION);
            vec3 n = normalize(cross(dpdy,dpdx));
            if (NEAR_CLIP_VALUE < 0.0) //effectively: if openGL
                n = normalize(cross(dpdx,dpdy));
            NORMAL = n;
        }
    }
    diffuse = vec4(color);
    BASE_COLOR = diffuse;
}

void AMBIENT_LIGHT()
{
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
