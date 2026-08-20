void MAIN() {
    vec2 texcoord = UV0;
    texcoord.y = 1.0 - texcoord.y;

    BASE_COLOR = texture(diffuse, texcoord);
}
