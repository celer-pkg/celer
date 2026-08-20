// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Graphs API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QSCATTERSERIES_P_H
#define QSCATTERSERIES_P_H

#include <QtGraphs/qscatterseries.h>
#include <private/qxyseries_p.h>

QT_BEGIN_NAMESPACE

class QScatterSeriesPrivate : public QXYSeriesPrivate
{
public:
    QScatterSeriesPrivate();

private:
    Q_DECLARE_PUBLIC(QScatterSeries)
};

QT_END_NAMESPACE

#endif
