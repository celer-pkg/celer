// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "qquickeventreplayservice.h"
#include "qquickeventreplayservicefactory.h"

QT_BEGIN_NAMESPACE

QQmlDebugService *QQuickEventReplayServiceFactory::create(const QString &key)
{
    if (key == QQuickEventReplayServiceImpl::s_key)
        return new QQuickEventReplayServiceImpl(this);
    return nullptr;
}

QT_END_NAMESPACE

#include "moc_qquickeventreplayservicefactory.cpp"
