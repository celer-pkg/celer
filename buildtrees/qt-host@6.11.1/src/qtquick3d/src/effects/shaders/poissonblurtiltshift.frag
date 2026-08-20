VARYING vec2 TexCoord0;
VARYING vec2 TexCoord1;
VARYING vec2 TexCoord2;
VARYING vec2 TexCoord3;
VARYING vec2 TexCoord4;

const vec3 poisson0 = vec3( 0.000000, 0.000000, 0.000000 );
const vec3 poisson1 = vec3( 0.527837, -0.085868, 0.534776 );
const vec3 poisson2 = vec3( -0.040088, 0.537087, 0.538581 );
const vec3 poisson3 = vec3( -0.670445, -0.017995, 0.670686 );
const vec3 poisson4 = vec3( -0.419418, -0.616039, 0.745262 );

float advancedGetTiltShiftMultiplier(vec2 inTexCoord, float inFocusBarHeight, float inFocusWidth,
                                     bool inVertical, bool inInvert)
{
    float texPos = inVertical ? inTexCoord.x : inTexCoord.y;
    float focusDiff = max(0.0, abs(texPos - inFocusBarHeight) - (inFocusWidth / 2.0))
            / inFocusWidth;
    float retval = clamp(focusDiff, 0.0, 1.0);
    return inInvert ? 1.0 - retval : retval;
}

vec4 advancedPoissonTiltShiftBlur(sampler2D inSampler, float inBarHeight,
                                  float inFocusWidth, bool inVertical, bool inInvert)
{
    float mult0 = (1.0 - poisson0.z) * advancedGetTiltShiftMultiplier(TexCoord0, inBarHeight,
                                                                      inFocusWidth, inVertical,
                                                                      inInvert);
    float mult1 = (1.0 - poisson1.z) * advancedGetTiltShiftMultiplier(TexCoord1, inBarHeight,
                                                                      inFocusWidth, inVertical,
                                                                      inInvert);
    float mult2 = (1.0 - poisson2.z) * advancedGetTiltShiftMultiplier(TexCoord2, inBarHeight,
                                                                      inFocusWidth, inVertical,
                                                                      inInvert);
    float mult3 = (1.0 - poisson3.z) * advancedGetTiltShiftMultiplier(TexCoord3, inBarHeight,
                                                                      inFocusWidth, inVertical,
                                                                      inInvert);
    float mult4 = (1.0 - poisson4.z) * advancedGetTiltShiftMultiplier(TexCoord4, inBarHeight,
                                                                      inFocusWidth, inVertical,
                                                                      inInvert);

    float multTotal = mult0 + mult1 + mult2 + mult3 + mult4;
    float multMultiplier = multTotal > 0.0 ? 1.0 / multTotal : 0.0;

    vec4 outColor = texture(inSampler, TexCoord0) * (mult0 * multMultiplier);
    outColor += texture(inSampler, TexCoord1) * (mult1 * multMultiplier);
    outColor += texture(inSampler, TexCoord2) * (mult2 * multMultiplier);
    outColor += texture(inSampler, TexCoord3) * (mult3 * multMultiplier);
    outColor += texture(inSampler, TexCoord4) * (mult4 * multMultiplier);
    return outColor;
}

void MAIN()
{
    float centerMultiplier = advancedGetTiltShiftMultiplier(INPUT_UV, focusPosition, focusWidth, isVertical, isInverted);
    vec4 blurColor = advancedPoissonTiltShiftBlur(INPUT, focusPosition, focusWidth, isVertical, isInverted);
    FRAGCOLOR = mix(texture(sourceSampler, INPUT_UV), blurColor, centerMultiplier);
}
