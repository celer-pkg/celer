// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquickbasictheme_p.h"

#include <QtQuickControls2/private/qquickstyleplugin_p.h>

QT_BEGIN_NAMESPACE

extern void qml_register_types_QtQuick_Controls_Basic();
Q_GHS_KEEP_REFERENCE(qml_register_types_QtQuick_Controls_Basic);

class QtQuickControls2BasicStylePlugin: public QQuickStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    QtQuickControls2BasicStylePlugin(QObject *parent = nullptr);

    QString name() const override;
    void initializeTheme(QQuickTheme *theme) override;
    void updateTheme() override;

    QQuickBasicTheme theme;
};

QtQuickControls2BasicStylePlugin::QtQuickControls2BasicStylePlugin(QObject *parent) : QQuickStylePlugin(parent)
{
    volatile auto registration = &qml_register_types_QtQuick_Controls_Basic;
    Q_UNUSED(registration);
}

QString QtQuickControls2BasicStylePlugin::name() const
{
    return QStringLiteral("Basic");
}

void QtQuickControls2BasicStylePlugin::initializeTheme(QQuickTheme *theme)
{
    this->theme.initialize(theme);
}

void QtQuickControls2BasicStylePlugin::updateTheme()
{
    this->theme.updateTheme();
}

QT_END_NAMESPACE

#include "qtquickcontrols2basicstyleplugin.moc"
