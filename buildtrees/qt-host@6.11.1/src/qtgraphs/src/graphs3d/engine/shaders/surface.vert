VARYING vec3 pos;
VARYING vec2 UV;
VARYING float overhigh;

void MAIN()
{
    UV = UV0 * (vertCount / size);

    float xStep = xDiff;
    float yStep = yDiff;
    if (flipU)
        xStep *= -1;
    if (flipV)
        yStep *= -1;

    vec2 uStep = vec2(xStep, 0.0);
    vec2 vStep = vec2(0.0, yStep);

    vec3 v = texture(height, UV).xyz;

    vec3 vRight = texture(height, UV + uStep).xyz;
    vec3 vLeft = texture(height, UV - uStep).xyz;
    vec3 vUp = texture(height, UV + vStep).xyz;
    vec3 vDown = texture(height, UV - vStep).xyz;

    vec3 tangent = vLeft - vRight;
    vec3 bitangent = vUp - vDown;

    vec3 n = normalize(cross(bitangent, tangent));

    overhigh = 0;

    if (fill) {
        if (v.y > graphHeight)
            overhigh = 1;

        if (any(greaterThan(UV0, vec2(1.05))) || any(lessThan(UV0, vec2(-0.05)))) // if sides bottom vertex
            v.y = -graphHeight;
        else if (v.y > graphHeight)
            v.y = -10000;
        // lower the unseen verts by a high amount to decrease the angle between the neighbouring seen verts
    }

    NORMAL = n;
    VERTEX = v;

    pos = VERTEX;
    vec4 pos = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
    pos.z += 0.000002 * (order + 1);
    POSITION = pos;
}
