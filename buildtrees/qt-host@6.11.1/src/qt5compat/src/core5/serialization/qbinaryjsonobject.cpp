// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qbinaryjsonobject_p.h"
#include "qbinaryjson_p.h"

#include <qjsonobject.h>

QT_BEGIN_NAMESPACE

QBinaryJsonObject::~QBinaryJsonObject()
{
    if (d && !d->ref.deref())
        delete d;
}

QBinaryJsonObject QBinaryJsonObject::fromJsonObject(const QJsonObject &object)
{
    QBinaryJsonObject binary;
    for (auto it = object.begin(), end = object.end(); it != end; ++it)
        binary.insert(it.key(), QBinaryJsonValue::fromJsonValue(it.value()));
    if (binary.d) // We want to compact it as it is a root item now
        binary.d->compactionCounter++;
    binary.compact();
    return binary;
}

void QBinaryJsonObject::insert(const QString &key, const QBinaryJsonValue &value)
{
    bool latinOrIntValue;
    uint valueSize = QBinaryJsonPrivate::Value::requiredStorage(value, &latinOrIntValue);

    bool latinKey = QBinaryJsonPrivate::useCompressed(key);
    uint valueOffset = sizeof(QBinaryJsonPrivate::Entry)
            + QBinaryJsonPrivate::qStringSize(key, latinKey);
    uint requiredSize = valueOffset + valueSize;

    if (!detach(requiredSize + sizeof(QBinaryJsonPrivate::offset))) // offset for the new index entry
        return;

    if (!o->length())
        o->tableOffset = sizeof(QBinaryJsonPrivate::Object);

    bool keyExists = false;
    uint pos = o->indexOf(key, &keyExists);
    if (keyExists)
        ++d->compactionCounter;

    uint off = o->reserveSpace(requiredSize, pos, 1, keyExists);
    if (!off)
        return;

    QBinaryJsonPrivate::Entry *e = o->entryAt(pos);
    e->value.setType(value.t);
    e->value.setIsLatinKey(latinKey);
    e->value.setIsLatinOrIntValue(latinOrIntValue);
    e->value.setValue(QBinaryJsonPrivate::Value::valueToStore(
            value, reinterpret_cast<char *>(e) - reinterpret_cast<char *>(o) + valueOffset));
    QBinaryJsonPrivate::copyString(reinterpret_cast<char *>(e + 1), key, latinKey);
    if (valueSize) {
        QBinaryJsonPrivate::Value::copyData(value, reinterpret_cast<char *>(e) + valueOffset,
                                            latinOrIntValue);
    }

    if (d->compactionCounter > 32U && d->compactionCounter >= unsigned(o->length()) / 2U)
        compact();
}

char *QBinaryJsonObject::takeRawData(uint *size) const
{
    if (d)
        return d->takeRawData(size);
    *size = 0;
    return nullptr;
}

bool QBinaryJsonObject::detach(uint reserve)
{
    if (!d) {
        if (reserve >= QBinaryJsonPrivate::Value::MaxSize) {
            qWarning("QBinaryJson: Document too large to store in data structure");
            return false;
        }
        d = new QBinaryJsonPrivate::MutableData(reserve, QJsonValue::Object);
        o = static_cast<QBinaryJsonPrivate::Object *>(d->header->root());
        d->ref.ref();
        return true;
    }
    if (reserve == 0 && d->ref.loadRelaxed() == 1)
        return true;

    QBinaryJsonPrivate::MutableData *x = d->clone(o, reserve);
    if (!x)
        return false;
    x->ref.ref();
    if (!d->ref.deref())
        delete d;
    d = x;
    o = static_cast<QBinaryJsonPrivate::Object *>(d->header->root());
    return true;
}

void QBinaryJsonObject::compact()
{
    if (!d || !d->compactionCounter)
        return;

    detach();
    d->compact();
    o = static_cast<QBinaryJsonPrivate::Object *>(d->header->root());
}

QT_END_NAMESPACE
