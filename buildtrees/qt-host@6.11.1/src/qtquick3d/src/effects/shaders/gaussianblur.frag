VARYING vec2 TexCoord0;
VARYING vec2 TexCoord1;
VARYING vec2 TexCoord2;
VARYING vec2 TexCoord3;
VARYING vec2 TexCoord4;
VARYING vec2 TexCoord5;
VARYING vec2 TexCoord6;

#define WT7_0 20.0
#define WT7_1 15.0
#define WT7_2 6.0
#define WT7_3 1.0
#define WT7_NORMALIZE (WT7_0+2.0*(WT7_1+WT7_2+WT7_3))

vec4 gaussianBlur(sampler2D inSampler)
{
    vec4 OutCol = vec4(0.0);
    OutCol += texture(inSampler, TexCoord0) * ( WT7_1/WT7_NORMALIZE );
    OutCol += texture(inSampler, TexCoord1) * ( WT7_2/WT7_NORMALIZE );
    OutCol += texture(inSampler, TexCoord2) * ( WT7_3/WT7_NORMALIZE );
    OutCol += texture(inSampler, TexCoord3) * ( WT7_0/WT7_NORMALIZE );
    OutCol += texture(inSampler, TexCoord4) * ( WT7_1/WT7_NORMALIZE );
    OutCol += texture(inSampler, TexCoord5) * ( WT7_2/WT7_NORMALIZE );
    OutCol += texture(inSampler, TexCoord6) * ( WT7_3/WT7_NORMALIZE );
    return OutCol;
}

void MAIN()
{
    FRAGCOLOR = gaussianBlur(INPUT);
}
