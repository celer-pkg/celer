VARYING vec2 TexCoord0;
VARYING vec2 TexCoord1;
VARYING vec2 TexCoord2;
VARYING vec2 TexCoord3;
VARYING vec2 TexCoord4;
VARYING vec2 TexCoord5;
VARYING vec2 TexCoord6;
VARYING vec2 TexCoord7;

const vec3 poisson0 = vec3( 0.000000, 0.000000, 0.000000 );
const vec3 poisson1 = vec3( 0.527837, -0.085868, 0.534776 );
const vec3 poisson2 = vec3( -0.040088, 0.537087, 0.538581 );
const vec3 poisson3 = vec3( -0.670445, -0.017995, 0.670686 );
const vec3 poisson4 = vec3( -0.419418, -0.616039, 0.745262 );
const vec3 poisson5 = vec3( 0.440453, -0.639399, 0.776421 );
const vec3 poisson6 = vec3( -0.757088, 0.349334, 0.833796 );
const vec3 poisson7 = vec3( 0.574619, 0.685879, 0.894772 );

vec4 poissonBlur(sampler2D inSampler)
{
    float mult0 = (1.0 - poisson0.z);
    float mult1 = (1.0 - poisson1.z);
    float mult2 = (1.0 - poisson2.z);
    float mult3 = (1.0 - poisson3.z);
    float mult4 = (1.0 - poisson4.z);
    float mult5 = (1.0 - poisson5.z);
    float mult6 = (1.0 - poisson6.z);
    float mult7 = (1.0 - poisson7.z);

    float multTotal = mult0 + mult1 + mult2 + mult3 + mult4 + mult5 + mult6 + mult7;
    float multMultiplier = (multTotal > 0.0 ? 1.0 / multTotal : 0.0) * negativeBlurFalloffExp2;

    vec4 outColor = texture(inSampler, TexCoord0, 1.0) * (mult0 * multMultiplier);
    outColor += texture(inSampler, TexCoord1, 1.0) * (mult1 * multMultiplier);
    outColor += texture(inSampler, TexCoord2, 1.0) * (mult2 * multMultiplier);
    outColor += texture(inSampler, TexCoord3, 1.0) * (mult3 * multMultiplier);
    outColor += texture(inSampler, TexCoord4, 1.0) * (mult4 * multMultiplier);
    outColor += texture(inSampler, TexCoord5, 1.0) * (mult5 * multMultiplier);
    outColor += texture(inSampler, TexCoord6, 1.0) * (mult6 * multMultiplier);
    outColor += texture(inSampler, TexCoord7, 1.0) * (mult7 * multMultiplier);
    return outColor;
}

void MAIN()
{
    FRAGCOLOR = poissonBlur(INPUT);
}
