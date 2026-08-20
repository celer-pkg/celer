VARYING vec4 vCustomData;
VARYING vec3 vGlobalPosition;

void MAIN()
{
    METALNESS = 0.0;
    ROUGHNESS = 1.0;
    FRESNEL_POWER = 5.0;

    float c;

    if (vCustomData.x > 0)
        c = 1.0 - (1.0 + sin(sqrt(vGlobalPosition.x*vGlobalPosition.x + vGlobalPosition.z*vGlobalPosition.z) - uTime/200.0)) * 0.2;
    else
        c = 1.0 - 0.25 * (UV0.x*UV0.x + UV0.y*UV0.y);

    BASE_COLOR = vec4(c, c, c, 1.0);
}
