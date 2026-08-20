// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtQml/qqml.h>
#include <QtQml/qqmlextensionplugin.h>
#include <QtQuickControls2/private/qquickstyleplugin_p.h>

#if defined(Q_OS_MACOS)
#  include "qquickmacfocusframe.h"
#elif defined(Q_OS_WINDOWS)
#  include "qquickwindowsfocusframe.h"
#endif

QT_BEGIN_NAMESPACE

extern void qml_register_types_QtQuick_NativeStyle();
Q_GHS_KEEP_REFERENCE(qml_register_types_QtQuick_NativeStyle);

namespace QQC2 {

class QtQuickControls2NativeStylePlugin : public QQuickStylePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    QtQuickControls2NativeStylePlugin(QObject *parent = nullptr);
    ~QtQuickControls2NativeStylePlugin() override = default;

    void initializeEngine(QQmlEngine *, const char * /*uri*/) override;

    void initializeTheme(QQuickTheme *) override { }
    QString name() const override { return QStringLiteral("NativeStyle"); }
};

QtQuickControls2NativeStylePlugin::QtQuickControls2NativeStylePlugin(QObject *parent):
    QQuickStylePlugin(parent)
{
    volatile auto registration = &qml_register_types_QtQuick_NativeStyle;
    Q_UNUSED(registration);
}

static std::unique_ptr<QQuickFocusFrame> g_focusFrame;

void QtQuickControls2NativeStylePlugin::initializeEngine(QQmlEngine *, const char *)
{
    if (g_focusFrame)
        return;

#if defined(Q_OS_MACOS)
    g_focusFrame = std::make_unique<QQuickMacFocusFrame>();
#elif defined(Q_OS_WIN)
    g_focusFrame = std::make_unique<QQuickWindowsFocusFrame>();
#endif
    if (!g_focusFrame)
        return;

    qAddPostRoutine([] {
        g_focusFrame.reset();
    });
}

} // namespace QQC2

QT_END_NAMESPACE

#include "qtquickcontrols2nativestyleplugin.moc"
