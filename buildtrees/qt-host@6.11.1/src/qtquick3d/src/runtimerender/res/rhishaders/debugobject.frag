#version 440

layout(location = 0) out vec4 fragOutput;

layout(location = 0) in vec3 var_color;

void main()
{
    fragOutput = vec4(var_color, 1.0);
}
