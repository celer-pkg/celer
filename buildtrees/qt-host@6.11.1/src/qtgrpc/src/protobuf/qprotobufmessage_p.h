// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPROTOBUFMESSAGE_P_H
#define QPROTOBUFMESSAGE_P_H

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

#include <QtProtobuf/qprotobufmessage.h>
#include <QtProtobuf/qprotobufpropertyordering.h>
#include <QtProtobuf/qtprotobufexports.h>

#include <QtCore/qhash.h>
#include <QtCore/qshareddata.h>

#include <optional>

QT_BEGIN_NAMESPACE

struct QMetaObject;
class QMetaProperty;

class Q_PROTOBUF_EXPORT QProtobufMessagePrivate : public QSharedData
{
public:
    QProtobufMessagePrivate() = default;
    explicit QProtobufMessagePrivate(const QMetaObject *metaObject,
                                     const QtProtobufPrivate::QProtobufPropertyOrdering *ordering);
    QProtobufMessagePrivate(const QProtobufMessagePrivate &other) = default;
    QProtobufMessagePrivate(QProtobufMessagePrivate &&other) = delete;
    QProtobufMessagePrivate &operator=(const QProtobufMessagePrivate &other) = delete;
    QProtobufMessagePrivate &operator=(QProtobufMessagePrivate &&other) = delete;
    virtual ~QProtobufMessagePrivate();

    // QHash of form <field index, data>.
    QHash<qint32, QByteArrayList> unknownEntries;
    const QMetaObject *metaObject = nullptr;
    const QtProtobufPrivate::QProtobufPropertyOrdering *ordering = nullptr;

    int propertyIndex(QAnyStringView propertyName) const;
    static void storeUnknownEntry(QProtobufMessage *message, QByteArrayView entry, int fieldNumber);

    std::optional<QMetaProperty> metaProperty(QAnyStringView name) const;
    std::optional<QMetaProperty>
    metaProperty(QtProtobufPrivate::QProtobufFieldInfo ord) const;

    static QProtobufMessagePrivate *get(QProtobufMessage *message) { return message->d_func(); }
    static const QProtobufMessagePrivate *get(const QProtobufMessage *message)
    {
        return message->d_func();
    }
};

QT_END_NAMESPACE

#endif // QPROTOBUFMESSAGE_P_H
