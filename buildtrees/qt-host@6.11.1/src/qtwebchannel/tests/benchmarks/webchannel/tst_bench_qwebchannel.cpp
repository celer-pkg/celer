// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// Copyright (C) 2019 Menlo Systems GmbH, author Arno Rehn <a.rehn@menlosystems.com>
// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "tst_bench_qwebchannel.h"

#include <QtTest/QtTest>

#include <QWebChannel>

#include <QtWebChannel/private/qmetaobjectpublisher_p.h>
#include <QtWebChannel/private/qwebchannel_p.h>

class tst_bench_QWebChannel : public QObject
{
    Q_OBJECT
public:
    tst_bench_QWebChannel(QObject *parent = nullptr);

private slots:
    void benchClassInfo();
    void benchInitializeClients();
    void benchPropertyUpdates();
    void benchRegisterObjects();
    void benchRemoveTransport();

private:
    DummyTransport *m_dummyTransport;
};

tst_bench_QWebChannel::tst_bench_QWebChannel(QObject *parent)
    : QObject(parent),
      m_dummyTransport(new DummyTransport(this))
{
}

static QHash<QString, QObject*> createObjects(QObject *parent)
{
    const int num = 100;
    QHash<QString, QObject*> objects;
    objects.reserve(num);
    for (int i = 0; i < num; ++i) {
        objects[QStringLiteral("obj%1").arg(i)] = new BenchObject(parent);
    }
    return objects;
}

void tst_bench_QWebChannel::benchClassInfo()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);

    QObject parent;
    const QHash<QString, QObject*> objects = createObjects(&parent);
    QMetaObjectPublisher *publisher = channel.d_func()->publisher;

    QBENCHMARK {
        for (const QObject *object : objects)
            publisher->classInfoForObject(object, m_dummyTransport);
    }
}

void tst_bench_QWebChannel::benchInitializeClients()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);

    QObject parent;
    channel.registerObjects(createObjects(&parent));

    QMetaObjectPublisher *publisher = channel.d_func()->publisher;
    QBENCHMARK {
        publisher->initializeClient(m_dummyTransport);

        publisher->propertyUpdatesInitialized = false;
        publisher->signalToPropertyMap.clear();
        publisher->signalHandlers.clear();
    }
}

void tst_bench_QWebChannel::benchPropertyUpdates()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);

    QObject parent;
    const QHash<QString, QObject*> objects = createObjects(&parent);
    QList<BenchObject *> objectList;
    objectList.reserve(objects.size());
    for (QObject *obj : objects)
        objectList << qobject_cast<BenchObject*>(obj);

    channel.registerObjects(objects);
    channel.d_func()->publisher->initializeClient(m_dummyTransport);

    QBENCHMARK {
        for (BenchObject *obj : std::as_const(objectList))
            obj->change();

        channel.d_func()->publisher->setClientIsIdle(true, m_dummyTransport);
        channel.d_func()->publisher->sendPendingPropertyUpdates();
    }
}

void tst_bench_QWebChannel::benchRegisterObjects()
{
    QWebChannel channel;
    channel.connectTo(m_dummyTransport);

    QObject parent;
    const QHash<QString, QObject*> objects = createObjects(&parent);

    QBENCHMARK {
        channel.registerObjects(objects);
    }
}

void tst_bench_QWebChannel::benchRemoveTransport()
{
    QWebChannel channel;
    std::vector<std::unique_ptr<DummyTransport>> dummyTransports(500);
    for (auto &e : dummyTransports)
        e = std::make_unique<DummyTransport>(this);

    std::vector<std::unique_ptr<QObject>> objs;
    QMetaObjectPublisher *pub = channel.d_func()->publisher;

    for (auto &e : dummyTransports) {
        DummyTransport *transport = e.get();
        channel.connectTo(transport);
        channel.d_func()->publisher->initializeClient(transport);

        /* 30 objects per transport */
        for (int i = 30; i > 0; i--) {
            auto obj = std::make_unique<QObject>();
            pub->wrapResult(QVariant::fromValue(obj.get()), transport);
            objs.push_back(std::move(obj));
        }
    }

    QBENCHMARK_ONCE {
        for (auto &transport : dummyTransports)
            pub->transportRemoved(transport.get());
    }
}

QTEST_MAIN(tst_bench_QWebChannel)

#include "tst_bench_qwebchannel.moc"
