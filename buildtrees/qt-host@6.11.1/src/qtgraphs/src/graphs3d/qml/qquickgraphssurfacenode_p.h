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
#ifndef QQUICKGRAPHSSURFACENODE_P_H
#define QQUICKGRAPHSSURFACENODE_P_H

#include "qquickgraphsnode_p.h"
#include "qquickgraphssurface_p.h"
#include "qsurface3dseries.h"

#include <private/qgraphsglobal_p.h>

QT_BEGIN_NAMESPACE

class QQuickGraphsSurfaceNode;

class Q_GRAPHS_EXPORT QQuickGraphsSurfaceNode : public QQuickGraphsNode
{
    Q_OBJECT
    Q_PROPERTY(QValue3DAxis *axisX READ axisX WRITE setAxisX NOTIFY axisXChanged)
    Q_PROPERTY(QValue3DAxis *axisY READ axisY WRITE setAxisY NOTIFY axisYChanged)
    Q_PROPERTY(QValue3DAxis *axisZ READ axisZ WRITE setAxisZ NOTIFY axisZChanged)
    Q_PROPERTY(QSurface3DSeries *selectedSeries READ selectedSeries NOTIFY selectedSeriesChanged)
    Q_PROPERTY(QQmlListProperty<QSurface3DSeries> seriesList READ seriesList CONSTANT)
    Q_PROPERTY(bool flipHorizontalGrid READ flipHorizontalGrid WRITE setFlipHorizontalGrid NOTIFY
                   flipHorizontalGridChanged)
    Q_CLASSINFO("DefaultProperty", "seriesList")

    QML_ADDED_IN_VERSION(6, 10)
    QML_NAMED_ELEMENT(Surface3DNode)

public:
    explicit QQuickGraphsSurfaceNode(QQuick3DNode *parent = nullptr);
    ~QQuickGraphsSurfaceNode() override;

    QValue3DAxis *axisX() const;
    void setAxisX(QValue3DAxis *axis);

    QValue3DAxis *axisY() const;
    void setAxisY(QValue3DAxis *axis);

    QValue3DAxis *axisZ() const;
    void setAxisZ(QValue3DAxis *axis);

    QSurface3DSeries *selectedSeries() const;

    void setSelectionMode(QtGraphs3D::SelectionFlags mode) override;

    bool flipHorizontalGrid() const;
    void setFlipHorizontalGrid(bool flip);

    QList<QSurface3DSeries *> surfaceSeriesList();
    QQmlListProperty<QSurface3DSeries> seriesList();
    static void appendSeriesFunc(QQmlListProperty<QSurface3DSeries> *list, QSurface3DSeries *series);
    static qsizetype countSeriesFunc(QQmlListProperty<QSurface3DSeries> *list);
    static QSurface3DSeries *atSeriesFunc(QQmlListProperty<QSurface3DSeries> *list, qsizetype index);
    static void clearSeriesFunc(QQmlListProperty<QSurface3DSeries> *list);

    Q_INVOKABLE void addSeries(QSurface3DSeries *series);
    Q_INVOKABLE void removeSeries(QSurface3DSeries *series);
    Q_INVOKABLE void clearSelection() override;

    Q_INVOKABLE bool doPicking(QPointF point) override;
    Q_INVOKABLE bool doRayPicking(const QVector3D &origin, const QVector3D &direction) override;

protected:
    void componentComplete() override;

Q_SIGNALS:
    void axisXChanged(QValue3DAxis *axis);
    void axisYChanged(QValue3DAxis *axis);
    void axisZChanged(QValue3DAxis *axis);
    void selectedSeriesChanged(QSurface3DSeries *series);
    void flipHorizontalGridChanged(bool flip);

private:
    QQuickGraphsSurface *graphSurface();
    const QQuickGraphsSurface *graphSurface() const;

    bool m_flipHorizontalGrid;
};

QT_END_NAMESPACE

#endif // QQUICKGRAPHSSURFACENODE_P_H
