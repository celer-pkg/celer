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

#ifndef QSCATTER3DSERIES_P_H
#define QSCATTER3DSERIES_P_H

#include "qscatter3dseries.h"
#include "qabstract3dseries_p.h"

QT_BEGIN_NAMESPACE

class QScatter3DSeriesPrivate : public QAbstract3DSeriesPrivate
{
    Q_OBJECT
public:
    QScatter3DSeriesPrivate(QScatter3DSeries *q);
    virtual ~QScatter3DSeriesPrivate();

    void setDataProxy(QAbstractDataProxy *proxy) override;
    void connectControllerAndProxy(Abstract3DController *newController) override;
    void createItemLabel() override;

    void setSelectedItem(int index);
    void setItemSize(float size);

private:
    QScatter3DSeries *qptr();
    int m_selectedItem;
    float m_itemSize;

private:
    friend class QScatter3DSeries;
};

QT_END_NAMESPACE

#endif
