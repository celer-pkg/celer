VARYING vec3 varViewVec;

void MAIN()
{
    FRAGCOLOR = vec4(normalize(varViewVec), 1.0);
}
