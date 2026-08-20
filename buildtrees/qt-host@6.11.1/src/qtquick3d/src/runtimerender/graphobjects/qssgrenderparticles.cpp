// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include <QtQuick3DRuntimeRender/private/qssgrenderparticles_p.h>
#include <cmath>

QT_BEGIN_NAMESPACE

static int divisibleBy(int a, int b)
{
    return (a % b) ? a + b - (a % b) : a;
}
static int ceilDivide(int a, int b)
{
    int x = a / b;
    int y = (a % b) ? 1 : 0;
    return x + y;
}

void QSSGParticleBuffer::resize(int particleCount, int particleSize)
{
    if (particleCount == 0) {
        m_particlesPerSlice = 0;
        m_particleCount = 0;
        m_sliceStride = 0;
        m_size = QSize();
        m_particleBuffer.resize(0);
        return;
    }
    int vec4PerParticle = ceilDivide(particleSize, 16);
    int vec4s = particleCount * vec4PerParticle;
    int width = divisibleBy(std::sqrt(vec4s), vec4PerParticle);
    int height = ceilDivide(vec4s, width);
    m_particlesPerSlice = width / vec4PerParticle;
    m_particleCount = particleCount;
    width = divisibleBy(width, 4);
    height = divisibleBy(height, 4);
    m_sliceStride = width * 16;
    m_size = QSize(width, height);
    m_particleBuffer.resize(m_sliceStride * height);
}

void QSSGParticleBuffer::resizeLine(int particleCount, int segmentCount)
{
    m_segments = segmentCount;
    resize(particleCount * segmentCount, sizeof(QSSGLineParticle));
}

void QSSGParticleBuffer::setBounds(const QSSGBounds3& bounds)
{
    m_bounds = bounds;
    m_serial++;
}

char *QSSGParticleBuffer::pointer()
{
    return m_particleBuffer.data();
}

const char *QSSGParticleBuffer::pointer() const
{
    return m_particleBuffer.constData();
}

int QSSGParticleBuffer::particlesPerSlice() const
{
    return m_particlesPerSlice;
}

int QSSGParticleBuffer::sliceStride() const
{
    return m_sliceStride;
}

int QSSGParticleBuffer::particleCount() const
{
    return m_particleCount;
}

QSize QSSGParticleBuffer::size() const
{
    return m_size;
}

int QSSGParticleBuffer::sliceCount() const
{
    return m_size.height();
}

QByteArray QSSGParticleBuffer::data() const
{
    return m_particleBuffer;
}

int QSSGParticleBuffer::bufferSize() const
{
    return m_particleBuffer.size();
}

int QSSGParticleBuffer::serial() const
{
    return m_serial;
}

int QSSGParticleBuffer::segments() const
{
    return m_segments;
}

QSSGBounds3 QSSGParticleBuffer::bounds() const
{
    return m_bounds;
}

QSSGRenderParticles::QSSGRenderParticles()
    : QSSGRenderNode(QSSGRenderGraphObject::Type::Particles)
{

}

QT_END_NAMESPACE
