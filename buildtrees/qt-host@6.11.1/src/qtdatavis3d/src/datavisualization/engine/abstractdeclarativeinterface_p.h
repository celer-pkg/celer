// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef ABSTRACTDECLARATIVEINTERFACE_P_H
#define ABSTRACTDECLARATIVEINTERFACE_P_H

#include <QtDataVisualization/qdatavisualizationglobal.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

QT_BEGIN_NAMESPACE

class Q_DATAVISUALIZATION_EXPORT AbstractDeclarativeInterface
{
protected:
    ~AbstractDeclarativeInterface();
public:
    virtual bool isReady() const = 0;
};

QT_END_NAMESPACE

#endif
