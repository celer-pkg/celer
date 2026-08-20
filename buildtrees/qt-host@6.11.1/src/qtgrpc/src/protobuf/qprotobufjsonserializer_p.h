// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROTOBUFJSONSERIALIZER_P_H
#define QPROTOBUFJSONSERIALIZER_P_H

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

#include <QtProtobuf/qtprotobufexports.h>

#include <QtCore/qtconfigmacros.h>

QT_BEGIN_NAMESPACE

class QProtobufMessage;
class QJsonValue;
class QMetaType;

namespace QtProtobufPrivate {

struct QProtobufFieldInfo;

using CustomJsonSerializer = QJsonValue (*)(const QProtobufMessage *);
using CustomJsonDeserializer = bool (*)(QProtobufMessage *, const QJsonValue &);

Q_PROTOBUF_EXPORT void registerCustomJsonHandler(QMetaType metaType,
                                                 CustomJsonSerializer serializer,
                                                 CustomJsonDeserializer deserializer);

[[nodiscard]] CustomJsonSerializer findCustomJsonSerializer(QMetaType metaType);
[[nodiscard]] CustomJsonDeserializer findCustomJsonDeserializer(QMetaType metaType);
}

QT_END_NAMESPACE

#endif // QPROTOBUFJSONSERIALIZER_P_H
