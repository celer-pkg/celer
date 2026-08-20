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

void MAIN()
{
    vec4 depthSample = texture(DEPTH_TEXTURE, INPUT_UV);
    float depthVal = getDepthValue(depthSample, CAMERA_PROPERTIES);
    float rawDepth = depthValueToLinearDistance(depthVal, CAMERA_PROPERTIES);

    float depthScale = abs(CAMERA_PROPERTIES.y - CAMERA_PROPERTIES.x);
    float depthDisp = abs(rawDepth - focusDepth) / depthScale;
    float finalDisperse = aberrationAmount * depthDisp;
    float effectAmt = texture(maskTexture, INPUT_UV).x;

    FRAGCOLOR = texture(INPUT, INPUT_UV);

    vec2 dispDir = normalize(INPUT_UV.xy - vec2(0.5)) / (2.0 * INPUT_SIZE);
    vec3 mixColor;
    mixColor = FRAGCOLOR.rgb;
    mixColor.r = texture(INPUT, INPUT_UV + dispDir * finalDisperse).r;
    mixColor.b = texture(INPUT, INPUT_UV - dispDir * finalDisperse).b;

    FRAGCOLOR.rgb = mix(FRAGCOLOR.rgb, mixColor, effectAmt);
}
