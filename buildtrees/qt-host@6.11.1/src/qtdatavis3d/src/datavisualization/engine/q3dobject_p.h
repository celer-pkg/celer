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

#ifndef Q3DOBJECT_P_H
#define Q3DOBJECT_P_H

#include "datavisualizationglobal_p.h"

QT_BEGIN_NAMESPACE

class Q3DObject;
class Q3DScene;

class Q3DObjectPrivate
{
public:
    Q3DObjectPrivate(Q3DObject *q);
    ~Q3DObjectPrivate();

public:
    Q3DObject *q_ptr;
    QVector3D m_position;
    bool m_isDirty;
};

QT_END_NAMESPACE

#endif
