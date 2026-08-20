// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXINPUTMANAGER_P_H
#define QQUICK3DXINPUTMANAGER_P_H

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

#include <QtQuick3DXr/private/qtquick3dxrglobal_p.h>

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QQuick3DGeometry;

class QQuick3DXrHandInput;
class QQuick3DXrController;
class QQuick3DXrInputManagerPrivate;

class QQuick3DXrInputManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QQuick3DXrInputManager)

public:
    using Hand = QtQuick3DXr::Hand;
    using HandPoseSpace = QtQuick3DXr::HandPoseSpace;

    static QQuick3DXrInputManager *instance();

    QQuick3DXrHandInput *leftHandInput() const;
    QQuick3DXrHandInput *rightHandInput() const;

    void registerController(QQuick3DXrController *controller);
    void unregisterController(QQuick3DXrController *controller);

    bool isValid() const;

private:
    explicit QQuick3DXrInputManager(QObject *parent = nullptr);
    ~QQuick3DXrInputManager() override;

    std::unique_ptr<QQuick3DXrInputManagerPrivate> d_ptr;
};


QT_END_NAMESPACE

#endif // QQUICK3DXINPUTMANAGER_P_H
