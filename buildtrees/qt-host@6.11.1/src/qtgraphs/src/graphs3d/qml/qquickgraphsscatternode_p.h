// Copyright (C) 2025 The Qt Company Ltd.
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
//
#ifndef QQUICKGRAPHSSCATTERNODE_P_H
#define QQUICKGRAPHSSCATTERNODE_P_H

#include "qquickgraphsnode_p.h"
#include "qquickgraphsscatter_p.h"
#include "qbar3dseries.h"

#include <private/qgraphsglobal_p.h>

QT_BEGIN_NAMESPACE

class QQuickGraphsScatterNode;

class Q_GRAPHS_EXPORT QQuickGraphsScatterNode : public QQuickGraphsNode
{
    Q_OBJECT
    Q_PROPERTY(QValue3DAxis *axisX READ axisX WRITE setAxisX NOTIFY axisXChanged)
    Q_PROPERTY(QValue3DAxis *axisY READ axisY WRITE setAxisY NOTIFY axisYChanged)
    Q_PROPERTY(QValue3DAxis *axisZ READ axisZ WRITE setAxisZ NOTIFY axisZChanged)
    Q_PROPERTY(QScatter3DSeries *selectedSeries READ selectedSeries NOTIFY selectedSeriesChanged)
    Q_PROPERTY(QQmlListProperty<QScatter3DSeries> seriesList READ seriesList CONSTANT)
    Q_CLASSINFO("DefaultProperty", "seriesList")

    QML_ADDED_IN_VERSION(6, 10)
    QML_NAMED_ELEMENT(Scatter3DNode)

public:
    explicit QQuickGraphsScatterNode(QQuick3DNode *parent = nullptr);
    ~QQuickGraphsScatterNode() override;

    QValue3DAxis *axisX() const;
    void setAxisX(QValue3DAxis *axis);

    QValue3DAxis *axisY() const;
    void setAxisY(QValue3DAxis *axis);

    QValue3DAxis *axisZ() const;
    void setAxisZ(QValue3DAxis *axis);

    QScatter3DSeries *selectedSeries() const;

    void setSelectionMode(QtGraphs3D::SelectionFlags mode) override;

    QList<QScatter3DSeries *> scatterSeriesList();
    QQmlListProperty<QScatter3DSeries> seriesList();
    static void appendSeriesFunc(QQmlListProperty<QScatter3DSeries> *list, QScatter3DSeries *series);
    static qsizetype countSeriesFunc(QQmlListProperty<QScatter3DSeries> *list);
    static QScatter3DSeries *atSeriesFunc(QQmlListProperty<QScatter3DSeries> *list, qsizetype index);
    static void clearSeriesFunc(QQmlListProperty<QScatter3DSeries> *list);

    Q_INVOKABLE void addSeries(QScatter3DSeries *series);
    Q_INVOKABLE void removeSeries(QScatter3DSeries *series);
    Q_INVOKABLE void clearSelection() override;

    Q_INVOKABLE bool doPicking(QPointF point) override;
    Q_INVOKABLE bool doRayPicking(const QVector3D &origin, const QVector3D &direction) override;

protected:
    void componentComplete() override;

Q_SIGNALS:
    void axisXChanged(QValue3DAxis *axis);
    void axisYChanged(QValue3DAxis *axis);
    void axisZChanged(QValue3DAxis *axis);
    void selectedSeriesChanged(QScatter3DSeries *series);

private:
    QQuickGraphsScatter *graphScatter();
    const QQuickGraphsScatter *graphScatter() const;

    bool m_multiSeriesUniform;
    QSizeF m_barSpacing;
    float m_barThickness;
    bool m_barSpacingRelative;
    QSizeF m_barSeriesMargin;
    QBar3DSeries *m_primarySeries = nullptr;
    QBar3DSeries *m_selectedSeries = nullptr;
    float m_floorLevel;
};

QT_END_NAMESPACE

#endif // QQUICKGRAPHSSCATTERNODE_P_H
