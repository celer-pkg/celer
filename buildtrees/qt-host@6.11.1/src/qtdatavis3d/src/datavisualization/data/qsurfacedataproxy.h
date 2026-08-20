// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QSURFACEDATAPROXY_H
#define QTDATAVIS3D_QSURFACEDATAPROXY_H

#include <QtDataVisualization/qabstractdataproxy.h>
#include <QtDataVisualization/qsurfacedataitem.h>

Q_MOC_INCLUDE(<QtDataVisualization/qsurface3dseries.h>)

QT_BEGIN_NAMESPACE

class QSurfaceDataProxyPrivate;
class QSurface3DSeries;

typedef QList<QSurfaceDataItem> QSurfaceDataRow;
typedef QList<QSurfaceDataRow *> QSurfaceDataArray;

class Q_DATAVISUALIZATION_EXPORT QSurfaceDataProxy : public QAbstractDataProxy
{
    Q_OBJECT

    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(int columnCount READ columnCount NOTIFY columnCountChanged)
    Q_PROPERTY(QSurface3DSeries *series READ series NOTIFY seriesChanged)

public:
    explicit QSurfaceDataProxy(QObject *parent = nullptr);
    virtual ~QSurfaceDataProxy();

    QSurface3DSeries *series() const;
    int rowCount() const;
    int columnCount() const;
    const QSurfaceDataArray *array() const;
    const QSurfaceDataItem *itemAt(int rowIndex, int columnIndex) const;
    const QSurfaceDataItem *itemAt(const QPoint &position) const;

    void resetArray(QSurfaceDataArray *newArray);

    void setRow(int rowIndex, QSurfaceDataRow *row);
    void setRows(int rowIndex, const QSurfaceDataArray &rows);

    void setItem(int rowIndex, int columnIndex, const QSurfaceDataItem &item);
    void setItem(const QPoint &position, const QSurfaceDataItem &item);

    int addRow(QSurfaceDataRow *row);
    int addRows(const QSurfaceDataArray &rows);

    void insertRow(int rowIndex, QSurfaceDataRow *row);
    void insertRows(int rowIndex, const QSurfaceDataArray &rows);

    void removeRows(int rowIndex, int removeCount);

Q_SIGNALS:
    void arrayReset();
    void rowsAdded(int startIndex, int count);
    void rowsChanged(int startIndex, int count);
    void rowsRemoved(int startIndex, int count);
    void rowsInserted(int startIndex, int count);
    void itemChanged(int rowIndex, int columnIndex);

    void rowCountChanged(int count);
    void columnCountChanged(int count);
    void seriesChanged(QSurface3DSeries *series);

protected:
    explicit QSurfaceDataProxy(QSurfaceDataProxyPrivate *d, QObject *parent = nullptr);
    QSurfaceDataProxyPrivate *dptr();
    const QSurfaceDataProxyPrivate *dptrc() const;

private:
    Q_DISABLE_COPY(QSurfaceDataProxy)

    friend class Surface3DController;
};

QT_END_NAMESPACE

#endif
