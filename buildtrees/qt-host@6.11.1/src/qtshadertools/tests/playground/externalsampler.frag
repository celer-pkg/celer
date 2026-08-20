#version 440

layout(location = 0) in vec2 textureCoords;
layout(location = 0) out vec4 fragColor;
layout(binding = 1) uniform sampler2D frameTexture;

void main()
{
    fragColor = texture(frameTexture, textureCoords).bgra;
}
