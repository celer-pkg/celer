// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dprofileradapterfactory.h"
#include "qquick3dprofileradapter.h"
#include <private/qqmldebugconnector_p.h>
#include <private/qqmldebugserviceinterfaces_p.h>

QT_BEGIN_NAMESPACE

QQmlAbstractProfilerAdapter *QQuick3DProfilerAdapterFactory::create(const QString &key)
{
    if (key != QLatin1String("QQuick3DProfilerAdapter"))
        return nullptr;
    return new QQuick3DProfilerAdapter(this);
}

QT_END_NAMESPACE

#include "moc_qquick3dprofileradapterfactory.cpp"
