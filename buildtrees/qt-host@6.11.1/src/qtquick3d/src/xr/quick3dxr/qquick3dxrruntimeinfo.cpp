// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dxrruntimeinfo_p.h"
#include "qquick3dxrmanager_p.h"
#include <QtQuick/QQuickWindow>
#include <rhi/qrhi.h>

#if defined(Q_OS_VISIONOS)
#include "visionos/qquick3dxrmanager_visionos_p.h"
#else
#include "openxr/qquick3dxrmanager_openxr_p.h"
#endif

QT_BEGIN_NAMESPACE

/*!
    \qmltype XrRuntimeInfo
    \inherits Item
    \inqmlmodule QtQuick3D.Xr
    \brief Displays information about the XR runtime.

    This type provides information about the XR runtime, including enabled
    extensions, runtime name, version, graphics API name, and whether multi-view
    rendering is supported.

    \note This type is automatically created by an \l XrView, and it can not be
    manually created.
*/

QQuick3DXrRuntimeInfo::QQuick3DXrRuntimeInfo(QQuick3DXrManager *manager, QObject *parent)
    : QObject(parent),
      m_xrmanager(manager)
{
}

/*!
    \qmlproperty list<string> XrRuntimeInfo::enabledExtensions
    \brief A list of enabled XR extensions.
    \readonly

    This property holds a QStringList containing the names of the
    XR extensions that are currently enabled for the runtime.

    \note This list may vary depending on the runtime implementation and can be
    empty.
*/

QStringList QQuick3DXrRuntimeInfo::enabledExtensions() const
{
    QQuick3DXrManagerPrivate *manager = QQuick3DXrManagerPrivate::get(m_xrmanager);
    return manager ? manager->enabledExtensions() : QStringList{};
}

/*!
    \qmlproperty string XrRuntimeInfo::runtimeName
    \brief The name of the XR runtime.
    \readonly

    This property provides the human-readable name of the XR runtime being
    used.
*/

QString QQuick3DXrRuntimeInfo::runtimeName() const
{
    QQuick3DXrManagerPrivate *manager = QQuick3DXrManagerPrivate::get(m_xrmanager);
    return manager ? manager->runtimeName() : QString{};
}

/*!
    \qmlproperty string XrRuntimeInfo::runtimeVersion
    \brief The version of the XR runtime.
    \readonly

    This property holds the version string of the XR runtime
    (for example, "1.0.0").
*/

QString QQuick3DXrRuntimeInfo::runtimeVersion() const
{
    QQuick3DXrManagerPrivate *manager = QQuick3DXrManagerPrivate::get(m_xrmanager);
    return manager ? manager->runtimeVersion().toString() : QString{};
}

/*!
    \qmlproperty string XrRuntimeInfo::graphicsApiName
    \brief The name of the graphics API used by the XR runtime.
    \readonly

    This property holds the name of the graphics API (for example, "Vulkan") that the
    XR runtime is utilizing.
*/

QString QQuick3DXrRuntimeInfo::graphicsApiName() const
{
    // This matches what Qt Quick's GraphicsInfo would expose to QML, but that
    // does not provide a string. We have seen way too many switch statements
    // in JS for this. So, have a string property here and call it a day.
    if (m_xrmanager->isValid() && m_xrmanager->m_quickWindow) {
        QRhi *rhi = m_xrmanager->m_quickWindow->rhi();
        if (rhi)
            return QString::fromLatin1(rhi->backendName());
    }
    return QLatin1String("Unknown");
}

QT_END_NAMESPACE
