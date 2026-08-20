void MAIN()
{
    // If the area directly behind the monkey is not blue but rather something
    // dark, then NORMAL is broken (not adjusted for double sidedness correctly)
    BASE_COLOR = vec4(NORMAL, 1.0);
}
