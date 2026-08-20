// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qabstractopenxrgraphics_p.h"

QT_BEGIN_NAMESPACE

QAbstractOpenXRGraphics::QAbstractOpenXRGraphics()
{

}

void QAbstractOpenXRGraphics::setupWindow(QQuickWindow *)
{

}

bool QAbstractOpenXRGraphics::hasExtension(const QVector<XrExtensionProperties> &extensions, const char *extensionName)
{
    for (const auto &extension : extensions) {
        if (!strcmp(extensionName, extension.extensionName)) {
            return true;
        }
    }
    return false;
}

QT_END_NAMESPACE
