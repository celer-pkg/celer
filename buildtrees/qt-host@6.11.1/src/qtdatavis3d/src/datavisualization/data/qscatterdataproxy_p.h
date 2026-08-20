// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QSCATTERDATAPROXY_P_H
#define QSCATTERDATAPROXY_P_H

#include "qscatterdataproxy.h"
#include "qabstractdataproxy_p.h"
#include "qscatterdataitem.h"

QT_BEGIN_NAMESPACE

class QAbstract3DAxis;

class QScatterDataProxyPrivate : public QAbstractDataProxyPrivate
{
    Q_OBJECT
public:
    QScatterDataProxyPrivate(QScatterDataProxy *q);
    virtual ~QScatterDataProxyPrivate();

    void resetArray(QScatterDataArray *newArray);
    void setItem(int index, const QScatterDataItem &item);
    void setItems(int index, const QScatterDataArray &items);
    int addItem(const QScatterDataItem &item);
    int addItems(const QScatterDataArray &items);
    void insertItem(int index, const QScatterDataItem &item);
    void insertItems(int index, const QScatterDataArray &items);
    void removeItems(int index, int removeCount);
    void limitValues(QVector3D &minValues, QVector3D &maxValues, QAbstract3DAxis *axisX,
                     QAbstract3DAxis *axisY, QAbstract3DAxis *axisZ) const;
    bool isValidValue(float axisValue, float value, QAbstract3DAxis *axis) const;

    void setSeries(QAbstract3DSeries *series) override;
private:
    QScatterDataProxy *qptr();
    QScatterDataArray *m_dataArray;

    friend class QScatterDataProxy;
};

QT_END_NAMESPACE

#endif
