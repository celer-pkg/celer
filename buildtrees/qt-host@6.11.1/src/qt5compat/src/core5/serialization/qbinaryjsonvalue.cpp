// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qbinaryjsonobject_p.h"
#include "qbinaryjsonvalue_p.h"
#include "qbinaryjsonarray_p.h"
#include "qbinaryjson_p.h"

#include <qjsonarray.h>
#include <qjsonobject.h>

QT_BEGIN_NAMESPACE

QBinaryJsonValue::QBinaryJsonValue(QBinaryJsonPrivate::MutableData *data,
                                   QBinaryJsonPrivate::Base *parent,
                                   const QBinaryJsonPrivate::Value &v)
    : t(QJsonValue::Type(v.type()))
{
    switch (t) {
    case QJsonValue::Undefined:
    case QJsonValue::Null:
        dbl = 0;
        break;
    case QJsonValue::Bool:
        b = v.toBoolean();
        break;
    case QJsonValue::Double:
        dbl = v.toDouble(parent);
        break;
    case QJsonValue::String:
        stringData = v.toString(parent);
        break;
    case QJsonValue::Array:
    case QJsonValue::Object:
        d = data;
        base = v.base(parent);
        break;
    }
    if (d)
        d->ref.ref();
}

QBinaryJsonValue::QBinaryJsonValue(QString string)
    : d(nullptr), t(QJsonValue::String)
{
    stringData = std::move(string);
}

QBinaryJsonValue::QBinaryJsonValue(const QBinaryJsonArray &a)
    : base(a.a), d(a.d), t(QJsonValue::Array)
{
    if (d)
        d->ref.ref();
}

QBinaryJsonValue::QBinaryJsonValue(const QBinaryJsonObject &o)
    : base(o.o), d(o.d), t(QJsonValue::Object)
{
    if (d)
        d->ref.ref();
}

QBinaryJsonValue::~QBinaryJsonValue()
{
    if (d && !d->ref.deref())
        delete d;
}

QBinaryJsonValue QBinaryJsonValue::fromJsonValue(const QJsonValue &json)
{
    switch (json.type()) {
    case QJsonValue::Bool:
        return QBinaryJsonValue(json.toBool());
    case QJsonValue::Double:
        return QBinaryJsonValue(json.toDouble());
    case QJsonValue::String:
        return QBinaryJsonValue(json.toString());
    case QJsonValue::Array:
        return QBinaryJsonArray::fromJsonArray(json.toArray());
    case QJsonValue::Object:
        return QBinaryJsonObject::fromJsonObject(json.toObject());
    case QJsonValue::Null:
        return QBinaryJsonValue(QJsonValue::Null);
    case QJsonValue::Undefined:
        return QBinaryJsonValue(QJsonValue::Undefined);
    }
    Q_UNREACHABLE_RETURN(QBinaryJsonValue(QJsonValue::Null));
}

QString QBinaryJsonValue::toString() const
{
    if (t != QJsonValue::String)
        return QString();
    return stringData;
}

void QBinaryJsonValue::detach()
{
    if (!d)
        return;

    QBinaryJsonPrivate::MutableData *x = d->clone(base);
    x->ref.ref();
    if (!d->ref.deref())
        delete d;
    d = x;
    base = static_cast<QBinaryJsonPrivate::Object *>(d->header->root());
}

QT_END_NAMESPACE
