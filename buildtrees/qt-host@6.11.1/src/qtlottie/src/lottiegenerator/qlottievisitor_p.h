// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QLOTTIEVISITOR_P_H
#define QLOTTIEVISITOR_P_H

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

#include <QJsonDocument>
#include <QJsonArray>

#include <QtCore/QStack>
#include <QtGui/qbrush.h>
#include <QtGui/qpen.h>

#include <QtGui/private/qfixed_p.h>

#include <private/qlottiebase_p.h>
#include <private/qlottielayer_p.h>
#include <private/qlottiegroup_p.h>
#include <private/qlottiebasictransform_p.h>
#include <private/qquickgenerator_p.h>
#include <private/qquicknodeinfo_p.h>
#include <private/qfixed_p.h>

#include <QtLottie/private/qlottierenderer_p.h>
#include <QtLottieVectorImageGenerator/qtlottievectorimagegeneratorexports.h>

QT_BEGIN_NAMESPACE

class QLottieRoot;

class Q_LOTTIEVECTORIMAGEGENERATOR_EXPORT QLottieVisitor : public QLottieRenderer
{
public:
    struct PaintInfo
    {
        QBrush fill = Qt::transparent;
        QQuickAnimatedProperty::PropertyAnimation fillColorAnimation;
        QQuickAnimatedProperty::PropertyAnimation fillOpacityAnimation;
        QPen stroke = QPen(Qt::transparent);
        QQuickAnimatedProperty::PropertyAnimation strokeColorAnimation;
        QQuickAnimatedProperty::PropertyAnimation strokeOpacityAnimation;
        qreal opacity = 1.0;
        Qt::FillRule fillRule = Qt::WindingFill;
        QPainterPath unitedPath;
        PathTrimInfo trim;

        QTransform transform;

        struct TransformAnimationInfo
        {
            QTransform::TransformationType animationType;

            QMap<int, QVariant> frames;
            QMap<int, QBezier> easingPerFrame;
        };
        QList<TransformAnimationInfo> transformAnimations;
    };

    QLottieVisitor(const QString lottieFileName, QQuickGenerator *generator);
    virtual ~QLottieVisitor() {}

    void saveState() override;
    void restoreState() override;

    void render(const QLottieRoot &layer);

    void render(const QLottieLayer &layer) override;
    void render(const QLottieSolidLayer &layer) override;
    void render(const QLottieGroup &group) override;

    void finish(const QLottieLayer &layer) override;
    void finish(const QLottieGroup &group) override;

    void render(const QLottieRect &rect) override;
    void render(const QLottieEllipse &ellipse) override;
    void render(const QLottiePolyStar &star) override;
    void render(const QLottieRound &round) override;
    void render(const QLottieFill &fill) override;
    void render(const QLottieGFill &shape) override;
    void render(const QLottieImage &image) override;
    void render(const QLottieStroke &stroke) override;
    void render(const QLottieBasicTransform &transform) override;
    void render(const QLottieShapeTransform &transform) override;
    void render(const QLottieFreeFormShape &shape) override;
    void render(const QLottieTrimPath &trim) override;
    void render(const QLottieFillEffect &effect) override;
    void render(const QLottieRepeater &repeater) override;
    void renderPathElements(const QList<QLottieBase *> &pathElements) override;

    void generateMatteNode(const QLottieLayer *layer, StructureNodeStage stage);
    void fillCommonNodeInfo(const QLottieBase *node, NodeInfo *info, const QString &suffix = QString{});
    void fillAnimationNodeInfo(const QLottieBase *node, NodeInfo *info);
    void fillBasicPathInfo(const QLottieShape *strokeOrFill, PathNodeInfo *pathInfo);
    void fillLayerAnimationInfo(const QLottieLayer *node, NodeInfo *info);
    void fillPathAnimationInfo(const QLottieShape *shape, PathNodeInfo *info);

private:
    static bool nodeIsGraphicElement(const QLottieBase *node);
    static bool nodeIsShape(const QLottieBase *node);
    static bool hasAnimations(const QLottieBasicTransform *transform, bool isShapeTransform = false);
    void processPath(const QLottieShape *shape, const QPainterPath &path);
    void processShape(const QLottieShape *shape, const QPainterPath &path);
    void collectTransformAnimations(const QLottieBasicTransform *transform,
                                    bool isShapeTransform = false);
    QQuickAnimatedProperty::PropertyAnimation makeColorAnimation(const QLottieProperty4D<QVector4D> &colorProperty);
    QQuickAnimatedProperty::PropertyAnimation makeOpacityAnimation(const QLottieProperty<qreal> &opacityProperty);

    QString idForNode(const QLottieBase *node);
    int timePointForFrame(qreal frameNo, bool doWrap = true) const;
    QString scrub(const QString &raw);

    QString m_lottieFileName;
    QQuickGenerator *m_generator;

    PaintInfo m_currentPaintInfo;

    QList<PaintInfo> m_savedPaintInfos;

    int m_frameRate = 30;
    int m_duration = 1000;
    qreal m_frameOffset = 0;

    QStack<const QLottieBase *> m_currentStructElements;

    int m_nodeIdCounter = 0;
    QHash<const QLottieBase *, QString> m_idForNodeId;
};

QT_END_NAMESPACE

#endif // QLOTTIEVISITOR_P_H

