// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXRRUNTIMEINFO_P_H
#define QQUICK3DXRRUNTIMEINFO_P_H

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

#include <QtQuick3DXr/qtquick3dxrglobal.h>
#include <QtCore/qobject.h>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QQuick3DXrManager;

class Q_QUICK3DXR_EXPORT QQuick3DXrRuntimeInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList enabledExtensions READ enabledExtensions CONSTANT)
    Q_PROPERTY(QString runtimeName READ runtimeName CONSTANT)
    Q_PROPERTY(QString runtimeVersion READ runtimeVersion CONSTANT)
    Q_PROPERTY(QString graphicsApiName READ graphicsApiName CONSTANT)

    QML_NAMED_ELEMENT(XrRuntimeInfo)
    QML_UNCREATABLE("Created by XrView")
    QML_ADDED_IN_VERSION(6, 8)

public:
    QQuick3DXrRuntimeInfo(QQuick3DXrManager *manager, QObject *parent = nullptr);
    QStringList enabledExtensions() const;
    QString runtimeName() const;
    QString runtimeVersion() const;
    QString graphicsApiName() const;

private:
    QPointer<QQuick3DXrManager> m_xrmanager;
};

QT_END_NAMESPACE

#endif // QQUICK3DXRRUNTIMEINFO_P_H
