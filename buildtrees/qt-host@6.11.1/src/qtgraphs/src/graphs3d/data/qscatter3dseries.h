// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QSCATTER3DSERIES_H
#define QTGRAPHS_QSCATTER3DSERIES_H

#include <QtGraphs/qabstract3dseries.h>
#include <QtGraphs/qscatterdataproxy.h>

QT_BEGIN_NAMESPACE

class QScatter3DSeriesPrivate;
class QValue3DAxis;

class Q_GRAPHS_EXPORT QScatter3DSeries : public QAbstract3DSeries
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QScatter3DSeries)
    Q_PROPERTY(QScatterDataProxy *dataProxy READ dataProxy WRITE setDataProxy NOTIFY dataProxyChanged FINAL)
    Q_PROPERTY(
        qsizetype selectedItem READ selectedItem WRITE setSelectedItem NOTIFY selectedItemChanged FINAL)
    Q_PROPERTY(float itemSize READ itemSize WRITE setItemSize NOTIFY itemSizeChanged FINAL)
    Q_PROPERTY(QScatterDataArray dataArray READ dataArray WRITE setDataArray NOTIFY dataArrayChanged FINAL)
    Q_PROPERTY(QList<QVector3D> scaleArray READ scaleArray WRITE setScaleArray NOTIFY
        scaleArrayChanged REVISION(6, 10))

    Q_PROPERTY(QValue3DAxis *axisX READ axisX WRITE setAxisX RESET resetAxisX NOTIFY axisXChanged
                       REVISION(6, 11))
    Q_PROPERTY(QValue3DAxis *axisY READ axisY WRITE setAxisY RESET resetAxisY NOTIFY axisYChanged
                       REVISION(6, 11))
    Q_PROPERTY(QValue3DAxis *axisZ READ axisZ WRITE setAxisZ RESET resetAxisZ NOTIFY axisZChanged
                       REVISION(6, 11))

    QML_ELEMENT
    QML_UNCREATABLE("Trying to create uncreatable: QScatter3DSeries, use Scatter3DSeries instead.")

public:
    explicit QScatter3DSeries(QObject *parent = nullptr);
    explicit QScatter3DSeries(QScatterDataProxy *dataProxy, QObject *parent = nullptr);
    ~QScatter3DSeries() override;

    void setDataProxy(QScatterDataProxy *proxy);
    QScatterDataProxy *dataProxy() const;

    void setSelectedItem(qsizetype index);
    qsizetype selectedItem() const;
    static qsizetype invalidSelectionIndex();

    void setItemSize(float size);
    float itemSize() const;

    void setDataArray(const QScatterDataArray &newDataArray);
    void clearArray();
    const QScatterDataArray &dataArray() const &;
    QScatterDataArray dataArray() &&;

    void setScaleArray(const QList<QVector3D> &newScaleArray);
    void clearScaleArray();
    const QList<QVector3D> &scaleArray() const &;
    QList<QVector3D> scaleArray() &&;

    void setAxisX(QValue3DAxis *axis);
    void setAxisY(QValue3DAxis *axis);
    void setAxisZ(QValue3DAxis *axis);
    QValue3DAxis *axisX() const;
    QValue3DAxis *axisY() const;
    QValue3DAxis *axisZ() const;
    void resetAxisX();
    void resetAxisY();
    void resetAxisZ();

Q_SIGNALS:
    void dataProxyChanged(QScatterDataProxy *proxy);
    void selectedItemChanged(qsizetype index);
    void itemSizeChanged(float size);
    void dataArrayChanged(const QScatterDataArray &array);
    Q_REVISION(6, 10) void scaleArrayChanged(const QList<QVector3D> &scaleArray);
    Q_REVISION(6, 11) void axisXChanged(QValue3DAxis *axis);
    Q_REVISION(6, 11) void axisYChanged(QValue3DAxis *axis);
    Q_REVISION(6, 11) void axisZChanged(QValue3DAxis *axis);

protected:
    explicit QScatter3DSeries(QScatter3DSeriesPrivate &d, QObject *parent = nullptr);

private:
    Q_DISABLE_COPY(QScatter3DSeries)

    friend class QQuickGraphsScatter;
};

QT_END_NAMESPACE

#endif
