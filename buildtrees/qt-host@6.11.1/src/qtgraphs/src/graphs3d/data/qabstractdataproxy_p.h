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

#ifndef QABSTRACTDATAPROXY_P_H
#define QABSTRACTDATAPROXY_P_H

#include <QtCore/private/qobject_p.h>
#include "qabstractdataproxy.h"

QT_BEGIN_NAMESPACE

class QAbstract3DSeries;

class QAbstractDataProxyPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAbstractDataProxy)

public:
    QAbstractDataProxyPrivate(QAbstractDataProxy::DataType type);
    ~QAbstractDataProxyPrivate() override;

    QAbstract3DSeries *series() const { return m_series; }
    virtual void setSeries(QAbstract3DSeries *series);

protected:
    QAbstractDataProxy::DataType m_type;
    QAbstract3DSeries *m_series;
};

QT_END_NAMESPACE

#endif
