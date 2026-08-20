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

#ifndef QQUICKGRAPHSBARSSERIES_P_H
#define QQUICKGRAPHSBARSSERIES_P_H

#include "gradientholder_p.h"
#include "qbar3dseries.h"
#include "theme/qquickgraphscolor_p.h"

#include <QtQml/qqml.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include <private/qgraphsglobal_p.h>
#include <private/qgraphstheme_p.h>

QT_BEGIN_NAMESPACE

class QQuickGraphsBar3DSeries : public QBar3DSeries
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> seriesChildren READ seriesChildren CONSTANT)
    // This property is overloaded to use QPointF instead of QPoint to work around
    // qml bug where Qt.point(0, 0) can't be assigned due to error "Cannot assign
    // QPointF to QPoint".
    Q_PROPERTY(
        QPointF selectedBar READ selectedBar WRITE setSelectedBar NOTIFY selectedBarChanged FINAL)
    // This is static method in parent class, overload as constant property for qml.
    Q_PROPERTY(QPointF invalidSelectionPosition READ invalidSelectionPosition CONSTANT)
    Q_PROPERTY(QQuickGradient *baseGradient READ baseGradient WRITE setBaseGradient NOTIFY
                   baseGradientChanged FINAL)
    Q_PROPERTY(QQuickGradient *singleHighlightGradient READ singleHighlightGradient WRITE
                   setSingleHighlightGradient NOTIFY singleHighlightGradientChanged FINAL)
    Q_PROPERTY(QQuickGradient *multiHighlightGradient READ multiHighlightGradient WRITE
                   setMultiHighlightGradient NOTIFY multiHighlightGradientChanged FINAL)
    Q_PROPERTY(QQmlListProperty<QQuickGraphsColor> rowColors READ rowColors CONSTANT)
    Q_CLASSINFO("DefaultProperty", "seriesChildren")

    QML_NAMED_ELEMENT(Bar3DSeries)

public:
    explicit QQuickGraphsBar3DSeries(QObject *parent = 0);
    ~QQuickGraphsBar3DSeries() override;

    QQmlListProperty<QObject> seriesChildren();
    static void appendSeriesChildren(QQmlListProperty<QObject> *list, QObject *element);

    void setSelectedBar(QPointF position);
    QPointF selectedBar() const;
    QPointF invalidSelectionPosition() const;

    void setBaseGradient(QQuickGradient *gradient);
    QQuickGradient *baseGradient() const;
    void setSingleHighlightGradient(QQuickGradient *gradient);
    QQuickGradient *singleHighlightGradient() const;
    void setMultiHighlightGradient(QQuickGradient *gradient);
    QQuickGradient *multiHighlightGradient() const;

    QQmlListProperty<QQuickGraphsColor> rowColors();
    static void appendRowColorsFunc(QQmlListProperty<QQuickGraphsColor> *list,
                                    QQuickGraphsColor *color);
    static qsizetype countRowColorsFunc(QQmlListProperty<QQuickGraphsColor> *list);
    static QQuickGraphsColor *atRowColorsFunc(QQmlListProperty<QQuickGraphsColor> *list,
                                              qsizetype index);
    static void clearRowColorsFunc(QQmlListProperty<QQuickGraphsColor> *list);

public Q_SLOTS:
    void handleBaseGradientUpdate();
    void handleSingleHighlightGradientUpdate();
    void handleMultiHighlightGradientUpdate();
    void handleRowColorUpdate();

Q_SIGNALS:
    void selectedBarChanged(QPointF position);
    void baseGradientChanged(QQuickGradient *gradient);
    void singleHighlightGradientChanged(QQuickGradient *gradient);
    void multiHighlightGradientChanged(QQuickGradient *gradient);

private:
    GradientHolder m_gradients;

    QList<QQuickGraphsColor *> m_rowColors;
    bool m_dummyColors = false;

    void addColor(QQuickGraphsColor *color);
    QList<QQuickGraphsColor *> colorList();
    void clearColors();
    void clearDummyColors();

    void setGradientHelper(QQuickGradient *newGradient,
                           QQuickGradient *memberGradient,
                           GradientType type);
};

QT_END_NAMESPACE

#endif
