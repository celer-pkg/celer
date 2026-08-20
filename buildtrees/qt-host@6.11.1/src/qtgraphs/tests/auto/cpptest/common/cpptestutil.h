// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef CPPTESTUTIL_H
#define CPPTESTUTIL_H

#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformintegration.h>

QT_BEGIN_NAMESPACE

namespace CpptestUtil {

[[maybe_unused]] static inline bool isOpenGLSupported()
{
    return QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::OpenGL);
}

} // CpptestUtil namespace

QT_END_NAMESPACE

#endif

