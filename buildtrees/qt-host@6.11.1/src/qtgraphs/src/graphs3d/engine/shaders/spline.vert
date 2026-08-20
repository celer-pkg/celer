void MAIN() {

    //Catmull-Rom spline

    float curveIndex = UV0.y;
    float step = 1.0f / float(points);

    //add small offset to avoid sampling the edge of each texture pixel
    float pixelOffset = step * 0.1f;


    vec3 p0 = texture(controlPoints, vec2(curveIndex + pixelOffset, 0.5f)).xyz;
    vec3 p1 = texture(controlPoints, vec2(curveIndex + step + pixelOffset , 0.5f)).xyz;
    vec3 p2 = texture(controlPoints, vec2(curveIndex + 2.0f*step + pixelOffset , 0.5f)).xyz;
    vec3 p3 = texture(controlPoints, vec2(curveIndex + 3.0f*step + pixelOffset , 0.5f)).xyz;

    // check if looping segment
    float lastThresh = 1.0f - (1.0f / float(points - 2.0f)) - 0.001f;
    bool lastSegment = (curveIndex * points) / (points - 2.0f) >= lastThresh;

    if (loop && lastSegment) {
        p2 = texture(controlPoints, vec2(step, 0.5f)).xyz;
        p3 = texture(controlPoints, vec2(2.0f* step, 0.5f)).xyz;
    }

    float t01 = pow(distance(p0, p1), knotting);
    float t12 = pow(distance(p1, p2), knotting);
    float t23 = pow(distance(p2, p3), knotting);

    vec3 m1 = (1.0f - tension) *
        (p2 - p1 + t12 * ((p1 - p0) / t01 - (p2 - p0) / (t01 + t12)));
    vec3 m2 = (1.0f - tension) *
        (p2 - p1 + t12 * ((p3 - p2) / t23 - (p3 - p1) / (t12 + t23)));

    vec3 A = 2.0f * (p1 - p2) + m1 + m2;
    vec3 B = -3.0f * (p1 - p2) - m1 - m1 - m2;
    vec3 C = m1;
    vec3 D = p1;

    float t = UV0.x;

    vec3 point =
            A * t * t * t +
            B * t * t +
            C * t +
            D;

    vec4 pos = MODELVIEWPROJECTION_MATRIX * vec4(point, 1.0f);
    POSITION = pos;
}
