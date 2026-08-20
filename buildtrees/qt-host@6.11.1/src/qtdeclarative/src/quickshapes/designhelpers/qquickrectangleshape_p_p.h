// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QQUICKRECTANGLESHAPE_P_P_H
#define QQUICKRECTANGLESHAPE_P_P_H

#include "qquickrectangleshape_p.h"

#include <QtQml/private/qqmlpropertyutils_p.h>
#include <QtQuickShapes/private/qquickshape_p_p.h>

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

QT_BEGIN_NAMESPACE

class Q_QUICKSHAPESDESIGNHELPERS_EXPORT QQuickRectangleShapePrivate : public QQuickShapePrivate
{
    Q_DECLARE_PUBLIC(QQuickRectangleShape)

public:
    QQuickRectangleShapePrivate() = default;

    static QQuickRectangleShapePrivate *get(QQuickRectangleShape *p) { return p->d_func(); }

    void setTopLeftRadius(int topLeftRadius, QQml::PropertyUtils::State propertyState);
    void setTopRightRadius(int topRightRadius, QQml::PropertyUtils::State propertyState);
    void setBottomLeftRadius(int bottomLeftRadius, QQml::PropertyUtils::State propertyState);
    void setBottomRightRadius(int bottomRightRadius, QQml::PropertyUtils::State propertyState);

    void setTopLeftBevel(bool topLeftBevel, QQml::PropertyUtils::State propertyState);
    void setTopRightBevel(bool topRightBevel, QQml::PropertyUtils::State propertyState);
    void setBottomLeftBevel(bool bottomLeftBevel, QQml::PropertyUtils::State propertyState);
    void setBottomRightBevel(bool bottomRightBevel, QQml::PropertyUtils::State propertyState);

    void maybeUpdateElements();

    void calculateIndependentRadii();

    enum class Edge {
        Top,
        Right,
        Bottom,
        Left,
        NEdges
    };

    QQuickShapePath *shapePath = nullptr;

    QQuickPathLine *topPathLine = nullptr;
    QQuickPathMove *topPathMove = nullptr;
    QQuickPathArc *topRightPathArc = nullptr;
    QQuickPathLine *rightPathLine = nullptr;
    QQuickPathMove *rightPathMove = nullptr;
    QQuickPathArc *bottomRightPathArc = nullptr;
    QQuickPathLine *bottomPathLine = nullptr;
    QQuickPathMove *bottomPathMove = nullptr;
    QQuickPathArc *bottomLeftPathArc = nullptr;
    QQuickPathLine *leftPathLine = nullptr;
    QQuickPathArc *topLeftPathArc = nullptr;

    static const int defaultRadius = 10;
    static const bool defaultDrawEdge = true;
    static const bool defaultBevel = false;

    int radius = defaultRadius;
    int topLeftRadius = defaultRadius;
    int topRightRadius = defaultRadius;
    int bottomLeftRadius = defaultRadius;
    int bottomRightRadius = defaultRadius;
    int effectiveTopLeftRadius = 0;
    int effectiveTopRightRadius = 0;
    int effectiveBottomLeftRadius = 0;
    int effectiveBottomRightRadius = 0;
    // If not set, the individual radii properties are equal to radius.
    bool explicitTopLeftRadius = false;
    bool explicitTopRightRadius = false;
    bool explicitBottomLeftRadius = false;
    bool explicitBottomRightRadius = false;
    bool drawTop = defaultDrawEdge;
    bool drawRight = defaultDrawEdge;
    bool drawBottom = defaultDrawEdge;
    bool drawLeft = defaultDrawEdge;
    bool bevel = defaultBevel;
    bool topLeftBevel = defaultBevel;
    bool topRightBevel = defaultBevel;
    bool bottomLeftBevel = defaultBevel;
    bool bottomRightBevel = defaultBevel;
    // If not set, the individual bevel properties are equal to bevel.
    bool explicitTopLeftBevel = false;
    bool explicitTopRightBevel = false;
    bool explicitBottomLeftBevel = false;
    bool explicitBottomRightBevel = false;
    qreal borderOffset = 0;
    qreal borderRadiusAdjustment = 0;
    Edge firstVisibleEdge = Edge::NEdges;
    QQuickRectangleShape::BorderMode borderMode = QQuickRectangleShape::BorderMode::Inside;

};

QT_END_NAMESPACE

#endif // QQUICKRECTANGLESHAPE_P_P_H
