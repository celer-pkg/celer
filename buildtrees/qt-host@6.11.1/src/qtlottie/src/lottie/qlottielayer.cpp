// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottielayer_p.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QLoggingCategory>
#include <QtCore/QScopedValueRollback>
#include <QString>

#include "qlottieflatlayers_p.h"
#include "qlottieshapelayer_p.h"
#include "qlottieprecomplayer_p.h"
#include "qlottiefilleffect_p.h"
#include "qlottiebasictransform_p.h"
#include "qlottierenderer_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottieLayer::QLottieLayer(const QLottieLayer &other)
    : QLottieBase(other)
{
    m_layerIndex = other.m_layerIndex;
    m_startFrame = other.m_startFrame;
    m_endFrame = other.m_endFrame;
    m_startTime = other.m_startTime;
    m_blendMode = other.m_blendMode;
    m_3dLayer = other.m_3dLayer;
    m_stretch = other.m_stretch;
    m_hasLinkedLayer = other.m_hasLinkedLayer;
    m_linkedLayerId = other.m_linkedLayerId;
    m_isMatte = other.m_isMatte;
    m_matteMode = other.m_matteMode;
    if (other.m_layerTransform) {
        m_layerTransform = new QLottieBasicTransform(*other.m_layerTransform);
        m_layerTransform->setParent(this);
    }
    m_size = other.m_size;
    if (other.m_effects) {
        m_effects = new QLottieBase;
        for (QLottieBase *effect : other.m_effects->children())
            m_effects->appendChild(effect->clone());
    }
    //m_transformAtFirstFrame = other.m_transformAtFirstFrame;
}

QLottieLayer::~QLottieLayer()
{
    if (m_layerTransform)
        delete m_layerTransform;
    if (m_effects)
        delete m_effects;
}

QLottieBase *QLottieLayer::clone() const
{
    return new QLottieLayer(*this);
}

QLottieLayer *QLottieLayer::construct(QJsonObject definition, const QMap<QString, QJsonObject> &assets)
{
    qCDebug(lcLottieQtLottieParser) << "QLottieLayer::parse()";

    QLottieLayer *layer = nullptr;
    int type = definition.value("ty"_L1).toInt();
    int ret = 0;
    switch (type) {
    case 0:
        qCDebug(lcLottieQtLottieParser) << "Parse precomp layer";
        layer = new QLottiePrecompLayer(assets);
        ret = layer->parse(definition);
        break;
    case 1:
        qCDebug(lcLottieQtLottieParser) << "Parse solid layer";
        layer = new QLottieSolidLayer();
        ret = layer->parse(definition);
        break;
    case 2:
        qCDebug(lcLottieQtLottieParser) << "Parse image layer";
        layer = new QLottieImageLayer();
        ret = layer->parse(definition);
        break;
    case 3:
        qCDebug(lcLottieQtLottieParser) << "Parse null layer";
        layer = new QLottieNullLayer();
        ret = layer->parse(definition);
        break;
    case 4:
        qCDebug(lcLottieQtLottieParser) << "Parse shape layer";
        layer = new QLottieShapeLayer();
        ret = layer->parse(definition);
        break;
    default:
        qCInfo(lcLottieQtLottieParser) << "Unsupported layer type:" << type;
    }

    if (ret < 0)
        return nullptr;

    return layer;
}

// Take the content of a lottie layers tag and construct the corresponding layer objects
// Also adds them as children to given parent
int QLottieLayer::constructLayers(QJsonArray jsonLayers, QLottieBase *parent,
                                  const QMap<QString, QJsonObject> &assets)
{
    if (jsonLayers.size() == 0) {
        qCCritical(lcLottieQtLottieParser) << "Layers are empty";
        return -1;
    }

    int layersAdded = 0;
    QJsonArray::const_iterator jsonLayerIt = jsonLayers.constEnd();
    while (jsonLayerIt != jsonLayers.constBegin()) {
        jsonLayerIt--;
        QJsonObject jsonLayer = (*jsonLayerIt).toObject();
        if (!jsonLayer.contains("ty"_L1)) {
            qCWarning(lcLottieQtLottieParser) << "Layer" << jsonLayer.value("nm"_L1).toString()
                                              << "is missing required key \"ty\"";
            return -1;
        }

        if (jsonLayer.value("ty"_L1).toInt() == 2) {
            if (!jsonLayer.contains("refId"_L1)) {
                qCWarning(lcLottieQtLottieParser) << "Layer" << jsonLayer.value("nm"_L1).toString()
                                                  << "is missing required key \"refId\"";
                return -1;
            }

            QString refId = jsonLayer.value("refId"_L1).toString();
            jsonLayer.insert("asset"_L1, assets.value(refId));
        }

        QLottieLayer *layer = QLottieLayer::construct(jsonLayer, assets);
        if (layer) {
            layer->setParent(parent);
            // Matte layers must be rendered before the layer they apply to, even though they
            // appear after in the list of layers. Hence, we move matte layers in front of
            // the layer they (by default) apply to, so it will be rendered first
            if (layer->isMatteLayer())
                parent->insertChildBeforeLast(layer);
            else
                parent->appendChild(layer);
            layersAdded++;
        }
    }
    return layersAdded;
}

bool QLottieLayer::active(int frame) const
{
    return (!m_hidden && ((frame >= m_startFrame && frame <= m_endFrame) || isStructureDumping()));
}

int QLottieLayer::parse(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottieLayer::parse():" << m_name;

    m_layerIndex = definition.value("ind"_L1).toVariant().toInt();
    if (!checkRequiredKeys(definition, "Layer"_L1, { "ip"_L1, "op"_L1, "ks"_L1 }, m_name))
        return -1;
    m_startFrame = definition.value("ip"_L1).toVariant().toInt();
    m_endFrame = definition.value("op"_L1).toVariant().toInt();
    m_blendMode = definition.value("lottie"_L1).toVariant().toInt();
    m_autoOrient = definition.value("ao"_L1).toBool();
    m_3dLayer = definition.value("ddd"_L1).toBool();
    m_stretch = definition.value("sr"_L1).toVariant().toReal();
    m_linkedLayerId = definition.value("parent"_L1).toVariant().toInt(&m_hasLinkedLayer);
    m_isMatte = definition.value("td"_L1).toInt() == 1;
    int matteMode = definition.value("tt"_L1).toInt(-1);
    if (matteMode > -1 && matteMode < 5)
        m_matteMode = static_cast<MatteClipMode>(matteMode);

    QJsonObject trans = definition.value("ks"_L1).toObject();
    m_layerTransform = new QLottieBasicTransform(this);
    if (m_layerTransform->parse(trans) < 0)
        return -1;

    QJsonArray effects = definition.value("ef"_L1).toArray();
    parseEffects(effects);

    if (m_matteMode > 2)
        qCInfo(lcLottieQtLottieParser)
                << "Lottie Layer: Only alpha matte layer supported:" << m_matteMode;
    if (m_blendMode > 0)
        qCInfo(lcLottieQtLottieParser)
                << "Lottie Layer: Unsupported blend mode" << m_blendMode;
    if (m_stretch > 1)
        qCInfo(lcLottieQtLottieParser)
                << "Lottie Layer: stretch not supported" << m_stretch;
    if (m_autoOrient)
        qCInfo(lcLottieQtLottieParser)
                << "Lottie Layer: auto-orient not supported";
    if (m_3dLayer)
        qCInfo(lcLottieQtLottieParser)
                << "Lottie Layer: is a 3D layer, but not handled";

    return 0;
}

void QLottieLayer::updateProperties(int frame)
{
    if (m_hasLinkedLayer)
        resolveLinkedLayer();

    int adjFrame = frame - m_startTime;
    m_isActive = active(adjFrame);
    if (!m_isActive)
        return;

    // Update first effects, as they are not children of the layer
    if (m_effects) {
        for (QLottieBase* effect : m_effects->children())
            effect->updateProperties(adjFrame);
    }

    m_layerTransform->updateProperties(adjFrame);

    QLottieBase::updateProperties(adjFrame);
}

void QLottieLayer::render(QLottieRenderer &renderer) const
{
    if (!m_isActive)
        return;

    // Render first effects, as they affect the children
    renderEffects(renderer);

    // In case there is a linked layer, apply its transform first
    // as it affects tranforms of this layer too
    applyLayerTransform(renderer);

    renderer.render(*this);

    renderChildren(renderer);
}

QLottieBase *QLottieLayer::findChild(const QString &childName)
{
    QLottieBase *child = nullptr;

    if (m_effects)
        child = m_effects->findChild(childName);

    if (child)
        return child;
    else
        return QLottieBase::findChild(childName);
}

QLottieLayer *QLottieLayer::resolveLinkedLayer()
{
    if (m_linkedLayer)
        return m_linkedLayer;

    Q_ASSERT(parent());

    for (QLottieBase *child : parent()->children()) {
        QLottieLayer *layer = static_cast<QLottieLayer*>(child);
        if (layer->layerId() == m_linkedLayerId) {
            m_linkedLayer = layer;
            break;
        }
    }
    return m_linkedLayer;
}

QLottieLayer *QLottieLayer::linkedLayer() const
{
    return m_linkedLayer;
}

bool QLottieLayer::isUsingMatteLayer() const
{
    return m_matteMode != NoClip;
}

bool QLottieLayer::isMatteLayer() const
{
    return m_isMatte;
}

QLottieLayer::MatteClipMode QLottieLayer::matteMode() const
{
    return m_matteMode;
}

int QLottieLayer::layerId() const
{
    return m_layerIndex;
}

QLottieBasicTransform *QLottieLayer::transform() const
{
    return m_layerTransform;
}

void QLottieLayer::renderEffects(QLottieRenderer &renderer) const
{
    if (!m_effects)
        return;

    for (QLottieBase* effect : m_effects->children()) {
        if (effect->hidden())
            continue;
        effect->render(renderer);
    }
}

void QLottieLayer::applyLayerTransform(QLottieRenderer &renderer) const
{
    if (m_applyingLayerTransform)
        return;
    QScopedValueRollback<bool> recursionGuard(m_applyingLayerTransform, true);

    if (!isStructureDumping()) {
        if (QLottieLayer *ll = linkedLayer())
            ll->applyLayerTransform(renderer);
    }
    if (m_layerTransform)
        m_layerTransform->render(renderer); // TBD: except opacity
}

QSize QLottieLayer::layerSize() const
{
    QSize res = m_size;
    if (!res.isValid() && parent())
        res = parent()->layerSize();
    return res;
}

const QLottieLayer *QLottieLayer::checkedCast(const QLottieBase *node)
{
    const QLottieLayer *res = nullptr;
    if (node && node->type() >= LOTTIE_LAYER_PRECOMP_IX && node->type() <= LOTTIE_LAYER_TEXT_IX)
        res = static_cast<const QLottieLayer *>(node);
    return res;
}

void QLottieLayer::parseEffects(const QJsonArray &definition, QLottieBase *effectRoot)
{
    QJsonArray::const_iterator it = definition.constEnd();
    while (it != definition.constBegin()) {
        // Create effects container if at least one effect found
        if (!effectRoot) {
            Q_ASSERT(!m_effects);
            effectRoot = new QLottieBase;
            m_effects = effectRoot;
        }
        it--;
        QJsonObject effect = (*it).toObject();
        int type = effect.value("ty"_L1).toInt();
        switch (type) {
        case 0:
        {
            QLottieBase *slider = new QLottieBase;
            slider->parse(effect);
            effectRoot->appendChild(slider);
            break;
        }
        case 5:
        {
            if (effect.value("en"_L1).toInt()) {
                QLottieBase *group = new QLottieBase;
                group->parse(effect);
                effectRoot->appendChild(group);
                parseEffects(effect.value("ef"_L1).toArray(), group);
            }
            break;
        }
        case 21:
        {
            QLottieFillEffect *fill = new QLottieFillEffect;
            fill->construct(effect);
            effectRoot->appendChild(fill);
            break;
        }
        default:
            qCInfo(lcLottieQtLottieParser)
                << "QLottieLayer: Unsupported effect" << type;
        }
    }
}

QT_END_NAMESPACE
