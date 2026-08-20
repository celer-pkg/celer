
/* This struct holds all the information about the Ray.
   - rayViewStart, rayViewEnd: Holds the position of the ray in View Space.
   - rayFragStart, rayFragEnd, rayFragCurr: Holds the fragment position in Screen Space.
   - rayFragStartDepth, rayFragEndDepth: Holds the ray depth in Screen Space.
   - rayCoveredPart: Holds how much of the ray is covered during marching. It is a value between 0 and 1.
   - hit: Indicates if the ray hit an object or not during marching.
   - objHitViewPos: The possible position of an object that the ray might hit. */
struct RayData{
    vec3 rayViewStart, rayViewEnd;
    vec2 rayFragStart, rayFragEnd, rayFragCurr;
    float rayFragStartDepth, rayFragEndDepth;
    float rayCoveredPart;
    int hit;
    vec3 objHitViewPos;
} rayData;

/* This function takes the pixel position in the viewport of (WIDTH, HEIGHT) and returns the
   frag position in view space */
vec3 viewPosFromScreen(vec2 fragPos, vec2 size);
float rayViewDepthFromScreen(vec2 size);
/* This function takes texture coordinates and change the y componenent according to the selected graphics API */
vec2 correctTextureCoordinates(vec2 uv);
/* This function checks if the ray fragment position is outside the screen or not */
bool rayFragOutOfBound(vec2 rayFrag, vec2 size);
/* This function follows the ray in screen space and tries to find if the ray hit an object. Each iteration the ray fragment position is incremented by
   the rayStepVector and the ray fragment position is converted to view space to check if there is an object at this position or not. */
void rayMarch(vec2 rayStepVector, vec2 size);
/* This function is called by rayMarch when the ray hit an object. It tries to find the exact location where the hit happened */
void refinementStep(vec2 rayStepVector, vec2 size);


void MAIN()
{
    vec2 size = vec2(textureSize(SCREEN_TEXTURE, 0));

    /* Calculate the reflected vector of the fragment view space position around the normal.
       The reflected vector is calculated in World Space then get converted to View Space.
       Calculations are not done directly in View Space to avoid using the inverse function
       to transform the normal. */
    vec3 unitPos = normalize(-VIEW_VECTOR);
    vec3 unitNormal = normalize(VAR_WORLD_NORMAL);
    vec3 reflected = reflect(unitPos, unitNormal);
    reflected = vec4(VIEW_MATRIX * vec4(reflected, 0.0)).xyz;
    reflected = normalize(reflected);

    /* Convert the fragment position from World Space to View Space */
    vec3 fragViewPos = vec4(VIEW_MATRIX * vec4(VAR_WORLD_POSITION , 1.0)).xyz;

    /* Calculate the starting and ending point of the reflected ray in View Space */
    rayData.rayViewStart = fragViewPos;
    rayData.rayViewEnd = fragViewPos + (reflected * rayMaxDistance);

    /* Convert the start position from view space to Screen Space in the size of the viewport (WIDTH, HEIGHT) */
    vec4 rayClipStart = PROJECTION_MATRIX * vec4(rayData.rayViewStart, 1.0);
    rayClipStart /= rayClipStart.w;
    rayData.rayFragStart = rayClipStart.xy * 0.5 + 0.5;
    rayData.rayFragStart *= size;
    rayData.rayFragStartDepth = rayClipStart.z;

    /* Convert the end position from View Space to Screen Space in the size of the viewport (WIDTH, HEIGHT) */
    vec4 rayClipEnd = PROJECTION_MATRIX * vec4(rayData.rayViewEnd, 1.0);
    rayClipEnd /= rayClipEnd.w;
    rayData.rayFragEnd = rayClipEnd.xy * 0.5 + 0.5;
    rayData.rayFragEnd *= size;
    rayData.rayFragEndDepth = rayClipEnd.z;

    /* Calculate the difference between the start and end fragment */
    vec2 diff = rayData.rayFragEnd - rayData.rayFragStart;

    /* Calculate the value of each step. The step depends on the marchSteps property set from the QML.
       Increasing marchSteps will result in a better quality but affects performance */
    vec2 rayStepVector = diff / max(marchSteps, 1);

    /* Start ray marching */
    rayData.rayFragCurr = rayData.rayFragStart;
    rayData.hit = 0;
    rayMarch(rayStepVector, size);

    /* If the resulting fragment Screen Space position is outside the screen return the material color. */
    bool isOutOfBound = rayFragOutOfBound(rayData.rayFragCurr, size);
    if(isOutOfBound)
    {
        BASE_COLOR = materialColor;
        return;
    }

    //! [visibilitycheck]
    float visibility = rayData.hit;
    /* Check if the ray hit an object behind the camera. This means information about the object can not be obtained from SCREEN_TEXTURE.
       Start fading the visibility according to how much the reflected ray is moving toward the opposite direction of the camera */
    visibility *= (1 - max(dot(-normalize(fragViewPos), reflected), 0));

    /* Fade out visibility according how far is the hit object from the fragment */
    visibility *= (1 - clamp(length(rayData.objHitViewPos - rayData.rayViewStart) / rayMaxDistance, 0, 1));
    visibility = clamp(visibility, 0, 1);
    //! [visibilitycheck]

    /* Calculate the reflection color from the SCREEN_TEXTURE */
    //! [reflectioncolor]
    vec2 uv = rayData.rayFragCurr / size;
    uv = correctTextureCoordinates(uv);
    vec3 reflectionColor = texture(SCREEN_TEXTURE, uv).rgb;
    reflectionColor *= specular;

    vec3 mixedColor = mix(materialColor.rgb, reflectionColor, visibility);
    BASE_COLOR = vec4(mixedColor, materialColor.a);
    //! [reflectioncolor]
}

//! [viewposfromscreen]
vec3 viewPosFromScreen(vec2 fragPos, vec2 size)
{
    vec2 uv = fragPos / size;
    vec2 texuv = correctTextureCoordinates(uv);

    float depth = textureLod(DEPTH_TEXTURE, texuv, 0).r;
    if(NEAR_CLIP_VALUE  < 0.0)
        depth = 2 * depth - 1.0;

    vec3 ndc = vec3(2 * uv - 1, depth);
    vec4 viewPos = INVERSE_PROJECTION_MATRIX * vec4(ndc, 1.0);
    viewPos /= viewPos.w;
    return viewPos.xyz;
}
//! [viewposfromscreen]

//! [rayviewposfromscreen]
float rayViewDepthFromScreen(vec2 size)
{
    vec2 uv = rayData.rayFragCurr / size;
    float depth = mix(rayData.rayFragStartDepth, rayData.rayFragEndDepth, rayData.rayCoveredPart);
    vec3 ndc = vec3(2 * uv - 1, depth);
    vec4 viewPos = INVERSE_PROJECTION_MATRIX * vec4(ndc, 1.0);
    viewPos /= viewPos.w;
    return viewPos.z;
}
//! [rayviewposfromscreen]

//! [rayoutofbound]
bool rayFragOutOfBound(vec2 rayFrag, vec2 size)
{
    if(rayFrag.x > size.x || rayFrag.y > size.y)
        return true;

    if(rayFrag.x < 0 || rayFrag.y < 0)
        return true;

    return false;
}
//! [rayoutofbound]

//! [refinementStep]
void refinementStep(vec2 rayStepVector, vec2 size)
{
    for(int i = 0; i < refinementSteps; i++)
    {
        rayData.rayCoveredPart = length(rayData.rayFragCurr - rayData.rayFragStart) / length(rayData.rayFragEnd - rayData.rayFragStart);
        rayData.rayCoveredPart = clamp(rayData.rayCoveredPart, 0.0, 1.0);
        float rayDepth = rayViewDepthFromScreen(size);
        rayData.objHitViewPos = viewPosFromScreen(rayData.rayFragCurr, size);
        float deltaDepth = rayDepth - rayData.objHitViewPos.z;

        rayStepVector *= 0.5;
        if(deltaDepth > 0 && deltaDepth < depthBias)
            rayData.rayFragCurr -= rayStepVector;
        else
            rayData.rayFragCurr += rayStepVector;
    }
}
//! [refinementStep]

//! [rayMarch]
void rayMarch(vec2 rayStepVector, vec2 size)
{
    for(int i = 0; i < marchSteps; i++)
    {
        rayData.rayFragCurr += rayStepVector;
        rayData.rayCoveredPart = length(rayData.rayFragCurr - rayData.rayFragStart) / length(rayData.rayFragEnd - rayData.rayFragStart);
        rayData.rayCoveredPart = clamp(rayData.rayCoveredPart, 0.0, 1.0);
        float rayDepth = rayViewDepthFromScreen(size);
        rayData.objHitViewPos = viewPosFromScreen(rayData.rayFragCurr, size);
        float deltaDepth = rayDepth - rayData.objHitViewPos.z;

        if(deltaDepth > 0 && deltaDepth < depthBias)
        {
            rayData.hit = 1;
            refinementStep(rayStepVector, size);
            return;
        }
    }
}
//! [rayMarch]

//! [correctTexture]
vec2 correctTextureCoordinates(vec2 uv)
{
    if(FRAMEBUFFER_Y_UP < 0 && NDC_Y_UP == 1)
        uv.y = 1 - uv.y;

    return uv;
}
//! [correctTexture]


