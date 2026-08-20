#version 440

layout(location = 0) in vec4 vertexCoord;
layout(location = 1) in vec2 inA;   // A B and C: control points in logical coordinates
layout(location = 2) in vec2 inB;
layout(location = 3) in vec2 inC;
layout(location = 4) in vec3 normalExt; // x and y: normal vector; z: strokeWidth multiplier (default 1)

layout(location = 0) out vec4 P;    // stroke edge
layout(location = 1) out vec2 A;    // A B and C: control points in logical coordinates
layout(location = 2) out vec2 B;
layout(location = 3) out vec2 C;
layout(location = 4) out vec4 HGOW; // H and G: args to solveDepressedCubic(); O: offset; W: adj strokeWidth

layout(std140, binding = 0) uniform buf {
#if QSHADER_VIEW_COUNT >= 2
    mat4 qt_Matrix[QSHADER_VIEW_COUNT];
#else
    mat4 qt_Matrix;
#endif

    float matrixScale;
    float opacity;
    float devicePixelRatio;
    float strokeWidth;

    vec4 strokeColor;

    float debug;
    float reserved5;
    float reserved6;
    float reserved7;
} ubuf;

#define SQRT2 1.41421356237

float qdot(vec2 a, vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

void main()
{
    vec4 normalVector = vec4(normalExt.xy, 0.0, 0.0);
    // If STROKE_EXPANDING is _not_ defined, vertexCoord starts out in
    // logical coordinates, already positioned outwards past an edge of the stroke.
    P = vertexCoord + normalVector * SQRT2 / ubuf.matrixScale;
    float adjStrokeWidth = abs(ubuf.strokeWidth); // passed to fragment shader in HGOW.w

#if defined(STROKE_EXPANDING)
    // vertexCoord starts out in logical coordinates, in the center of the stroke.
    // Move the vertex by stroke width * normalVector to fill the stroke width;
    // and in case of a cosmetic stroke, divide by matrixScale to undo the scaling.
    // In the case of a miter joint, the tip vertices need to be moved farther,
    // so the length of normalVector may be > 1.
    P += normalVector * adjStrokeWidth; // expand to stroke width

    // Negative ubuf.strokeWidth means we want a "cosmetic pen", which means
    // we make the stroke triangles wider or narrower to adjust for the difference
    // between zoomed stroke width and cosmetic stroke width.
    if (ubuf.strokeWidth < 0.) { // cosmetic stroke
        adjStrokeWidth *= ubuf.devicePixelRatio / ubuf.matrixScale;
        float strokeDiff = max(ubuf.devicePixelRatio / ubuf.matrixScale, adjStrokeWidth) *
                (ubuf.matrixScale / ubuf.devicePixelRatio - 1) / 2.;
        P -= normalVector * strokeDiff; // cosmetic adjustment
    }

    // adjust stroke width by the given multiplier (usually 1)
    adjStrokeWidth *= normalExt.z;
#endif // if the shader is expected to expand the stroke by moving vertices outwards

    A = inA;
    B = inB;
    C = inC;

    // Find the parameters H, G for the depressed cubic
    // t^2+H*t+G=0
    // that results from the equation
    // Q'(s).(p-Q(s)) = 0
    // O (HGOW.z) is the static offset between s and t:
    // s = t - b/(3a)
    // use it to get back the parameter t

    // this is a constant for the curve
    float a = -2. * qdot(A, A);
    // this is a constant for the curve
    float b = -3. * qdot(A, B);
    //this is linear in p so it can be put into the shader with vertex data
    float c = 2. * qdot(A, P.xy) - qdot(B, B) - 2. * qdot(A, C);
    //this is linear in p so it can be put into the shader with vertex data
    float d = qdot(B, P.xy) - qdot(B, C);
    // convert to depressed cubic.
    // both functions are linear in c and d and thus linear in p
    float H = (3. * a * c - b * b) / (3. * a * a);
    float G = (2. * b * b * b - 9. * a * b * c + 27. * a * a * d) / (27. * a * a * a);
    HGOW = vec4(H, G, b / (3 * a), adjStrokeWidth);

#if QSHADER_VIEW_COUNT >= 2
    gl_Position = ubuf.qt_Matrix[gl_ViewIndex] * P;
#else
    gl_Position = ubuf.qt_Matrix * P;
#endif
}
