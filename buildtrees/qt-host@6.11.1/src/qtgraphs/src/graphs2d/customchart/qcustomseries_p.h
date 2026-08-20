// Copyright (C) 2025 The Qt Company Ltd.
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

#ifndef QCUSTOMSERIES_P_H
#define QCUSTOMSERIES_P_H

#include <QtGraphs/qcustomseries.h>
#include <private/qabstractseries_p.h>

QT_BEGIN_NAMESPACE

class QCustomSeriesData;

class QCustomSeriesPrivate : public QAbstractSeriesPrivate
{
public:
    static QCustomSeriesPrivate *get(QCustomSeries *item) { return item->d_func(); }
    static const QCustomSeriesPrivate *get(const QCustomSeries *item) { return item->d_func(); }

    QCustomSeriesPrivate();

protected:
    QQmlComponent *m_delegate = nullptr;
    QList<QCustomSeriesData *> m_dataItems;

private:
    Q_DECLARE_PUBLIC(QCustomSeries)
};

QT_END_NAMESPACE

#endif
