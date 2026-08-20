VARYING vec3 pos;

void MAIN() {
    highp vec2 absPos = min(vec2(1.0, 1.0), abs(pos.xy));
    if (absPos.x > sliceFrameWidth.x || absPos.y > sliceFrameWidth.y)
        FRAGCOLOR = color;
    else
        discard;
}

