// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QSURFACEDATAPROXY_H
#define QTGRAPHS_QSURFACEDATAPROXY_H

#include <QtGraphs/qabstractdataproxy.h>
#include <QtGraphs/qsurfacedataitem.h>

QT_BEGIN_NAMESPACE

class QSurfaceDataProxyPrivate;
class QSurface3DSeries;

using QSurfaceDataRow = QList<QSurfaceDataItem>;
using QSurfaceDataArray = QList<QSurfaceDataRow>;

class Q_GRAPHS_EXPORT QSurfaceDataProxy : public QAbstractDataProxy
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSurfaceDataProxy)
    Q_PROPERTY(qsizetype rowCount READ rowCount NOTIFY rowCountChanged FINAL)
    Q_PROPERTY(qsizetype columnCount READ columnCount NOTIFY columnCountChanged FINAL)
    Q_PROPERTY(QSurface3DSeries *series READ series NOTIFY seriesChanged FINAL)
    QML_NAMED_ELEMENT(SurfaceDataProxy)
    QML_UNCREATABLE("")

public:
    explicit QSurfaceDataProxy(QObject *parent = nullptr);
    ~QSurfaceDataProxy() override;

    QSurface3DSeries *series() const;
    qsizetype rowCount() const;
    qsizetype columnCount() const;
    const QSurfaceDataItem &itemAt(qsizetype rowIndex, qsizetype columnIndex) const;
    const QSurfaceDataItem &itemAt(QPoint position) const;

    void resetArray();
    void resetArray(QSurfaceDataArray newArray);

    void setRow(qsizetype rowIndex, QSurfaceDataRow row);
    void setRows(qsizetype rowIndex, QSurfaceDataArray rows);

    void setItem(qsizetype rowIndex, qsizetype columnIndex, QSurfaceDataItem item);
    void setItem(QPoint position, QSurfaceDataItem item);

    qsizetype addRow(QSurfaceDataRow row);
    qsizetype addRows(QSurfaceDataArray rows);

    void insertRow(qsizetype rowIndex, QSurfaceDataRow row);
    void insertRows(qsizetype rowIndex, QSurfaceDataArray rows);

    void removeRows(qsizetype rowIndex, qsizetype removeCount);

Q_SIGNALS:
    void arrayReset();
    void rowsAdded(qsizetype startIndex, qsizetype count);
    void rowsChanged(qsizetype startIndex, qsizetype count);
    void rowsRemoved(qsizetype startIndex, qsizetype count);
    void rowsInserted(qsizetype startIndex, qsizetype count);
    void itemChanged(qsizetype rowIndex, qsizetype columnIndex);

    void rowCountChanged(qsizetype count);
    void columnCountChanged(qsizetype count);
    void seriesChanged(QSurface3DSeries *series);

protected:
    explicit QSurfaceDataProxy(QSurfaceDataProxyPrivate &d, QObject *parent = nullptr);

private:
    Q_DISABLE_COPY(QSurfaceDataProxy)

    friend class QQuickGraphsSurface;
};

QT_END_NAMESPACE

#endif
