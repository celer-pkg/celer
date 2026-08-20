// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLEVECTORDIRECTION_H
#define QQUICK3DPARTICLEVECTORDIRECTION_H

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

#include <QVector3D>
#include <QtQuick3DParticles/private/qquick3dparticledirection_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleVectorDirection : public QQuick3DParticleDirection
{
    Q_OBJECT
    Q_PROPERTY(QVector3D direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(QVector3D directionVariation READ directionVariation WRITE setDirectionVariation NOTIFY directionVariationChanged)
    Q_PROPERTY(bool normalized READ normalized WRITE setNormalized NOTIFY normalizedChanged)
    QML_NAMED_ELEMENT(VectorDirection3D)
    QML_ADDED_IN_VERSION(6, 2)

public:
    QQuick3DParticleVectorDirection(QObject *parent = nullptr);

    QVector3D direction() const;
    QVector3D directionVariation() const;
    bool normalized() const;

public Q_SLOTS:
    void setDirection(const QVector3D &direction);
    void setDirectionVariation(const QVector3D &directionVariation);
    void setNormalized(bool normalized);

Q_SIGNALS:
    void directionChanged();
    void directionVariationChanged();
    void normalizedChanged();

private:
    QVector3D sample(const QQuick3DParticleData &d) override;
    QVector3D m_direction = {0.0f, 100.0f, 0.0f};
    QVector3D m_directionVariation;
    bool m_normalized = false;
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLEVECTORDIRECTION_H
