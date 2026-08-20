// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:trivial-impl-only

#ifndef QBINARYJSONOBJECT_H
#define QBINARYJSONOBJECT_H

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

#include "qbinaryjsonvalue_p.h"

QT_BEGIN_NAMESPACE

class QBinaryJsonObject
{
    Q_DISABLE_COPY(QBinaryJsonObject)
public:
    QBinaryJsonObject() = default;
    ~QBinaryJsonObject();

    QBinaryJsonObject(QBinaryJsonObject &&other) noexcept
        : d(other.d), o(other.o)
    {
        other.d = nullptr;
        other.o = nullptr;
    }

    QBinaryJsonObject &operator =(QBinaryJsonObject &&other) noexcept
    {
        qt_ptr_swap(d, other.d);
        qt_ptr_swap(o, other.o);
        return *this;
    }

    static QBinaryJsonObject fromJsonObject(const QJsonObject &object);
    char *takeRawData(uint *size) const;

private:
    friend class QBinaryJsonValue;

    void insert(const QString &key, const QBinaryJsonValue &value);
    bool detach(uint reserve = 0);
    void compact();

    QBinaryJsonPrivate::MutableData *d = nullptr;
    QBinaryJsonPrivate::Object *o = nullptr;
};

QT_END_NAMESPACE

#endif // QBINARYJSONOBJECT_P_H
