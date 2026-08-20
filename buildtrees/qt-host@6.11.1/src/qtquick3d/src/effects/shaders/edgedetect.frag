VARYING vec4 TexCoordBLL;
VARYING vec4 TexCoordTLT;
VARYING vec4 TexCoordTRR;
VARYING vec4 TexCoordBRB;

void MAIN()
{
    vec4 centerTap = texture(INPUT, INPUT_UV);
    vec4 edgeTap = texture(INPUT, TexCoordBLL.xy)
            + texture(INPUT, TexCoordBLL.zw)
            + texture(INPUT, TexCoordTLT.xy)
            + texture(INPUT, TexCoordTLT.zw)
            + texture(INPUT, TexCoordTRR.xy)
            + texture(INPUT, TexCoordTRR.zw)
            + texture(INPUT, TexCoordBRB.xy)
            + texture(INPUT, TexCoordBRB.zw);
    vec3 edgeDetect = 8.0 * (centerTap.rgb - (0.125 * edgeTap.rgb));
    edgeDetect = clamp(edgeDetect, 0.0, centerTap.a);

    FRAGCOLOR = vec4(mix(centerTap.rgb, edgeDetect, edgeStrength), centerTap.a);
}
