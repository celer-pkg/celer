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

#ifndef QSCATTERDATAPROXY_P_H
#define QSCATTERDATAPROXY_P_H

#include "qabstractdataproxy_p.h"
#include "qscatterdataitem.h"
#include "qscatterdataproxy.h"

QT_BEGIN_NAMESPACE

class QAbstract3DAxis;

class QScatterDataProxyPrivate : public QAbstractDataProxyPrivate
{
    Q_DECLARE_PUBLIC(QScatterDataProxy)

public:
    QScatterDataProxyPrivate();
    ~QScatterDataProxyPrivate() override;

    void resetArray(QScatterDataArray &&newArray);
    void resetScaleArray(QList<QVector3D> &&newArray);
    void setItem(qsizetype index, QScatterDataItem &&item);
    void setItems(qsizetype index, QScatterDataArray &&items);
    qsizetype addItem(QScatterDataItem &&item);
    qsizetype addItems(QScatterDataArray &&items);
    void insertItem(qsizetype index, QScatterDataItem &&item);
    void insertItems(qsizetype index, QScatterDataArray &&items);
    void removeItems(qsizetype index, qsizetype removeCount);
    void limitValues(QVector3D &minValues,
                     QVector3D &maxValues,
                     QAbstract3DAxis *axisX,
                     QAbstract3DAxis *axisY,
                     QAbstract3DAxis *axisZ) const;
    bool isValidValue(float axisValue, float value, QAbstract3DAxis *axis) const;

    void setSeries(QAbstract3DSeries *series) override;
};

QT_END_NAMESPACE

#endif
