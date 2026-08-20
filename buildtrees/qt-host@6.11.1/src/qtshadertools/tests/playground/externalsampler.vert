#version 440

layout(location = 0) in vec4 vertexCoordsArray;
layout(location = 1) in vec2 textureCoordArray;
layout(location = 0) out vec2 textureCoords;

layout(std140, binding = 0) uniform buf {
    mat4 texMatrix;
};

void main()
{
    gl_Position = vertexCoordsArray;
    textureCoords = (texMatrix * vec4(textureCoordArray, 0.0, 1.0)).xy;
}
