#version 440

#ifdef MY_DEFINE
#ifdef ANOTHER_DEFINE
layout(location = 0) in vec2 v_texcoord;
layout(location = 0) out vec4 fragColor;
layout(binding = 1) uniform sampler2D tex;
#endif
#endif // else breaks compilation

void main()
{
    fragColor = texture(tex, v_texcoord);
}
