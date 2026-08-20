// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QSCATTERDATAPROXY_H
#define QTGRAPHS_QSCATTERDATAPROXY_H

#include <QtGraphs/qabstractdataproxy.h>
#include <QtGraphs/qscatterdataitem.h>

QT_BEGIN_NAMESPACE

class QScatterDataProxyPrivate;
class QScatter3DSeries;

using QScatterDataArray = QList<QScatterDataItem>;

class Q_GRAPHS_EXPORT QScatterDataProxy : public QAbstractDataProxy
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QScatterDataProxy)
    Q_PROPERTY(qsizetype itemCount READ itemCount NOTIFY itemCountChanged FINAL)
    Q_PROPERTY(QScatter3DSeries *series READ series NOTIFY seriesChanged FINAL)
    QML_NAMED_ELEMENT(ScatterDataProxy)
    QML_UNCREATABLE("")

public:
    explicit QScatterDataProxy(QObject *parent = nullptr);
    ~QScatterDataProxy() override;

    QScatter3DSeries *series() const;
    qsizetype itemCount() const;
    const QScatterDataItem &itemAt(qsizetype index) const;
    QVector3D scaleAt(qsizetype index) const;

    void resetArray();
    void resetArray(QScatterDataArray newArray);
    void resetScaleArray(QList<QVector3D> newArray);

    void setItem(qsizetype index, QScatterDataItem item);
    void setItems(qsizetype index, QScatterDataArray items);

    qsizetype addItem(QScatterDataItem item);
    qsizetype addItem(QScatterDataItem &&item);
    qsizetype addItems(QScatterDataArray items);
    qsizetype addItems(QScatterDataArray &&items);

    void insertItem(qsizetype index, QScatterDataItem item);
    void insertItems(qsizetype index, QScatterDataArray items);

    void removeItems(qsizetype index, qsizetype removeCount);

Q_SIGNALS:
    void arrayReset();
    void scaleArrayReset();
    void itemsAdded(qsizetype startIndex, qsizetype count);
    void itemsChanged(qsizetype startIndex, qsizetype count);
    void itemsRemoved(qsizetype startIndex, qsizetype count);
    void itemsInserted(qsizetype startIndex, qsizetype count);

    void itemCountChanged(qsizetype count);
    void seriesChanged(QScatter3DSeries *series);

protected:
    explicit QScatterDataProxy(QScatterDataProxyPrivate &d, QObject *parent = nullptr);

private:
    Q_DISABLE_COPY(QScatterDataProxy)

    friend class QQuickGraphsScatter;
};

QT_END_NAMESPACE

#endif
