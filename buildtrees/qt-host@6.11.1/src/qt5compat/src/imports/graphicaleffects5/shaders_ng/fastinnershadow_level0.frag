#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    vec4 color;
    float horizontalOffset;
    float verticalOffset;
};
layout(binding = 1) uniform sampler2D source;

void main() {
    vec2 pos = qt_TexCoord0 - vec2(horizontalOffset, verticalOffset);
    float ea = step(0.0, pos.x) * step(0.0, pos.y) * step(pos.x, 1.0) * step(pos.y, 1.0);
    float eb = 1.0 - ea;
    fragColor = (eb * color + ea * color * (1.0 - texture(source, pos).a)) * qt_Opacity;
}
