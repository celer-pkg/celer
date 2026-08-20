// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RENDER_PARTICLES_H
#define QSSG_RENDER_PARTICLES_H

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

#include <QtQuick3DRuntimeRender/private/qssgrendernode_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendercustommaterial_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderlight_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrenderparticleshaderkeys_p.h>
#include <QtQuick3DUtils/private/qssgrenderbasetypes_p.h>

QT_BEGIN_NAMESPACE

struct QSSGRenderImage;

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGParticleSimple
{
    QVector3D position;
    float size;
    QVector3D rotation;
    float age;
    QVector4D color;
    // total 48 bytes
};

Q_STATIC_ASSERT_X(sizeof(QSSGParticleSimple) == 48, "size of QSSGParticleSimple must be 48");

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGParticleAnimated
{
    QVector3D position;
    float size;
    QVector3D rotation;
    float age;
    QVector4D color;
    float animationFrame;
    // Padding for full 4 * 16 bytes, take into use as needed.
    // See particleSize in vertex shader
    QVector3D unusedPadding;
    // total 64 bytes
};

Q_STATIC_ASSERT_X(sizeof(QSSGParticleAnimated) == 64, "size of QSSGParticleAnimated must be 64");

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGTriangleParticle
{
    QVector3D position; // particle position
    float size;
    QVector3D rotation;
    float age;
    QVector4D color;
    QVector3D center;   // center of the origin triangle
    float fill;
    // total 64 bytes
};

Q_STATIC_ASSERT_X(sizeof(QSSGTriangleParticle) == 64, "size of QSSGTriangleParticle must be 64");

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGLineParticle
{
    QVector3D position;
    float size;
    QVector4D color;
    QVector3D binormal;
    float animationFrame;
    float age;
    float length;
    QVector2D fill;
    // total 64 bytes
};

Q_STATIC_ASSERT_X(sizeof(QSSGLineParticle) == 64, "size of QSSGLineParticle must be 64");

struct Q_QUICK3DRUNTIMERENDER_EXPORT QSSGParticleBuffer
{
    void resize(int particleCount, int particleSize = sizeof(QSSGParticleSimple));
    void resizeLine(int particleCount, int segmentCount);
    void setBounds(const QSSGBounds3& bounds);

    char *pointer();
    const char *pointer() const;
    int particlesPerSlice() const;
    int sliceStride() const;
    int particleCount() const;
    int sliceCount() const;
    QSize size() const;
    QByteArray data() const;
    QSSGBounds3 bounds() const;
    int bufferSize() const;
    int serial() const;
    int segments() const;

private:
    int m_particlesPerSlice = 0;
    int m_sliceStride = 0;
    int m_particleCount = 0;
    int m_serial = 0;
    int m_segments = 0;
    QSize m_size;
    QByteArray m_particleBuffer;
    QSSGBounds3 m_bounds;
};

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderParticles : public QSSGRenderNode
{
public:
    enum class BlendMode : quint8
    {
        SourceOver = 0,
        Screen,
        Multiply
    };
    enum class FeatureLevel : quint8
    {
        Simple = 0,
        Mapped,
        Animated,
        SimpleVLight,
        MappedVLight,
        AnimatedVLight,
        Line,
        LineMapped,
        LineAnimated,
        LineVLight,
        LineMappedVLight,
        LineAnimatedVLight,
    };

    Q_DISABLE_COPY(QSSGRenderParticles)

    QSSGParticleBuffer m_particleBuffer;

    QVarLengthArray<QSSGRenderLight *, 4> m_lights;

    QSSGRenderParticles::BlendMode m_blendMode = BlendMode::SourceOver;
    QSSGRenderImage *m_sprite = nullptr;
    int m_spriteImageCount = 1;
    float m_depthBiasSq = 0.0f; // Squared as our sorting is based on the squared distance!
    float m_sizeModifier = 0.0f;
    float m_alphaFade = 0.0f;
    float m_texcoordScale = 1.0f;
    bool m_blendImages = true;
    bool m_billboard = true;
    bool m_hasTransparency = true;
    bool m_depthSorting = false;
    QSSGRenderImage *m_colorTable = nullptr;
    QSSGRenderParticles::FeatureLevel m_featureLevel = FeatureLevel::Simple;
    bool m_castsReflections = true;
    QSSGShaderParticleMaterialKey materialKey;

    QSSGRenderParticles();
    ~QSSGRenderParticles() = default;
};


QT_END_NAMESPACE

#endif
