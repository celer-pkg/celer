// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTPROTOBUFSERIALIZERHELPERS_P_H
#define QTPROTOBUFSERIALIZERHELPERS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of the Qt Protobuf API. This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtProtobuf/qtprotobufexports.h>

#include <QtCore/qtconfigmacros.h>

QT_BEGIN_NAMESPACE

class QVariant;
class QProtobufMessage;
struct QMetaObject;

namespace QtProtobufPrivate {
struct QProtobufFieldInfo;
}

namespace QtProtobufSerializerHelpers {
QVariant messageProperty(const QProtobufMessage *message,
                         const QtProtobufPrivate::QProtobufFieldInfo &fieldInfo,
                         bool allowInitialize = false);
bool setMessageProperty(QProtobufMessage *message,
                        const QtProtobufPrivate::QProtobufFieldInfo &fieldInfo,
                        const QVariant &value);
bool setMessageProperty(QProtobufMessage *message,
                        const QtProtobufPrivate::QProtobufFieldInfo &fieldInfo, QVariant &&value);

const QMetaObject *messageMetaObject(const QProtobufMessage *message);
}

QT_END_NAMESPACE

#endif // QTPROTOBUFSERIALIZERHELPERS_P_H
