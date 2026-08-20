// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>
#include <QtQml/qqmlengine.h>

#include <QtQuick3DPhysics/qtquick3dphysicsglobal.h>

QT_BEGIN_NAMESPACE

class QtQuick3DPhysicsPlugin : public QQmlEngineExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    QtQuick3DPhysicsPlugin(QObject *parent = nullptr) : QQmlEngineExtensionPlugin(parent)
    {
        volatile auto registration = &qml_register_types_QtQuick3D_Physics;
        Q_UNUSED(registration);
    }
};

QT_END_NAMESPACE

#include "plugin.moc"
