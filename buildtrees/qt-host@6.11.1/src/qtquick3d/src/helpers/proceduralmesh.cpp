// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "proceduralmesh_p.h"

#include <QtQuick3D/private/qquick3dobject_p.h>

#include <QtQuick/QQuickWindow>

#include <rhi/qrhi.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype ProceduralMesh
    \inqmlmodule QtQuick3D.Helpers
    \inherits Geometry
    \brief Allows creation of Geometry from QML.
    \since 6.6

    ProceduralMesh is a helper type that allows creation of Geometry instances
    from QML. The Geometry component is Abstract, and is usually created
    from C++.

    \qml
    component TorusMesh : ProceduralMesh {
        property real rings: 50
        property real segments: 50
        property real radius: 100.0
        property real tubeRadius: 10.0
        property var meshArrays: generateTorus(rings, segments, radius, tubeRadius)
        positions: meshArrays.verts
        normals: meshArrays.normals
        uv0s: meshArrays.uvs
        indexes: meshArrays.indices

        function generateTorus(rings: real, segments: real, radius: real, tubeRadius: real) {
            let verts = []
            let normals = []
            let uvs = []
            let indices = []

            for (let i = 0; i <= rings; ++i) {
                for (let j = 0; j <= segments; ++j) {
                    let u = i / rings * Math.PI * 2;
                    let v = j / segments * Math.PI * 2;

                    let centerX = radius * Math.cos(u);
                    let centerZ = radius * Math.sin(u);

                    let posX = centerX + tubeRadius * Math.cos(v) * Math.cos(u);
                    let posY = tubeRadius * Math.sin(v);
                    let posZ = centerZ + tubeRadius * Math.cos(v) * Math.sin(u);

                    verts.push(Qt.vector3d(posX, posY, posZ));

                    let normal = Qt.vector3d(posX - centerX, posY, posZ - centerZ).normalized();
                    normals.push(normal);

                    uvs.push(Qt.vector2d(i / rings, j / segments));
                }
            }

            for (let i = 0; i < rings; ++i) {
                for (let j = 0; j < segments; ++j) {
                    let a = (segments + 1) * i + j;
                    let b = (segments + 1) * (i + 1) + j;
                    let c = (segments + 1) * (i + 1) + j + 1;
                    let d = (segments + 1) * i + j + 1;

                    // Generate two triangles for each quad in the mesh
                    // Adjust order to be counter-clockwise
                    indices.push(a, d, b);
                    indices.push(b, d, c);
                }
            }
            return { verts: verts, normals: normals, uvs: uvs, indices: indices }
        }
    }
    \endqml

    The above code defines a component TorusMesh that can be used as Geometry for use
    with a Model component. When the ring, segments, radius or tubeRadius properties
    are modified the geometry will be updated.

    The ProceduralMesh component is not as flexible nor as performant as creating
    Geometry in C++, but makes up for it in convenience and simplicity. The
    properties are fixed attribute lists that when filled will automatically
    generate the necessary buffers.

*/

/*!
    \qmlproperty List<QVector3D> ProceduralMesh::positions
    The positions attribute list. If this list remains empty nothing no geometry
    will be generated.
*/

/*!
    \qmlproperty List<QVector3D> ProceduralMesh::normals
    Holds the normals attribute list.
*/

/*!
    \qmlproperty List<QVector3D> ProceduralMesh::tangents
    Holds the tangents attribute list.
*/

/*!
    \qmlproperty List<QVector3D> ProceduralMesh::binormals
    Holds the binormals attribute list.
*/

/*!
    \qmlproperty List<QVector2D> ProceduralMesh::uv0s
    This property defines a list of uv coordinates for the first uv channel (uv0)
*/

/*!
    \qmlproperty List<QVector2D> ProceduralMesh::uv1s
    This property defines a list of uv coordinates for the second uv channel (uv1)
*/

/*!
    \qmlproperty List<QVector4D> ProceduralMesh::colors
    This property defines a list of vertex color values.
*/

/*!
    \qmlproperty List<QVector4D> ProceduralMesh::joints
    This property defines a list of joint indices for skinning.
*/

/*!
    \qmlproperty List<QVector4D> ProceduralMesh::weights
    This property defines a list of joint weights for skinning.
*/

/*!
    \qmlproperty List<int> ProceduralMesh::indexes
    This property defines a list of indexes into the attribute lists. If this list remains empty
    the vertex buffer values will be used directly.
*/

/*!
    \qmlproperty enumeration ProceduralMesh::primitiveMode

    This property defines the primitive mode to use when rendering the geometry.

    \value ProceduralMesh.Points The points primitive mode is used.
    \value ProceduralMesh.LineStrip The line strip primitive mode is used.
    \value ProceduralMesh.Lines The lines primitive mode is used.
    \value ProceduralMesh.TriangleStrip The triangles strip primitive mode is
    used.
    \value ProceduralMesh.TriangleFan The triangle fan primitive mode is used.
    \value ProceduralMesh.Triangles The triangles primitive mode is used.
    \default ProceduralMesh.Triangles

    \note Not all modes are supported on all rendering backends.
*/

/*!
    \qmlproperty List<ProceduralMeshSubset> ProceduralMesh::subsets

    This property defines a list of subsets to split the geometry data into.
    Each subset can have it's own material.  The order of this array
    corresponds to the materials list of Model when using this geometry.

    This property is optional and when empty results in a single subset.

    \note Any subset that specifies values outside of the range of available
    vertex/index values will lead to that subset being ignored.
*/

/*!
    \qmltype ProceduralMeshSubset
    \inqmlmodule QtQuick3D.Helpers
    \inherits QtObject
    \brief Defines a subset of a ProceduralMesh.
    \since 6.6

    This type defines a subset of a ProceduralMesh. Each subset can have it's own
    material and can be used to split the geometry into multiple draw calls.

    \sa ProceduralMesh::subsets

*/

/*!
    \qmlproperty int ProceduralMeshSubset::offset
    This property defines the starting index for this subset. \default 0
*/

/*!
    \qmlproperty int ProceduralMeshSubset::count
    This property defines the number of indices to use for this subset. This property must be set for the subset to have content.

    \default 0
*/

/*!
    \qmlproperty Material ProceduralMeshSubset::name
    This property defines a name of the subset. This property is optional, and is only used to
    tag the subset for debugging purposes.
*/

ProceduralMesh::ProceduralMesh()
{

}

QList<QVector3D> ProceduralMesh::positions() const
{
    return m_positions;
}

void ProceduralMesh::setPositions(const QList<QVector3D> &newPositions)
{
    if (m_positions == newPositions)
        return;
    m_positions = newPositions;
    Q_EMIT positionsChanged();
    requestUpdate();
}

ProceduralMesh::PrimitiveMode ProceduralMesh::primitiveMode() const
{
    return m_primitiveMode;
}

void ProceduralMesh::setPrimitiveMode(PrimitiveMode newPrimitiveMode)
{
    if (m_primitiveMode == newPrimitiveMode)
        return;

    // Do some sanity checking
    if (newPrimitiveMode < Points || newPrimitiveMode > Triangles) {
        qWarning() << "Invalid primitive mode specified";
        return;
    }

    if (newPrimitiveMode == PrimitiveMode::TriangleFan) {
        if (!supportsTriangleFanPrimitive()) {
            qWarning() << "TriangleFan is not supported by the current backend";
            return;
        }
    }

    m_primitiveMode = newPrimitiveMode;
    Q_EMIT primitiveModeChanged();
    requestUpdate();
}

void ProceduralMesh::requestUpdate()
{
    if (!m_updateRequested) {
        QMetaObject::invokeMethod(this, "updateGeometry", Qt::QueuedConnection);
        m_updateRequested = true;
    }
}

void ProceduralMesh::updateGeometry()
{
    m_updateRequested = false;
    // reset the geometry
    clear();

    setPrimitiveType(PrimitiveType(m_primitiveMode));

    // Figure out which attributes are being used
    const auto expectedLength = m_positions.size();
    bool hasPositions = !m_positions.isEmpty();
    if (!hasPositions) {
        setStride(0);
        update();
        return; // If there are no positions, there is no point :-)
    }
    bool hasNormals = m_normals.size() >= expectedLength;
    bool hasTangents = m_tangents.size() >= expectedLength;
    bool hasBinormals = m_binormals.size() >= expectedLength;
    bool hasUV0s = m_uv0s.size() >= expectedLength;
    bool hasUV1s = m_uv1s.size() >= expectedLength;
    bool hasColors = m_colors.size() >= expectedLength;
    bool hasJoints = m_joints.size() >= expectedLength;
    bool hasWeights = m_weights.size() >= expectedLength;
    bool hasIndexes = !m_indexes.isEmpty();

    int offset = 0;
    if (hasPositions) {
        addAttribute(Attribute::Semantic::PositionSemantic, offset, Attribute::ComponentType::F32Type);
        offset += 3 * sizeof(float);
    }

    if (hasNormals) {
        addAttribute(Attribute::Semantic::NormalSemantic, offset, Attribute::ComponentType::F32Type);
        offset += 3 * sizeof(float);
    }

    if (hasTangents) {
        addAttribute(Attribute::Semantic::TangentSemantic, offset, Attribute::ComponentType::F32Type);
        offset += 3 * sizeof(float);
    }

    if (hasBinormals) {
        addAttribute(Attribute::Semantic::BinormalSemantic, offset, Attribute::ComponentType::F32Type);
        offset += 3 * sizeof(float);
    }

    if (hasUV0s) {
        addAttribute(Attribute::Semantic::TexCoord0Semantic, offset, Attribute::ComponentType::F32Type);
        offset += 2 * sizeof(float);
    }

    if (hasUV1s) {
        addAttribute(Attribute::Semantic::TexCoord1Semantic, offset, Attribute::ComponentType::F32Type);
        offset += 2 * sizeof(float);
    }

    if (hasColors) {
        addAttribute(Attribute::Semantic::ColorSemantic, offset, Attribute::ComponentType::F32Type);
        offset += 4 * sizeof(float);
    }

    if (hasJoints) {
        addAttribute(Attribute::Semantic::JointSemantic, offset, Attribute::ComponentType::F32Type);
        offset += 4 * sizeof(float);
    }

    if (hasWeights) {
        addAttribute(Attribute::Semantic::WeightSemantic, offset, Attribute::ComponentType::F32Type);
        offset += 4 * sizeof(float);
    }

    if (hasIndexes)
        addAttribute(Attribute::Semantic::IndexSemantic, 0, Attribute::ComponentType::U32Type);

    // Set up the vertex buffer
    const int stride = offset;
    const qsizetype bufferSize = expectedLength * stride;
    setStride(stride);

    QVector<float> vertexBufferData;
    vertexBufferData.reserve(bufferSize / sizeof(float));

    QVector3D minBounds;
    QVector3D maxBounds;

    for (qsizetype i = 0; i < expectedLength; ++i) {
        // start writing float values to vertexBuffer
        if (hasPositions) {
            const auto &position = m_positions[i];
            vertexBufferData.append(position.x());
            vertexBufferData.append(position.y());
            vertexBufferData.append(position.z());
            minBounds.setX(qMin(minBounds.x(), position.x()));
            maxBounds.setX(qMax(maxBounds.x(), position.x()));
            minBounds.setY(qMin(minBounds.y(), position.y()));
            maxBounds.setY(qMax(maxBounds.y(), position.y()));
            minBounds.setZ(qMin(minBounds.z(), position.z()));
            maxBounds.setZ(qMax(maxBounds.z(), position.z()));
        }
        if (hasNormals) {
            const auto &normal = m_normals[i];
            vertexBufferData.append(normal.x());
            vertexBufferData.append(normal.y());
            vertexBufferData.append(normal.z());
        }

        if (hasBinormals) {
            const auto &binormal = m_binormals[i];
            vertexBufferData.append(binormal.x());
            vertexBufferData.append(binormal.y());
            vertexBufferData.append(binormal.z());
        }

        if (hasTangents) {
            const auto &tangent = m_tangents[i];
            vertexBufferData.append(tangent.x());
            vertexBufferData.append(tangent.y());
            vertexBufferData.append(tangent.z());
        }

        if (hasUV0s) {
            const auto &uv0 = m_uv0s[i];
            vertexBufferData.append(uv0.x());
            vertexBufferData.append(uv0.y());
        }

        if (hasUV1s) {
            const auto &uv1 = m_uv1s[i];
            vertexBufferData.append(uv1.x());
            vertexBufferData.append(uv1.y());
        }

        if (hasColors) {
            const auto &color = m_colors[i];
            vertexBufferData.append(color.x());
            vertexBufferData.append(color.y());
            vertexBufferData.append(color.z());
            vertexBufferData.append(color.w());
        }

        if (hasJoints) {
            const auto &joint = m_joints[i];
            vertexBufferData.append(joint.x());
            vertexBufferData.append(joint.y());
            vertexBufferData.append(joint.z());
            vertexBufferData.append(joint.w());
        }

        if (hasWeights) {
            const auto &weight = m_weights[i];
            vertexBufferData.append(weight.x());
            vertexBufferData.append(weight.y());
            vertexBufferData.append(weight.z());
            vertexBufferData.append(weight.w());
        }
    }

    setBounds(minBounds, maxBounds);
    QByteArray vertexBuffer(reinterpret_cast<char *>(vertexBufferData.data()), bufferSize);
    setVertexData(vertexBuffer);

    // Index Buffer
    if (hasIndexes) {
        const qsizetype indexLength = m_indexes.size();
        QByteArray indexBuffer;
        indexBuffer.reserve(indexLength * sizeof(unsigned int));
        for (qsizetype i = 0; i < indexLength; ++i) {
            const auto &index = m_indexes[i];
            indexBuffer.append(reinterpret_cast<const char *>(&index), sizeof(unsigned int));
        }
        setIndexData(indexBuffer);
    }

    // Subsets
    // Subsets are optional so if none are specified the whole mesh is a single submesh
    if (!m_subsets.isEmpty()) {
        for (const auto &subset : m_subsets) {
            QVector3D subsetMinBounds;
            QVector3D subsetMaxBounds;
            // Range checking is necessary because the user could have specified subset values
            // that are out of range of the vertex/index buffer
            bool outOfRange = false;
            for (qsizetype i = subset->offset(); i < subset->offset() + subset->count(); ++i) {
                if (hasPositions) {
                    qsizetype index = i;
                    if (hasIndexes) {
                        if (i < m_indexes.size()) {
                            index = m_indexes[i];
                        } else {
                            outOfRange = true;
                            break;
                        }
                    }
                    if (index < m_positions.size()) {
                        const auto &position = m_positions[index];
                        subsetMinBounds.setX(qMin(subsetMinBounds.x(), position.x()));
                        subsetMaxBounds.setX(qMax(subsetMaxBounds.x(), position.x()));
                        subsetMinBounds.setY(qMin(subsetMinBounds.y(), position.y()));
                        subsetMaxBounds.setY(qMax(subsetMaxBounds.y(), position.y()));
                        subsetMinBounds.setZ(qMin(subsetMinBounds.z(), position.z()));
                        subsetMaxBounds.setZ(qMax(subsetMaxBounds.z(), position.z()));
                    } else {
                        outOfRange = true;
                        break;
                    }
                }
            }
            if (!outOfRange)
                addSubset(subset->offset(), subset->count(), subsetMinBounds, subsetMaxBounds, subset->name());
            else
                qWarning("Skipping invalid subset: Out of Range");
        }
    }

    update();
    emit geometryChanged();
}

void ProceduralMesh::subsetDestroyed(QObject *subset)
{
    if (m_subsets.removeAll(subset))
        requestUpdate();
}

bool ProceduralMesh::supportsTriangleFanPrimitive() const
{
    static bool supportQueried = false;
    static bool triangleFanSupported = false;
    if (!supportQueried) {
        const auto &manager = QQuick3DObjectPrivate::get(this)->sceneManager;
        if (manager) {
            auto window = manager->window();
            if (window) {
                auto rhi = window->rhi();
                if (rhi) {
                    triangleFanSupported = rhi->isFeatureSupported(QRhi::TriangleFanTopology);
                    supportQueried = true;
                }
            }
        }
    }

    return triangleFanSupported;
}

void ProceduralMesh::qmlAppendProceduralMeshSubset(QQmlListProperty<ProceduralMeshSubset> *list, ProceduralMeshSubset *subset)
{
    if (subset == nullptr)
        return;
    ProceduralMesh *self = static_cast<ProceduralMesh *>(list->object);
    self->m_subsets.push_back(subset);

    connect(subset, &ProceduralMeshSubset::isDirty, self, &ProceduralMesh::requestUpdate);
    connect(subset, &QObject::destroyed, self, &ProceduralMesh::subsetDestroyed);

    self->requestUpdate();
}

ProceduralMeshSubset *ProceduralMesh::qmlProceduralMeshSubsetAt(QQmlListProperty<ProceduralMeshSubset> *list, qsizetype index)
{
    ProceduralMesh *self = static_cast<ProceduralMesh *>(list->object);
    return self->m_subsets.at(index);

}

qsizetype ProceduralMesh::qmlProceduralMeshSubsetCount(QQmlListProperty<ProceduralMeshSubset> *list)
{
    ProceduralMesh *self = static_cast<ProceduralMesh *>(list->object);
    return self->m_subsets.count();
}

void ProceduralMesh::qmlClearProceduralMeshSubset(QQmlListProperty<ProceduralMeshSubset> *list)
{
    ProceduralMesh *self = static_cast<ProceduralMesh *>(list->object);
    self->m_subsets.clear();
    self->requestUpdate();
}

QList<unsigned int> ProceduralMesh::indexes() const
{
    return m_indexes;
}

void ProceduralMesh::setIndexes(const QList<unsigned int> &newIndexes)
{
    if (m_indexes == newIndexes)
        return;
    m_indexes = newIndexes;
    Q_EMIT indexesChanged();
    requestUpdate();
}

QList<QVector3D> ProceduralMesh::normals() const
{
    return m_normals;
}

void ProceduralMesh::setNormals(const QList<QVector3D> &newNormals)
{
    if (m_normals == newNormals)
        return;
    m_normals = newNormals;
    Q_EMIT normalsChanged();
    requestUpdate();
}

QList<QVector3D> ProceduralMesh::tangents() const
{
    return m_tangents;
}

void ProceduralMesh::setTangents(const QList<QVector3D> &newTangents)
{
    if (m_tangents == newTangents)
        return;
    m_tangents = newTangents;
    Q_EMIT tangentsChanged();
    requestUpdate();
}

QList<QVector3D> ProceduralMesh::binormals() const
{
    return m_binormals;
}

void ProceduralMesh::setBinormals(const QList<QVector3D> &newBinormals)
{
    if (m_binormals == newBinormals)
        return;
    m_binormals = newBinormals;
    Q_EMIT binormalsChanged();
    requestUpdate();
}

QList<QVector2D> ProceduralMesh::uv0s() const
{
    return m_uv0s;
}

void ProceduralMesh::setUv0s(const QList<QVector2D> &newUv0s)
{
    if (m_uv0s == newUv0s)
        return;
    m_uv0s = newUv0s;
    Q_EMIT uv0sChanged();
    requestUpdate();
}

QList<QVector2D> ProceduralMesh::uv1s() const
{
    return m_uv1s;
}

void ProceduralMesh::setUv1s(const QList<QVector2D> &newUv1s)
{
    if (m_uv1s == newUv1s)
        return;
    m_uv1s = newUv1s;
    Q_EMIT uv1sChanged();
    requestUpdate();
}

QList<QVector4D> ProceduralMesh::colors() const
{
    return m_colors;
}

void ProceduralMesh::setColors(const QList<QVector4D> &newColors)
{
    if (m_colors == newColors)
        return;
    m_colors = newColors;
    Q_EMIT colorsChanged();
    requestUpdate();
}

QList<QVector4D> ProceduralMesh::joints() const
{
    return m_joints;
}

void ProceduralMesh::setJoints(const QList<QVector4D> &newJoints)
{
    if (m_joints == newJoints)
        return;
    m_joints = newJoints;
    Q_EMIT jointsChanged();
    requestUpdate();
}

QList<QVector4D> ProceduralMesh::weights() const
{
    return m_weights;
}

void ProceduralMesh::setWeights(const QList<QVector4D> &newWeights)
{
    if (m_weights == newWeights)
        return;
    m_weights = newWeights;
    Q_EMIT weightsChanged();
    requestUpdate();
}

QQmlListProperty<ProceduralMeshSubset> ProceduralMesh::subsets()
{
    return QQmlListProperty<ProceduralMeshSubset>(this,
                                                  nullptr,
                                                  ProceduralMesh::qmlAppendProceduralMeshSubset,
                                                  ProceduralMesh::qmlProceduralMeshSubsetCount,
                                                  ProceduralMesh::qmlProceduralMeshSubsetAt,
                                                  ProceduralMesh::qmlClearProceduralMeshSubset);
}

int ProceduralMeshSubset::offset() const
{
    return m_offset;
}

void ProceduralMeshSubset::setOffset(int newOffset)
{
    if (m_offset == newOffset)
        return;

    m_offset = newOffset;
    Q_EMIT offsetChanged();
    Q_EMIT isDirty();
}

int ProceduralMeshSubset::count() const
{
    return m_count;
}

void ProceduralMeshSubset::setCount(int newCount)
{
    if (m_count == newCount)
        return;

    m_count = newCount;
    Q_EMIT countChanged();
    Q_EMIT isDirty();
}

QString ProceduralMeshSubset::name() const
{
    return m_name;
}

void ProceduralMeshSubset::setName(const QString &newName)
{
    if (m_name == newName)
        return;

    m_name = newName;
    Q_EMIT nameChanged();
    Q_EMIT isDirty();
}

QT_END_NAMESPACE
