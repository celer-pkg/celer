#version 440

layout(location = 0) in vec2 texCoord0;
layout(location = 1) in vec2 texCoord1;
layout(location = 2) in vec2 texCoord2;
layout(location = 3) in vec2 texCoord3;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec2 offset;
};

layout(binding = 1) uniform sampler2D src;

void main() {
    vec4 sourceColor = (texture(src, texCoord0) + texture(src, texCoord1) +
                        texture(src, texCoord2) + texture(src, texCoord3)) * 0.25;
    fragColor = sourceColor * qt_Opacity;
}
