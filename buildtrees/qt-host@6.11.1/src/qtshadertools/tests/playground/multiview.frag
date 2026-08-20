#version 440

layout(location = 0) out vec4 fragColor;

void main()
{
    // qsb on a fragment shader with --view-count should still work, even if it
    // does not actually enable GL_EXT_multiview. (mainly due to OpenGL /
    // SPIRV-Cross (?) issues due to only supporting gl_ViewIndex and co. in
    // vertex shaders) It defines QSHADER_VIEW_COUNT however.
#if QSHADER_VIEW_COUNT < 2
    error, needs --view-count=2, or something is broken
#endif
    // Should this (using gl_ViewIndex) work or not?
    //fragColor = vec4(gl_ViewIndex, 0.0, 0.0, 1.0);
    fragColor = vec4(1.0);
}
