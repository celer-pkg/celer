void MAIN()
{
    float r = texture(DEPTH_TEXTURE, INPUT_UV).r;

    float zNear = CAMERA_PROPERTIES.x;
    float zFar = CAMERA_PROPERTIES.y;
    float zRange = zFar - zNear;

    float z_n = 2.0 * r - 1.0;
    float d = 2.0 * zNear * zFar / (zFar + zNear - z_n * zRange);
    d /= zFar;

    FRAGCOLOR = vec4(d, d, d, 1.0);
}
