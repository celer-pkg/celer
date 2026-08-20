// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQUICKICON_P_P_H
#define QQUICKICON_P_P_H

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

#include <QtCore/qshareddata.h>
#include <QtCore/qstring.h>
#include <QtCore/qurl.h>
#include <QtGui/qcolor.h>
#include <QtQuickTemplates2/qtquicktemplates2exports.h>

QT_BEGIN_NAMESPACE

class QQuickIcon;

class Q_QUICKTEMPLATES2_EXPORT QQuickIconPrivate : public QSharedData
{
public:
    static bool isResolved(const QQuickIcon &icon, int resolveMask);

    // This is based on QFont's resolve_mask.
    enum ResolveProperties {
        NameResolved = 0x0001,
        SourceResolved = 0x0002,
        WidthResolved = 0x0004,
        HeightResolved = 0x0008,
        ColorResolved = 0x0010,
        CacheResolved = 0x0020,
        AllPropertiesResolved = 0x1ffff
    };
    int resolveMask = 0;

    QString name;
    QUrl source;
    QUrl resolvedSource;
    int width = 0;
    int height = 0;
    QColor color = Qt::transparent;
    bool cache = true;
};

QT_END_NAMESPACE

#endif // QQUICKICON_P_P_H
