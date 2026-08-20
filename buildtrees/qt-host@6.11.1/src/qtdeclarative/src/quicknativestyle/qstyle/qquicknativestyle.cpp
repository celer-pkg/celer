// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquicknativestyle_p.h"

#include <QtCore/qapplicationstatic.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qthread.h>
#include <QtGui/qguiapplication.h>
#include <QtGui/qstylehints.h>

#if defined(Q_OS_MACOS)
#  include "qquickmacstyle_mac_p.h"
#elif defined(Q_OS_WINDOWS)
#  include "qquickwindowsxpstyle_p.h"
#endif

#include <memory>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(lcNativeStylePlugin, "qt.quick.plugins.nativestyle");

namespace QQC2 {

namespace {

// When we delete QStyle, it will free up its own internal resources. Especially
// on macOS, this means releasing a lot of NSViews and NSCells from the QMacStyle
// destructor. If we did this from ~QtQuickControls2NativeStylePlugin, it would
// happen when the plugin was unloaded from a Q_DESTRUCTOR_FUNCTION in QLibrary,
// which is very late in the tear-down process, and after qGuiApp has been set to
// nullptr, NSApplication has stopped running, and perhaps also other static platform
// variables (e.g in AppKit?) have been deleted. And to our best guess, this is also why
// we see a crash in AppKit from the destructor in QMacStyle. So for this reason, we
// delete QStyle from a post routine rather than from the destructor.
//
// Furthermore we need to ensure to recreate the QQuickFocusFrame when re-creating the qApplication

struct StyleSingleton
{
    StyleSingleton();
    ~StyleSingleton();
    QStyle *style() { return m_style.get(); }

private:
    std::unique_ptr<QStyle> m_style;
};

Q_APPLICATION_STATIC(StyleSingleton, styleSingleton);

StyleSingleton::StyleSingleton()
{
    qCDebug(lcNativeStylePlugin) << "Creating native style";

    // Enable commonstyle as a reference style while
    // the native styles are under development.
    if (qEnvironmentVariable("QQC2_COMMONSTYLE") == u"true")
        m_style = std::make_unique<QCommonStyle>();
    else if (const QString envStyle = qEnvironmentVariable("QQC2_STYLE"); !envStyle.isNull()) {
        if (envStyle == u"common")
            m_style = std::make_unique<QCommonStyle>();
#if defined(Q_OS_MACOS)
        else if (envStyle == u"mac")
            m_style.reset(QMacStyle::create());
#endif
#if defined(Q_OS_WINDOWS)
        else if (envStyle == u"windows")
            m_style = std::make_unique<QWindowsStyle>();
        else if (envStyle == u"windowsxp")
            m_style = std::make_unique<QWindowsXPStyle>();
#endif
    }

    if (!m_style) {
#if defined(Q_OS_MACOS)
        m_style.reset(QMacStyle::create());
#elif defined(Q_OS_WINDOWS)
        m_style = std::make_unique<QWindowsXPStyle>();
        if (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark)
            qobject_cast<QWindowsStyle *>(m_style.get())->refreshPalette();
#endif
    }

    if (!m_style)
        m_style = std::make_unique<QCommonStyle>();

    // The native style plugin is neither the current style or fallback style
    // during QQuickStylePlugin::registerTypes, so it's not given a chance to
    // initialize or update the theme. But since it's used as an implementation
    // detail of some of the other style plugins, it might need to know about
    // theme changes.
    Q_ASSERT(m_style->thread()->isMainThread());
    QObject::connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, m_style.get(),
                     [this] {
        m_style->handleThemeChange();
    });
}

StyleSingleton::~StyleSingleton()
{
    qCDebug(lcNativeStylePlugin) << "Destroying native style";

    m_style.reset();
}

}; // namespace

QStyle *style()
{
    return styleSingleton()->style();
}

} // namespace QQC2

QT_END_NAMESPACE
