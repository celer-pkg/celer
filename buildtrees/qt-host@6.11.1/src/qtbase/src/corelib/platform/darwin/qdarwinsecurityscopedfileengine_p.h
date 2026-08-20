// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDARWINSECURITYSCOPEDFILEENGINE_H
#define QDARWINSECURITYSCOPEDFILEENGINE_H

//
//  W A R N I N G
//  -------------
//
// This file is part of the QPA API and is not meant to be used
// in applications. Usage of this API may make your code
// source and binary incompatible with future versions of Qt.
//
// We mean it.
//

#include <QtCore/qurl.h>

Q_FORWARD_DECLARE_OBJC_CLASS(NSURL);

QT_BEGIN_NAMESPACE

Q_CORE_EXPORT QUrl qt_apple_urlFromPossiblySecurityScopedURL(NSURL *url);

QT_END_NAMESPACE

#endif // QDARWINSECURITYSCOPEDFILEENGINE_H
