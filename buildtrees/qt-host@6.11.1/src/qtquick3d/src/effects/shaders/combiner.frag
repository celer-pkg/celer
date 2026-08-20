void MAIN()
{
    vec4 sourceColor = texture(INPUT, INPUT_UV);
    vec4 summation = texture(downsample2, INPUT_UV) + texture(downsample4, INPUT_UV);
    vec3 bloom_result = sourceColor.xyz * exposureExp2 + summation.xyz;
    vec3 thresholded = clamp(bloom_result.xyz, 0.0, channelThreshold) / channelThreshold;
    vec3 tonemapped =  pow(thresholded, vec3(1.0 / gamma));
    vec3 final_color = mix(thresholded, tonemapped, tonemappingLerp);
    float resultAlpha = max(summation.a, sourceColor.a);
    FRAGCOLOR = vec4(final_color, resultAlpha);
}
