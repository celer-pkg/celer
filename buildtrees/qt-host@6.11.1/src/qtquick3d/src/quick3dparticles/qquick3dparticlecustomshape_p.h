// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLECUSTOMSHAPE_H
#define QQUICK3DPARTICLECUSTOMSHAPE_H

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

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleCustomShape : public QQuick3DParticleAbstractShape
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(bool randomizeData READ randomizeData WRITE setRandomizeData NOTIFY randomizeDataChanged)
    QML_NAMED_ELEMENT(ParticleCustomShape3D)
    QML_ADDED_IN_VERSION(6, 3)
public:
    explicit QQuick3DParticleCustomShape(QObject *parent = nullptr);

    QUrl source() const;
    bool randomizeData() const;

    // Returns point inside this shape
    QVector3D getPosition(int particleIndex) override;
    Q_REVISION(6, 10) QVector3D getSurfaceNormal(int particleIndex) override;

public Q_SLOTS:
    void setSource(const QUrl &source);
    void setRandomizeData(bool random);

Q_SIGNALS:
    void sourceChanged();
    void randomizeDataChanged();

private:
    void loadFromSource();
    void doRandomizeData();

    QUrl m_source;
    bool m_random = false;
    bool m_randomizeDirty = false;
    QList<QVector3D> m_positions;
    QVector3D m_center;
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLECUSTOMSHAPE_H
