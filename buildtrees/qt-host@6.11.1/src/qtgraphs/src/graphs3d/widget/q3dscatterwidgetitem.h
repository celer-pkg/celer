// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_Q3DSCATTERWIDGETITEM_H
#define QTGRAPHS_Q3DSCATTERWIDGETITEM_H

#include <QtGraphs/qscatter3dseries.h>
#include <QtGraphs/qvalue3daxis.h>
#include <QtGraphsWidgets/q3dgraphswidgetitem.h>

QT_BEGIN_NAMESPACE

class QQuickGraphsScatter;
class Q3DScatterWidgetItemPrivate;

class Q_GRAPHSWIDGETS_EXPORT Q3DScatterWidgetItem : public Q3DGraphsWidgetItem
{
    Q_OBJECT
    Q_PROPERTY(QValue3DAxis *axisX READ axisX WRITE setAxisX NOTIFY axisXChanged)
    Q_PROPERTY(QValue3DAxis *axisY READ axisY WRITE setAxisY NOTIFY axisYChanged)
    Q_PROPERTY(QValue3DAxis *axisZ READ axisZ WRITE setAxisZ NOTIFY axisZChanged)
    Q_PROPERTY(QScatter3DSeries *selectedSeries READ selectedSeries NOTIFY selectedSeriesChanged)

public:
    explicit Q3DScatterWidgetItem(QObject *parent = nullptr);
    ~Q3DScatterWidgetItem() override;

    void addSeries(QScatter3DSeries *series);
    void removeSeries(QScatter3DSeries *series);
    QList<QScatter3DSeries *> seriesList() const;

    void setAxisX(QValue3DAxis *axis);
    QValue3DAxis *axisX() const;
    void setAxisY(QValue3DAxis *axis);
    QValue3DAxis *axisY() const;
    void setAxisZ(QValue3DAxis *axis);
    QValue3DAxis *axisZ() const;
    void addAxis(QValue3DAxis *axis);
    void releaseAxis(QValue3DAxis *axis);
    QList<QValue3DAxis *> axes() const;

    QScatter3DSeries *selectedSeries() const;

protected:
    bool event(QEvent *event) override;

Q_SIGNALS:
    void axisXChanged(QValue3DAxis *axis);
    void axisYChanged(QValue3DAxis *axis);
    void axisZChanged(QValue3DAxis *axis);
    void selectedSeriesChanged(QScatter3DSeries *series);

private:
    Q_DECLARE_PRIVATE(Q3DScatterWidgetItem)
    QQuickGraphsScatter *graphScatter();
    const QQuickGraphsScatter *graphScatter() const;
    Q_DISABLE_COPY(Q3DScatterWidgetItem)
};

QT_END_NAMESPACE

#endif
