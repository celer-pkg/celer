// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_Q3DSURFACEWIDGETITEM_H
#define QTGRAPHS_Q3DSURFACEWIDGETITEM_H

#include <QtGraphs/qsurface3dseries.h>
#include <QtGraphs/qvalue3daxis.h>
#include <QtGraphsWidgets/q3dgraphswidgetitem.h>

QT_BEGIN_NAMESPACE

class QQuickGraphsSurface;
class Q3DSurfaceWidgetItemPrivate;

class Q_GRAPHSWIDGETS_EXPORT Q3DSurfaceWidgetItem : public Q3DGraphsWidgetItem
{
    Q_OBJECT
    Q_PROPERTY(QValue3DAxis *axisX READ axisX WRITE setAxisX NOTIFY axisXChanged)
    Q_PROPERTY(QValue3DAxis *axisY READ axisY WRITE setAxisY NOTIFY axisYChanged)
    Q_PROPERTY(QValue3DAxis *axisZ READ axisZ WRITE setAxisZ NOTIFY axisZChanged)
    Q_PROPERTY(QSurface3DSeries *selectedSeries READ selectedSeries NOTIFY selectedSeriesChanged)
    Q_PROPERTY(bool flipHorizontalGrid READ flipHorizontalGrid WRITE setFlipHorizontalGrid NOTIFY
                   flipHorizontalGridChanged)

public:
    explicit Q3DSurfaceWidgetItem(QObject *parent = nullptr);
    ~Q3DSurfaceWidgetItem() override;

    void addSeries(QSurface3DSeries *series);
    void removeSeries(QSurface3DSeries *series);
    QList<QSurface3DSeries *> seriesList() const;

    // Axes
    void setAxisX(QValue3DAxis *axis);
    QValue3DAxis *axisX() const;
    void setAxisY(QValue3DAxis *axis);
    QValue3DAxis *axisY() const;
    void setAxisZ(QValue3DAxis *axis);
    QValue3DAxis *axisZ() const;
    void addAxis(QValue3DAxis *axis);
    void releaseAxis(QValue3DAxis *axis);
    QList<QValue3DAxis *> axes() const;

    QSurface3DSeries *selectedSeries() const;
    void setFlipHorizontalGrid(bool flip);
    bool flipHorizontalGrid() const;

    Q_REVISION(6, 10)
    void renderSliceToImage(int index, int requestedIndex,
                            QtGraphs3D::SliceCaptureType sliceType);

protected:
    bool event(QEvent *event) override;

Q_SIGNALS:
    void axisXChanged(QValue3DAxis *axis);
    void axisYChanged(QValue3DAxis *axis);
    void axisZChanged(QValue3DAxis *axis);
    void selectedSeriesChanged(QSurface3DSeries *series);
    void flipHorizontalGridChanged(bool flip);
    Q_REVISION(6, 10)
    void sliceImageChanged(const QImage &image);

private:
    Q_DECLARE_PRIVATE(Q3DSurfaceWidgetItem)
    QQuickGraphsSurface *graphSurface();
    const QQuickGraphsSurface *graphSurface() const;
};

QT_END_NAMESPACE

#endif
