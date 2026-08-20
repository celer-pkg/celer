VARYING vec3 pos;
VARYING vec3 rayDir;

void MAIN() {
    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);

    highp vec3 minBoundsNorm = minBounds;
    highp vec3 maxBoundsNorm = maxBounds;

    // Y and Z are flipped in bounds to be directly usable in texture calculations,
    // so flip them back to normal for position calculations
    minBoundsNorm.yz = -minBoundsNorm.yz;
    maxBoundsNorm.yz = -maxBoundsNorm.yz;

    minBoundsNorm = 0.5 * (minBoundsNorm + 1.0);
    maxBoundsNorm = 0.5 * (maxBoundsNorm + 1.0);

    pos = VERTEX
            + ((1.0 - VERTEX) * minBoundsNorm)
            - ((1.0 + VERTEX) * (1.0 - maxBoundsNorm));

    if (useOrtho) {
        vec3 camPos = (inverse(MODELVIEWPROJECTION_MATRIX) * vec4(0.0, 0.0, 1.0, 1.0)).xyz;
        camPos = -(camPos
                    + ((1.0 - camPos) * minBoundsNorm)
                    - ((1.0 + camPos) * (1.0 - maxBoundsNorm)));

        rayDir = -(camPos - pos);
    } else {
        vec3 camPos = (inverse(MODEL_MATRIX) * vec4(CAMERA_POSITION, 1.0)).xyz;
        camPos = camPos
                    + ((1.0 - camPos) * minBoundsNorm)
                    - ((1.0 + camPos) * (1.0 - maxBoundsNorm));

        rayDir = -(camPos - pos);
    }

    // Flip Y and Z so QImage bits work directly for texture and first image is in the front
    rayDir.yz = -rayDir.yz;
    pos.yz = -pos.yz;
}
