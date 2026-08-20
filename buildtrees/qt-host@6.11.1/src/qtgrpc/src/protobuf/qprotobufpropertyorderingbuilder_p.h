// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROTOBUFPROPERTYORDERINGBUILDER_P_H
#define QPROTOBUFPROPERTYORDERINGBUILDER_P_H

// Source is in qprotobufpropertyordering.cpp
// The intent is to share some of the code

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of other Qt classes. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtProtobuf/qprotobufpropertyordering.h>
#include <QtProtobuf/qtprotobufexports.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qtclasshelpermacros.h>

QT_BEGIN_NAMESPACE

namespace QtProtobufPrivate {

class QProtobufPropertyOrderingBuilderPrivate;
class QProtobufPropertyOrderingBuilder
{
public:
    Q_PROTOBUF_EXPORT explicit QProtobufPropertyOrderingBuilder(QByteArray packageName);
    Q_PROTOBUF_EXPORT ~QProtobufPropertyOrderingBuilder();

    Q_PROTOBUF_EXPORT void addV0Field(QByteArray jsonName, uint fieldNumber, uint propertyIndex,
                                      FieldFlags flags);
    Q_PROTOBUF_EXPORT QProtobufPropertyOrdering::Data *build() const;

private:
    QProtobufPropertyOrderingBuilderPrivate *d_ptr;
    Q_DECLARE_PRIVATE(QProtobufPropertyOrderingBuilder)
    Q_DISABLE_COPY_MOVE(QProtobufPropertyOrderingBuilder)
};

} // namespace QtProtobufPrivate

QT_END_NAMESPACE

#endif // QPROTOBUFPROPERTYORDERINGBUILDER_P_H
