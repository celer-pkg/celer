// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtGraphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QBARDATAPROXY_P_H
#define QBARDATAPROXY_P_H

#include "qabstractdataproxy_p.h"
#include "qbardataproxy.h"

QT_BEGIN_NAMESPACE

class QBarDataProxyPrivate : public QAbstractDataProxyPrivate
{
    Q_DECLARE_PUBLIC(QBarDataProxy)

public:
    QBarDataProxyPrivate();
    ~QBarDataProxyPrivate() override;

    void resetArray(QBarDataArray &&newArray, QStringList &&rowLabels, QStringList &&columnLabels);
    void setRow(qsizetype rowIndex, QBarDataRow &&row, QString &&label);
    void setRows(qsizetype rowIndex, QBarDataArray &&rows, QStringList &&labels);
    void setItem(qsizetype rowIndex, qsizetype columnIndex, QBarDataItem &&item);
    qsizetype addRow(QBarDataRow &&row, QString &&label);
    qsizetype addRows(QBarDataArray &&rows, QStringList &&labels);
    void insertRow(qsizetype rowIndex, QBarDataRow &&row, QString &&label);
    void insertRows(qsizetype rowIndex, QBarDataArray &&rows, QStringList &&labels);
    void removeRows(qsizetype rowIndex, qsizetype removeCount, bool removeLabels);

    QPair<float, float> limitValues(qsizetype startRow,
                                    qsizetype startColumn,
                                    qsizetype rowCount,
                                    qsizetype columnCount) const;

    void setSeries(QAbstract3DSeries *series) override;
};

QT_END_NAMESPACE

#endif
