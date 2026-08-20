#define SAMPLE_NORMAL(uv) normalize(texture(NORMAL_ROUGHNESS_TEXTURE, uv).rgb)

void MAIN()
{
    FRAGCOLOR = vec4(SAMPLE_NORMAL(INPUT_UV), 1.0);
}
