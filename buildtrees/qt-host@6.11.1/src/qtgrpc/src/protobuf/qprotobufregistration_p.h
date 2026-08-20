// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROTOBUFREGISTRATION_P_H
#define QPROTOBUFREGISTRATION_P_H

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

#include <QtProtobuf/qprotobufregistration.h>
#include <QtProtobuf/qtprotobufexports.h>

#include <QtCore/qxpfunctional.h>

QT_BEGIN_NAMESPACE

namespace QtProtobufPrivate {

inline void ensureValue(const void *valuePtr)
{
    Q_ASSERT_X(valuePtr != nullptr, "QAbstractProtobufSerializer", "Value is nullptr");
}

using MessageFieldSerializer = qxp::function_ref<void(const QProtobufMessage *,
                                                      const QProtobufFieldInfo &)>;
using MessageFieldDeserializer = qxp::function_ref<bool(QProtobufMessage *)>;

using Serializer = void (*)(MessageFieldSerializer, const void *, const QProtobufFieldInfo &);
using Deserializer = void (*)(MessageFieldDeserializer, void *);

struct SerializationHandler
{
    Serializer serializer = nullptr; /*!< serializer assigned to class */
    Deserializer deserializer = nullptr; /*!< deserializer assigned to class */
};

extern Q_PROTOBUF_EXPORT void registerHandler(QMetaType type, Serializer serializer,
                                              Deserializer deserializer);

extern Q_PROTOBUF_EXPORT SerializationHandler findHandler(QMetaType type);
}

QT_END_NAMESPACE

#endif // QPROTOBUFREGISTRATION_P_H
