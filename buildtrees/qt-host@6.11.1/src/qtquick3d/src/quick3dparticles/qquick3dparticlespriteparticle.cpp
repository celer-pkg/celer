// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticlespriteparticle_p.h"
#include "qquick3dparticleemitter_p.h"

#include <QtQuick3D/private/qquick3dobject_p.h>

#include <QtQuick3DUtils/private/qssgutils_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype SpriteParticle3D
    \inherits Particle3D
    \inqmlmodule QtQuick3D.Particles3D
    \brief Particle using a 2D sprite texture.
    \since 6.2

    The SpriteParticle3D is a logical particle element that creates particles
    from a 2D sprite texture.
*/

QQuick3DParticleSpriteParticle::QQuick3DParticleSpriteParticle(QQuick3DNode *parent)
    : QQuick3DParticle(parent)
{
    m_connections.insert("maxAmount", QObject::connect(this, &QQuick3DParticle::maxAmountChanged, this, [this]() {
        handleMaxAmountChanged(m_maxAmount);
    }));
    m_connections.insert("system", QObject::connect(this, &QQuick3DParticle::systemChanged, this, [this]() {
        handleSystemChanged(system());
    }));
    m_connections.insert("sortMode", QObject::connect(this, &QQuick3DParticle::sortModeChanged, this, [this]() {
        markNodesDirty();
    }));
}

QQuick3DParticleSpriteParticle::~QQuick3DParticleSpriteParticle()
{
    if (m_spriteSequence)
        m_spriteSequence->m_parentParticle = nullptr;
    for (const auto &connection : std::as_const(m_connections))
        QObject::disconnect(connection);
    deleteNodes();

    auto lightList = lights();
    qmlClearLights(&lightList);
}

void QQuick3DParticleSpriteParticle::deleteNodes()
{
    for (const PerEmitterData &value : std::as_const(m_perEmitterData)) {
        value.particleUpdateNode->m_particle = nullptr;
        delete value.particleUpdateNode;
    }
    m_perEmitterData.clear();
}

/*!
    \qmlproperty enumeration SpriteParticle3D::BlendMode

    Defines the blending mode for the particles.

    \value SpriteParticle3D.SourceOver
        Blend particles with SourceOver mode.
    \value SpriteParticle3D.Screen
        Blend particles with Screen mode.
    \value SpriteParticle3D.Multiply
        Blend particles with Multiply mode.
*/

/*!
    \qmlproperty BlendMode SpriteParticle3D::blendMode

    This property defines the blending mode used for rendering the particles.

    The default value is \c SpriteParticle3D.SourceOver.
*/
QQuick3DParticleSpriteParticle::BlendMode QQuick3DParticleSpriteParticle::blendMode() const
{
    return m_blendMode;
}

/*!
    \qmlproperty Texture SpriteParticle3D::sprite

    This property defines the \l Texture used for the particles.

    For example, to use "snowFlake.png" as the particles texture:

    \qml
    SpriteParticle3D {
        id: snowParticle
        ...
        sprite: Texture {
            source: "images/snowflake.png"
        }
    }
    \endqml
*/
QQuick3DTexture *QQuick3DParticleSpriteParticle::sprite() const
{
    return m_sprite;
}

/*!
    \qmlproperty SpriteSequence3D SpriteParticle3D::spriteSequence

    This property defines the sprite sequence properties for the particle.
    If the \l sprite texture contains a frame sequence, set this property
    to define the frame count, animation direction etc. features.
*/

QQuick3DParticleSpriteSequence *QQuick3DParticleSpriteParticle::spriteSequence() const
{
    return m_spriteSequence;
}

/*!
    \qmlproperty bool SpriteParticle3D::billboard

    This property defines if the particle texture should always be aligned
    face towards the screen.

    \note When set to \c true, \l Particle3D \l {Particle3D::alignMode}{alignMode}
    property does not have an effect.

    The default value is \c false.
*/
bool QQuick3DParticleSpriteParticle::billboard() const
{
    return m_billboard;
}

/*!
    \qmlproperty real SpriteParticle3D::particleScale

    This property defines the scale multiplier of the particles.
    To adjust the particles sizes in the emitter, use \ ParticleEmitter3D
    \l {ParticleEmitter3D::particleScale}{particleScale},
    \l {ParticleEmitter3D::particleEndScale}{particleEndScale}, and
    \l {ParticleEmitter3D::particleScaleVariation}{particleScaleVariation}
    properties.

    The default value is \c 5.0.
*/
float QQuick3DParticleSpriteParticle::particleScale() const
{
    return m_particleScale;
}

/*!
    \qmlproperty Texture SpriteParticle3D::colorTable

    This property defines the \l Texture used for coloring the particles.
    The image can be a 1D or a 2D texture. Horizontal pixels determine the particle color over its
    \l {ParticleEmitter3D::lifeSpan}{lifeSpan}. For example, when the particle is halfway through
    its life, it will have the color specified halfway across the image. If the image is 2D,
    vertical row is randomly selected for each particle. For example, a c {256 x 4} image
    contains \c 4 different coloring options for particles.
*/
QQuick3DTexture *QQuick3DParticleSpriteParticle::colorTable() const
{
    return m_colorTable;
}

/*!
    \qmlproperty list<Light> SpriteParticle3D::lights
    \since 6.3

    This property contains a list of \l [QtQuick3D QML] {Light}{lights} used
    for rendering the particles.
    \note For optimal performance, define lights only if they are needed and keep
    the amount of lights at minimum.
*/

QQmlListProperty<QQuick3DAbstractLight> QQuick3DParticleSpriteParticle::lights()
{
    return QQmlListProperty<QQuick3DAbstractLight>(this,
                                            nullptr,
                                            QQuick3DParticleSpriteParticle::qmlAppendLight,
                                            QQuick3DParticleSpriteParticle::qmlLightsCount,
                                            QQuick3DParticleSpriteParticle::qmlLightAt,
                                            QQuick3DParticleSpriteParticle::qmlClearLights);
}

/*!
    \qmlproperty real SpriteParticle3D::offsetX
    \since 6.3

    This property defines the particles offset in the X axis
*/
float QQuick3DParticleSpriteParticle::offsetX() const
{
    return m_offset.x();
}

/*!
    \qmlproperty real SpriteParticle3D::offsetY
    \since 6.3

    This property defines the particles offset in the Y axis
*/
float QQuick3DParticleSpriteParticle::offsetY() const
{
    return m_offset.y();
}

/*!
    \qmlproperty bool SpriteParticle3D::castsReflections
    \since 6.4

    When this property is set to \c true, the sprite is rendered by reflection probes and can be
    seen in the reflections.
*/
bool QQuick3DParticleSpriteParticle::castsReflections() const
{
    return m_castsReflections;
}

void QQuick3DParticleSpriteParticle::setBlendMode(BlendMode blendMode)
{
    if (m_blendMode == blendMode)
        return;
    m_blendMode = blendMode;
    markNodesDirty();
    Q_EMIT blendModeChanged();
}

void QQuick3DParticleSpriteParticle::setSprite(QQuick3DTexture *sprite)
{
    if (m_sprite == sprite)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DParticleSpriteParticle::setSprite, sprite, m_sprite);

    m_sprite = sprite;
    markNodesDirty();
    Q_EMIT spriteChanged();
}

void QQuick3DParticleSpriteParticle::setSpriteSequence(QQuick3DParticleSpriteSequence *spriteSequence)
{
    if (m_spriteSequence == spriteSequence)
        return;

    m_spriteSequence = spriteSequence;
    updateFeatureLevel();
    markNodesDirty();
    Q_EMIT spriteSequenceChanged();
}

void QQuick3DParticleSpriteParticle::setBillboard(bool billboard)
{
    if (m_billboard == billboard)
        return;
    m_billboard = billboard;
    markNodesDirty();
    Q_EMIT billboardChanged();
}

void QQuick3DParticleSpriteParticle::setParticleScale(float scale)
{
    if (qFuzzyCompare(scale, m_particleScale))
        return;
    m_particleScale = scale;
    markNodesDirty();
    Q_EMIT particleScaleChanged();
}

void QQuick3DParticleSpriteParticle::setColorTable(QQuick3DTexture *colorTable)
{
    if (m_colorTable == colorTable)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &QQuick3DParticleSpriteParticle::setColorTable, colorTable, m_colorTable);

    m_colorTable = colorTable;
    updateFeatureLevel();
    markNodesDirty();
    Q_EMIT colorTableChanged();
}

void QQuick3DParticleSpriteParticle::setOffsetX(float value)
{
    if (qFuzzyCompare(value, m_offset.x()))
        return;

    m_offset.setX(value);
    emit offsetXChanged();
}

void QQuick3DParticleSpriteParticle::setOffsetY(float value)
{
    if (qFuzzyCompare(value, m_offset.y()))
        return;

    m_offset.setY(value);
    emit offsetYChanged();
}

void QQuick3DParticleSpriteParticle::setCastsReflections(bool castsReflections)
{
    if (m_castsReflections == castsReflections)
        return;
    m_castsReflections = castsReflections;
    emit castsReflectionsChanged();
}

void QQuick3DParticleSpriteParticle::itemChange(QQuick3DObject::ItemChange change,
                                          const QQuick3DObject::ItemChangeData &value)
{
    if (change == QQuick3DObject::ItemSceneChange)
        updateSceneManager(value.sceneManager);
}

static QSSGRenderParticles::BlendMode mapBlendMode(QQuick3DParticleSpriteParticle::BlendMode mode)
{
    switch (mode) {
    case QQuick3DParticleSpriteParticle::SourceOver:
        return QSSGRenderParticles::BlendMode::SourceOver;
    case QQuick3DParticleSpriteParticle::Screen:
        return QSSGRenderParticles::BlendMode::Screen;
    case QQuick3DParticleSpriteParticle::Multiply:
        return QSSGRenderParticles::BlendMode::Multiply;
    }

    Q_UNREACHABLE_RETURN(QSSGRenderParticles::BlendMode::SourceOver);
}

QSSGRenderParticles::FeatureLevel QQuick3DParticleSpriteParticle::mapFeatureLevel(QQuick3DParticleSpriteParticle::FeatureLevel level)
{
    switch (level) {
    case QQuick3DParticleSpriteParticle::Simple:
        return QSSGRenderParticles::FeatureLevel::Simple;
    case QQuick3DParticleSpriteParticle::Mapped:
        return QSSGRenderParticles::FeatureLevel::Mapped;
    case QQuick3DParticleSpriteParticle::Animated:
        return QSSGRenderParticles::FeatureLevel::Animated;
    case QQuick3DParticleSpriteParticle::SimpleVLight:
        return QSSGRenderParticles::FeatureLevel::SimpleVLight;
    case QQuick3DParticleSpriteParticle::MappedVLight:
        return QSSGRenderParticles::FeatureLevel::MappedVLight;
    case QQuick3DParticleSpriteParticle::AnimatedVLight:
        return QSSGRenderParticles::FeatureLevel::AnimatedVLight;
    }

    Q_UNREACHABLE_RETURN(QSSGRenderParticles::FeatureLevel::Simple);
}

QSSGRenderGraphObject *QQuick3DParticleSpriteParticle::ParticleUpdateNode::updateSpatialNode(QSSGRenderGraphObject *node)
{
    if (m_particle) {
        node = m_particle->updateParticleNode(this, node);
        QQuick3DNode::updateSpatialNode(node);
        Q_QUICK3D_PROFILE_ASSIGN_ID_SG(m_particle, node);
        auto particles = static_cast<QSSGRenderParticles *>(node);

        if (m_particle->m_featureLevel == QQuick3DParticleSpriteParticle::Animated || m_particle->m_featureLevel == QQuick3DParticleSpriteParticle::AnimatedVLight)
            m_particle->updateAnimatedParticleBuffer(this, particles);
        else
            m_particle->updateParticleBuffer(this, particles);

        m_nodeDirty = false;
    }
    return node;
}

QQuick3DParticleSpriteParticle::PerEmitterData &QQuick3DParticleSpriteParticle::perEmitterData(const QQuick3DNode *updateNode)
{
    for (auto &perEmitter : m_perEmitterData) {
        if (perEmitter.particleUpdateNode == updateNode)
            return perEmitter;
    }
    return n_noPerEmitterData;
}

QQuick3DParticleSpriteParticle::PerEmitterData &QQuick3DParticleSpriteParticle::perEmitterData(int emitterIndex)
{
    for (auto &perEmitter : m_perEmitterData) {
        if (perEmitter.emitterIndex == emitterIndex)
            return perEmitter;
    }
    return n_noPerEmitterData;
}

QSSGRenderGraphObject *QQuick3DParticleSpriteParticle::updateParticleNode(const ParticleUpdateNode *updateNode,
                                                                          QSSGRenderGraphObject *node)
{
    if (!node) {
        markAllDirty();
        node = new QSSGRenderParticles();
    }

    auto particles = static_cast<QSSGRenderParticles *>(node);
    const auto &perEmitter = perEmitterData(updateNode);

    if (!updateNode->m_nodeDirty)
        return particles;

    if (perEmitter.particleCount == 0)
        return particles;

    if (m_sprite)
        particles->m_sprite = m_sprite->getRenderImage();
    else
        particles->m_sprite = nullptr;

    if (m_spriteSequence) {
        particles->m_spriteImageCount = m_spriteSequence->m_frameCount;
        particles->m_blendImages = m_spriteSequence->m_interpolate;
    } else {
        particles->m_spriteImageCount = 1;
        particles->m_blendImages = true;
    }

    particles->m_hasTransparency = hasTransparency();

    if (m_colorTable)
        particles->m_colorTable = m_colorTable->getRenderImage();
    else
        particles->m_colorTable = nullptr;

    if (!m_lights.isEmpty()) {
        // Matches to QSSGRenderParticles lights
        QVarLengthArray<QSSGRenderLight *, 4> lightNodes;
        for (auto light : std::as_const(m_lights)) {
            auto lightPrivate = QQuick3DObjectPrivate::get(light);
            auto lightNode  = static_cast<QSSGRenderLight *>(lightPrivate->spatialNode);
            lightNodes.append(lightNode);
        }
        particles->m_lights = lightNodes;
    }

    particles->m_blendMode = mapBlendMode(m_blendMode);
    particles->m_billboard = m_billboard;
    particles->m_depthBiasSq = QSSGRenderNode::signedSquared(perEmitter.emitter->depthBias());
    particles->m_featureLevel = mapFeatureLevel(m_featureLevel);
    particles->m_depthSorting = sortMode() == QQuick3DParticle::SortDistance;
    particles->m_castsReflections = m_castsReflections;

    return particles;
}

void QQuick3DParticleSpriteParticle::handleMaxAmountChanged(int amount)
{
    if (m_particleData.size() == amount)
        return;

    m_particleData.resize(amount);
    m_spriteParticleData.resize(amount);
    reset();
}

void QQuick3DParticleSpriteParticle::handleSystemChanged(QQuick3DParticleSystem *system)
{
    for (PerEmitterData &value : m_perEmitterData) {
        delete value.particleUpdateNode;
        value.particleUpdateNode = new ParticleUpdateNode(system);
        value.particleUpdateNode->m_particle = this;
    }
}

void QQuick3DParticleSpriteParticle::updateNodes()
{
    for (const PerEmitterData &value : std::as_const(m_perEmitterData))
        value.particleUpdateNode->update();
}

void QQuick3DParticleSpriteParticle::markNodesDirty()
{
    for (const PerEmitterData &value : std::as_const(m_perEmitterData))
        value.particleUpdateNode->m_nodeDirty = true;
}

void QQuick3DParticleSpriteParticle::updateFeatureLevel()
{
    FeatureLevel featureLevel = FeatureLevel::Simple;
    if (m_lights.isEmpty()) {
        if (m_colorTable)
            featureLevel = FeatureLevel::Mapped;
        if (m_spriteSequence)
            featureLevel = FeatureLevel::Animated;
    } else {
        featureLevel = FeatureLevel::SimpleVLight;
        if (m_colorTable)
            featureLevel = FeatureLevel::MappedVLight;
        if (m_spriteSequence)
            featureLevel = FeatureLevel::AnimatedVLight;
    }
    if (featureLevel != m_featureLevel)
        m_featureLevel = featureLevel;
}

void QQuick3DParticleSpriteParticle::componentComplete()
{
    if (!system() && qobject_cast<QQuick3DParticleSystem *>(parentItem()))
        setSystem(qobject_cast<QQuick3DParticleSystem *>(parentItem()));

    QQuick3DParticle::componentComplete();
}

void QQuick3DParticleSpriteParticle::reset()
{
    QQuick3DParticle::reset();
    deleteNodes();
    m_nextEmitterIndex = 0;
    m_spriteParticleData.fill({});
}

void QQuick3DParticleSpriteParticle::commitParticles(float)
{
    markAllDirty();
    update();
    updateNodes();
}

int QQuick3DParticleSpriteParticle::nextCurrentIndex(const QQuick3DParticleEmitter *emitter)
{
    if (!m_perEmitterData.contains(emitter)) {
        m_perEmitterData.insert(emitter, PerEmitterData());
        auto &perEmitter = m_perEmitterData[emitter];
        perEmitter.particleUpdateNode = new ParticleUpdateNode(system());
        perEmitter.emitter = emitter;
        perEmitter.particleUpdateNode->m_particle = this;
        perEmitter.emitterIndex = m_nextEmitterIndex++;
    }
    auto &perEmitter = m_perEmitterData[emitter];
    int index = QQuick3DParticle::nextCurrentIndex(emitter);
    if (m_spriteParticleData[index].emitterIndex != perEmitter.emitterIndex) {
        if (m_spriteParticleData[index].emitterIndex >= 0)
            perEmitterData(m_spriteParticleData[index].emitterIndex).particleCount--;
        perEmitter.particleCount++;
    }
    m_spriteParticleData[index].emitterIndex = perEmitter.emitterIndex;
    return index;
}

void QQuick3DParticleSpriteParticle::setParticleData(int particleIndex,
                     const QVector3D &position,
                     const QVector3D &rotation,
                     const QVector4D &color,
                     float size, float age,
                     float animationFrame)
{
    auto &dst = m_spriteParticleData[particleIndex];
    dst = {position, rotation, color, size, age, animationFrame, dst.emitterIndex};
}

void QQuick3DParticleSpriteParticle::resetParticleData(int particleIndex)
{
    auto &dst = m_spriteParticleData[particleIndex];
    if (dst.size > 0.0f)
        dst = {{}, {}, {}, 0.0f, 0.0f, -1.0f, dst.emitterIndex};
}

void QQuick3DParticleSpriteParticle::updateParticleBuffer(ParticleUpdateNode *updateNode, QSSGRenderGraphObject *spatialNode)
{
    const auto &perEmitter = perEmitterData(updateNode);
    const auto &particles = m_spriteParticleData;
    QSSGRenderParticles *node = static_cast<QSSGRenderParticles *>(spatialNode);
    if (!node)
        return;
    const int particleCount = perEmitter.particleCount;
    if (node->m_particleBuffer.particleCount() != particleCount || m_useAnimatedParticle)
        node->m_particleBuffer.resize(particleCount, sizeof(QSSGParticleSimple));

    m_useAnimatedParticle = false;
    char *dest = node->m_particleBuffer.pointer();
    const SpriteParticleData *src = particles.data();
    const int pps = node->m_particleBuffer.particlesPerSlice();
    const int ss = node->m_particleBuffer.sliceStride();
    const int slices = node->m_particleBuffer.sliceCount();
    const int emitterIndex = perEmitter.emitterIndex;
    int i = 0;
    QSSGBounds3 bounds;
    const auto smode = sortMode();
    if (smode == QQuick3DParticle::SortNewest || smode == QQuick3DParticle::SortOldest) {
        int offset = m_currentIndex;
        int step = (smode == QQuick3DParticle::SortNewest) ? -1 : 1;
        int li = 0;
        const auto sourceIndex = [&](int linearIndex, int offset, int wrap) -> int {
            return (linearIndex + offset + wrap) % wrap;
        };
        for (int s = 0; s < slices; s++) {
            QSSGParticleSimple *dp = reinterpret_cast<QSSGParticleSimple *>(dest);
            for (int p = 0; p < pps && i < particleCount; ) {
                const SpriteParticleData *data = src + sourceIndex(li * step, offset, m_maxAmount);
                if (data->emitterIndex == emitterIndex) {
                    if (data->size > 0.0f)
                        bounds.include(data->position);
                    dp->position = data->position;
                    dp->rotation = data->rotation * float(M_PI / 180.0f);
                    dp->color = data->color;
                    dp->size = data->size * m_particleScale;
                    dp->age = data->age;
                    dp++;
                    p++;
                    i++;
                }
                li++;
            }
            dest += ss;
        }
    } else {
        for (int s = 0; s < slices; s++) {
            QSSGParticleSimple *dp = reinterpret_cast<QSSGParticleSimple *>(dest);
            for (int p = 0; p < pps && i < particleCount; ) {
                if (src->emitterIndex == emitterIndex) {
                    if (src->size > 0.0f)
                        bounds.include(src->position);
                    dp->position = src->position;
                    dp->rotation = src->rotation * float(M_PI / 180.0f);
                    dp->color = src->color;
                    dp->size = src->size * m_particleScale;
                    dp->age = src->age;
                    dp++;
                    p++;
                    i++;
                }
                src++;
            }
            dest += ss;
        }
    }
    node->m_particleBuffer.setBounds(bounds);
}

void QQuick3DParticleSpriteParticle::updateAnimatedParticleBuffer(ParticleUpdateNode *updateNode, QSSGRenderGraphObject *spatialNode)
{
    const auto &perEmitter = perEmitterData(updateNode);
    const auto &particles = m_spriteParticleData;
    QSSGRenderParticles *node = static_cast<QSSGRenderParticles *>(spatialNode);
    if (!node)
        return;
    const int particleCount = perEmitter.particleCount;
    if (node->m_particleBuffer.particleCount() != particleCount || !m_useAnimatedParticle)
        node->m_particleBuffer.resize(particleCount, sizeof(QSSGParticleAnimated));

    m_useAnimatedParticle = true;
    char *dest = node->m_particleBuffer.pointer();
    const SpriteParticleData *src = particles.data();
    const int pps = node->m_particleBuffer.particlesPerSlice();
    const int ss = node->m_particleBuffer.sliceStride();
    const int slices = node->m_particleBuffer.sliceCount();
    const int emitterIndex = perEmitter.emitterIndex;
    int i = 0;
    QSSGBounds3 bounds;
    const auto smode = sortMode();
    if (smode == QQuick3DParticle::SortNewest || smode == QQuick3DParticle::SortOldest) {
        int offset = m_currentIndex;
        int step = (smode == QQuick3DParticle::SortNewest) ? -1 : 1;
        int li = 0;
        const auto sourceIndex = [&](int linearIndex, int offset, int wrap) -> int {
            return (linearIndex + offset + wrap) % wrap;
        };
        for (int s = 0; s < slices; s++) {
            QSSGParticleAnimated *dp = reinterpret_cast<QSSGParticleAnimated *>(dest);
            for (int p = 0; p < pps && i < particleCount; ) {
                const SpriteParticleData *data = src + sourceIndex(li * step, offset, m_maxAmount);
                if (data->emitterIndex == emitterIndex) {
                    if (data->size > 0.0f)
                        bounds.include(data->position);
                    dp->position = data->position;
                    dp->rotation = data->rotation * float(M_PI / 180.0f);
                    dp->color = data->color;
                    dp->size = data->size * m_particleScale;
                    dp->age = data->age;
                    dp->animationFrame = data->animationFrame;
                    dp++;
                    p++;
                    i++;
                }
                li++;
            }
            dest += ss;
        }
    } else {
        for (int s = 0; s < slices; s++) {
            QSSGParticleAnimated *dp = reinterpret_cast<QSSGParticleAnimated *>(dest);
            for (int p = 0; p < pps && i < particleCount; ) {
                if (src->emitterIndex == emitterIndex) {
                    if (src->size > 0.0f)
                        bounds.include(src->position);
                    dp->position = src->position;
                    dp->rotation = src->rotation * float(M_PI / 180.0f);
                    dp->color = src->color;
                    dp->size = src->size * m_particleScale;
                    dp->age = src->age;
                    dp->animationFrame = src->animationFrame;
                    dp++;
                    p++;
                    i++;
                }
                src++;
            }
            dest += ss;
        }
    }
    node->m_particleBuffer.setBounds(bounds);
}

void QQuick3DParticleSpriteParticle::updateSceneManager(QQuick3DSceneManager *sceneManager)
{
    // Check all the resource value's scene manager, and update as necessary.
    if (sceneManager) {
        QQuick3DObjectPrivate::refSceneManager(m_sprite, *sceneManager);
        QQuick3DObjectPrivate::refSceneManager(m_colorTable, *sceneManager);
    } else {
        QQuick3DObjectPrivate::derefSceneManager(m_sprite);
        QQuick3DObjectPrivate::derefSceneManager(m_colorTable);
    }
}

// Lights
void QQuick3DParticleSpriteParticle::onLightDestroyed(QObject *object)
{
    bool found = false;
    for (int i = 0; i < m_lights.size(); ++i) {
        if (m_lights[i] == object) {
            m_lights.removeAt(i--);
            found = true;
        }
    }
    if (found) {
        updateFeatureLevel();
        markNodesDirty();
    }
}

void QQuick3DParticleSpriteParticle::qmlAppendLight(QQmlListProperty<QQuick3DAbstractLight> *list, QQuick3DAbstractLight *light)
{
    if (!light)
        return;

    // Light must be id of an existing View3D light and not inline light element
    if (light->parentItem()) {
        QQuick3DParticleSpriteParticle *self = static_cast<QQuick3DParticleSpriteParticle *>(list->object);
        self->m_lights.push_back(light);
        self->updateFeatureLevel();
        self->markNodesDirty();
        // Make sure ligths are removed when destroyed
        connect(light, &QQuick3DParticleSpriteParticle::destroyed, self, &QQuick3DParticleSpriteParticle::onLightDestroyed);
    }
}

QQuick3DAbstractLight *QQuick3DParticleSpriteParticle::qmlLightAt(QQmlListProperty<QQuick3DAbstractLight> *list, qsizetype index)
{
    QQuick3DParticleSpriteParticle *self = static_cast<QQuick3DParticleSpriteParticle *>(list->object);
    if (index >= self->m_lights.size()) {
        qWarning("The index exceeds the range of valid light targets.");
        return nullptr;
    }
    return self->m_lights.at(index);
}

qsizetype QQuick3DParticleSpriteParticle::qmlLightsCount(QQmlListProperty<QQuick3DAbstractLight> *list)
{
    QQuick3DParticleSpriteParticle *self = static_cast<QQuick3DParticleSpriteParticle *>(list->object);
    return self->m_lights.size();
}

void QQuick3DParticleSpriteParticle::qmlClearLights(QQmlListProperty<QQuick3DAbstractLight> *list)
{
    QQuick3DParticleSpriteParticle *self = static_cast<QQuick3DParticleSpriteParticle *>(list->object);
    for (const auto &light : std::as_const(self->m_lights)) {
        if (light->parentItem() == nullptr)
            QQuick3DObjectPrivate::get(light)->derefSceneManager();
        disconnect(light, &QQuick3DParticleSpriteParticle::destroyed, self, &QQuick3DParticleSpriteParticle::onLightDestroyed);
    }
    self->m_lights.clear();
    self->updateFeatureLevel();
    self->markNodesDirty();
}

QT_END_NAMESPACE
