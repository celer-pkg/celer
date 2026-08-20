VARYING vec2 TexCoord0;
VARYING vec2 TexCoord1;
VARYING vec2 TexCoord2;

const vec3 poisson0 = vec3( 0.000000, 0.000000, 0.000000 );
const vec3 poisson1 = vec3( 0.527837, -0.085868, 0.534776 );
const vec3 poisson2 = vec3( -0.040088, 0.537087, 0.538581 );

float getDepthValue( vec4 depth_texture_sample, vec2 cameraProperties )
{
    float zNear = cameraProperties.x;
    float zFar = cameraProperties.y;
    float zRange = zFar - zNear;
    float z_b = depth_texture_sample.x;
    float z_n = 2.0 * z_b - 1.0;
    float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zRange));
    return 1.0 - ((z_e - cameraProperties.x) / (zRange));
}

float depthValueToLinearDistance( float depth_value, vec2 cameraProperties )
{
    float FarClipDistance = cameraProperties.y;
    float NearClipDistance = cameraProperties.x;
    float DepthRange = FarClipDistance - NearClipDistance;
    float linearDepth = NearClipDistance + (DepthRange * (1.0 - depth_value));
    return linearDepth;
}

float getDepthMultiplier(vec2 inTexCoord, sampler2D inDepthSampler, float inFocusDistance, float inFocusWidth, float inFocusPenumbra)
{
    float depthTap = getDepthValue( texture(inDepthSampler, inTexCoord), CAMERA_PROPERTIES );
    float linearDepth = depthValueToLinearDistance( depthTap, CAMERA_PROPERTIES );
    float depthDiff = max( 0.0, abs( linearDepth - inFocusDistance ) - (inFocusWidth/2.0) ) / inFocusPenumbra;
    return clamp( depthDiff, 0.0, 1.0 );
}

vec4 poissonDepthBlur(sampler2D inSampler,sampler2D inDepthSampler,
                      float inFocusDistance, float inFocusWidth, float inFocusPenumbra )
{
    float mult0 = (1.0 - poisson0.z) * getDepthMultiplier(TexCoord0, inDepthSampler, inFocusDistance, inFocusWidth, inFocusPenumbra);
    float mult1 = (1.0 - poisson1.z) * getDepthMultiplier(TexCoord1, inDepthSampler, inFocusDistance, inFocusWidth, inFocusPenumbra);
    float mult2 = (1.0 - poisson2.z) * getDepthMultiplier(TexCoord2, inDepthSampler, inFocusDistance, inFocusWidth, inFocusPenumbra);

    float multTotal = mult0 + mult1 + mult2;
    float multMultiplier = multTotal > 0.0 ? 1.0 / multTotal : 0.0;

    vec4 outColor = texture( inSampler, TexCoord0 ) * (mult0 * multMultiplier);
    outColor += texture( inSampler, TexCoord1 ) * (mult1 * multMultiplier);
    outColor += texture( inSampler, TexCoord2 ) * (mult2 * multMultiplier);
    return outColor;
}

void MAIN()
{
    float centerMultiplier = getDepthMultiplier(INPUT_UV, DEPTH_TEXTURE, focusDistance, focusRange, focusRange);
    vec4 blurColor = poissonDepthBlur(INPUT, DEPTH_TEXTURE, focusDistance, focusRange, focusRange);
    FRAGCOLOR = mix(texture(sourceSampler, INPUT_UV), blurColor, centerMultiplier);
}
