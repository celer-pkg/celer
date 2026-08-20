#version 440

QC_INCLUDE "customvert.glsl"

void main()
{
    texCoord = tcoord;
    fragCoord = vertex;
    vec2 v = (vertMatrix * vec3(vertex, 1.0)).xy;

    // TODO: Why something like this causes nothing to be rendered?
    // Some data alignment issue?
    //v.y += 0.0 * sin(iTime);

    if (ndcIsYDown != 0)
        gl_Position = vec4(2.0 * (v.x + viewRect.x) / viewRect.z - 1.0, -1.0 + 2.0 * (v.y + viewRect.y) / viewRect.w, 0.0, 1.0);
    else
        gl_Position = vec4(2.0 * (v.x + viewRect.x) / viewRect.z - 1.0, 1.0 - 2.0 * (v.y + viewRect.y) / viewRect.w, 0.0, 1.0);
    gl_Position.y += (0.05 * sin(iTime + 0.02 * fragCoord.x));
}
