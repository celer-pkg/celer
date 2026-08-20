// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qbinaryjsonarray_p.h"
#include "qbinaryjson_p.h"

#include <qjsonarray.h>

QT_BEGIN_NAMESPACE

QBinaryJsonArray::~QBinaryJsonArray()
{
    if (d && !d->ref.deref())
        delete d;
}

QBinaryJsonArray QBinaryJsonArray::fromJsonArray(const QJsonArray &array)
{
    QBinaryJsonArray binary;
    for (const QJsonValue value : array)
        binary.append(QBinaryJsonValue::fromJsonValue(value));
    if (binary.d) // We want to compact it as it is a root item now
        binary.d->compactionCounter++;
    binary.compact();
    return binary;
}

void QBinaryJsonArray::append(const QBinaryJsonValue &value)
{
    const uint i = a ? a->length() : 0;

    bool compressed;
    uint valueSize = QBinaryJsonPrivate::Value::requiredStorage(value, &compressed);

    if (!detach(valueSize + sizeof(QBinaryJsonPrivate::Value)))
        return;

    if (!a->length())
        a->tableOffset = sizeof(QBinaryJsonPrivate::Array);

    uint valueOffset = a->reserveSpace(valueSize, i, 1, false);
    if (!valueOffset)
        return;

    QBinaryJsonPrivate::Value *v = a->at(i);
    v->setType(value.t == QJsonValue::Undefined ? QJsonValue::Null : value.t);
    v->setIsLatinOrIntValue(compressed);
    v->setIsLatinKey(false);
    v->setValue(QBinaryJsonPrivate::Value::valueToStore(value, valueOffset));
    if (valueSize) {
        QBinaryJsonPrivate::Value::copyData(value, reinterpret_cast<char *>(a) + valueOffset,
                                            compressed);
    }
}

char *QBinaryJsonArray::takeRawData(uint *size)
{
    if (d)
        return d->takeRawData(size);
    *size = 0;
    return nullptr;
}

bool QBinaryJsonArray::detach(uint reserve)
{
    if (!d) {
        if (reserve >= QBinaryJsonPrivate::Value::MaxSize) {
            qWarning("QBinaryJson: Document too large to store in data structure");
            return false;
        }
        d = new QBinaryJsonPrivate::MutableData(reserve, QJsonValue::Array);
        a = static_cast<QBinaryJsonPrivate::Array *>(d->header->root());
        d->ref.ref();
        return true;
    }
    if (reserve == 0 && d->ref.loadRelaxed() == 1)
        return true;

    QBinaryJsonPrivate::MutableData *x = d->clone(a, reserve);
    if (!x)
        return false;
    x->ref.ref();
    if (!d->ref.deref())
        delete d;
    d = x;
    a = static_cast<QBinaryJsonPrivate::Array *>(d->header->root());
    return true;
}

void QBinaryJsonArray::compact()
{
    if (!d || !d->compactionCounter)
        return;

    detach();
    d->compact();
    a = static_cast<QBinaryJsonPrivate::Array *>(d->header->root());
}

QT_END_NAMESPACE

