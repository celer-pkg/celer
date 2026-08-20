// Copyright (C) 2022 The Qt Company Ltd.
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

#ifndef LOOKATNODE_H
#define LOOKATNODE_H

#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQuick3DHelpers/qtquick3dhelpersexports.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3DHELPERS_EXPORT LookAtNode : public QQuick3DNode
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DNode *target READ target WRITE setTarget NOTIFY targetChanged)
    QML_NAMED_ELEMENT(LookAtNode)
    QML_ADDED_IN_VERSION(6, 4)

public:
    LookAtNode();
    ~LookAtNode() override;

    QQuick3DNode *target() const;

public Q_SLOTS:
    void setTarget(QQuick3DNode *node);

Q_SIGNALS:
    void targetChanged();

private Q_SLOTS:
    void updateLookAt();

private:
    QQuick3DNode *m_target = nullptr;
};

QT_END_NAMESPACE

#endif
