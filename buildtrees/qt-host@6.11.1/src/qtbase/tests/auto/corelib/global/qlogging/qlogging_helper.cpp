// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QThread>

#ifdef Q_CC_GNU
#define NEVER_INLINE __attribute__((__noinline__))
#else
#define NEVER_INLINE
#endif

struct T {
    T() { qDebug("static constructor"); }
    ~T() { qDebug("static destructor"); }
} t;

class MyClass : public QObject
{
    Q_OBJECT
public slots:
    virtual NEVER_INLINE QString mySlot1();
private:
    virtual NEVER_INLINE void myFunction(int a);
};

QString MyClass::mySlot1()
{
    myFunction(34);
    return QString();
}

void MyClass::myFunction(int a)
{
    qDebug() << "from_a_function" << a;
}

class Thread : public QThread
{
public:
    Thread() {
        setObjectName("1234567890ABCDE"); // 16 bytes incl. NUL
    }
protected:
    void run() final {
        qDebug("qDebug from another thread");
    }
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("tst_qlogging");

    qSetMessagePattern("[%{type}] %{message}");

    qDebug("qDebug");
    qInfo("qInfo");
    qWarning("qWarning");
    qCritical("qCritical");

    QLoggingCategory cat("category");
    qCWarning(cat) << "qDebug with category";

    qSetMessagePattern(QString());

    qDebug("qDebug2");

    MyClass cl;
    QMetaObject::invokeMethod(&cl, "mySlot1");

    Thread thread;
    thread.start();
    thread.wait();

    return 0;
}

#include "qlogging_helper.moc"
