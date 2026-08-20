// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef PROCEDURALMESH_H
#define PROCEDURALMESH_H

#include <QtQuick3DHelpers/qtquick3dhelpersexports.h>
#include <QQuick3DGeometry>
#include <QQmlEngine>
#include <QList>
#include <QVector3D>

QT_BEGIN_NAMESPACE

class Q_QUICK3DHELPERS_EXPORT ProceduralMeshSubset : public QObject {
    Q_OBJECT
    Q_PROPERTY(unsigned int offset READ offset WRITE setOffset NOTIFY offsetChanged FINAL)
    Q_PROPERTY(unsigned int count READ count WRITE setCount NOTIFY countChanged FINAL)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    QML_NAMED_ELEMENT(ProceduralMeshSubset)
    QML_ADDED_IN_VERSION(6, 6)

public:
    int offset() const;
    void setOffset(int newOffset);

    int count() const;
    void setCount(int newCount);

    QString name() const;
    void setName(const QString &newName);
Q_SIGNALS:
    void offsetChanged();
    void countChanged();
    void nameChanged();
    void isDirty();

private:
    int m_offset = 0;
    int m_count = 0;
    QString m_name;
};

class Q_QUICK3DHELPERS_EXPORT ProceduralMesh : public QQuick3DGeometry
{
    Q_OBJECT
    Q_PROPERTY(QList<QVector3D> positions READ positions WRITE setPositions NOTIFY positionsChanged FINAL)
    Q_PROPERTY(QList<QVector3D> normals READ normals WRITE setNormals NOTIFY normalsChanged FINAL)
    Q_PROPERTY(QList<QVector3D> tangents READ tangents WRITE setTangents NOTIFY tangentsChanged FINAL)
    Q_PROPERTY(QList<QVector3D> binormals READ binormals WRITE setBinormals NOTIFY binormalsChanged FINAL)
    Q_PROPERTY(QList<QVector2D> uv0s READ uv0s WRITE setUv0s NOTIFY uv0sChanged FINAL)
    Q_PROPERTY(QList<QVector2D> uv1s READ uv1s WRITE setUv1s NOTIFY uv1sChanged FINAL)
    Q_PROPERTY(QList<QVector4D> colors READ colors WRITE setColors NOTIFY colorsChanged FINAL)
    Q_PROPERTY(QList<QVector4D> joints READ joints WRITE setJoints NOTIFY jointsChanged FINAL)
    Q_PROPERTY(QList<QVector4D> weights READ weights WRITE setWeights NOTIFY weightsChanged FINAL)
    Q_PROPERTY(QList<unsigned int> indexes READ indexes WRITE setIndexes NOTIFY indexesChanged FINAL)
    Q_PROPERTY(QQmlListProperty<ProceduralMeshSubset> subsets READ subsets FINAL)
    Q_PROPERTY(PrimitiveMode primitiveMode READ primitiveMode WRITE setPrimitiveMode NOTIFY primitiveModeChanged FINAL)
    QML_ELEMENT
    QML_ADDED_IN_VERSION(6, 6)
public:
    enum PrimitiveMode {
        Points,
        LineStrip,
        Lines,
        TriangleStrip,
        TriangleFan,
        Triangles
    };
    Q_ENUM(PrimitiveMode)

    ProceduralMesh();
    QList<QVector3D> positions() const;
    void setPositions(const QList<QVector3D> &newPositions);
    PrimitiveMode primitiveMode() const;
    void setPrimitiveMode(PrimitiveMode newPrimitiveMode);

    QList<unsigned int> indexes() const;
    void setIndexes(const QList<unsigned int> &newIndexes);

    QList<QVector3D> normals() const;
    void setNormals(const QList<QVector3D> &newNormals);

    QList<QVector3D> tangents() const;
    void setTangents(const QList<QVector3D> &newTangents);

    QList<QVector3D> binormals() const;
    void setBinormals(const QList<QVector3D> &newBinormals);

    QList<QVector2D> uv0s() const;
    void setUv0s(const QList<QVector2D> &newUv0s);

    QList<QVector2D> uv1s() const;
    void setUv1s(const QList<QVector2D> &newUv1s);

    QList<QVector4D> colors() const;
    void setColors(const QList<QVector4D> &newColors);

    QList<QVector4D> joints() const;
    void setJoints(const QList<QVector4D> &newJoints);

    QList<QVector4D> weights() const;
    void setWeights(const QList<QVector4D> &newWeights);

    QQmlListProperty<ProceduralMeshSubset> subsets();

Q_SIGNALS:
    void positionsChanged();
    void primitiveModeChanged();
    void indexesChanged();
    void normalsChanged();
    void tangentsChanged();
    void binormalsChanged();
    void uv0sChanged();
    void uv1sChanged();
    void colorsChanged();
    void jointsChanged();
    void weightsChanged();

private Q_SLOTS:
    void requestUpdate();
    void updateGeometry();
    void subsetDestroyed(QObject *subset);

private:
    bool supportsTriangleFanPrimitive() const;

    static void qmlAppendProceduralMeshSubset(QQmlListProperty<ProceduralMeshSubset> *list, ProceduralMeshSubset *subset);
    static ProceduralMeshSubset *qmlProceduralMeshSubsetAt(QQmlListProperty<ProceduralMeshSubset> *list, qsizetype index);
    static qsizetype qmlProceduralMeshSubsetCount(QQmlListProperty<ProceduralMeshSubset> *list);
    static void qmlClearProceduralMeshSubset(QQmlListProperty<ProceduralMeshSubset> *list);

    bool m_updateRequested = false;
    PrimitiveMode m_primitiveMode = Triangles;
    QList<QVector3D> m_positions;
    QList<unsigned int> m_indexes;
    QList<QVector3D> m_normals;
    QList<QVector3D> m_tangents;
    QList<QVector3D> m_binormals;
    QList<QVector2D> m_uv0s;
    QList<QVector2D> m_uv1s;
    QList<QVector4D> m_colors;
    QList<QVector4D> m_joints;
    QList<QVector4D> m_weights;
    QList<ProceduralMeshSubset *> m_subsets;
};

QT_END_NAMESPACE

#endif // PROCEDURALMESH_H
