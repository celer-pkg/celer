// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottievisitor_p.h"
#include <private/qquickgenerator_p.h>
#include <private/qquicknodeinfo_p.h>
#include <QtLottie/private/qlottieshape_p.h>
#include <QtLottie/private/qlottiestroke_p.h>
#include <QtLottie/private/qlottierect_p.h>
#include <QtLottie/private/qlottiepolystar_p.h>
#include <QtLottie/private/qlottieellipse_p.h>
#include <QtLottie/private/qlottiefreeformshape_p.h>
#include <QtLottie/private/qlottieshapetransform_p.h>
#include <QtLottie/private/qlottiegfill_p.h>
#include <QtLottie/private/qlottieround_p.h>
#include <QtLottie/private/qlottieroot_p.h>
#include <QtLottie/private/qlottieflatlayers_p.h>
#include <QtLottie/private/qlottieimage_p.h>

#include <QtGui/private/qfixed_p.h>

#include <QFile>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcLottieQtVisitor, "qt.lottieqt.visitor")

using namespace Qt::Literals::StringLiterals;

#define QLOTTIEVISITOR_DEBUG \
    qCDebug(lcLottieQtVisitor).noquote().nospace() \
        << QByteArray().fill(' ', m_savedPaintInfos.size() * 4) \
        << ((trimmingState() == Sequential) ? QByteArray("trimmed") : QByteArray{})

QLottieVisitor::QLottieVisitor(const QString lottieFileName, QQuickGenerator *generator)
    : m_lottieFileName(lottieFileName), m_generator(generator)
{
}

void QLottieVisitor::render(const QLottieRoot &root)
{
    StructureNodeInfo info;
    fillCommonNodeInfo(&root, &info);

    const QJsonObject rootObj = root.definition();
    info.size = root.layerSize();

    int frameRate = rootObj.value("fr"_L1).toVariant().toInt();
    if (frameRate > 0)
        m_frameRate = frameRate;

    m_duration = qRound(1000 * rootObj.value("op"_L1).toDouble(100.0) / m_frameRate);
    info.viewBox = QRectF(QPointF(0, 0), info.size);

    QLOTTIEVISITOR_DEBUG << "[root viewbox=" << info.viewBox << ", frame rate=" << m_frameRate << ", duration=" << m_duration << "ms ]";

    info.stage = StructureNodeStage::Start;
    info.nodeId = "_q_animation"_L1; // # centralize

    m_generator->generateRootNode(info);

    root.render(*this);

    info.stage = StructureNodeStage::End;
    m_generator->generateRootNode(info);
}

int QLottieVisitor::timePointForFrame(qreal frameNo, bool doWrap) const
{
    int res = qRound(1000 * (frameNo + m_frameOffset) / qreal(m_frameRate));
    if (doWrap && m_frameOffset != 0) {
        res %= m_duration;
        if (res < 0)
            res += m_duration;
    }
    return res;
}

QString QLottieVisitor::idForNode(const QLottieBase *node)
{
    QString idForNull;
    QString &id = node ? m_idForNodeId[node] : idForNull;
    if (id.isNull())
        id = QStringLiteral("_qt_node%1").arg(m_nodeIdCounter++);
    return id;
}

QString QLottieVisitor::scrub(const QString &raw)
{
    QString res(raw.left(80));

    if (!res.isEmpty()) {
        constexpr QLatin1StringView legalSymbols("_-.:/[](){}*| "); // No quot. mark or backslash!
        qsizetype i = 0;
        do {
            if (res.at(i).isLetterOrNumber() || legalSymbols.contains(res.at(i)))
                i++;
            else
                res.remove(i, 1);
        } while (i < res.size());
    }

    return res;
}

void QLottieVisitor::fillCommonNodeInfo(const QLottieBase *node,
                                        NodeInfo *info,
                                        const QString &suffix)
{
    info->id = idForNode(node) + suffix;
    if (node != nullptr) {
        info->nodeId = scrub(node->name());
        info->typeName = QStringLiteral("Type%1").arg(node->type());
    }
}

void QLottieVisitor::fillLayerAnimationInfo(const QLottieLayer *node, NodeInfo *info)
{
    constexpr bool wrapTimePoints = false;
    const int startTime = timePointForFrame(node->startFrame(), wrapTimePoints);
    const int endTime = timePointForFrame(node->endFrame(), wrapTimePoints);

    if (startTime <= 0 && endTime >= m_duration)
        return; // Always visible (which is default), so no animation needed

    QQuickAnimatedProperty::PropertyAnimation animation;

    if (startTime >= m_duration || endTime <= 0 || endTime <= startTime) {
        // Always invisible
        info->visibility.setDefaultValue(QVariant::fromValue(false));
        animation.frames[0] = QVariant::fromValue(false);
        animation.frames[m_duration] = QVariant::fromValue(false);
    } else {
        if (startTime <= 0) {
            animation.frames[0] = QVariant::fromValue(true);
        } else {
            animation.frames[0] = QVariant::fromValue(false);
            animation.frames[startTime] = QVariant::fromValue(true);
        }

        if (endTime < m_duration)
            animation.frames[endTime] = QVariant::fromValue(false);
        animation.frames[m_duration] = animation.frames.last();
    }

    animation.flags |= QQuickAnimatedProperty::PropertyAnimation::FreezeAtEnd;
    info->visibility.addAnimation(animation);
}

void QLottieVisitor::fillAnimationNodeInfo(const QLottieBase *node, NodeInfo *info)
{
    Q_UNUSED(node);
    for (const PaintInfo::TransformAnimationInfo &animInfo : std::as_const(m_currentPaintInfo.transformAnimations)) {
        if (!animInfo.frames.isEmpty()) {
            const bool hasPaths = animInfo.frames.first().typeId() == qMetaTypeId<QPainterPath>();

            QQuickAnimatedProperty::PropertyAnimation animation;
            animation.frames = animInfo.frames;
            animation.flags |= QQuickAnimatedProperty::PropertyAnimation::FreezeAtEnd;

            animation.frames[0] = animation.frames.first();

            animation.easingPerFrame = animInfo.easingPerFrame;

            if (animInfo.animationType == QTransform::TxTranslate && hasPaths) {
                if (animation.frames.lastKey() < m_duration)
                    animation.frames[m_duration] = QVariant::fromValue(QPainterPath{});
                info->motionPath.addAnimation(animation);
            } else {
                animation.frames[m_duration] = animation.frames.last();
                animation.subtype = animInfo.animationType;
                if (animInfo.animationType == QTransform::TxNone)
                    info->opacity.addAnimation(animation);
                else
                    info->transform.addAnimation(animation);
            }
        }
    }
}

void QLottieVisitor::saveState()
{
    m_savedPaintInfos.append(m_currentPaintInfo);
}

void QLottieVisitor::restoreState()
{
    Q_ASSERT(!m_savedPaintInfos.isEmpty());
    m_currentPaintInfo = m_savedPaintInfos.takeLast();
}

void QLottieVisitor::generateMatteNode(const QLottieLayer *layer, StructureNodeStage stage)
{
    const QString suffix(QStringLiteral("_box"));

    if (layer->isMatteLayer()) {
        MaskNodeInfo maskInfo;
        maskInfo.stage = stage;
        fillCommonNodeInfo(layer, &maskInfo, suffix);
        maskInfo.nodeId.clear();
        maskInfo.typeName.clear();
        maskInfo.bounds = QRect(QPoint(), layer->layerSize());
        maskInfo.maskRect = maskInfo.bounds;
        m_generator->generateMaskNode(maskInfo);
    } else if (layer->isUsingMatteLayer() && layer->parent()) {
        StructureNodeInfo info;
        info.stage = stage;
        fillCommonNodeInfo(layer, &info, suffix);
        info.nodeId.clear();
        info.typeName.clear();
        info.bounds = QRect(QPoint(), layer->layerSize());
        const QLottieLayer::MatteClipMode mode = layer->matteMode();
        info.isMaskAlpha = (mode == QLottieLayer::Alpha || mode == QLottieLayer::InvertedAlpha);
        info.isMaskInverted = (mode == QLottieLayer::InvertedAlpha || mode == QLottieLayer::InvertedLuminence);
        // Find id of matte layer, assume preceding layer
        const QList<QLottieBase *> &siblings = layer->parent()->children();
        const QLottieBase *prevSibling = siblings.value(siblings.indexOf(layer) - 1);
        const QLottieLayer *precedingLayer = QLottieLayer::checkedCast(prevSibling);
        if (precedingLayer && precedingLayer->isMatteLayer()) {
            info.maskId = idForNode(precedingLayer) + suffix;
            m_generator->generateStructureNode(info);
        }
        QLOTTIEVISITOR_DEBUG << "  matte link resolved to layer: " << info.maskId;
    }
}

void QLottieVisitor::render(const QLottieLayer &layer)
{
    QLOTTIEVISITOR_DEBUG << "[layer '" << layer.name() << "' type " << Qt::hex << layer.type()
                         << (layer.isMatteLayer() ? " matte" : "") << "]";

    if (layer.isMatteLayer() || layer.isUsingMatteLayer())
        generateMatteNode(&layer, StructureNodeStage::Start);

    StructureNodeInfo info;
    fillCommonNodeInfo(&layer, &info);
    info.customItemType = QStringLiteral("LayerItem");
    info.stage = StructureNodeStage::Start;
    info.transform.setDefaultValue(QVariant::fromValue(m_currentPaintInfo.transform));
    info.isDefaultTransform = m_currentPaintInfo.transform.isIdentity();
    info.opacity.setDefaultValue(m_currentPaintInfo.opacity);
    info.isDefaultOpacity = qFuzzyCompare(m_currentPaintInfo.opacity, 1.0);

    fillLayerAnimationInfo(&layer, &info);
    if (layer.type() == LOTTIE_LAYER_PRECOMP_IX)
        m_frameOffset += layer.frameOffset();

    fillAnimationNodeInfo(&layer, &info);
    if (layer.hasLinkedLayer() && layer.parent()) {
        for (const QLottieBase *sibling : layer.parent()->children()) {
            if (auto siblingLayer = QLottieLayer::checkedCast(sibling)) {
                if (siblingLayer != &layer && siblingLayer->layerId() == layer.linkedLayerId())
                    info.transformReferenceId = idForNode(siblingLayer);
            }
        }
        QLOTTIEVISITOR_DEBUG << "  xf link resolved to layer: " << info.transformReferenceId;
    }

    m_generator->generateStructureNode(info);

    m_currentPaintInfo = {};
}

void QLottieVisitor::render(const QLottieSolidLayer &layer)
{
    render(static_cast<const QLottieLayer &>(layer));
    if (!layer.layerSize().isEmpty()) {
        m_currentPaintInfo.fill = layer.color();
        QPainterPath layerRect;
        layerRect.addRect(QRect(QPoint(), layer.layerSize()));
        processShape(nullptr, layerRect);
    }
}

void QLottieVisitor::render(const QLottieGroup &group)
{
    QLOTTIEVISITOR_DEBUG << "[group '" << group.name() << "' #children " << group.children().size() << "]";

    bool hasPaths = false;
    bool hasGroups = false;
    for (const QLottieBase *child : group.children()) {
        if (child->type() == LOTTIE_SHAPE_GROUP_IX)
            hasGroups = true;
        else if (child->isPathElement())
            hasPaths = true;
    }

    // There are two cases where we want to generate a structure node for a LottieGroup
    // 1) If the group contains concrete shapes, then we want a Shape node to contain the paths
    // 2) If the group contains (sub)groups and has a transform. Since the subgroups will have
    //    their own transforms, this group's transform must be applied first with a structure node

    const QLottieBase *groupXf = group.children().first();
    if (groupXf->type() == LOTTIE_SHAPE_TRANS_IX) { // Always true in wellformed lottie file
        // We must apply the group xf already here, so it will become the structure node's xf.
        // If non-identity, m_currentStructElements is used to avoid re-applying it on normal visit.
        render(*static_cast<const QLottieShapeTransform *>(groupXf));
    }
    const bool groupHasTransform = !m_currentPaintInfo.transform.isIdentity()
            || m_currentPaintInfo.transformAnimations.size() > 0;
    if (hasPaths || (hasGroups && groupHasTransform)) {
        StructureNodeInfo info;
        fillCommonNodeInfo(&group, &info);
        info.stage = StructureNodeStage::Start;
        info.isPathContainer = hasPaths;
        info.transform.setDefaultValue(QVariant::fromValue(m_currentPaintInfo.transform));
        info.isDefaultTransform = m_currentPaintInfo.transform.isIdentity();
        info.opacity.setDefaultValue(m_currentPaintInfo.opacity);
        info.isDefaultOpacity = qFuzzyCompare(m_currentPaintInfo.opacity, 1.0);

        fillAnimationNodeInfo(&group, &info);

        m_generator->generateStructureNode(info);
        m_currentStructElements.push(&group);

        m_currentPaintInfo.transform.reset();
        m_currentPaintInfo.transformAnimations.clear();
        m_currentPaintInfo.opacity = 1.0;
    }
}

void QLottieVisitor::finish(const QLottieLayer &layer)
{
    QLOTTIEVISITOR_DEBUG << "[layer '" << layer.name() << "' finish]";

    if (layer.type() == LOTTIE_LAYER_PRECOMP_IX)
        m_frameOffset -= layer.frameOffset();

    StructureNodeInfo info;
    info.stage = StructureNodeStage::End;
    fillCommonNodeInfo(&layer, &info);
    m_generator->generateStructureNode(info);

    if (layer.isMatteLayer() || layer.isUsingMatteLayer())
        generateMatteNode(&layer, StructureNodeStage::End);
}

void QLottieVisitor::finish(const QLottieGroup &group)
{
    QLOTTIEVISITOR_DEBUG << "[group '" << group.name() << "' finish]";

    if (!m_currentStructElements.isEmpty() && m_currentStructElements.top() == &group) {
        bool hasPaths = false;
        for (const QLottieBase *child : group.children()) {
            if (child->isPathElement()) {
                hasPaths = true;
                break;
            }
        }

        StructureNodeInfo info;
        info.stage = StructureNodeStage::End;
        info.isPathContainer = hasPaths;

        fillCommonNodeInfo(&group, &info);
        m_generator->generateStructureNode(info);
        m_currentStructElements.pop();
    }
}

void QLottieVisitor::render(const QLottieRect &rect)
{
    QLOTTIEVISITOR_DEBUG << "[rect]";

    processPath(&rect, rect.path());
}

void QLottieVisitor::render(const QLottieEllipse &ellipse)
{
    QLOTTIEVISITOR_DEBUG << "[ellipse]";

    processPath(&ellipse, ellipse.path());
}

void QLottieVisitor::render(const QLottiePolyStar &star)
{
    QLOTTIEVISITOR_DEBUG << "[star]";

    processPath(&star, star.path());
}

void QLottieVisitor::render(const QLottieRound &round)
{
    QLOTTIEVISITOR_DEBUG << "[round]";

    // ### Not implemented: path rounding modifier
    Q_UNUSED(round);
}

void QLottieVisitor::render(const QLottieFill &fill)
{
    QLOTTIEVISITOR_DEBUG << "[fill color=" << fill.color() << ", opacity=" << fill.opacity() << "]";

    QColor color = fill.color();
    color.setAlphaF(color.alphaF() * (fill.opacity() / 100.0));
    m_currentPaintInfo.fill = color;
    m_currentPaintInfo.fillColorAnimation = makeColorAnimation(fill.colorProperty());
    m_currentPaintInfo.fillOpacityAnimation = makeOpacityAnimation(fill.opacityProperty());
    m_currentPaintInfo.fillRule = fill.fillRule();
}

void QLottieVisitor::render(const QLottieGFill &gradient)
{
    QLOTTIEVISITOR_DEBUG << "[fill gradient]";

    if (gradient.value() != nullptr)
        m_currentPaintInfo.fill = *gradient.value();
    m_currentPaintInfo.fillRule = gradient.fillRule();
}

void QLottieVisitor::render(const QLottieImage &image)
{
    QLOTTIEVISITOR_DEBUG << "[image size=" << image.size() << "]";

    ImageNodeInfo info;
    fillCommonNodeInfo(&image, &info);
    info.image = image.image();
    info.rect = QRectF(QPointF(), image.size());
    info.externalFileReference = image.url().toLocalFile();

    m_generator->generateImageNode(info);
}

void QLottieVisitor::render(const QLottieStroke &stroke)
{
    QLOTTIEVISITOR_DEBUG << "[stroke color=" << stroke.pen().color()
                         << ", opacity=" << stroke.opacity() << "]";

    const QPen pen = stroke.pen();
    m_currentPaintInfo.stroke = pen;

    QColor color = pen.color();
    color.setAlphaF(color.alphaF() * (stroke.opacity() / 100.0));
    m_currentPaintInfo.stroke.setColor(color);

    m_currentPaintInfo.strokeColorAnimation = makeColorAnimation(stroke.colorProperty());
    m_currentPaintInfo.strokeOpacityAnimation = makeOpacityAnimation(stroke.opacityProperty());
}

void QLottieVisitor::render(const QLottieBasicTransform &transform)
{
    QLOTTIEVISITOR_DEBUG << "[basic transform s=" << transform.scale()
                         << ", r=" << transform.rotation()
                         << ", o=" << transform.opacity() << "]";
    if (hasAnimations(&transform))
        collectTransformAnimations(&transform);
    else
        applyTransform(&m_currentPaintInfo.transform, transform, false);

    m_currentPaintInfo.opacity *= transform.opacity();
}

namespace {
    template<typename T>
    QLottieVisitor::PaintInfo::TransformAnimationInfo
        collectAnimations(const T &property,
                          QTransform::TransformationType type,
                          std::function<void(qreal,
                                             const QVariant &,
                                             QLottieVisitor::PaintInfo::TransformAnimationInfo *,
                                             std::optional<QBezier>)> storeAnimationFrame,
                          std::function<QVariantList(const QVariant &)> createParams)
    {
        const auto easingCurves = property.easingCurves();
        QLottieVisitor::PaintInfo::TransformAnimationInfo info;
        info.animationType = type;
        if (easingCurves.isEmpty()) {
            const QVariantList params = createParams(QVariant::fromValue(property.value()));
            storeAnimationFrame(0, params, &info, std::nullopt);
        } else {
            std::optional<QBezier> easingBezier;
            for (const auto &curve : easingCurves) {
                const auto value = curve.startValue;
                const QVariantList params = createParams(QVariant::fromValue(value));
                storeAnimationFrame(curve.startFrame, params, &info, easingBezier);

                easingBezier = curve.easing.bezier(); // belongs to generator's next keyframe
            }
        }
        return info;
    }
}

void QLottieVisitor::collectTransformAnimations(const QLottieBasicTransform *transform,
                                                bool isShapeTransform)
{
    Q_UNUSED(isShapeTransform);
    const QLottieProperty<QPointF> anchorPoints = transform->anchorPointProperty();
    const QLottieProperty<qreal> rotations = transform->rotationProperty();
    const QLottieProperty<QPointF> scales = transform->scaleProperty();
    const QLottieSpatialProperty positions = transform->positionProperty();
    const QLottieProperty<qreal> opacities = transform->opacityProperty();
    const QLottieProperty<qreal> xPositions = transform->xPosProperty();
    const QLottieProperty<qreal> yPositions = transform->yPosProperty();

    const bool positionHasCurves = positions.hasCurves();

    auto storeAnimationFrame = [&](qreal lottieFrameNumber,
                                   const QVariant &propertyValue,
                                   PaintInfo::TransformAnimationInfo *info,
                                   std::optional<QBezier> easingBezier = std::nullopt) {
        const int timePointMs = timePointForFrame(lottieFrameNumber);
        info->frames[timePointMs] = propertyValue;
        if (easingBezier)
            info->easingPerFrame[timePointMs] = *easingBezier;
    };

    QLottieVisitor::PaintInfo::TransformAnimationInfo info;
    if (!transform->splitPosition()) {
        if (positionHasCurves) { // Use the motionPath property
            const auto easingCurves = positions.easingCurves();
            const auto &subPaths = positions.subPaths();
            Q_ASSERT(!subPaths.isEmpty());

            QLottieVisitor::PaintInfo::TransformAnimationInfo info;
            info.animationType = QTransform::TxTranslate;

            if (easingCurves.isEmpty()) {
                const QVariant params = QVariant::fromValue(subPaths.first());
                storeAnimationFrame(0, params, &info, std::nullopt);
            } else {
                std::optional<QBezier> easingBezier;
                for (qsizetype i = 0; i < easingCurves.size(); ++i) {
                    const auto &curve = easingCurves.at(i);
                    if (i == 0) {
                        // Store an empty path keyframe to mark the start time of the animation
                        const QVariant startParams = QVariant::fromValue(QPainterPath{});
                        storeAnimationFrame(curve.startFrame, startParams, &info, easingBezier);
                    } else {
                        // Like the easing, also the motion path is expected in the ending keyframe
                        const QVariant params = QVariant::fromValue(subPaths.at(i - 1));
                        storeAnimationFrame(curve.startFrame, params, &info, easingBezier);
                    }
                    easingBezier = curve.easing.bezier(); // belongs to generator's next keyframe
                }
            }

            m_currentPaintInfo.transformAnimations.append(info);
        } else {
            info = collectAnimations(positions,
                                     QTransform::TxTranslate,
                                     storeAnimationFrame,
                                     [](const QVariant &v)
                                     {
                                         return QVariantList{ v };
                                     });
        }
        m_currentPaintInfo.transformAnimations.append(info);
    } else {
        info = collectAnimations(xPositions,
                                 QTransform::TxTranslate,
                                 storeAnimationFrame,
                                 [](const QVariant &v)
                                 {
                                     return QVariantList{ QVariant::fromValue(QPointF(v.toReal(), 0.0)) };
                                 });
        m_currentPaintInfo.transformAnimations.append(info);

        info = collectAnimations(yPositions,
                                 QTransform::TxTranslate,
                                 storeAnimationFrame,
                                 [](const QVariant &v)
                                 {
                                     return QVariantList{ QVariant::fromValue(QPointF(0.0, v.toReal())) };
                                 });
        m_currentPaintInfo.transformAnimations.append(info);
    }

    auto storeRotationParameter = [](const QVariant &v) {
        return QVariantList{ QVariant::fromValue(QPointF(0, 0)), v };
    };
    info = collectAnimations(rotations,
                             QTransform::TxRotate,
                             storeAnimationFrame,
                             storeRotationParameter);
    m_currentPaintInfo.transformAnimations.append(info);

    if (isShapeTransform) {
        const QLottieShapeTransform *shapeTransform =
            static_cast<const QLottieShapeTransform *>(transform);

        const QLottieProperty<qreal> skews = shapeTransform->skewProperty();
        const QLottieProperty<qreal> skewAxes = shapeTransform->skewAxisProperty();

        // Lottie shear transforms work by first rotating by skew axis angle, then applying
        // the skewAngle as the shear along the X-axis, and then rotating back.
        info = collectAnimations(skewAxes,
                                 QTransform::TxRotate,
                                 storeAnimationFrame,
                                 [](const QVariant &v) {
                                     return QVariantList{ QVariant::fromValue(QPointF(0, 0)),
                                                          QVariant::fromValue(-1.0 * v.toReal()) };
                                 });
        m_currentPaintInfo.transformAnimations.append(info);

        info = collectAnimations(skews,
                                 QTransform::TxShear,
                                 storeAnimationFrame,
                                 [](const QVariant &v) {
                                     return QVariantList{ QVariant::fromValue(QPointF(-1.0 * v.toReal(), 0.0)) };
                                 });
        m_currentPaintInfo.transformAnimations.append(info);

        info = collectAnimations(skewAxes,
                                 QTransform::TxRotate,
                                 storeAnimationFrame,
                                 storeRotationParameter);
        m_currentPaintInfo.transformAnimations.append(info);
    }

    info = collectAnimations(scales,
                             QTransform::TxScale,
                             storeAnimationFrame,
                             [](const QVariant &v) {
                                 return QVariantList{ QVariant::fromValue(v.toPointF() / 100.0) };
                             });
    m_currentPaintInfo.transformAnimations.append(info);

    info = collectAnimations(anchorPoints,
                             QTransform::TxTranslate,
                             storeAnimationFrame,
                             [](const QVariant &v) {
                                 return QVariantList{ QVariant::fromValue(v.toPointF() * -1.0) };
                             });
    m_currentPaintInfo.transformAnimations.append(info);

    {
        const QList<EasingSegment<qreal> > easingCurves = opacities.easingCurves();
        PaintInfo::TransformAnimationInfo info;
        info.animationType = QTransform::TxNone;

        std::optional<QBezier> easingBezier;
        for (const auto &curve : easingCurves) {
            const auto value = curve.startValue / 100;
            const QVariant params = QVariant::fromValue(value);
            storeAnimationFrame(curve.startFrame, params, &info, easingBezier);

            easingBezier = curve.easing.bezier(); // For next keyframe
        }
        m_currentPaintInfo.transformAnimations.append(info);
    }
}

QQuickAnimatedProperty::PropertyAnimation QLottieVisitor::makeColorAnimation(const QLottieProperty4D<QVector4D> &colorProperty)
{
    QQuickAnimatedProperty::PropertyAnimation animation;
    animation.flags |= QQuickAnimatedProperty::PropertyAnimation::FreezeAtEnd;

    std::optional<QBezier> easingBezier;
    for (const auto &curve : colorProperty.easingCurves()) {
        const QVector4D raw = curve.startValue;
        QColor value = QColor::fromRgbF(raw.x(), raw.y(), raw.z(), raw.w());
        int frameTime = timePointForFrame(curve.startFrame);
        animation.frames[frameTime] = value;
        if (easingBezier)
            animation.easingPerFrame[frameTime] = *easingBezier;
        easingBezier = curve.easing.bezier(); // For next keyframe
    }
    if (!animation.frames.isEmpty()) {
        animation.frames[0] = animation.frames.first();
        animation.frames[m_duration] = animation.frames.last();
    }
    return animation;
}

QQuickAnimatedProperty::PropertyAnimation QLottieVisitor::makeOpacityAnimation(const QLottieProperty<qreal> &opacityProperty)
{
    QQuickAnimatedProperty::PropertyAnimation animation;
    animation.flags |= QQuickAnimatedProperty::PropertyAnimation::FreezeAtEnd;

    std::optional<QBezier> easingBezier;
    for (const auto &curve : opacityProperty.easingCurves()) {
        qreal value = curve.startValue / qreal(100);
        int frameTime = timePointForFrame(curve.startFrame);
        animation.frames[frameTime] = value;
        if (easingBezier)
            animation.easingPerFrame[frameTime] = *easingBezier;
        easingBezier = curve.easing.bezier(); // For next keyframe
    }
    if (!animation.frames.isEmpty()) {
        animation.frames[0] = animation.frames.first();
        animation.frames[m_duration] = animation.frames.last();
    }
    return animation;
}

bool QLottieVisitor::hasAnimations(const QLottieBasicTransform *transform, bool isShapeTransform)
{
    bool hasAnimations = transform->rotationProperty().startFrame() < transform->rotationProperty().endFrame()
                         || transform->positionProperty().startFrame() < transform->positionProperty().endFrame()
                         || transform->scaleProperty().startFrame() < transform->scaleProperty().endFrame()
                         || transform->opacityProperty().startFrame() < transform->opacityProperty().endFrame();

    if (transform->splitPosition() && !hasAnimations) {
        hasAnimations = transform->xPosProperty().startFrame() < transform->xPosProperty().endFrame()
                        || transform->yPosProperty().startFrame() < transform->yPosProperty().endFrame();
    }

    if (isShapeTransform && !hasAnimations) {
        const QLottieShapeTransform *shapeTransform = static_cast<const QLottieShapeTransform *>(transform);
        hasAnimations = shapeTransform->skewProperty().startFrame() < shapeTransform->skewProperty().endFrame()
            || shapeTransform->skewAxisProperty().startFrame() < shapeTransform->skewAxisProperty().endFrame();
    }

    if (qEnvironmentVariableIntValue("QLOTTIEVISITOR_DISABLE_ANIMATIONS"))
        return false;

    return hasAnimations;
}

void QLottieVisitor::render(const QLottieShapeTransform &transform)
{
    if (!m_currentStructElements.isEmpty() && transform.parent() == m_currentStructElements.top()) {
        // This transform was already applied as part of a group structure node
        return;
    }

    QLOTTIEVISITOR_DEBUG << "[shape transform s=" << transform.scale()
        << ", r=" << transform.rotation()
        << ", o=" << transform.opacity() << "]";

    if (hasAnimations(&transform, true))
        collectTransformAnimations(&transform, true);
    else
        applyTransform(&m_currentPaintInfo.transform, transform, true);

    m_currentPaintInfo.opacity *= transform.opacity();
}

void QLottieVisitor::render(const QLottieFreeFormShape &shape)
{
    QLOTTIEVISITOR_DEBUG << "[freeform]";

    processPath(&shape, shape.path());
}

void QLottieVisitor::render(const QLottieTrimPath &trim)
{
    QLOTTIEVISITOR_DEBUG << "[trim, isParallel: " << trim.isParallel() << "]";

    m_currentPaintInfo.trim.enabled = true;
    m_currentPaintInfo.trim.start.setDefaultValue(trim.start() / 100.0);
    m_currentPaintInfo.trim.end.setDefaultValue(trim.end() / 100.0);
    m_currentPaintInfo.trim.offset.setDefaultValue(trim.offset() / 360.0);

    auto registerAnimations = [&](QQuickAnimatedProperty *outProperty,
                                  const QLottieProperty<qreal> &inProperty,
                                  qreal scale) {
        const QList<EasingSegment<qreal> > easingCurves = inProperty.easingCurves();

        QQuickAnimatedProperty::PropertyAnimation animation;
        animation.flags |= QQuickAnimatedProperty::PropertyAnimation::FreezeAtEnd;

        for (int i = 0; i < easingCurves.size(); i++) {
            const auto &curve = easingCurves.at(i);
            const qreal startValue = curve.startValue * scale;
            int startFrameTime = timePointForFrame(curve.startFrame);
            animation.frames[startFrameTime] = startValue;
            if (i > 0)
                animation.easingPerFrame[startFrameTime] = easingCurves.at(i - 1).easing.bezier();
        }
        if (!animation.frames.isEmpty()) {
            animation.frames[0] = animation.frames.first();
            animation.frames[m_duration] = animation.frames.last();
            outProperty->addAnimation(animation);
        }
    };

    registerAnimations(&m_currentPaintInfo.trim.start, trim.startProperty(), 1.0 / 100.0);
    registerAnimations(&m_currentPaintInfo.trim.end, trim.endProperty(), 1.0 / 100.0);
    registerAnimations(&m_currentPaintInfo.trim.offset, trim.offsetProperty(), 1.0 / 360.0);

    if (!trim.isParallel())
        processShape(&trim, m_currentPaintInfo.unitedPath);
}

void QLottieVisitor::render(const QLottieFillEffect &effect)
{
    QLOTTIEVISITOR_DEBUG << "[effect]";

    // ### What are you?
    Q_UNUSED(effect);
}

void QLottieVisitor::render(const QLottieRepeater &repeater)
{
    QLOTTIEVISITOR_DEBUG << "[repeater]";

    // ### Repeats the following shapes N times with different transforms
    Q_UNUSED(repeater);
}

void QLottieVisitor::fillPathAnimationInfo(const QLottieShape *shape, PathNodeInfo *info)
{
    if (shape->type() == LOTTIE_SHAPE_SHAPE_IX) {
        const QLottieFreeFormShape *ffShape = static_cast<const QLottieFreeFormShape *>(shape);
        if (ffShape->isAnimated()) {
            QLottieFreeFormShape copy(*ffShape);
            QQuickAnimatedProperty::PropertyAnimation pa;
            const auto easingCurves = copy.startPointProperty().easingCurves();
            for (int i = 0; i < easingCurves.size(); i++) {
                const auto &curve = easingCurves.at(i);
                copy.updateProperties(qRound(curve.startFrame));
                int timePointMs = timePointForFrame(curve.startFrame);
                pa.frames[timePointMs] = QVariant::fromValue(copy.path());
                if (i > 0)
                    pa.easingPerFrame[timePointMs] = easingCurves.at(i - 1).easing.bezier();
            }
            pa.frames[0] = pa.frames.first();
            pa.frames[m_duration] = pa.frames.last();
            pa.flags |= QQuickAnimatedProperty::PropertyAnimation::FreezeAtEnd;
            info->path.addAnimation(pa);
        }
    }
}

void QLottieVisitor::renderPathElements(const QList<QLottieBase *> &pathElements)
{
    QLOTTIEVISITOR_DEBUG << "[path elements, count = " << pathElements.size() << "]";

    bool hasAnimatedElements = false;
    for (const QLottieBase *element : pathElements) {
        if (element->type() == LOTTIE_SHAPE_SHAPE_IX) {
            const QLottieFreeFormShape *ffShape = static_cast<const QLottieFreeFormShape *>(element);
            if (ffShape->isAnimated()) {
                hasAnimatedElements = true;
                break;
            }
        }
    }

    if (pathElements.size() == 1 || hasAnimatedElements || trimmingState() == Parallel) {
        // Need individual QML path items
        for (const QLottieBase *element : pathElements) {
            element->render(*this);
        }
    } else if (pathElements.size() > 1) {
        // Combine into one path item
        QPainterPath renderPath;
        const QLottieShape *firstShape = nullptr;
        for (const QLottieBase *element : pathElements) {
            Q_ASSERT(element->isPathElement());
            const QLottieShape *shape = static_cast<const QLottieShape *>(element);
            renderPath.addPath(shape->path());
            if (!firstShape)
                firstShape = shape;
        }
        processPath(firstShape, renderPath);
    }
}

void QLottieVisitor::processShape(const QLottieShape *shape, const QPainterPath &path)
{
    QLOTTIEVISITOR_DEBUG << "[drawing shape with"
                         << " stroke=" << m_currentPaintInfo.stroke
                         << ", fill=" << m_currentPaintInfo.fill;

    if (path.isEmpty())
        return;

    StructureNodeInfo info;
    if (m_currentStructElements.isEmpty()) {
        fillCommonNodeInfo(shape, &info);
        info.stage = StructureNodeStage::Start;
        info.isPathContainer = true;

        info.transform.setDefaultValue(QVariant::fromValue(m_currentPaintInfo.transform));
        info.isDefaultTransform = m_currentPaintInfo.transform.isIdentity();
        info.opacity.setDefaultValue(m_currentPaintInfo.opacity);
        info.isDefaultOpacity = qFuzzyCompare(m_currentPaintInfo.opacity, 1.0);

        fillAnimationNodeInfo(shape, &info);

        m_generator->generateStructureNode(info);
    }

    PathNodeInfo pathInfo;
    fillCommonNodeInfo(shape, &pathInfo, QStringLiteral("_path"));

    pathInfo.path.setDefaultValue(QVariant::fromValue(path));
    if (shape)
        fillPathAnimationInfo(shape, &pathInfo);

    pathInfo.fillRule = m_currentPaintInfo.fillRule;
    pathInfo.fillColor.setDefaultValue(QVariant::fromValue(m_currentPaintInfo.fill.color()));
    if (!m_currentPaintInfo.fillColorAnimation.isConstant())
        pathInfo.fillColor.addAnimation(m_currentPaintInfo.fillColorAnimation);
    if (!m_currentPaintInfo.fillOpacityAnimation.isConstant())
        pathInfo.fillOpacity.addAnimation(m_currentPaintInfo.fillOpacityAnimation);
    pathInfo.strokeStyle = StrokeStyle::fromPen(m_currentPaintInfo.stroke);
    pathInfo.strokeStyle.color.setDefaultValue(QVariant::fromValue(m_currentPaintInfo.stroke.color()));
    if (!m_currentPaintInfo.strokeColorAnimation.isConstant())
        pathInfo.strokeStyle.color.addAnimation(m_currentPaintInfo.strokeColorAnimation);
    if (!m_currentPaintInfo.strokeOpacityAnimation.isConstant())
        pathInfo.strokeStyle.opacity.addAnimation(m_currentPaintInfo.strokeOpacityAnimation);
    if (m_currentPaintInfo.fill.gradient() != nullptr)
        pathInfo.grad = *m_currentPaintInfo.fill.gradient();
    if (trimmingState() != TrimmingState::Off)
        pathInfo.trim = m_currentPaintInfo.trim;
    m_generator->generatePath(pathInfo);

    if (m_currentStructElements.isEmpty()) {
        info.stage = StructureNodeStage::End;
        m_generator->generateStructureNode(info);
    }
}

void QLottieVisitor::processPath(const QLottieShape *shape, const QPainterPath &path)
{
    QLOTTIEVISITOR_DEBUG << "[path bounds=" << path.controlPointRect() << "]";

    if (trimmingState() == Sequential) {
        QPainterPath p = m_currentPaintInfo.transform.map(path);
        p.addPath(m_currentPaintInfo.unitedPath);
        m_currentPaintInfo.unitedPath = p;
    } else {
        processShape(shape, path);
    }
}

QT_END_NAMESPACE
