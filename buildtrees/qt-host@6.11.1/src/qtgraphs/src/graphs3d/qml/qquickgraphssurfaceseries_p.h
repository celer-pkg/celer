// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QQUICKGRAPHSSURFACESERIES_P_H
#define QQUICKGRAPHSSURFACESERIES_P_H

#include "gradientholder_p.h"
#include "qsurface3dseries.h"
#include "theme/qquickgraphscolor_p.h"

#include <QtQml/qqml.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include <private/qgraphsglobal_p.h>
#include <private/qgraphstheme_p.h>

QT_BEGIN_NAMESPACE

class QQuickGraphsSurface3DSeries : public QSurface3DSeries
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> seriesChildren READ seriesChildren CONSTANT)
    // This property is overloaded to use QPointF instead of QPoint to work
    // around qml bug where Qt.point(0, 0) can't be assigned due to error
    // "Cannot assign QPointF to QPoint".
    Q_PROPERTY(QPointF selectedPoint READ selectedPoint WRITE setSelectedPoint NOTIFY
                   selectedPointChanged FINAL)
    // This is static method in parent class, overload as constant property for
    // qml.
    Q_PROPERTY(QPointF invalidSelectionPosition READ invalidSelectionPosition CONSTANT)
    Q_PROPERTY(QQuickGradient *baseGradient READ baseGradient WRITE setBaseGradient NOTIFY
                   baseGradientChanged FINAL)
    Q_PROPERTY(QQuickGradient *singleHighlightGradient READ singleHighlightGradient WRITE
                   setSingleHighlightGradient NOTIFY singleHighlightGradientChanged FINAL)
    Q_PROPERTY(QQuickGradient *multiHighlightGradient READ multiHighlightGradient WRITE
                   setMultiHighlightGradient NOTIFY multiHighlightGradientChanged FINAL)
    Q_CLASSINFO("DefaultProperty", "seriesChildren")

    QML_NAMED_ELEMENT(Surface3DSeries)

public:
    QQuickGraphsSurface3DSeries(QObject *parent = 0);
    ~QQuickGraphsSurface3DSeries() override;

    void setSelectedPoint(QPointF position);
    QPointF selectedPoint() const;
    QPointF invalidSelectionPosition() const;

    QQmlListProperty<QObject> seriesChildren();
    static void appendSeriesChildren(QQmlListProperty<QObject> *list, QObject *element);

    void setBaseGradient(QQuickGradient *gradient);
    QQuickGradient *baseGradient() const;
    void setSingleHighlightGradient(QQuickGradient *gradient);
    QQuickGradient *singleHighlightGradient() const;
    void setMultiHighlightGradient(QQuickGradient *gradient);
    QQuickGradient *multiHighlightGradient() const;

public Q_SLOTS:
    void handleBaseGradientUpdate();
    void handleSingleHighlightGradientUpdate();
    void handleMultiHighlightGradientUpdate();

Q_SIGNALS:
    void selectedPointChanged(QPointF position);
    void baseGradientChanged(QQuickGradient *gradient);
    void singleHighlightGradientChanged(QQuickGradient *gradient);
    void multiHighlightGradientChanged(QQuickGradient *gradient);

    void gradientsChanged();

private:
    GradientHolder m_gradients;

    void setGradientHelper(QQuickGradient *newGradient,
                           QQuickGradient *memberGradient,
                           GradientType type);
};

QT_END_NAMESPACE

#endif
