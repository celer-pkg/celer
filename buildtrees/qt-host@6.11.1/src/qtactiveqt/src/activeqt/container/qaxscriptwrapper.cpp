// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#include "qaxobject.h"

#include <QtAxServer/qaxfactory.h>

#include <qt_windows.h>

#include <QtCore/private/qcomptr_p.h>

QT_BEGIN_NAMESPACE

QAxBase *qax_create_object_wrapper(QObject *object)
{
    ComPtr<IDispatch> dispatch;
    QAxObject *wrapper = nullptr;
    qAxFactory()->createObjectWrapper(object, &dispatch);
    if (dispatch) {
        wrapper = new QAxObject(dispatch.Get(), object);
        wrapper->setObjectName(object->objectName());
    }
    return wrapper;
}

QT_END_NAMESPACE
