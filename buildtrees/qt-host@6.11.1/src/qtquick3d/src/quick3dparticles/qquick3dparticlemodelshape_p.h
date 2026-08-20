// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLEMODELSHAPE_H
#define QQUICK3DPARTICLEMODELSHAPE_H

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

#include "qquick3dparticleabstractshape_p.h"
#include <QVector3D>

QT_BEGIN_NAMESPACE

class QQuick3DModel;
class QQmlComponent;

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleModelShape : public QQuick3DParticleAbstractShape
{
    Q_OBJECT
    Q_PROPERTY(bool fill READ fill WRITE setFill NOTIFY fillChanged)
    Q_PROPERTY(QQmlComponent *delegate READ delegate WRITE setDelegate NOTIFY delegateChanged)
    QML_NAMED_ELEMENT(ParticleModelShape3D)
    QML_ADDED_IN_VERSION(6, 2)

public:
    QQuick3DParticleModelShape(QObject *parent = nullptr);
    ~QQuick3DParticleModelShape() override;

    bool fill() const;
    QQmlComponent *delegate() const;

public Q_SLOTS:
    void setFill(bool fill);
    void setDelegate(QQmlComponent *delegate);

    // Returns point inside this shape
    QVector3D getPosition(int particleIndex) override;
    QVector3D getSurfaceNormal(int particleIndex) override;

Q_SIGNALS:
    void fillChanged();
    void delegateChanged();

private:
    QVector3D randomPositionModel(int particleIndex);
    void createModel();
    void clearModelVertexPositions();
    void calculateModelVertexPositions();

    QQmlComponent *m_delegate = nullptr;
    QQuick3DModel *m_model = nullptr;
    QVector<QVector3D> m_vertexPositions;
    float m_modelTriangleAreasSum = 0;
    QVector<float> m_modelTriangleAreas;
    QVector3D m_modelTriangleCenter;
    QVector3D m_cachedNormal;
    int m_cachedIndex = -1;
    bool m_fill = true;
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLEMODELSHAPE_H
