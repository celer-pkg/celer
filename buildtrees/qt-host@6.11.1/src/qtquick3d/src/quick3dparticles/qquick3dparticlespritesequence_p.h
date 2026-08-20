// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLESPRITESEQUENCE_H
#define QQUICK3DPARTICLESPRITESEQUENCE_H

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

#include <QObject>
#include <QQmlEngine>
#include <QtQml/qqmlparserstatus.h>
#include <QtQuick3DParticles/qtquick3dparticlesglobal.h>
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DParticleSpriteParticle;

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleSpriteSequence : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(int frameCount READ frameCount WRITE setFrameCount NOTIFY frameCountChanged)
    Q_PROPERTY(int frameIndex READ frameIndex WRITE setFrameIndex NOTIFY frameIndexChanged)
    Q_PROPERTY(bool interpolate READ interpolate WRITE setInterpolate NOTIFY interpolateChanged)
    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(int durationVariation READ durationVariation WRITE setDurationVariation NOTIFY durationVariationChanged)
    Q_PROPERTY(bool randomStart READ randomStart WRITE setRandomStart NOTIFY randomStartChanged)
    Q_PROPERTY(AnimationDirection animationDirection READ animationDirection WRITE setAnimationDirection NOTIFY animationDirectionChanged)

    QML_NAMED_ELEMENT(SpriteSequence3D)
    Q_INTERFACES(QQmlParserStatus)
    QML_ADDED_IN_VERSION(6, 2)

public:
    enum AnimationDirection
    {
        Normal = 0,
        Reverse,
        Alternate,
        AlternateReverse,
        SingleFrame
    };
    Q_ENUM(AnimationDirection)

    QQuick3DParticleSpriteSequence(QObject *parent = nullptr);
    ~QQuick3DParticleSpriteSequence() override;

    int frameCount() const;
    int frameIndex() const;
    bool interpolate() const;
    int duration() const;
    int durationVariation() const;
    bool randomStart() const;
    AnimationDirection animationDirection() const;

public Q_SLOTS:
    void setFrameCount(int frameCount);
    void setFrameIndex(int frameIndex);
    void setInterpolate(bool interpolate);
    void setDuration(int duration);
    void setDurationVariation(int durationVariation);
    void setRandomStart(bool randomStart);
    void setAnimationDirection(QQuick3DParticleSpriteSequence::AnimationDirection animationDirection);

Q_SIGNALS:
    void frameCountChanged();
    void frameIndexChanged();
    void interpolateChanged();
    void durationChanged();
    void durationVariationChanged();
    void randomStartChanged();
    void animationDirectionChanged();

protected:
    // From QQmlParserStatus
    void componentComplete() override;
    void classBegin() override {}

private:
    friend class QQuick3DParticleSpriteParticle;
    friend class QQuick3DParticleSystem;

    void markNodesDirty();
    float firstFrame(int index, bool singleFrame);

    QQuick3DParticleSpriteParticle *m_parentParticle = nullptr;
    int m_frameCount = 1;
    int m_frameIndex = 0;
    bool m_interpolate = true;
    int m_duration = -1;
    int m_durationVariation = 0;
    bool m_randomStart = false;
    AnimationDirection m_animationDirection = AnimationDirection::Normal;
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLESPRITESEQUENCE_H
