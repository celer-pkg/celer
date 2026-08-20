// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QBARDATAPROXY_H
#define QTGRAPHS_QBARDATAPROXY_H

#include <QtCore/qlist.h>
#include <QtCore/qstringlist.h>
#include <QtGraphs/qabstractdataproxy.h>
#include <QtGraphs/qbardataitem.h>

QT_BEGIN_NAMESPACE

class QBarDataProxyPrivate;
class QBar3DSeries;

using QBarDataRow = QList<QBarDataItem>;
using QBarDataArray = QList<QBarDataRow>;

class Q_GRAPHS_EXPORT QBarDataProxy : public QAbstractDataProxy
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QBarDataProxy)
    Q_PROPERTY(qsizetype rowCount READ rowCount NOTIFY rowCountChanged FINAL)
    Q_PROPERTY(qsizetype colCount READ colCount NOTIFY colCountChanged FINAL)
    Q_PROPERTY(QBar3DSeries *series READ series NOTIFY seriesChanged FINAL)
    QML_NAMED_ELEMENT(BarDataProxy)
    QML_UNCREATABLE("")

public:
    explicit QBarDataProxy(QObject *parent = nullptr);
    ~QBarDataProxy() override;

    enum class RemoveLabels { No, Yes };
    Q_ENUM(RemoveLabels)

    QBar3DSeries *series() const;
    qsizetype rowCount() const;
    qsizetype colCount() const;

    const QBarDataRow &rowAt(qsizetype rowIndex) const;
    const QBarDataItem &itemAt(qsizetype rowIndex, qsizetype columnIndex) const;
    const QBarDataItem &itemAt(QPoint position) const;

    void resetArray();
    void resetArray(QBarDataArray newArray);
    void resetArray(QBarDataArray newArray, QStringList rowLabels, QStringList columnLabels);

    void setRow(qsizetype rowIndex, QBarDataRow row);
    void setRow(qsizetype rowIndex, QBarDataRow row, QString label);
    void setRows(qsizetype rowIndex, QBarDataArray rows);
    void setRows(qsizetype rowIndex, QBarDataArray rows, QStringList labels);

    void setItem(qsizetype rowIndex, qsizetype columnIndex, QBarDataItem item);
    void setItem(QPoint position, QBarDataItem item);

    qsizetype addRow(QBarDataRow row);
    qsizetype addRow(QBarDataRow row, QString label);
    qsizetype addRows(QBarDataArray rows);
    qsizetype addRows(QBarDataArray rows, QStringList labels);

    void insertRow(qsizetype rowIndex, QBarDataRow row);
    void insertRow(qsizetype rowIndex, QBarDataRow row, QString label);
    void insertRows(qsizetype rowIndex, QBarDataArray rows);
    void insertRows(qsizetype rowIndex, QBarDataArray rows, QStringList labels);

    void removeRows(qsizetype rowIndex,
                    qsizetype removeCount,
                    RemoveLabels removeLabels = RemoveLabels::No);

Q_SIGNALS:
    void arrayReset();
    void rowsAdded(qsizetype startIndex, qsizetype count);
    void rowsChanged(qsizetype startIndex, qsizetype count);
    void rowsRemoved(qsizetype startIndex, qsizetype count);
    void rowsInserted(qsizetype startIndex, qsizetype count);
    void itemChanged(qsizetype rowIndex, qsizetype columnIndex);

    void rowCountChanged(qsizetype count);
    void colCountChanged(qsizetype count);
    void seriesChanged(QBar3DSeries *series);

protected:
    explicit QBarDataProxy(QBarDataProxyPrivate &d, QObject *parent = nullptr);

private:
    Q_DISABLE_COPY(QBarDataProxy)

    friend class QQuickGraphsBars;
};

QT_END_NAMESPACE

#endif
