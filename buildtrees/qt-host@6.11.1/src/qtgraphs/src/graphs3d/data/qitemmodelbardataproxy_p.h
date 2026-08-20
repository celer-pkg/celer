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

#ifndef QITEMMODELBARDATAPROXY_P_H
#define QITEMMODELBARDATAPROXY_P_H

#include "qbardataproxy_p.h"
#include "qitemmodelbardataproxy.h"

QT_BEGIN_NAMESPACE

class BarItemModelHandler;

class QItemModelBarDataProxyPrivate : public QBarDataProxyPrivate
{
    Q_DECLARE_PUBLIC(QItemModelBarDataProxy)

public:
    QItemModelBarDataProxyPrivate(QItemModelBarDataProxy *q);
    ~QItemModelBarDataProxyPrivate() override;

    void connectItemModelHandler();

private:
    BarItemModelHandler *m_itemModelHandler;

    QString m_rowRole;
    QString m_columnRole;
    QString m_valueRole;
    QString m_rotationRole;

    // For row/column items, sort items into these categories. Other categories
    // are ignored.
    QStringList m_rowCategories;
    QStringList m_columnCategories;

    bool m_useModelCategories;
    bool m_autoRowCategories;
    bool m_autoColumnCategories;

    QRegularExpression m_rowRolePattern;
    QRegularExpression m_columnRolePattern;
    QRegularExpression m_valueRolePattern;
    QRegularExpression m_rotationRolePattern;

    QString m_rowRoleReplace;
    QString m_columnRoleReplace;
    QString m_valueRoleReplace;
    QString m_rotationRoleReplace;

    QItemModelBarDataProxy::MultiMatchBehavior m_multiMatchBehavior;

    friend class BarItemModelHandler;
};

QT_END_NAMESPACE

#endif
