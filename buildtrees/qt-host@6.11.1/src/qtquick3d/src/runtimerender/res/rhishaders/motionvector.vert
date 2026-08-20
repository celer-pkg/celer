// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
#version 440
layout (location = 0) in vec3 attr_pos;

// SKIN INSTANCE layouts ####################
#if defined(QSSG_SKIN) && defined(QSSG_INSTANCE)
layout (location = 1) in ivec4 attr_joints;
layout (location = 2) in vec4 attr_weights;

layout (location = 3) in vec4 qt_instanceTransform0;
layout (location = 4) in vec4 qt_instanceTransform1;
layout (location = 5) in vec4 qt_instanceTransform2;
layout (location = 6) in vec4 qt_instanceColor;
layout (location = 7) in vec4 qt_instanceData;

layout (binding = 2) uniform sampler2D qt_boneTexture;
layout (binding = 3) uniform sampler2D lastBoneTexture;
layout (binding = 4) uniform sampler2D lastInstanceTexture;

#ifdef QSSG_MORPH
layout (binding = 5) uniform sampler2D morphWeightTexture;
layout (binding = 6) uniform sampler2D lastMorphWeightTexture;
layout (binding = 7) uniform sampler2DArray qt_morphTargetTexture;
#endif

// SKIN layouts ####################
#elif defined(QSSG_SKIN)
layout (location = 1) in ivec4 attr_joints;
layout (location = 2) in vec4 attr_weights;

layout (binding = 1) uniform sampler2D qt_boneTexture;
layout (binding = 2) uniform sampler2D lastBoneTexture;

#ifdef QSSG_MORPH
layout (binding = 3) uniform sampler2D morphWeightTexture;
layout (binding = 4) uniform sampler2D lastMorphWeightTexture;
layout (binding = 5) uniform sampler2DArray qt_morphTargetTexture;
#endif

// INSTANCE layouts ####################
#elif  defined(QSSG_INSTANCE)
layout (location = 1) in vec4 qt_instanceTransform0;
layout (location = 2) in vec4 qt_instanceTransform1;
layout (location = 3) in vec4 qt_instanceTransform2;
layout (location = 4) in vec4 qt_instanceColor;
layout (location = 5) in vec4 qt_instanceData;

layout (binding = 2) uniform sampler2D lastInstanceTexture;

#ifdef QSSG_MORPH
layout (binding = 3) uniform sampler2D morphWeightTexture;
layout (binding = 4) uniform sampler2D lastMorphWeightTexture;
layout (binding = 5) uniform sampler2DArray qt_morphTargetTexture;
#endif

// BASE layouts ####################
#else

#ifdef QSSG_MORPH
layout (binding = 1) uniform sampler2D morphWeightTexture;
layout (binding = 2) uniform sampler2D lastMorphWeightTexture;
layout (binding = 3) uniform sampler2DArray qt_morphTargetTexture;
#endif

#endif

out gl_PerVertex { vec4 gl_Position; };
layout (location = 0) out vec4 vPosition;
layout (location = 1) out vec4 vPrevPosition;
layout (std140, binding = 0) uniform buf {
    mat4 mvp;
    mat4 prevMVP;
    mat4 instanceLocal;
    mat4 instanceGlobal;
    mat4 prevInstanceLocal;
    mat4 prevInstanceGlobal;
    vec4 currentAndLastJitter;
    float velocityAmount;
    int morphTargetCount;
    vec2 padding;
} ubuf;

#if defined(QSSG_SKIN) || defined(QSSG_INSTANCE)
mat4 getTexMatrix(int index, sampler2D textureStorage)
{
    mat4 ret;
    int width = textureSize(textureStorage, 0).x;
    int matId = index * 4;
    ivec2 p;
    // Rounding mode of integers is undefined for GLES 3.0
    // https://www.khronos.org/registry/OpenGL/specs/es/3.0/GLSL_ES_Specification_3.00.pdf (section 12.33)
    p.x = matId % width;
    p.y = (matId - p.x) / width;
    ret[0] = texelFetch(textureStorage, p, 0);
    p.x = (matId + 1) % width;
    p.y = (matId + 1 - p.x) / width;
    ret[1] = texelFetch(textureStorage, p, 0);
    p.x = (matId + 2) % width;
    p.y = (matId + 2 - p.x) / width;
    ret[2] = texelFetch(textureStorage, p, 0);
    p.x = (matId + 3) % width;
    p.y = (matId + 3 - p.x) / width;
    ret[3] = texelFetch(textureStorage, p, 0);
    return ret;
}
#endif

#ifdef QSSG_SKIN
mat4 getSkinMatrix(ivec4 joints, vec4 weights, sampler2D boneTexture)
{
    return getTexMatrix(joints.x * 2, boneTexture) * weights.x
            + getTexMatrix(joints.y * 2, boneTexture) * weights.y
            + getTexMatrix(joints.z * 2, boneTexture) * weights.z
            + getTexMatrix(joints.w * 2, boneTexture) * weights.w;
}
#endif

#ifdef QSSG_MORPH
    vec4 getTargetTexValue(ivec3 texCoord, sampler2DArray targetTexture)
    {
        return texelFetch(targetTexture, texCoord, 0);
    }

    vec4 qt_getTargetValue(vec4 orgValue, int offset, sampler2DArray targetTexture, sampler2D weightTexture)
    {
        vec4 result = orgValue;
        ivec3 texCoord;
        int texWidth = textureSize(targetTexture, 0).x;
        texCoord.x = gl_VertexIndex % texWidth;
        texCoord.y = (gl_VertexIndex - texCoord.x) / texWidth;

        for (int i = 0; i < ubuf.morphTargetCount; ++i) {
            texCoord.z = offset + i;
            vec4 targetValue = getTargetTexValue(texCoord, targetTexture);
            result += texelFetch(weightTexture, ivec2(i, 0), 0).x * (targetValue - orgValue);
        }

        return vec4(result.xyz, 1.);
    }
#endif

void main()
{
    vec4 vp = vec4(attr_pos, 1.0);
    vec4 vpPre = vec4(attr_pos, 1.0);

#ifdef QSSG_MORPH
    vp = qt_getTargetValue(vp, 0, qt_morphTargetTexture, morphWeightTexture);
    vpPre = qt_getTargetValue(vpPre, 0, qt_morphTargetTexture, lastMorphWeightTexture);
#endif


#ifdef QSSG_INSTANCE
    mat4 instanceMat = transpose(mat4(qt_instanceTransform0,
                                         qt_instanceTransform1,
                                         qt_instanceTransform2,
                                         vec4(0.0, 0.0, 0.0, 1.0)));
    mat4 lastInstanceMat = getTexMatrix(gl_InstanceIndex, lastInstanceTexture);

    instanceMat = ubuf.instanceGlobal * instanceMat;
    lastInstanceMat = ubuf.prevInstanceGlobal * lastInstanceMat;

#ifndef QSSG_SKIN
    instanceMat *= ubuf.instanceLocal;
    lastInstanceMat *= ubuf.prevInstanceLocal;
#endif

    vpPre = instanceMat * vpPre;
    vp = lastInstanceMat * vp;
#endif

#ifdef QSSG_SKIN
    if (attr_weights != vec4(0.0)) {
        mat4 skinMat = getSkinMatrix(attr_joints, attr_weights, qt_boneTexture);
        vp = skinMat * vp;
        mat4 lastSkinMat = getSkinMatrix(attr_joints, attr_weights, lastBoneTexture);
        vpPre = lastSkinMat * vpPre;
    }
#endif

    gl_Position = vPosition = ubuf.mvp * vp;
    vPrevPosition = ubuf.prevMVP * vpPre;
}
