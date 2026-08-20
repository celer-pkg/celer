// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef ASSIMPUTILS_H
#define ASSIMPUTILS_H

#include <QtCore/qglobal.h>
#include <QtQuick3DUtils/private/qssgmesh_p.h>

struct aiScene;
struct aiMesh;

QT_BEGIN_NAMESPACE

class QString;

namespace AssimpUtils
{

using BoneIndexMap = QHash<QString, qint32>;
using MeshList = QVector<const aiMesh *>;

QSSGMesh::Mesh generateMeshData(const aiScene &scene,
                                const MeshList &meshes,
                                bool useFloatJointIndices,
                                bool generateLevelsOfDetail,
                                float normalMergeAngle,
                                float normalSplitAngle,
                                QString &errorString);
}

inline size_t qHash(const QVector3D &vector, size_t seed = 0) {
    if (vector.isNull())
        return seed;
    return qHashBits(&vector, sizeof(float) * 3, seed);
}

QT_END_NAMESPACE

#endif // ASSIMPUTILS_H
