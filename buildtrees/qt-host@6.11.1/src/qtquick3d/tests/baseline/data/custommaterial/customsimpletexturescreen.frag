VARYING vec2 texcoord;

void MAIN()
{
    BASE_COLOR = texture(tex1, texcoord);
    vec2 screencoord = texcoord;
    // the tex coords from the rectangle (or cube etc.) are almost suitable,
    // except that on non-GL we need to flip them
    if (FRAMEBUFFER_Y_UP < 0.0)
        screencoord.y = 1.0 - screencoord.y;
    BASE_COLOR *= texture(SCREEN_TEXTURE, screencoord);
}
