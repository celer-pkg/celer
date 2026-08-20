// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QSCATTER3DSERIES_H
#define QTDATAVIS3D_QSCATTER3DSERIES_H

#include <QtDataVisualization/qabstract3dseries.h>
#include <QtDataVisualization/qscatterdataproxy.h>

QT_BEGIN_NAMESPACE

class QScatter3DSeriesPrivate;

class Q_DATAVISUALIZATION_EXPORT QScatter3DSeries : public QAbstract3DSeries
{
    Q_OBJECT
    Q_PROPERTY(QScatterDataProxy *dataProxy READ dataProxy WRITE setDataProxy NOTIFY dataProxyChanged)
    Q_PROPERTY(int selectedItem READ selectedItem WRITE setSelectedItem NOTIFY selectedItemChanged)
    Q_PROPERTY(float itemSize READ itemSize WRITE setItemSize NOTIFY itemSizeChanged)

public:
    explicit QScatter3DSeries(QObject *parent = nullptr);
    explicit QScatter3DSeries(QScatterDataProxy *dataProxy, QObject *parent = nullptr);
    virtual ~QScatter3DSeries();

    void setDataProxy(QScatterDataProxy *proxy);
    QScatterDataProxy *dataProxy() const;

    void setSelectedItem(int index);
    int selectedItem() const;
    static int invalidSelectionIndex();

    void setItemSize(float size);
    float itemSize() const;

Q_SIGNALS:
    void dataProxyChanged(QScatterDataProxy *proxy);
    void selectedItemChanged(int index);
    void itemSizeChanged(float size);

protected:
    explicit QScatter3DSeries(QScatter3DSeriesPrivate *d, QObject *parent = nullptr);
    QScatter3DSeriesPrivate *dptr();
    const QScatter3DSeriesPrivate *dptrc() const;

private:
    Q_DISABLE_COPY(QScatter3DSeries)

    friend class Scatter3DController;
};

QT_END_NAMESPACE

#endif
