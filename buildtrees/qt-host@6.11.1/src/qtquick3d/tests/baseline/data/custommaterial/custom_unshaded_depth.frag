void MAIN()
{
    ivec2 iSize = textureSize(DEPTH_TEXTURE, 0);
    vec2 smpUV = (gl_FragCoord.xy) / vec2(iSize);
    vec4 depthSample = texture(DEPTH_TEXTURE, smpUV);

    float zNear = CAMERA_PROPERTIES.x;
    float zFar = CAMERA_PROPERTIES.y;
    float zRange = zFar - zNear;

    float z_n = 2.0 * depthSample.r - 1.0;
    float d = 2.0 * zNear * zFar / (zFar + zNear - z_n * zRange);
    d /= zFar;

    FRAGCOLOR = vec4(d, d, d, 1.0);
}
