// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLEATTRACTOR_H
#define QQUICK3DPARTICLEATTRACTOR_H

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

#include <QtQuick3DParticles/private/qquick3dparticleaffector_p.h>
#include <QtQuick3DParticles/private/qquick3dparticleshape_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleAttractor : public QQuick3DParticleAffector
{
    Q_OBJECT
    Q_PROPERTY(QVector3D positionVariation READ positionVariation WRITE setPositionVariation NOTIFY positionVariationChanged)
    Q_PROPERTY(QQuick3DParticleAbstractShape *shape READ shape WRITE setShape NOTIFY shapeChanged)
    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(int durationVariation READ durationVariation WRITE setDurationVariation NOTIFY durationVariationChanged)
    Q_PROPERTY(bool hideAtEnd READ hideAtEnd WRITE setHideAtEnd NOTIFY hideAtEndChanged)
    Q_PROPERTY(bool useCachedPositions READ useCachedPositions WRITE setUseCachedPositions NOTIFY useCachedPositionsChanged)
    Q_PROPERTY(int positionsAmount READ positionsAmount WRITE setPositionsAmount NOTIFY positionsAmountChanged)
    QML_NAMED_ELEMENT(Attractor3D)
    QML_ADDED_IN_VERSION(6, 2)

public:
    QQuick3DParticleAttractor(QQuick3DNode *parent = nullptr);

    QVector3D positionVariation() const;
    QQuick3DParticleAbstractShape *shape() const;
    int duration() const;
    int durationVariation() const;
    bool hideAtEnd() const;
    bool useCachedPositions() const;
    int positionsAmount() const;

public Q_SLOTS:
    void setPositionVariation(const QVector3D &positionVariation);
    void setShape(QQuick3DParticleAbstractShape *shape);
    void setDuration(int duration);
    void setDurationVariation(int durationVariation);
    void setHideAtEnd(bool hideAtEnd);
    void setUseCachedPositions(bool useCachedPositions);
    void setPositionsAmount(int positionsAmount);

Q_SIGNALS:
    void positionVariationChanged();
    void shapeChanged();
    void durationChanged();
    void durationVariationChanged();
    void hideAtEndChanged();
    void useCachedPositionsChanged();
    void positionsAmountChanged();

protected:
    void prepareToAffect() override;
    void affectParticle(const QQuick3DParticleData &sd, QQuick3DParticleDataCurrent *d, float time) override;

private:
    void updateShapePositions();

    QQuick3DParticleAbstractShape *m_shape = nullptr;
    QList<QVector3D> m_shapePositionList;
    QVector3D m_centerPos;
    QMatrix4x4 m_particleTransform;
    bool m_shapeDirty = false;
    int m_duration = -1;
    int m_durationVariation = 0;
    QVector3D m_positionVariation;
    bool m_hideAtEnd = false;
    bool m_useCachedPositions = true;
    int m_positionsAmount = 0;
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLEATTRACTOR_H
