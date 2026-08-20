#version 440

layout(location = 0) in vec4 qt_Vertex;
layout(location = 1) in vec2 qt_MultiTexCoord0;
layout(location = 0) out vec2 qt_TexCoord0;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float expandX;
    float expandY;
};

out gl_PerVertex { vec4 gl_Position; };

void main() {
    vec2 texCoord = qt_MultiTexCoord0;
    texCoord.s = (texCoord.s - expandX) / (1.0 - 2.0 * expandX);
    texCoord.t = (texCoord.t - expandY) / (1.0 - 2.0 * expandY);
    qt_TexCoord0 = texCoord;
    gl_Position = qt_Matrix * qt_Vertex;
}
