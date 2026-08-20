// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKNODEINFO_P_H
#define QQUICKNODEINFO_P_H

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

#include <QString>
#include <QPainter>
#include <QPainterPath>
#include <QMatrix4x4>
#include <QQuickItem>
#include <QtGui/private/qfixed_p.h>
#include <QtCore/qmap.h>
#include <QtQuick/qsgtexture.h>

#include "qquickanimatedproperty_p.h"

QT_BEGIN_NAMESPACE

namespace QQuickVectorImageGenerator {

struct NodeInfo
{
    QString id;
    QString nodeId;
    QString typeName;
    QString maskId;
    QString filterId;
    QString transformReferenceId;
    QString customItemType;
    QQuickAnimatedProperty transform = QQuickAnimatedProperty(QVariant::fromValue(QTransform{}));
    QQuickAnimatedProperty opacity = QQuickAnimatedProperty(QVariant::fromValue(1.0));

    // Special case: Motion path holds curves that the node moves along. The default value
    // holds additional parameters: First of the pair is whether or not the node adapts to
    // the angle of the tangent of the path. Second is an additional rotation to apply on top
    QQuickAnimatedProperty motionPath = QQuickAnimatedProperty(QVariant::fromValue(QVariantPair(false, 0.0)));

    bool isDefaultTransform = true;
    bool isDefaultOpacity = true;
    bool isVisible = true;
    bool isDisplayed = true; // TODO: Map to display enum in QtSvg
    bool isMaskAlpha = false;
    bool isMaskInverted = false;
    QQuickAnimatedProperty visibility = QQuickAnimatedProperty(QVariant::fromValue(true));
    int visibilityEndTime = -1;
    QRectF bounds;
};

struct ImageNodeInfo : NodeInfo
{
    QImage image;
    QRectF rect;
    QString externalFileReference;
};

struct StrokeStyle
{
    Qt::PenCapStyle lineCapStyle = Qt::SquareCap;
    Qt::PenJoinStyle lineJoinStyle = Qt::MiterJoin;
    int miterLimit = 4;
    bool cosmetic = false;
    qreal dashOffset = 0;
    QList<qreal> dashArray;
    QQuickAnimatedProperty color = QQuickAnimatedProperty(QVariant::fromValue(QColorConstants::Transparent));
    QQuickAnimatedProperty opacity = QQuickAnimatedProperty(QVariant::fromValue(qreal(1.0)));
    qreal width = 1.0;

    static StrokeStyle fromPen(const QPen &p)
    {
        StrokeStyle style;
        style.lineCapStyle = p.capStyle();
        style.lineJoinStyle = p.joinStyle() == Qt::SvgMiterJoin ? Qt::MiterJoin : p.joinStyle(); //TODO support SvgMiterJoin
        style.miterLimit = qRound(p.miterLimit());
        style.cosmetic = p.isCosmetic();
        style.dashOffset = p.dashOffset();
        style.dashArray = p.dashPattern();
        style.width = p.widthF();

        return style;
    }
};

struct PathTrimInfo
{
    bool enabled = false;
    QQuickAnimatedProperty start = QQuickAnimatedProperty(QVariant::fromValue(0.0));
    QQuickAnimatedProperty end = QQuickAnimatedProperty(QVariant::fromValue(1.0));
    QQuickAnimatedProperty offset = QQuickAnimatedProperty(QVariant::fromValue(0.0));
};

struct PathNodeInfo : NodeInfo
{
    QQuickAnimatedProperty path = QQuickAnimatedProperty(QVariant::fromValue(QPainterPath{}));
    Qt::FillRule fillRule = Qt::FillRule::WindingFill;
    QQuickAnimatedProperty fillColor = QQuickAnimatedProperty(QVariant::fromValue(QColor{}));
    QQuickAnimatedProperty fillOpacity = QQuickAnimatedProperty(QVariant::fromValue(qreal(1.0)));
    StrokeStyle strokeStyle;
    QGradient grad;
    QTransform fillTransform;
    PathTrimInfo trim;
};

struct TextNodeInfo : NodeInfo
{
    bool isTextArea;
    bool needsRichText;
    QPointF position;
    QSizeF size;
    QString text;
    QFont font;
    Qt::Alignment alignment;
    QQuickAnimatedProperty fillColor = QQuickAnimatedProperty(QVariant::fromValue(QColor{}));
    QQuickAnimatedProperty fillOpacity = QQuickAnimatedProperty(QVariant::fromValue(qreal(1.0)));
    QQuickAnimatedProperty strokeColor = QQuickAnimatedProperty(QVariant::fromValue(QColor{}));
    QQuickAnimatedProperty strokeOpacity = QQuickAnimatedProperty(QVariant::fromValue(qreal(1.0)));
};

struct AnimateColorNodeInfo : NodeInfo
{
};

enum class StructureNodeStage
{
    Start,
    End
};

struct UseNodeInfo : NodeInfo
{
    StructureNodeStage stage;
};

struct StructureNodeInfo : NodeInfo
{
    StructureNodeStage stage = StructureNodeStage::Start;
    bool forceSeparatePaths = false;
    QRectF viewBox;
    QSize size;
    QRectF clipBox;
    bool isPathContainer = false;
};

struct MaskNodeInfo : NodeInfo
{
    StructureNodeStage stage = StructureNodeStage::Start;

    bool isMaskRectRelativeCoordinates = false;
    QRectF maskRect;
};

struct FilterNodeInfo : NodeInfo
{
    enum class Type {
        None,
        GaussianBlur,
        ColorMatrix,
        Offset,
        Flood,
        CompositeOver,
        CompositeIn,
        CompositeOut,
        CompositeAtop,
        CompositeXor,
        CompositeLighter,
        CompositeArithmetic,
        BlendNormal,
        BlendMultiply,
        BlendScreen,
        BlendDarken,
        BlendLighten,
        Merge,
        MergeNode
    };

    enum class CoordinateSystem {
        Absolute,
        Relative,
        MatchFilterRect // Special case
    };

    enum class FilterInput {
        None,
        SourceAlpha,
        SourceColor,
        Name
    };

    QRectF filterRect;
    CoordinateSystem csFilterRect = CoordinateSystem::Absolute;
    QSGTexture::WrapMode wrapMode = QSGTexture::ClampToEdge;

    struct FilterStep {
        Type filterType = Type::None;
        QRectF filterPrimitiveRect;
        QVariant filterParameter;
        CoordinateSystem csFilterParameter = CoordinateSystem::Absolute;

        QString outputName;

        FilterInput input1 = FilterInput::SourceColor;
        FilterInput input2 = FilterInput::None; // Only used by some effects

        QString namedInput1;
        QString namedInput2;
    };
    QList<FilterStep> steps;
};

}

QT_END_NAMESPACE

#endif //QQUICKNODEINFO_P_H
