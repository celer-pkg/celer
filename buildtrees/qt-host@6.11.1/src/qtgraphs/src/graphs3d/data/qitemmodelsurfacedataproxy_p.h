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

#ifndef QITEMMODELSURFACEDATAPROXY_P_H
#define QITEMMODELSURFACEDATAPROXY_P_H

#include "qitemmodelsurfacedataproxy.h"
#include "qsurfacedataproxy_p.h"

QT_BEGIN_NAMESPACE

class SurfaceItemModelHandler;

class QItemModelSurfaceDataProxyPrivate : public QSurfaceDataProxyPrivate
{
    Q_DECLARE_PUBLIC(QItemModelSurfaceDataProxy)

public:
    QItemModelSurfaceDataProxyPrivate(QItemModelSurfaceDataProxy *q);
    ~QItemModelSurfaceDataProxyPrivate() override;

    void connectItemModelHandler();

private:
    SurfaceItemModelHandler *m_itemModelHandler;

    QString m_rowRole;
    QString m_columnRole;
    QString m_xPosRole;
    QString m_yPosRole;
    QString m_zPosRole;

    // For row/column items, sort items into these categories. Other categories
    // are ignored.
    QStringList m_rowCategories;
    QStringList m_columnCategories;

    bool m_useModelCategories;
    bool m_autoRowCategories;
    bool m_autoColumnCategories;

    QRegularExpression m_rowRolePattern;
    QRegularExpression m_columnRolePattern;
    QRegularExpression m_xPosRolePattern;
    QRegularExpression m_yPosRolePattern;
    QRegularExpression m_zPosRolePattern;

    QString m_rowRoleReplace;
    QString m_columnRoleReplace;
    QString m_xPosRoleReplace;
    QString m_yPosRoleReplace;
    QString m_zPosRoleReplace;

    QItemModelSurfaceDataProxy::MultiMatchBehavior m_multiMatchBehavior;

    friend class SurfaceItemModelHandler;
};

QT_END_NAMESPACE

#endif
