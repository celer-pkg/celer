// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
#ifndef Q3DBARSWIDGETITEM_P_H
#define Q3DBARSWIDGETITEM_P_H
#include <QtGraphsWidgets/q3dbarswidgetitem.h>
#include <private/q3dgraphswidgetitem_p.h>

QT_BEGIN_NAMESPACE

class Q_GRAPHSWIDGETS_EXPORT Q3DBarsWidgetItemPrivate : public Q3DGraphsWidgetItemPrivate
{
    Q_DECLARE_PUBLIC(Q3DBarsWidgetItem)
    Q_DISABLE_COPY_MOVE(Q3DBarsWidgetItemPrivate)

public:
    Q3DBarsWidgetItemPrivate() = default;
};

QT_END_NAMESPACE

#endif // Q3DBARSWIDGETITEM_P_H
