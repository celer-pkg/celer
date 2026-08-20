VARYING vec2 TexCoord0;
VARYING vec2 TexCoord1;
VARYING vec2 TexCoord2;
VARYING vec2 TexCoord3;

float advancedGetTiltShiftMultiplier(vec2 inTexCoord, float inFocusBarHeight, float inFocusWidth,
                                     bool inVertical, bool inInvert)
{
    float texPos = inVertical ? inTexCoord.x : inTexCoord.y;
    float focusDiff = max(0.0, abs(texPos - inFocusBarHeight) - (inFocusWidth / 2.0))
            / inFocusWidth;
    float retval = clamp(focusDiff, 0.0, 1.0);
    return inInvert ? 1.0 - retval : retval;
}

vec4 advancedBoxTiltShiftBlur(sampler2D inBlurSampler,
                              float inFocusBarHeight, float inFocusWidth,
                              bool inVertical, bool inInvert)
{
    float mult0 = .25 * advancedGetTiltShiftMultiplier(TexCoord0, inFocusBarHeight, inFocusWidth,
                                                       inVertical, inInvert);
    float mult1 = .25 * advancedGetTiltShiftMultiplier(TexCoord1, inFocusBarHeight, inFocusWidth,
                                                       inVertical, inInvert);
    float mult2 = .25 * advancedGetTiltShiftMultiplier(TexCoord2, inFocusBarHeight, inFocusWidth,
                                                       inVertical, inInvert);
    float mult3 = .25 * advancedGetTiltShiftMultiplier(TexCoord3, inFocusBarHeight, inFocusWidth,
                                                       inVertical, inInvert);
    float multTotal = mult0 + mult1 + mult2 + mult3;
    float totalDivisor = multTotal != 0.0 ? 1.0 / multTotal : 0.0;
    vec4 outCol = texture(inBlurSampler, TexCoord0) * mult0;
    outCol += texture(inBlurSampler, TexCoord1) * mult1;
    outCol += texture(inBlurSampler, TexCoord2) * mult2;
    outCol += texture(inBlurSampler, TexCoord3) * mult3;
    return outCol * totalDivisor;
}

void MAIN()
{
    FRAGCOLOR = advancedBoxTiltShiftBlur(INPUT, focusPosition, focusWidth, isVertical, isInverted);
}
