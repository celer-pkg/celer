#version 440

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in vec2 v_texCoord[];
layout(location = 1) in vec3 v_worldPos[];
layout(location = 2) in vec3 v_normal[];

layout(location = 0) out vec2 g_texCoord;
layout(location = 1) out vec3 g_worldPos;
layout(location = 2) out vec3 g_normal;

layout(std140, binding = 0) uniform buf {
    mat4 mvp;
    mat4 modelMatrix;
    vec3 voxelSpaceSize;
};

void main()
{
    const vec3 planeNormal = cross(v_worldPos[1] - v_worldPos[0], v_worldPos[2] - v_worldPos[0]);
    const vec3 p = abs(planeNormal);

    for (uint i = 0; i < 3; ++i) {
        g_texCoord = v_texCoord[i];
        g_worldPos = v_worldPos[i];
        g_normal = v_normal[i];

        const vec3 pos = g_worldPos / voxelSpaceSize;
        if (p.z > p.x && p.z > p.y)
            gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
        else if (p.x > p.y && p.x > p.z)
            gl_Position = vec4(pos.y, pos.z, 0.0, 1.0);
        else
            gl_Position = vec4(pos.x, pos.z, 0.0, 1.0);

        EmitVertex();
    }

    EndPrimitive();
}
