#version 440

layout(vertices = 3) out;

layout(location = 0) in vec3 inColor[];

layout(location = 0) patch out vec3 stuff;
layout(location = 1) patch out float more_stuff;

void main()
{
    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = 4.0;
        gl_TessLevelOuter[1] = 4.0;
        gl_TessLevelOuter[2] = 4.0;

        gl_TessLevelInner[0] = 4.0;
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    stuff = vec3(1.0);
    more_stuff = stuff.x + 0.5;
}
