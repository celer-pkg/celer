#version 440

layout(triangles, fractional_odd_spacing, ccw) in;

layout(location = 0) in vec3 inColor[];

layout(location = 1) patch in vec3 stuff;
layout(location = 2) patch in float more_stuff;

layout(location = 0) out vec3 outColor;

layout(std140, binding = 0) uniform buf {
    mat4 mvp;
    float time;
    float amplitude;
};

void main()
{
    gl_Position = mvp * ((gl_TessCoord.x * gl_in[0].gl_Position) + (gl_TessCoord.y * gl_in[1].gl_Position) + (gl_TessCoord.z * gl_in[2].gl_Position));
    gl_Position.x += sin(time + gl_Position.y) * amplitude * more_stuff * stuff.y;
    outColor = gl_TessCoord.x * inColor[0] + gl_TessCoord.y * inColor[1] + gl_TessCoord.z * inColor[2];
}
