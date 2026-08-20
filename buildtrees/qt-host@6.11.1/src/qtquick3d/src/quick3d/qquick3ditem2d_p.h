// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGITEM2D_H
#define QSSGITEM2D_H

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

#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQuick3D/private/qquick3dscenemanager_p.h>
#include <QtQuick/private/qquickitemchangelistener_p.h>
#include <QtQuick/QSGNode>
#include <QtCore/QPointer>
#include <QtCore/QVector>

QT_BEGIN_NAMESPACE

class QSGLayer;
class QSGRootNode;
class QQuickRootItem;

class Q_AUTOTEST_EXPORT QQuick3DItem2D : public QQuick3DNode, public QQuickItemChangeListener
{
    Q_OBJECT
public:
    explicit QQuick3DItem2D(QQuickItem* item, QQuick3DNode *parent = nullptr);
    ~QQuick3DItem2D() override;

    void addChildItem(QQuickItem *item);
    void removeChildItem(QQuickItem *item);
    QQuickItem *contentItem() const;
    void itemDestroyed(QQuickItem *item) override;

private Q_SLOTS:
    void updatePicking();
    void derefWindow(QObject *win);

Q_SIGNALS:
    void allChildrenRemoved();

protected:
    void preSync() override;

private:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;
    void markAllDirty() override;

    QVector<QQuickItem *> m_sourceItems;
    QQuickWindow *m_window = nullptr;
    QQuickItem *m_contentItem = nullptr;
    bool m_pickingDirty = true;
};

QT_END_NAMESPACE

#endif // QSSGITEM2D_H
