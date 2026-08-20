// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// Copyright (C) 2019 Menlo Systems GmbH, author Arno Rehn <a.rehn@menlosystems.com>
// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TST_BENCH_QWEBCHANNEL_H
#define TST_BENCH_QWEBCHANNEL_H

#include <QObject>
#include <QWebChannelAbstractTransport>

QT_USE_NAMESPACE

class DummyTransport : public QWebChannelAbstractTransport
{
    Q_OBJECT
public:
    explicit DummyTransport(QObject *parent = nullptr)
        : QWebChannelAbstractTransport(parent)
    {}
    ~DummyTransport() {};

public slots:
    void sendMessage(const QJsonObject &message) override
    {
        Q_UNUSED(message);
    }
};

class BenchObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int p0 MEMBER m_p0 NOTIFY p0Changed)
    Q_PROPERTY(int p1 MEMBER m_p1 NOTIFY p1Changed)
    Q_PROPERTY(int p2 MEMBER m_p2 NOTIFY p2Changed)
    Q_PROPERTY(int p3 MEMBER m_p3 NOTIFY p3Changed)
    Q_PROPERTY(int p4 MEMBER m_p4 NOTIFY p4Changed)
    Q_PROPERTY(int p5 MEMBER m_p5 NOTIFY p5Changed)
    Q_PROPERTY(int p6 MEMBER m_p6 NOTIFY p6Changed)
    Q_PROPERTY(int p7 MEMBER m_p7 NOTIFY p7Changed)
    Q_PROPERTY(int p8 MEMBER m_p8 NOTIFY p8Changed)
    Q_PROPERTY(int p9 MEMBER m_p9 NOTIFY p9Changed)
public:
    explicit BenchObject(QObject *parent = 0)
        : QObject(parent)
          , m_p0(0)
          , m_p1(0)
          , m_p2(0)
          , m_p3(0)
          , m_p4(0)
          , m_p5(0)
          , m_p6(0)
          , m_p7(0)
          , m_p8(0)
          , m_p9(0)
    { }

    void change()
    {
        m_p0++;
        m_p1++;
        m_p2++;
        m_p3++;
        m_p4++;
        m_p5++;
        m_p6++;
        m_p7++;
        m_p8++;
        m_p9++;
        emit p0Changed(m_p0);
        emit p1Changed(m_p1);
        emit p2Changed(m_p2);
        emit p3Changed(m_p3);
        emit p4Changed(m_p4);
        emit p5Changed(m_p5);
        emit p6Changed(m_p6);
        emit p7Changed(m_p7);
        emit p8Changed(m_p8);
        emit p9Changed(m_p9);
    }

signals:
    void s0();
    void s1();
    void s2();
    void s3();
    void s4();
    void s5();
    void s6();
    void s7();
    void s8();
    void s9();

    void p0Changed(int);
    void p1Changed(int);
    void p2Changed(int);
    void p3Changed(int);
    void p4Changed(int);
    void p5Changed(int);
    void p6Changed(int);
    void p7Changed(int);
    void p8Changed(int);
    void p9Changed(int);

public slots:
    void m0(){};
    void m1(){};
    void m2(){};
    void m3(){};
    void m4(){};
    void m5(){};
    void m6(){};
    void m7(){};
    void m8(){};
    void m9(){};

private:
    int m_p0, m_p1, m_p2, m_p3, m_p4, m_p5, m_p6, m_p7, m_p8, m_p9;
};

#endif // TST_BENCH_QWEBCHANNEL_H
