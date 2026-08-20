void MAIN()
{
    vec2 flippedCoord;
    flippedCoord.x = flipHorizontally ? 1.0 - INPUT_UV.x : INPUT_UV.x;
    flippedCoord.y = flipVertically ? 1.0 - INPUT_UV.y : INPUT_UV.y;

    FRAGCOLOR = texture(INPUT, flippedCoord);
}
