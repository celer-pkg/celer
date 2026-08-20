void MAIN()
{
    SPECULAR_AMOUNT = 0.0;
    vec3 normalValue = texture(normalMap, UV0).rgb;
    normalValue.xy = normalValue.xy * 2.0 - 1.0;
    normalValue.z = sqrt(max(0.0, 1.0 - dot(normalValue.xy, normalValue.xy)));
    NORMAL = normalize(mix(NORMAL, TANGENT * normalValue.x + BINORMAL * normalValue.y + NORMAL * normalValue.z, 1.0));
}
