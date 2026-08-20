// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TAKENUMBER_H
#define TAKENUMBER_H

#include <QtCore/qobject.h>
#include <QtQml/qqml.h>

struct Numbers
{
    Q_GADGET
    QML_VALUE_TYPE(numbers)
    QML_STRUCTURED_VALUE

    Q_PROPERTY(int i MEMBER m_i)
    Q_PROPERTY(int n MEMBER m_n)
    Q_PROPERTY(qsizetype s MEMBER m_s)
    Q_PROPERTY(qlonglong l MEMBER m_l)
public:
    int m_i = 0;
    int m_n = 0;
    qsizetype m_s = 0;
    qlonglong m_l = 0;

    friend bool operator==(const Numbers &a, const Numbers &b)
    {
        return a.m_i == b.m_i && a.m_n == b.m_n && a.m_s == b.m_s && a.m_l == b.m_l;
    }

    friend bool operator!=(const Numbers &a, const Numbers &b)
    {
        return !(a == b);
    }
};

class TakeNumber : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int propertyInt MEMBER propertyInt NOTIFY propertyIntChanged)
    Q_PROPERTY(int propertyNegativeInt MEMBER propertyNegativeInt NOTIFY propertyNegativeIntChanged)
    Q_PROPERTY(qsizetype propertyQSizeType MEMBER propertyQSizeType NOTIFY propertyQSizeTypeChanged)
    Q_PROPERTY(qlonglong propertyQLongLong MEMBER propertyQLongLong NOTIFY propertyQLongLongChanged)
    Q_PROPERTY(Numbers propertyNumbers MEMBER propertyNumbers NOTIFY propertyNumbersChanged)

public:
    explicit TakeNumber(QObject *parent = nullptr);

    Q_INVOKABLE void takeInt(int a);
    Q_INVOKABLE void takeNegativeInt(int a);
    Q_INVOKABLE void takeQSizeType(qsizetype a);
    Q_INVOKABLE void takeQLongLong(qlonglong a);
    Q_INVOKABLE void takeNumbers(const Numbers &a);

    int takenInt = 0;
    int takenNegativeInt = 0;
    qsizetype takenQSizeType = 0;
    qlonglong takenQLongLong = 0;
    Numbers takenNumbers;

    int propertyInt = 0;
    int propertyNegativeInt = 0;
    qsizetype propertyQSizeType = 0;
    qsizetype propertyQLongLong = 0;
    Numbers propertyNumbers;

signals:
    void propertyIntChanged();
    void propertyNegativeIntChanged();
    void propertyQSizeTypeChanged();
    void propertyQLongLongChanged();
    void propertyNumbersChanged();
};

#endif // TAKENUMBER_H
