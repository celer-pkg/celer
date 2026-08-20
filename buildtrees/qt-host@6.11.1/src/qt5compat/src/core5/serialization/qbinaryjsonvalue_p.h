// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:trivial-impl-only

#ifndef QBINARYJSONVALUE_P_H
#define QBINARYJSONVALUE_P_H

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

#include <QtCore5Compat/qcore5global.h>

#include <QtCore/qstring.h>
#include <QtCore/qjsonvalue.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QBinaryJsonArray;
class QBinaryJsonObject;

namespace QBinaryJsonPrivate {
class ConstData;
class MutableData;
class Base;
class Value;
class Object;
class Array;
}

class Q_CORE5COMPAT_EXPORT QBinaryJsonValue
{
    Q_DISABLE_COPY(QBinaryJsonValue)
public:
    explicit QBinaryJsonValue(QJsonValue::Type type) : ui(0), t(type) {}
    explicit QBinaryJsonValue(bool b) : b(b), t(QJsonValue::Bool) {}
    explicit QBinaryJsonValue(double n) : dbl(n), t(QJsonValue::Double) {}
    explicit QBinaryJsonValue(QString s);
    QBinaryJsonValue(const QBinaryJsonArray &a);
    QBinaryJsonValue(const QBinaryJsonObject &o);

    ~QBinaryJsonValue();

    QBinaryJsonValue(QBinaryJsonValue &&other) noexcept
        : ui(other.ui),
          stringData(std::move(other.stringData)),
          d(other.d),
          t(other.t)
    {
        other.ui = 0;
        other.d = nullptr;
        other.t = QJsonValue::Null;
    }

    QBinaryJsonValue &operator =(QBinaryJsonValue &&other) noexcept
    {
        stringData.swap(other.stringData);
        std::swap(ui, other.ui);
        qt_ptr_swap(d, other.d);
        std::swap(t, other.t);
        return *this;
    }

    static QBinaryJsonValue fromJsonValue(const QJsonValue &json);
    QJsonValue::Type type() const { return t; }
    bool toBool() const { return (t == QJsonValue::Bool) && b; }
    double toDouble() const { return (t == QJsonValue::Double) ? dbl : 0; }
    QString toString() const;

private:
    friend class QBinaryJsonPrivate::Value;
    friend class QBinaryJsonArray;
    friend class QBinaryJsonObject;

    QBinaryJsonValue(QBinaryJsonPrivate::MutableData *d, QBinaryJsonPrivate::Base *parent,
                     const QBinaryJsonPrivate::Value &v);

    void detach();

    union {
        quint64 ui;
        bool b;
        double dbl;
        const QBinaryJsonPrivate::Base *base;
    };
    QString stringData;
    QBinaryJsonPrivate::MutableData *d = nullptr; // needed for Objects and Arrays
    QJsonValue::Type t = QJsonValue::Null;
};

QT_END_NAMESPACE

#endif // QBINARYJSONVALUE_P_H
