#version 440

layout(location = 0) in vec3 position;
layout(location = 0) out vec4 v_position;

void main()
{
    v_position = vec4(position, 1.0);
    gl_Position = v_position;
}
