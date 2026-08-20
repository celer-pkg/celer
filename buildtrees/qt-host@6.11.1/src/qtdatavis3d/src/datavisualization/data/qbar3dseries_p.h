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

#ifndef QBAR3DSERIES_P_H
#define QBAR3DSERIES_P_H

#include "qbar3dseries.h"
#include "qabstract3dseries_p.h"

QT_BEGIN_NAMESPACE

class QQuaternion;

class QBar3DSeriesPrivate : public QAbstract3DSeriesPrivate
{
    Q_OBJECT
public:
    QBar3DSeriesPrivate(QBar3DSeries *q);
    virtual ~QBar3DSeriesPrivate();

    void setDataProxy(QAbstractDataProxy *proxy) override;
    void connectControllerAndProxy(Abstract3DController *newController) override;
    void createItemLabel() override;

    void handleMeshRotationChanged(const QQuaternion &rotation);

    void setSelectedBar(const QPoint &position);

    void connectSignals();

    void setRowColors(const QList<QColor> &colors);

private:
    QBar3DSeries *qptr();

    QPoint m_selectedBar;

    QList<QColor> m_rowColors;

private:
    friend class QBar3DSeries;
};

QT_END_NAMESPACE

#endif
