// Luma coefficients according to ITU-R Recommendation BT.709 (http://en.wikipedia.org/wiki/Rec._709)
const vec3 qt_yCoeff_709 = vec3(0.2126, 0.7152, 0.0722);

float qt_luminance(in vec3 v )
{
    return dot(v, qt_yCoeff_709);
}

vec3 RGBToYPbPr( in vec3 v )
{
    vec3 ypp;
    ypp.x = qt_luminance( v );
    ypp.y = 0.5 * (v.b - ypp.x) / (1.0 - qt_yCoeff_709.b);
    ypp.z = 0.5 * (v.r - ypp.x) / (1.0 - qt_yCoeff_709.r);

    return ypp;
}

vec3 YPbPrToRGB( in vec3 v )
{
    vec3 outRGB;
    outRGB.x = dot(vec3(1.0, 0.0, 1.575), v);
    outRGB.y = dot(vec3(1.0, -0.187, -0.468), v);
    outRGB.z = dot(vec3(1.0, 1.856, 0.0), v);

    return outRGB;
}

float remapLum( float inLum, float whitePt )
{
    return clamp( inLum / whitePt, 0.0, 1.0 );
}

float exposeLum( float inLum, float exposure )
{
    return 1.0 - exp2( -inLum / exposure );
}

vec3 gammaCorrect( vec3 inColor, float gammaExp )
{
    return pow( inColor, vec3( 1.0 / gammaExp ) );
}

vec3 adjSaturation( vec3 inRGB, float satFactor )
{
    // Must be done in linear space (before gamma correction)
    float P = sqrt( qt_luminance( inRGB * inRGB ) );
    vec3 outCol;
    outCol = (inRGB - vec3(P)) * satFactor;
    outCol += vec3(P);
    return outCol;
}

float curveCompute( float inLum, float slope0, float slope1 )
{
    float a1 = slope0;
    float a2 = 3.0 - 2.0 * slope0 - slope1;
    float a3 = slope1 + slope0 - 2.0;

    // Cubic curve fit.  This results in a curve that is 0 where inColor is 0
    // equals 1 when inColor is 1, and the derivative at 0 is slope0 and the
    // derivative at 1 is slope1
    return ((((a3 * inLum) + a2)*inLum) + a1)*inLum;
}

float toeEmphasize( float inParam )
{
    float a1 = 1.0 - toeEmphasis;
    float a2 = 2.0 * toeEmphasis;
    float a3 = -toeEmphasis;

    return ((((a3 * inParam) + a2) * inParam) + a1) * inParam;
}

float shoulderEmphasize( float inParam )
{
    float a1 = 1.0;
    float a2 = shoulderEmphasis;
    float a3 = -shoulderEmphasis;

    return ((((a3 * inParam) + a2) * inParam) + a1) * inParam;
}

void MAIN()
{
    // k = shadow slope, m = midtone slope, n = highlight slope
    float k = toeSlope;
    float m = 1.0 + contrastBoost;
    float n = shoulderSlope;

    vec4 sourceColor = texture(INPUT, INPUT_UV);
    vec3 sourceSep = RGBToYPbPr(sourceColor.rgb);

    float lum = sourceSep.r;

    if (useExposure)
        lum = exposeLum( lum, exposureValue );
    else
        lum = remapLum( lum, whitePoint );

    float param0 = toeEmphasize( 2.0 * lum );               // Parametrization for Curve Part 1
    float param1 = shoulderEmphasize( 2.0 * lum - 1.0 );    // Parametrization for Curve Part 2

    float lum0 = 0.5 * curveCompute( param0, k, m );
    float lum1 = 0.5 * curveCompute( param1, m, n ) + 0.5;
    sourceSep.r = (lum > 0.5) ? lum1 : lum0;

    // Convert back to RGB and gamma correct
    vec3 finalColor = YPbPrToRGB( sourceSep );
    finalColor = gammaCorrect( adjSaturation( finalColor, saturationLevel ), gammaValue );
    FRAGCOLOR = vec4( finalColor, sourceColor.a );
}
