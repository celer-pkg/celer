#version 440

layout(location=0) in vec3 v_nearPoint;
layout(location=1) in vec3 v_farPoint;
layout(location=2) in flat uint v_viewIndex;

layout(location=0) out vec4 fragColor;

layout(std140, binding=0) uniform buf {
#if QSHADER_VIEW_COUNT >= 2
    mat4 viewProj[QSHADER_VIEW_COUNT];
    mat4 invViewProj[QSHADER_VIEW_COUNT];
#else
    mat4 viewProj;
    mat4 invViewProj;
#endif
    float near;
    float far;
    float scale;
    float yFactor;
    uint gridFlags;
} u_buf;

// gridFlag enum values:

const uint drawAxisFlag = 1;

vec4 grid(vec2 fragPos2D, float scale, bool drawAxis, float lineAlpha) {
    vec2 coord = fragPos2D * scale; // use the scale variable to set the distance between the lines
    vec2 derivative = fwidth(coord); // derivative is pixel spacing in grid coords (1/derivative is distance between grid lines in pixels)
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative; // distance from grid in pixels

    float line = min(grid.x, grid.y);
    float alpha = smoothstep(0.0, 1, 1 - line) * lineAlpha;

    vec4 color = vec4(0.5, 0.5, 0.5, alpha); // NOT premultipled

    float minimumx = min(derivative.x, 1.0);
    float minimumz = min(derivative.y, 1.0);

    if (drawAxis) {
        vec2 axisDistance = abs(coord/derivative);
        if (axisDistance.y < 1) // distance to first axis
            color = vec4(1.0, 0.0, 0.0, alpha);
        if (axisDistance.x < 1) // distance to second axis
            color = vec4(0.0, 0.0, 1.0, alpha);
    }

    // Fade the lines when they get too close together
    const float fadeLimit = 0.1; // Start fading at grid spacing 1/fadeLimit
    const float fadeGradient = 1.5; // Fade out completely at length(derivative) == fadeLimit + 1/fadeGradient

    float d2 = length(derivative);
    if (d2 > fadeLimit)
        color.a *= max(0.0, 1 - fadeGradient * (d2 - fadeLimit));

    return color;
}

float computeLinearDepth(float fragDepth, float near, float far) {
    if (near <= 0.0)
        return fragDepth;
    float linearDepth = (2.0 * near * far) / (near + far - fragDepth * (far - near));
    return linearDepth / far;
}

void main(void)
{
    const float nearFloor = v_nearPoint.y;
    const float farFloor = v_farPoint.y;
    // Find where the sight line intersects the floor
    const float t = -nearFloor / (farFloor - nearFloor);

    // Fragment posision in 3D world space
    const vec3 fragPos3D = v_nearPoint + t * (v_farPoint - v_nearPoint);

    const vec2 fragPos2D = fragPos3D.xz;

    // Getting the fragment position in clip space
#if QSHADER_VIEW_COUNT >= 2
    const vec4 clipSpacePos = u_buf.viewProj[v_viewIndex] * vec4(fragPos3D, 1.0);
#else
    const vec4 clipSpacePos = u_buf.viewProj * vec4(fragPos3D, 1.0);
#endif

    // The out color is a combination of a larger grid and a smaller grid.
    // The grid is only shown at y=0.

    const bool drawAxis = (u_buf.gridFlags & drawAxisFlag) != 0;
    const vec4 outColor = max(grid(fragPos2D, 10.0*u_buf.scale, drawAxis, 0.5), grid(fragPos2D, 1.0*u_buf.scale, drawAxis, 1.0));

    // Fade at far clip: Calculating the depth of the fragment, the linear
    // depth and using the linear depth to calculate the fading constant
    // Do a hard cutoff at near clip, since that is less common, and we don't have an
    // instinct to expect a near fade anyway
    const float fragDepth = clipSpacePos.z / clipSpacePos.w;
    const float linearDepth = computeLinearDepth(fragDepth, u_buf.near, u_buf.far);
    const float fading = float(fragDepth > -1 && fragDepth < 1) * (1-max((linearDepth - 0.7)/0.3, 0.0));

    fragColor = vec4(outColor.rgb, outColor.a * fading);
    gl_FragDepth = 0.5 * fragDepth + 0.5;
}
