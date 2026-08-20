// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:trivial-impl-only

#ifndef QBINARYJSONARRAY_P_H
#define QBINARYJSONARRAY_P_H

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

class QBinaryJsonArray
{
    Q_DISABLE_COPY(QBinaryJsonArray)
public:
    QBinaryJsonArray() = default;
    ~QBinaryJsonArray();

    QBinaryJsonArray(QBinaryJsonArray &&other) noexcept
        : d(other.d),
          a(other.a)
    {
        other.d = nullptr;
        other.a = nullptr;
    }

    QBinaryJsonArray &operator =(QBinaryJsonArray &&other) noexcept
    {
        qt_ptr_swap(d, other.d);
        qt_ptr_swap(a, other.a);
        return *this;
    }

    static QBinaryJsonArray fromJsonArray(const QJsonArray &array);
    char *takeRawData(uint *size);

private:
    friend class QBinaryJsonValue;

    void append(const QBinaryJsonValue &value);
    void compact();
    bool detach(uint reserve = 0);

    QBinaryJsonPrivate::MutableData *d = nullptr;
    QBinaryJsonPrivate::Array *a = nullptr;
};

QT_END_NAMESPACE

#endif // QBINARYJSONARRAY_P_H
