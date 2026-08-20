VARYING vec4 vCustomData;

void MAIN()
{
    METALNESS = vCustomData.x;
    ROUGHNESS = vCustomData.y;
    FRESNEL_POWER = vCustomData.z;
    SPECULAR_AMOUNT = vCustomData.w;
    BASE_COLOR = uDiffuse;
}
