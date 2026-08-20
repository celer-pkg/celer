void MAIN() // Create RGBL buffer
{
    vec4 color = texture(INPUT, INPUT_UV);
    color.a = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    FRAGCOLOR = color;
}
