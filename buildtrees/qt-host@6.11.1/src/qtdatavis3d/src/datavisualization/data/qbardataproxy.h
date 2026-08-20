// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QBARDATAPROXY_H
#define QTDATAVIS3D_QBARDATAPROXY_H

#include <QtDataVisualization/qabstractdataproxy.h>
#include <QtDataVisualization/qbardataitem.h>
#include <QtCore/QList>
#include <QtCore/QStringList>

Q_MOC_INCLUDE(<QtDataVisualization/qbar3dseries.h>)

QT_BEGIN_NAMESPACE

class QBarDataProxyPrivate;
class QBar3DSeries;

typedef QList<QBarDataItem> QBarDataRow;
typedef QList<QBarDataRow *> QBarDataArray;

class Q_DATAVISUALIZATION_EXPORT QBarDataProxy : public QAbstractDataProxy
{
    Q_OBJECT

    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(QStringList rowLabels READ rowLabels WRITE setRowLabels NOTIFY rowLabelsChanged)
    Q_PROPERTY(QStringList columnLabels READ columnLabels WRITE setColumnLabels NOTIFY columnLabelsChanged)
    Q_PROPERTY(QBar3DSeries *series READ series NOTIFY seriesChanged)
public:
    explicit QBarDataProxy(QObject *parent = nullptr);
    virtual ~QBarDataProxy();

    QBar3DSeries *series() const;
    int rowCount() const;

    QStringList rowLabels() const;
    void setRowLabels(const QStringList &labels);
    QStringList columnLabels() const;
    void setColumnLabels(const QStringList &labels);

    const QBarDataArray *array() const;
    const QBarDataRow *rowAt(int rowIndex) const;
    const QBarDataItem *itemAt(int rowIndex, int columnIndex) const;
    const QBarDataItem *itemAt(const QPoint &position) const;

    void resetArray();
    void resetArray(QBarDataArray *newArray);
    void resetArray(QBarDataArray *newArray, const QStringList &rowLabels,
                    const QStringList &columnLabels);

    void setRow(int rowIndex, QBarDataRow *row);
    void setRow(int rowIndex, QBarDataRow *row, const QString &label);
    void setRows(int rowIndex, const QBarDataArray &rows);
    void setRows(int rowIndex, const QBarDataArray &rows, const QStringList &labels);

    void setItem(int rowIndex, int columnIndex, const QBarDataItem &item);
    void setItem(const QPoint &position, const QBarDataItem &item);

    int addRow(QBarDataRow *row);
    int addRow(QBarDataRow *row, const QString &label);
    int addRows(const QBarDataArray &rows);
    int addRows(const QBarDataArray &rows, const QStringList &labels);

    void insertRow(int rowIndex, QBarDataRow *row);
    void insertRow(int rowIndex, QBarDataRow *row, const QString &label);
    void insertRows(int rowIndex, const QBarDataArray &rows);
    void insertRows(int rowIndex, const QBarDataArray &rows, const QStringList &labels);

    void removeRows(int rowIndex, int removeCount, bool removeLabels = true);

Q_SIGNALS:
    void arrayReset();
    void rowsAdded(int startIndex, int count);
    void rowsChanged(int startIndex, int count);
    void rowsRemoved(int startIndex, int count);
    void rowsInserted(int startIndex, int count);
    void itemChanged(int rowIndex, int columnIndex);

    void rowCountChanged(int count);
    void rowLabelsChanged();
    void columnLabelsChanged();
    void seriesChanged(QBar3DSeries *series);

protected:
    explicit QBarDataProxy(QBarDataProxyPrivate *d, QObject *parent = nullptr);
    QBarDataProxyPrivate *dptr();
    const QBarDataProxyPrivate *dptrc() const;

private:
    Q_DISABLE_COPY(QBarDataProxy)

    friend class Bars3DController;
};

QT_END_NAMESPACE

#endif
