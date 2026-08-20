void MAIN()
{
    // the resulting color does not matter, what matters it that the shader compiles at all
    mat4 m = MODELVIEWPROJECTION_MATRIX;
    m *= VIEWPROJECTION_MATRIX;
    m *= MODEL_MATRIX;
    m *= VIEW_MATRIX;
    m *= mat4(NORMAL_MATRIX);
    m *= PROJECTION_MATRIX;
    m *= INVERSE_PROJECTION_MATRIX;
    m[0].xyz *= CAMERA_POSITION;
    m[1].xyz *= CAMERA_DIRECTION;
    m[2].xy *= CAMERA_PROPERTIES;
    BASE_COLOR = m[0] * m[1] * m[2] * m[3];
}
