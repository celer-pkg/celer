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

#ifndef QQUICKGRAPHSSCATTERSERIES_P_H
#define QQUICKGRAPHSSCATTERSERIES_P_H

#include "gradientholder_p.h"
#include "qscatter3dseries.h"
#include "theme/qquickgraphscolor_p.h"

#include <QtQml/qqml.h>
#include <QtQuick/private/qquickrectangle_p.h>
#include <private/qgraphsglobal_p.h>
#include <private/qgraphstheme_p.h>

QT_BEGIN_NAMESPACE

class QQuickGraphsScatter3DSeries : public QScatter3DSeries
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
    Q_PROPERTY(qsizetype invalidSelectionIndex READ invalidSelectionIndex CONSTANT)
    Q_CLASSINFO("DefaultProperty", "seriesChildren")

    QML_NAMED_ELEMENT(Scatter3DSeries)

public:
    QQuickGraphsScatter3DSeries(QObject *parent = 0);
    ~QQuickGraphsScatter3DSeries() override;

    QQmlListProperty<QObject> seriesChildren();
    static void appendSeriesChildren(QQmlListProperty<QObject> *list, QObject *element);

    void setBaseGradient(QQuickGradient *gradient);
    QQuickGradient *baseGradient() const;
    void setSingleHighlightGradient(QQuickGradient *gradient);
    QQuickGradient *singleHighlightGradient() const;
    void setMultiHighlightGradient(QQuickGradient *gradient);
    QQuickGradient *multiHighlightGradient() const;

    qsizetype invalidSelectionIndex() const;

public Q_SLOTS:
    void handleBaseGradientUpdate();
    void handleSingleHighlightGradientUpdate();
    void handleMultiHighlightGradientUpdate();

Q_SIGNALS:
    void baseGradientChanged(QQuickGradient *gradient);
    void singleHighlightGradientChanged(QQuickGradient *gradient);
    void multiHighlightGradientChanged(QQuickGradient *gradient);

private:
    GradientHolder m_gradients;

    void setGradientHelper(QQuickGradient *newGradient,
                           QQuickGradient *memberGradient,
                           GradientType type);
};

QT_END_NAMESPACE

#endif
