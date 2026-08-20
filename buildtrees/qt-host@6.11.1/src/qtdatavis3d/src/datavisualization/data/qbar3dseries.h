// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QBAR3DSERIES_H
#define QTDATAVIS3D_QBAR3DSERIES_H

#include <QtDataVisualization/qabstract3dseries.h>
#include <QtDataVisualization/qbardataproxy.h>
#include <QtCore/QPoint>

QT_BEGIN_NAMESPACE

class QBar3DSeriesPrivate;

class Q_DATAVISUALIZATION_EXPORT QBar3DSeries : public QAbstract3DSeries
{
    Q_OBJECT
    Q_PROPERTY(QBarDataProxy *dataProxy READ dataProxy WRITE setDataProxy NOTIFY dataProxyChanged)
    Q_PROPERTY(QPoint selectedBar READ selectedBar WRITE setSelectedBar NOTIFY selectedBarChanged)
    Q_PROPERTY(float meshAngle READ meshAngle WRITE setMeshAngle NOTIFY meshAngleChanged)
    Q_PROPERTY(QList<QColor> rowColors READ rowColors WRITE setRowColors NOTIFY rowColorsChanged REVISION(6, 3))

public:
    explicit QBar3DSeries(QObject *parent = nullptr);
    explicit QBar3DSeries(QBarDataProxy *dataProxy, QObject *parent = nullptr);
    virtual ~QBar3DSeries();

    void setDataProxy(QBarDataProxy *proxy);
    QBarDataProxy *dataProxy() const;

    void setSelectedBar(const QPoint &position);
    QPoint selectedBar() const;
    static QPoint invalidSelectionPosition();

    void setMeshAngle(float angle);
    float meshAngle() const;

    QList<QColor> rowColors() const;
    void setRowColors(const QList<QColor> &colors);

Q_SIGNALS:
    void dataProxyChanged(QBarDataProxy *proxy);
    void selectedBarChanged(const QPoint &position);
    void meshAngleChanged(float angle);
    Q_REVISION(6, 3) void rowColorsChanged(const QList<QColor> &rowcolors);

protected:
    QBar3DSeriesPrivate *dptr();
    const QBar3DSeriesPrivate *dptrc() const;

private:
    Q_DISABLE_COPY(QBar3DSeries)

    friend class Bars3DController;
};

QT_END_NAMESPACE

#endif
