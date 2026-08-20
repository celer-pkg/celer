#version 440

layout(points) in;
layout(triangle_strip, max_vertices = 14) out;

layout(location = 0) in vec4 v_position[];
layout(location = 1) in vec4 v_color[];

layout(location = 0) out vec4 g_color;

layout(std140, binding = 1) uniform buf {
    mat4 viewProjection;
    vec3 voxelSpaceSize;
    uint voxelGridSize;
};

// 14 vertices, triangle strip, 0..1
// See GPU Gems 3 Chapter 6
vec3 createCube(uint vertex_id)
{
    uint x = (0x287A >> vertex_id) & 1;
    uint y = (0x02AF >> vertex_id) & 1;
    uint z = (0x31E3 >> vertex_id) & 1;
    return vec3(uvec3(x, y, z));
}

void main()
{
    if (v_color[0].a > 0.0) {
        for (uint i = 0; i < 14; ++i) {
            g_color = v_color[0];
            gl_Position = v_position[0];
            gl_Position.xyz = (gl_Position.xyz / float(voxelGridSize)) * 2.0 - 1; // -1..1
            gl_Position.xyz *= voxelGridSize; // -64..64
            gl_Position.xyz += (createCube(i) - vec3(0.5)) * 2.0;

            vec3 worldSpaceUnitsPerVoxel = voxelSpaceSize / float(voxelGridSize);
            gl_Position.xyz *= worldSpaceUnitsPerVoxel;

            gl_Position = viewProjection * vec4(gl_Position.xyz, 1.0);

            EmitVertex();
        }
        EndPrimitive();
    }
}
