// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef WAYLANDTEXTURESHARINGEXTENSIONPLUGIN_H
#define WAYLANDTEXTURESHARINGEXTENSIONPLUGIN_H

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

#include <QtQml/qqmlregistration.h>
#include "QtWaylandCompositor/private/qwltexturesharingextension_p.h"
#include <QtWaylandCompositor/private/qwayland-server-qt-texture-sharing-unstable-v1.h>

QT_BEGIN_NAMESPACE
struct QWaylandTextureSharingExtensionQuickExtensionForeign
{
    Q_GADGET
    QML_FOREIGN(QWaylandTextureSharingExtensionQuickExtension);
    QML_NAMED_ELEMENT(TextureSharingExtension);
    QML_ADDED_IN_VERSION(1, 0);
};
struct QWaylandTextureSharingExtensionQuickExtensionBaseForeign
{
    Q_GADGET
    QML_FOREIGN(QWaylandCompositorExtensionTemplate<QWaylandTextureSharingExtension>);
    QML_ANONYMOUS;
};
struct QWaylandTextureSharingExtensionQuickExtensionBase2Foreign
{
    Q_GADGET
    QML_FOREIGN(QtWaylandServer::zqt_texture_sharing_v1);
    QML_ANONYMOUS;
};
QT_END_NAMESPACE

#endif // WAYLANDTEXTURESHARINGEXTENSIONPLUGIN_H
