#if QSHADER_VIEW_COUNT >= 2
#define SAMPLE_INPUT(uv) texture(INPUT, vec3(uv, VIEW_INDEX))
#else
#define SAMPLE_INPUT(uv) texture(INPUT, uv)
#endif

void MAIN()
{
    FRAGCOLOR = SAMPLE_INPUT(INPUT_UV);
}
