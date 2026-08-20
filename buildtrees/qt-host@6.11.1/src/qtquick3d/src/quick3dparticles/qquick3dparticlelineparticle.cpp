// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticlelineparticle_p.h"
#include "qquick3dparticlesystem_p.h"
#include "qquick3dparticlerandomizer_p.h"
#include "qquick3dparticleutils_p.h"
#include <private/qquick3dobject_p.h>

#include <algorithm>

QT_BEGIN_NAMESPACE

/*!
    \qmltype LineParticle3D
    \inherits SpriteParticle3D
    \inqmlmodule QtQuick3D.Particles3D
    \brief Line particle.
    \since 6.4

    The LineParticle3D creates line shaped sprite particles.

    The line is created from the path of the particle when it moves.
    The length of the line is specified either by the \l length parameter
    or the segment count and minimum delta between points. In latter case the
    length of the line may vary if the particles speed varies.
*/

QQuick3DParticleLineParticle::QQuick3DParticleLineParticle(QQuick3DNode *parent)
    : QQuick3DParticleSpriteParticle(parent)
{
}

QQuick3DParticleLineParticle::~QQuick3DParticleLineParticle()
{
}

/*!
    \qmlproperty int LineParticle3D::segmentCount

    This property holds the number of segments in the line. The line is drawn using
    segment + 1 points, where the additional one comes from the particles current position.
    The default value is 1.
*/
int QQuick3DParticleLineParticle::segmentCount() const
{
    return m_segmentCount;
}

/*!
    \qmlproperty real LineParticle3D::alphaFade

    This property holds the alpha fade factor of the line. The alphaFade value range is [0, 1].
    When the value is greater than 0.0, causes the line to fade the further the segment is from the
    first particle segment. The alpha for a segment is calculated like this:
        segmentAlpha(s) = (1.0 - alphaFade) ^ s, where s is the segment index.
    The default value is 0.0.
*/
float QQuick3DParticleLineParticle::alphaFade() const
{
    return m_alphaFade;
}

/*!
    \qmlproperty real LineParticle3D::scaleMultiplier

    This property holds the scale multiplier of the line. The scaleMultiplier value range is [0, 2].
    The scaleMultiplier modifies the line size for the line segments. If the value is less than 1.0,
    the line gets smaller the further a segment is from the first segment and if the value is greater
    than 1.0 the line gets bigger. The size for a segment is calculated like this:
        size(s) = scaleMultiplier ^ s, where s is the segment index.
*/
float QQuick3DParticleLineParticle::scaleMultiplier() const
{
    return m_scaleMultiplier;
}

/*!
    \qmlproperty real LineParticle3D::texcoordMultiplier

    This property holds the texture coordinate multiplier of the line. This value is factored
    to the texture coordinate values of the line. The default value is 1.0.
*/
float QQuick3DParticleLineParticle::texcoordMultiplier() const
{
    return m_texcoordMultiplier;
}

/*!
    \qmlproperty real LineParticle3D::length

    This property holds the length of the line. If the value is set, the lines length is limited
    to the value. In this case the minimum delta of the line is the length divided
    by the segment count. If the value is not set, the line length varies based on the
    speed the particle moves as well as segment count and minimum delta. The default value is -1.0.
*/
float QQuick3DParticleLineParticle::length() const
{
    return m_length;
}

/*!
    \qmlproperty real LineParticle3D::lengthVariation

    This property holds the length variation applied to each line. Variation is applied
    only if the \l length property is also set. The resulting line length clamps to positive
    values.

    The default value is 0.0.
*/

float QQuick3DParticleLineParticle::lengthVariation() const
{
    return m_lengthVariation;
}

/*!
    \qmlproperty real LineParticle3D::lengthDeltaMin

    This property holds the minimum length between segment points. This parameter is
    ignored if the length parameter is set. The default value is 10.0.
*/
float QQuick3DParticleLineParticle::lengthDeltaMin() const
{
    return m_lengthDeltaMin;
}

/*!
    \qmlproperty int LineParticle3D::eolFadeOutDuration

    This property holds the end-of-life fade-out duration of the line. If set, each line remains
    in the place it was when the particle reached end of its lifetime, then fades out during this
    time period. The default value is 0.
*/
int QQuick3DParticleLineParticle::eolFadeOutDuration() const
{
    return m_eolFadeOutDuration;
}

/*!
    \qmlproperty enumeration LineParticle3D::TexcoordMode

    Defines the texture coordinate mode of line particle.

    \value LineParticle3D.Absolute
        Texture coordinates are specified relative to the world position.
    \value LineParticle3D.Relative
        Texture coordinates are specified relative to line first line point.
    \value LineParticle3D.Fill
        Texture coordinates are specified such that the texture fills the whole line.
*/

/*!
    \qmlproperty TexcoordMode LineParticle3D::texcoordMode

    This property holds the texture coordinate mode of the line.
*/
QQuick3DParticleLineParticle::TexcoordMode QQuick3DParticleLineParticle::texcoordMode() const
{
    return m_texcoordMode;
}

void QQuick3DParticleLineParticle::setSegmentCount(int count)
{
    count = qMax(1, count);
    if (m_segmentCount == count)
        return;
    m_segmentCount = count;
    handleSegmentCountChanged();
    Q_EMIT segmentCountChanged();
}

void QQuick3DParticleLineParticle::setAlphaFade(float fade)
{
    fade = qBound(0.0f, fade, 1.0f);
    if (qFuzzyCompare(m_alphaFade, fade))
        return;
    m_alphaFade = fade;
    Q_EMIT alphaFadeChanged();
}

void QQuick3DParticleLineParticle::setScaleMultiplier(float multiplier)
{
    multiplier = qBound(0.0f, multiplier, 2.0f);
    if (qFuzzyCompare(m_scaleMultiplier, multiplier))
        return;
    m_scaleMultiplier = multiplier;
    Q_EMIT scaleMultiplierChanged();
}

void QQuick3DParticleLineParticle::setTexcoordMultiplier(float multiplier)
{
    if (qFuzzyCompare(m_texcoordMultiplier, multiplier))
        return;
    m_texcoordMultiplier = multiplier;
    Q_EMIT texcoordMultiplierChanged();
}

void QQuick3DParticleLineParticle::setLength(float length)
{
    length = length != -1.0f ? qMax(length, 0.0f) : -1.0f;
    if (qFuzzyCompare(m_length, length))
        return;
    m_length = length;
    Q_EMIT lengthChanged();
}

void QQuick3DParticleLineParticle::setLengthVariation(float lengthVariation)
{
    lengthVariation = qMax(lengthVariation, 0.0f);
    if (qFuzzyCompare(m_lengthVariation, lengthVariation))
        return;
    m_lengthVariation = lengthVariation;
    Q_EMIT lengthVariationChanged();
}

void QQuick3DParticleLineParticle::setLengthDeltaMin(float min)
{
    min = qMax(min, 0.0f);
    if (qFuzzyCompare(m_lengthDeltaMin, min))
        return;
    m_lengthDeltaMin = min;
    Q_EMIT lengthDeltaMinChanged();
}

void QQuick3DParticleLineParticle::setEolFadeOutDuration(int duration)
{
    duration = qMax(0, duration);
    if (duration == m_eolFadeOutDuration)
        return;
    m_eolFadeOutDuration = duration;
    Q_EMIT eolFadeOutDurationChanged();
}

void QQuick3DParticleLineParticle::setTexcoordMode(TexcoordMode mode)
{
    if (mode == m_texcoordMode)
        return;
    m_texcoordMode = mode;
    Q_EMIT texcoordModeChanged();
}

QSSGRenderParticles::FeatureLevel lineFeatureLevel(QQuick3DParticleSpriteParticle::FeatureLevel in)
{
    switch (in) {
    case QQuick3DParticleSpriteParticle::FeatureLevel::Simple:
         return QSSGRenderParticles::FeatureLevel::Line;
    case QQuick3DParticleSpriteParticle::FeatureLevel::Mapped:
         return QSSGRenderParticles::FeatureLevel::LineMapped;
    case QQuick3DParticleSpriteParticle::FeatureLevel::Animated:
         return QSSGRenderParticles::FeatureLevel::LineAnimated;
    case QQuick3DParticleSpriteParticle::FeatureLevel::SimpleVLight:
         return QSSGRenderParticles::FeatureLevel::LineVLight;
    case QQuick3DParticleSpriteParticle::FeatureLevel::MappedVLight:
         return QSSGRenderParticles::FeatureLevel::LineMappedVLight;
    case QQuick3DParticleSpriteParticle::FeatureLevel::AnimatedVLight:
         return QSSGRenderParticles::FeatureLevel::LineAnimatedVLight;
    }
    return QSSGRenderParticles::FeatureLevel::Line;
}

QSSGRenderGraphObject *QQuick3DParticleLineParticle::updateLineNode(QSSGRenderGraphObject *node)
{
    auto particles = static_cast<QSSGRenderParticles *>(node);

    float frames = 1.0f;
    if (sprite() && spriteSequence())
        frames = float(spriteSequence()->frameCount());

    particles->m_sizeModifier = m_scaleMultiplier;
    particles->m_alphaFade = 1.0f - m_alphaFade;
    if (m_texcoordMode == TexcoordMode::Fill)
        particles->m_texcoordScale = frames * m_texcoordMultiplier;
    else
        particles->m_texcoordScale = frames / particleScale() * m_texcoordMultiplier;
    particles->m_featureLevel = lineFeatureLevel(m_featureLevel);

    return particles;
}

void QQuick3DParticleLineParticle::handleMaxAmountChanged(int amount)
{
    if (m_lineData.size() == amount)
        return;

    m_lineData.resize(m_segmentCount * amount);
    m_lineHeaderData.resize(amount);
    QQuick3DParticleSpriteParticle::handleMaxAmountChanged(amount);
}

void QQuick3DParticleLineParticle::handleSystemChanged(QQuick3DParticleSystem *system)
{
    for (PerEmitterData &value : m_perEmitterData) {
        delete value.particleUpdateNode;
        value.particleUpdateNode = new LineParticleUpdateNode(system);
        value.particleUpdateNode->m_particle = this;
    }
}

QSSGRenderGraphObject *QQuick3DParticleLineParticle::LineParticleUpdateNode::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (m_particle) {
        QQuick3DParticleLineParticle *lineParticle = qobject_cast<QQuick3DParticleLineParticle *>(m_particle);
        node = lineParticle->updateParticleNode(this, node);
        lineParticle->updateLineNode(node);
        QQuick3DNode::updateSpatialNode(node);
        Q_QUICK3D_PROFILE_ASSIGN_ID_SG(lineParticle, node);
        auto particles = static_cast<QSSGRenderParticles *>(node);

        lineParticle->updateLineBuffer(this, particles);

        m_nodeDirty = false;
    }
    return node;
}

void QQuick3DParticleLineParticle ::reset()
{
    QQuick3DParticleSpriteParticle::reset();
    m_lineData.fill({});
    m_lineHeaderData.fill({});
    m_fadeOutData.clear();
}

void QQuick3DParticleLineParticle::commitParticles(float time)
{
    QQuick3DParticleSpriteParticle::commitParticles(time);

    for (auto iter = m_fadeOutData.begin(); iter != m_fadeOutData.end(); ) {
        if (time >= iter->beginTime && time < iter->endTime)
            iter++;
        else
            iter = m_fadeOutData.erase(iter);
    }
}

int QQuick3DParticleLineParticle::nextCurrentIndex(const QQuick3DParticleEmitter *emitter)
{
    if (!m_perEmitterData.contains(emitter)) {
        m_perEmitterData.insert(emitter, PerEmitterData());
        auto &perEmitter = m_perEmitterData[emitter];
        perEmitter.particleUpdateNode = new LineParticleUpdateNode(system());
        perEmitter.emitter = emitter;
        perEmitter.particleUpdateNode->m_particle = this;
        perEmitter.emitterIndex = m_nextEmitterIndex++;
    }
    int index = QQuick3DParticleSpriteParticle::nextCurrentIndex(emitter);
    clearSegment(index);
    m_lineHeaderData[index].emitterIndex = m_perEmitterData[emitter].emitterIndex;
    if (m_length > 0.0f)
        m_lineHeaderData[index].length = qMax(0.0f, m_length + m_lengthVariation * (system()->rand()->get(index) - 0.5f));
    return index;
}

void QQuick3DParticleLineParticle::setParticleData(int particleIndex,
                     const QVector3D &position,
                     const QVector3D &rotation,
                     const QVector4D &color,
                     float size, float age,
                     float animationFrame)
{
    auto &dst = m_spriteParticleData[particleIndex];
    bool update = size > 0.0f || dst.size > 0.0f;
    QQuick3DParticleSpriteParticle::setParticleData(particleIndex, position, rotation, color, size, age, animationFrame);
    if (update)
        updateLineSegment(particleIndex);
}

void QQuick3DParticleLineParticle::resetParticleData(int particleIndex)
{
    LineDataHeader *header = m_lineHeaderData.data() + particleIndex;
    if (header->pointCount) {
        header->currentIndex = 0;
        header->pointCount = 0;
    }
    QQuick3DParticleSpriteParticle::resetParticleData(particleIndex);
}

void QQuick3DParticleLineParticle::saveLineSegment(int particleIndex, float time)
{
    if (m_eolFadeOutDuration > 0 && m_lineHeaderData[particleIndex].pointCount > 0) {
        FadeOutLineData data;
        data.endPoint = m_spriteParticleData[particleIndex];
        data.beginTime = time;
        data.endTime = time + m_eolFadeOutDuration * 0.001f;
        data.timeFactor = 1000.0f / ((float)m_eolFadeOutDuration);
        data.header = m_lineHeaderData[particleIndex];
        data.lineData = m_lineData.mid(particleIndex * m_segmentCount, m_segmentCount);
        data.emitterIndex = m_spriteParticleData[particleIndex].emitterIndex;
        m_fadeOutData.emplaceBack(data);
        clearSegment(particleIndex);
    }
}

static QVector3D qt_normalFromRotation(const QVector3D &eulerRotation)
{
    float x = qDegreesToRadians(eulerRotation.x());
    float y = qDegreesToRadians(eulerRotation.y());
    if (qFuzzyIsNull(x) && qFuzzyIsNull(y))
        return QVector3D(0, 0, -1);
    float a = qCos(x);
    float b = qSin(x);
    float c = qCos(y);
    float d = qSin(y);
    return QVector3D(d, -b * c, a * c);
}

void QQuick3DParticleLineParticle::updateLineBuffer(LineParticleUpdateNode *updateNode, QSSGRenderGraphObject *spatialNode)
{
    const auto &perEmitter = perEmitterData(updateNode);
    QSSGRenderParticles *node = static_cast<QSSGRenderParticles *>(spatialNode);
    if (!node)
        return;

    int lineCount = 0;
    for (int i = 0; i < m_lineHeaderData.size(); i++) {
        if (m_lineHeaderData[i].pointCount && m_lineHeaderData[i].emitterIndex == perEmitter.emitterIndex)
            lineCount++;
    }
    int totalCount = lineCount;
    if (m_perEmitterData.size() > 1) {
        for (int i = 0; i < m_fadeOutData.size(); i++) {
            if (m_fadeOutData[i].emitterIndex == perEmitter.emitterIndex)
                totalCount++;
        }
    } else {
        totalCount += m_fadeOutData.size();
    }

    if (node->m_particleBuffer.particleCount() != totalCount)
        node->m_particleBuffer.resizeLine(totalCount, m_segmentCount + 1);

    if (!totalCount) return;

    const int segments = m_segmentCount;
    const int particlesPerSlice = node->m_particleBuffer.particlesPerSlice();
    const int sliceStride = node->m_particleBuffer.sliceStride();
    int sliceParticleIdx = 0;
    int slice = 0;
    char *dest = node->m_particleBuffer.pointer();
    QSSGBounds3 bounds;

    const LineDataHeader *header = m_lineHeaderData.constData();
    const LineData *lineData = m_lineData.constData();
    const SpriteParticleData *src = m_spriteParticleData.constData();

    auto nextParticle = [](char *&buffer, int &slice, int &sliceParticleIdx, int particlesPerSlice, int sliceStride) -> QSSGLineParticle* {
        QSSGLineParticle *ret = reinterpret_cast<QSSGLineParticle *>(buffer) + sliceParticleIdx;
        sliceParticleIdx++;
        if (sliceParticleIdx == particlesPerSlice) {
            slice++;
            buffer += sliceStride;
            sliceParticleIdx = 0;
        }
        return ret;
    };

    auto genLine = [&](const SpriteParticleData &sdata, const LineDataHeader &header, const LineData *tdata,
            QSSGBounds3 &bounds, int segments, float particleScale, float alpha,
            char *&buffer, int &slice, int &sliceParticleIdx, int particlesPerSlice, int sliceStride, bool absolute, bool fill) {
        QSSGLineParticle *particle = nextParticle(buffer, slice, sliceParticleIdx, particlesPerSlice, sliceStride);
        int idx = header.currentIndex;
        particle->color = sdata.color;
        particle->color.setW(sdata.color.w() * alpha);
        QVector3D tangent = (tdata[idx].position - sdata.position).normalized();
        QVector3D binormal = QVector3D::crossProduct(qt_normalFromRotation(sdata.rotation), tangent);
        particle->binormal = binormal;
        particle->position = sdata.position;
        particle->age = sdata.age;
        particle->animationFrame = sdata.animationFrame;
        particle->size = sdata.size * particleScale;
        float partialLength = (tdata[idx].position - sdata.position).length();
        float length0 = tdata[idx].length + partialLength;
        particle->length = 0.0f;
        float lineLength = header.length;
        int lastIdx = (idx + 1 + segments - header.pointCount) % segments;
        float lengthScale = -1.0f;

        if (absolute) {
            particle->length = length0;
            length0 = 0;
        }

        if (fill) {
            if (lineLength > 0.0f) {
                lengthScale = -1.0f / lineLength;
            } else {
                float totalLength = tdata[idx].length - tdata[lastIdx].length;
                if (header.pointCount < segments)
                    totalLength += partialLength;
                if (!qFuzzyIsNull(totalLength))
                    lengthScale = -1.0f / totalLength;
            }
        }
        bounds.include(sdata.position);

        QSSGLineParticle *prevGood = particle;
        int segmentIdx = 0;
        int prevIdx = 0;
        Q_ASSERT(header.pointCount <= m_segmentCount);

        if (header.length >= 0.0f) {
            float totalLength = 0;
            float prevLength = tdata[idx].length + partialLength;
            for (segmentIdx = 0; segmentIdx < header.pointCount && totalLength < header.length; segmentIdx++) {
                particle = nextParticle(buffer, slice, sliceParticleIdx, particlesPerSlice, sliceStride);
                particle->size = tdata[idx].size * particleScale;
                if (particle->size > 0.0f) {
                    bounds.include(tdata[idx].position);
                    particle->color = tdata[idx].color;
                    particle->color.setW(tdata[idx].color.w() * alpha);
                    particle->binormal = tdata[idx].binormal;
                    particle->position = tdata[idx].position;
                    particle->animationFrame = sdata.animationFrame;
                    particle->age = sdata.age;
                    particle->length = (length0 - tdata[idx].length) * lengthScale;
                    float segmentLength = prevLength - tdata[idx].length;
                    prevLength = tdata[idx].length;
                    if (totalLength + segmentLength > header.length) {
                        float diff = totalLength + segmentLength - header.length;
                        particle->position -= tdata[idx].tangent * diff;
                        particle->length -= diff * lengthScale;
                        segmentLength -= diff;
                    }
                    totalLength += segmentLength;
                    prevGood = particle;
                    prevIdx = idx;
                }
                idx = idx ? (idx - 1) : (segments - 1);
            }
        } else {
            for (segmentIdx = 0; segmentIdx < header.pointCount; segmentIdx++) {
                particle = nextParticle(buffer, slice, sliceParticleIdx, particlesPerSlice, sliceStride);
                particle->size = tdata[idx].size * particleScale;
                if (particle->size > 0.0f) {
                    bounds.include(tdata[idx].position);
                    particle->color = tdata[idx].color;
                    particle->color.setW(tdata[idx].color.w() * alpha);
                    particle->binormal = tdata[idx].binormal;
                    particle->position = tdata[idx].position;
                    particle->animationFrame = sdata.animationFrame;
                    particle->age = sdata.age;
                    particle->length = (length0 - tdata[idx].length) * lengthScale;
                    prevGood = particle;
                    prevIdx = idx;
                }
                idx = idx ? (idx - 1) : (segments - 1);
            }
        }
        for (;segmentIdx < segments; segmentIdx++) {
            particle = nextParticle(buffer, slice, sliceParticleIdx, particlesPerSlice, sliceStride);
            *particle = *prevGood;
            particle->size = 0.0f;
            particle->length = 0.0f;
            idx = idx ? (idx - 1) : (segments - 1);
        }
        // Do only for full segment
        if (prevGood == particle && header.length < 0.0f && segments > 1) {
            prevGood->position -= tdata[prevIdx].tangent * partialLength;
            if (!fill)
                prevGood->length -= partialLength * lengthScale;
        }
    };

    const bool absolute = m_texcoordMode == TexcoordMode::Absolute;
    const bool fill = m_texcoordMode == TexcoordMode::Fill;
    int i = 0;
    while (i < lineCount) {
        if (header->pointCount && header->emitterIndex == perEmitter.emitterIndex) {
            genLine(*src, *header, lineData, bounds, segments, particleScale(), 1.0f, dest,
                    slice, sliceParticleIdx, particlesPerSlice, sliceStride, absolute, fill);
            i++;
        }
        header++;
        lineData += segments;
        src++;
    }

    float time = system()->currentTime() * 0.001f;
    for (const FadeOutLineData &fdata : m_fadeOutData) {
        if (fdata.emitterIndex == perEmitter.emitterIndex) {
            float factor = 1.0f - (time - fdata.beginTime) * fdata.timeFactor;
            genLine(fdata.endPoint, fdata.header, fdata.lineData.data(), bounds, segments,
                    particleScale(), factor, dest, slice, sliceParticleIdx, particlesPerSlice,
                    sliceStride, absolute, fill);
        }
    }
    node->m_particleBuffer.setBounds(bounds);
}

void QQuick3DParticleLineParticle::handleSegmentCountChanged()
{
    markNodesDirty();
    m_lineData.resize(m_segmentCount * m_maxAmount);
    m_lineData.fill({});
    m_lineHeaderData.resize(m_maxAmount);
    m_lineHeaderData.fill({});
    m_fadeOutData.clear();
    if (!m_spriteParticleData.isEmpty()) {
        auto count = qMin(m_maxAmount, m_spriteParticleData.size());
        for (int i = 0; i < count; i++)
            m_lineHeaderData[i].emitterIndex = m_spriteParticleData[i].emitterIndex;
    }
}

void QQuick3DParticleLineParticle::updateLineSegment(int particleIndex)
{
    if (m_lineData.isEmpty()) {
        qWarning () << "Line particle updated before having been initialized";
        return;
    }
    LineDataHeader *header = m_lineHeaderData.data() + particleIndex;
    int idx = header->currentIndex;
    LineData *cur = m_lineData.data() + particleIndex * m_segmentCount;
    LineData *prev = header->pointCount ? cur + idx : nullptr;
    const SpriteParticleData &src = m_spriteParticleData.at(particleIndex);

    if (prev && m_segmentCount > 1) {
        float length = (prev->position - src.position).length();
        float minLength = m_lengthDeltaMin;
        if (header->length >= 0.0f)
            minLength = header->length / (m_segmentCount - 1);
        if (length < minLength)
            return;
    }

    if (header->pointCount < m_segmentCount)
        header->pointCount++;

    if (prev)
        idx = (idx + 1) % m_segmentCount;
    header->currentIndex = idx;
    cur = cur + idx;

    cur->color = src.color;
    cur->size = src.size;
    cur->normal = qt_normalFromRotation(src.rotation);

    if (prev && m_segmentCount == 1) {
        QVector3D t = prev->position - src.position;
        float l = t.length();
        t.normalize();
        float minLength = m_lengthDeltaMin;
        if (header->length >= 0.0f)
            minLength = header->length;
        cur->position = src.position + minLength * t;
        cur->length += l;
        cur->tangent = t;
        cur->binormal = QVector3D::crossProduct(cur->normal, t);
    } else {
        cur->position = src.position;
        cur->length = 0.0f;
    }

    if (prev && prev != cur) {
        prev->tangent = prev->position - src.position;
        cur->length = prev->tangent.length();
        prev->tangent /= cur->length;
        cur->length += prev->length;
        cur->binormal = QVector3D::crossProduct(cur->normal, prev->tangent);
        if (header->pointCount == 1)
            prev->binormal = cur->binormal;
        else
            prev->binormal = (prev->binormal + cur->binormal).normalized();
    }
}

void QQuick3DParticleLineParticle::clearSegment(int particleIndex)
{
    if (m_lineData.isEmpty())
        return;
    LineDataHeader *header = m_lineHeaderData.data() + particleIndex;
    if (header->pointCount) {
        auto data = m_lineData.begin() + particleIndex * m_segmentCount;
        std::fill_n(data, m_segmentCount, LineData());
    }
    header->emitterIndex = -1;
    header->currentIndex = 0;
    header->pointCount = 0;
    header->length = -1.0f;
}

QT_END_NAMESPACE
