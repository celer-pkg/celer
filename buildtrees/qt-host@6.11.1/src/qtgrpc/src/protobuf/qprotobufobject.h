// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROTOBUFOBJECT_H
#define QPROTOBUFOBJECT_H

#if 0
#  pragma qt_sync_skip_header_check
#  pragma qt_sync_stop_processing
#endif


#include <QtProtobuf/qprotobufpropertyordering.h>

#include <QtCore/qtmetamacros.h>

#define Q_PROTOBUF_OBJECT Q_PROTOBUF_OBJECT_EXPORT(/* not exported */)
#define Q_PROTOBUF_OBJECT_EXPORT(...)\
    Q_GADGET_EXPORT(__VA_ARGS__)\
    public:\
        __VA_ARGS__ static const QtProtobufPrivate::QProtobufPropertyOrdering staticPropertyOrdering;\
    private:

#endif // QPROTOBUFOBJECT_H
