// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLELINEPARTICLE_H
#define QQUICK3DPARTICLELINEPARTICLE_H

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

#include <QtQuick3DParticles/private/qquick3dparticlespriteparticle_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleLineParticle : public QQuick3DParticleSpriteParticle
{
    Q_OBJECT
    Q_PROPERTY(int segmentCount READ segmentCount WRITE setSegmentCount NOTIFY segmentCountChanged)
    Q_PROPERTY(float alphaFade READ alphaFade WRITE setAlphaFade NOTIFY alphaFadeChanged)
    Q_PROPERTY(float scaleMultiplier READ scaleMultiplier WRITE setScaleMultiplier NOTIFY scaleMultiplierChanged)
    Q_PROPERTY(float texcoordMultiplier READ texcoordMultiplier WRITE setTexcoordMultiplier NOTIFY texcoordMultiplierChanged)
    Q_PROPERTY(float length READ length WRITE setLength NOTIFY lengthChanged)
    Q_PROPERTY(float lengthVariation READ lengthVariation WRITE setLengthVariation NOTIFY lengthVariationChanged)
    Q_PROPERTY(float lengthDeltaMin READ lengthDeltaMin WRITE setLengthDeltaMin NOTIFY lengthDeltaMinChanged)
    Q_PROPERTY(int eolFadeOutDuration READ eolFadeOutDuration WRITE setEolFadeOutDuration NOTIFY eolFadeOutDurationChanged)
    Q_PROPERTY(TexcoordMode texcoordMode READ texcoordMode WRITE setTexcoordMode NOTIFY texcoordModeChanged)

    QML_NAMED_ELEMENT(LineParticle3D)
    QML_ADDED_IN_VERSION(6, 4)

public:
    enum TexcoordMode
    {
        Absolute,
        Relative,
        Fill,
    };
    Q_ENUM(TexcoordMode)

    QQuick3DParticleLineParticle(QQuick3DNode *parent = nullptr);
    ~QQuick3DParticleLineParticle() override;

    int segmentCount() const;
    float alphaFade() const;
    float scaleMultiplier() const;
    float texcoordMultiplier() const;
    float length() const;
    float lengthVariation() const;
    float lengthDeltaMin() const;
    int eolFadeOutDuration() const;
    TexcoordMode texcoordMode() const;

public Q_SLOTS:
    void setSegmentCount(int count);
    void setAlphaFade(float fade);
    void setScaleMultiplier(float multiplier);
    void setTexcoordMultiplier(float multiplier);
    void setLength(float length);
    void setLengthVariation(float length);
    void setLengthDeltaMin(float min);
    void setEolFadeOutDuration(int duration);
    void setTexcoordMode(QQuick3DParticleLineParticle::TexcoordMode mode);

Q_SIGNALS:
    void segmentCountChanged();
    void alphaFadeChanged();
    void scaleMultiplierChanged();
    void texcoordMultiplierChanged();
    void lengthChanged();
    void lengthVariationChanged();
    void lengthDeltaMinChanged();
    void eolFadeOutDurationChanged();
    void texcoordModeChanged();

private:

    struct LineDataHeader
    {
        int emitterIndex = -1;
        int pointCount = 0;
        int currentIndex = 0;
        float length = 0.0f;
    };
    struct LineData
    {
        QVector3D position;
        QVector3D normal;
        QVector4D color;
        QVector3D tangent;
        QVector3D binormal;
        float size = 0.0f;
        float length = 0.0f;
    };
    struct FadeOutLineData
    {
        int emitterIndex;
        SpriteParticleData endPoint;
        LineDataHeader header;
        QVector<LineData> lineData;
        float beginTime;
        float endTime;
        float timeFactor;
    };

    friend class QQuick3DParticleSystem;

    class LineParticleUpdateNode : public ParticleUpdateNode
    {
    public:
        LineParticleUpdateNode(QQuick3DNode *parent = nullptr)
            : ParticleUpdateNode(parent)
        {
        }
        QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    };

    void updateLineBuffer(LineParticleUpdateNode *updateNode, QSSGRenderGraphObject *node);
    QSSGRenderGraphObject *updateLineNode(QSSGRenderGraphObject *node);
    void handleSegmentCountChanged();
    void updateLineSegment(int particleIndex);
    void clearSegment(int particleIndex);
    void handleMaxAmountChanged(int amount) override;
    void handleSystemChanged(QQuick3DParticleSystem *system) override;
    void reset() override;
    void commitParticles(float time) override;
    int nextCurrentIndex(const QQuick3DParticleEmitter *emitter) override;
    void saveLineSegment(int particleIndex, float time);
    void setParticleData(int particleIndex,
                         const QVector3D &position,
                         const QVector3D &rotation,
                         const QVector4D &color,
                         float size, float age,
                         float animationFrame) override;
    void resetParticleData(int particleIndex) override;

    QVector<LineDataHeader> m_lineHeaderData;
    QVector<LineData> m_lineData;
    QVector<FadeOutLineData> m_fadeOutData;

    float m_alphaFade = 0.0f;
    float m_scaleMultiplier = 1.0f;
    float m_texcoordMultiplier = 1.0f;
    float m_lengthDeltaMin = 10.0f;
    float m_length = -1.0f;
    float m_lengthVariation = 0.0f;
    int m_segmentCount = 1;
    int m_eolFadeOutDuration = 0;
    TexcoordMode m_texcoordMode = TexcoordMode::Absolute;
};

QT_END_NAMESPACE

#endif
