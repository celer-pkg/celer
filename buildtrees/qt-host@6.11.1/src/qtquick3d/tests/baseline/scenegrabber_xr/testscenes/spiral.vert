VARYING vec2 center_vec;
void MAIN()
{
    center_vec = INPUT_UV - vec2(0.5, 0.5);
    center_vec.y *= INPUT_SIZE.y / INPUT_SIZE.x;
}
