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

#ifndef Q3DSURFACE_P_H
#define Q3DSURFACE_P_H

#include "surface3dcontroller_p.h"
#include "qabstract3dgraph_p.h"

QT_BEGIN_NAMESPACE

class Q3DSurface;

class Q3DSurfacePrivate : public QAbstract3DGraphPrivate
{
    Q_OBJECT
public:
    Q3DSurfacePrivate(Q3DSurface *q);
    ~Q3DSurfacePrivate();

    void handleAxisXChanged(QAbstract3DAxis *axis) override;
    void handleAxisYChanged(QAbstract3DAxis *axis) override;
    void handleAxisZChanged(QAbstract3DAxis *axis) override;

    Q3DSurface *qptr();

    Surface3DController *m_shared;
};

QT_END_NAMESPACE

#endif
