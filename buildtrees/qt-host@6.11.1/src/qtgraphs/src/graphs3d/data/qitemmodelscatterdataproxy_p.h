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

#ifndef QITEMMODELSCATTERDATAPROXY_P_H
#define QITEMMODELSCATTERDATAPROXY_P_H

#include "qitemmodelscatterdataproxy.h"
#include "qscatterdataproxy_p.h"

QT_BEGIN_NAMESPACE

class ScatterItemModelHandler;

class QItemModelScatterDataProxyPrivate : public QScatterDataProxyPrivate
{
    Q_DECLARE_PUBLIC(QItemModelScatterDataProxy)

public:
    QItemModelScatterDataProxyPrivate(QItemModelScatterDataProxy *q);
    ~QItemModelScatterDataProxyPrivate() override;

    void connectItemModelHandler();

private:
    ScatterItemModelHandler *m_itemModelHandler;
    QString m_xPosRole;
    QString m_yPosRole;
    QString m_zPosRole;
    QString m_rotationRole;
    QString m_scaleRole;

    QRegularExpression m_xPosRolePattern;
    QRegularExpression m_yPosRolePattern;
    QRegularExpression m_zPosRolePattern;
    QRegularExpression m_rotationRolePattern;
    QRegularExpression m_scaleRolePattern;

    QString m_xPosRoleReplace;
    QString m_yPosRoleReplace;
    QString m_zPosRoleReplace;
    QString m_rotationRoleReplace;
    QString m_scaleRoleReplace;
};

QT_END_NAMESPACE

#endif
