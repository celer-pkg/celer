#version 440
#extension GL_EXT_multiview : require

// For HLSL, Shader Model 6.1 or higher is needed to transpile this.
//
// For OpenGL we'd need layout(num_views = 2) in; but that's not a thing with
// Vulkan so will rather rely on QShaderBaker/qsb passing the view count to
// SPIRV-Cross which then inserts the num_views line to the OpenGL-targeted GLSL
// code. To do this, run qsb with --view-count 2
//
// Metal is currently unknown. It probably needs some additional options in the
// shader pipeline. Running with --msl 12 succeeds but the generated MSL code is
// not multiview-capable. To be investigated.

layout(location = 0) in vec4 pos;

layout(std140, binding = 0) uniform buf
{
    mat4 mvp[2];
};

void main()
{
    gl_Position = mvp[gl_ViewIndex] * pos;
}
