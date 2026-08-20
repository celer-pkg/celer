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

#ifndef Q3DGRAPHSWIDGETITEM_P_H
#define Q3DGRAPHSWIDGETITEM_P_H

#include <QtGraphsWidgets/q3dgraphswidgetitem.h>
#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

class Q3DGraphsWidgetItemPrivate : public QObjectPrivate
{
public:
    Q3DGraphsWidgetItemPrivate() = default;

    void createGraph();
    void onWheel(QQuickWheelEvent *event);

    std::unique_ptr<QQuickGraphsItem> m_graphsItem;
    QQuickWidget *m_widget = nullptr;
    QString m_graphType;

private:
    Q_DISABLE_COPY_MOVE(Q3DGraphsWidgetItemPrivate)
    Q_DECLARE_PUBLIC(Q3DGraphsWidgetItem)
};

QT_END_NAMESPACE

#endif // Q3DGRAPHSWIDGETITEM_P_H
