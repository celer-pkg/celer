#extension GL_OES_EGL_image_external : require

varying highp vec2 textureCoords;
uniform samplerExternalOES frameTexture;

void main()
{
    gl_FragColor = texture2D(frameTexture, textureCoords).bgra;
}
