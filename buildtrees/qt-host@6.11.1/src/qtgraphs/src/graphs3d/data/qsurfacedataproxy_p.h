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

#ifndef QSURFACEDATAPROXY_P_H
#define QSURFACEDATAPROXY_P_H

#include "qabstractdataproxy_p.h"
#include "qsurfacedataproxy.h"

QT_BEGIN_NAMESPACE

class QAbstract3DAxis;

class QSurfaceDataProxyPrivate : public QAbstractDataProxyPrivate
{
    Q_DECLARE_PUBLIC(QSurfaceDataProxy)

public:
    QSurfaceDataProxyPrivate();
    ~QSurfaceDataProxyPrivate() override;

    void resetArray(QSurfaceDataArray &&newArray);
    void setRow(qsizetype rowIndex, QSurfaceDataRow &&row);
    void setRows(qsizetype rowIndex, QSurfaceDataArray &&rows);
    void setItem(qsizetype rowIndex, qsizetype columnIndex, QSurfaceDataItem &&item);
    qsizetype addRow(QSurfaceDataRow &&row);
    qsizetype addRows(QSurfaceDataArray &&rows);
    void insertRow(qsizetype rowIndex, QSurfaceDataRow &&row);
    void insertRows(qsizetype rowIndex, QSurfaceDataArray &&rows);
    void removeRows(qsizetype rowIndex, qsizetype removeCount);
    void limitValues(QVector3D &minValues,
                     QVector3D &maxValues,
                     QAbstract3DAxis *axisX,
                     QAbstract3DAxis *axisY,
                     QAbstract3DAxis *axisZ) const;
    bool isValidValue(float value, QAbstract3DAxis *axis) const;

    void setSeries(QAbstract3DSeries *series) override;
};

QT_END_NAMESPACE

#endif
