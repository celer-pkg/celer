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

#ifndef QQUICKGRAPHSSPLINESERIES_P_H
#define QQUICKGRAPHSSPLINESERIES_P_H

#include "theme/qquickgraphscolor_p.h"
#include <qspline3dseries.h>

#include <QtQml/qqml.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include <private/qgraphsglobal_p.h>

QT_BEGIN_NAMESPACE

class QQuickGraphsSpline3DSeries : public QSpline3DSeries
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> seriesChildren READ seriesChildren CONSTANT)
    Q_PROPERTY(QQuickGradient *baseGradient READ baseGradient WRITE setBaseGradient NOTIFY
                   baseGradientChanged FINAL)
    Q_PROPERTY(QQuickGradient *singleHighlightGradient READ singleHighlightGradient WRITE
                   setSingleHighlightGradient NOTIFY singleHighlightGradientChanged FINAL)
    Q_PROPERTY(QQuickGradient *multiHighlightGradient READ multiHighlightGradient WRITE
                   setMultiHighlightGradient NOTIFY multiHighlightGradientChanged FINAL)
    // This is static method in parent class, overload as constant property for qml.
    Q_PROPERTY(int invalidSelectionIndex READ invalidSelectionIndex CONSTANT)
    Q_CLASSINFO("DefaultProperty", "seriesChildren")

    QML_ADDED_IN_VERSION(6, 9)
    QML_NAMED_ELEMENT(Spline3DSeries)

public:
    QQuickGraphsSpline3DSeries(QObject *parent = 0);
    ~QQuickGraphsSpline3DSeries() override;

    QQmlListProperty<QObject> seriesChildren();
    static void appendSeriesChildren(QQmlListProperty<QObject> *list, QObject *element);

    void setBaseGradient(QQuickGradient *gradient);
    QQuickGradient *baseGradient() const;
    void setSingleHighlightGradient(QQuickGradient *gradient);
    QQuickGradient *singleHighlightGradient() const;
    void setMultiHighlightGradient(QQuickGradient *gradient);
    QQuickGradient *multiHighlightGradient() const;

    int invalidSelectionIndex() const;

public Q_SLOTS:
    void handleBaseGradientUpdate();
    void handleSingleHighlightGradientUpdate();
    void handleMultiHighlightGradientUpdate();

Q_SIGNALS:
    void baseGradientChanged(QQuickGradient *gradient);
    void singleHighlightGradientChanged(QQuickGradient *gradient);
    void multiHighlightGradientChanged(QQuickGradient *gradient);

private:
    QQuickGradient *m_baseGradient = nullptr;
    QQuickGradient *m_singleHighlightGradient = nullptr;
    QQuickGradient *m_multiHighlightGradient = nullptr;

    void setGradientHelper(QQuickGradient *newGradient,
                           QQuickGradient *memberGradient,
                           GradientType type);
};

QT_END_NAMESPACE

#endif
