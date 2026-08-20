VARYING vec2 TexCoord0;
VARYING vec2 TexCoord1;
VARYING vec2 TexCoord2;

#define WT3_0 1.0
#define WT3_1 0.6
#define WT3_NORMALIZE (WT3_0+2.0*(WT3_1))

vec4 gaussianBlur3TapPremultiplied( sampler2D inSampler )
{
    vec4 OutCol = vec4(0.0);
    OutCol += texture(inSampler, TexCoord0) * ( WT3_1/WT3_NORMALIZE );
    OutCol += texture(inSampler, TexCoord1) * ( WT3_0/WT3_NORMALIZE );
    OutCol += texture(inSampler, TexCoord2) * ( WT3_1/WT3_NORMALIZE );
    return OutCol;
}

void MAIN()
{
    FRAGCOLOR = gaussianBlur3TapPremultiplied(INPUT);
}
