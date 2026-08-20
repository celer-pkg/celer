// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtTest/qtest.h>
#include <QtTest/qsignalspy.h>
#include <QtCore/qjniobject.h>
#include <QtCore/qjnitypes.h>
#include <QtCore/qcoreapplication_platform.h>

#include <jni.h>

Q_DECLARE_JNI_CLASS(QtSignalQueue, "org/qtproject/qt/android/QtSignalQueue")
Q_DECLARE_JNI_CLASS(SignalListenerInfo, "org/qtproject/qt/android/QtSignalQueue$SignalListenerInfo")
Q_DECLARE_JNI_CLASS(TestQtQuickView, "org/qtproject/qt/android/TestQtQuickView")
Q_DECLARE_JNI_CLASS(QtQuickView, "org/qtproject/qt/android/QtQuickView")
Q_DECLARE_JNI_CLASS(Queue, "java/util/Queue")

class tst_qtquickview_signalqueue : public QObject
{
    Q_OBJECT

public:
    explicit tst_qtquickview_signalqueue(QObject *parent = nullptr);

signals:
    void onConnectSignalListener(QString signalName, QJniArray<jclass> argTypes,
                                 QJniObject listener, int id);

private Q_SLOTS:
    void pushEmpty();
    void removeEmpty();
    void pushRemove();
    void callConnect();

private:
    static void onConnectSignalListenerCalled(JNIEnv *, jclass, QtJniTypes::String signalName,
                                              QJniArray<jclass> argTypes, QJniObject listener,
                                              jint id);
    Q_DECLARE_JNI_NATIVE_METHOD_IN_CURRENT_SCOPE(onConnectSignalListenerCalled)
};

static tst_qtquickview_signalqueue *s_testInstance = nullptr;

using namespace QtJniTypes;

tst_qtquickview_signalqueue::tst_qtquickview_signalqueue(QObject *parent) : QObject(parent)
{
    s_testInstance = this;
    TestQtQuickView::registerNativeMethods({
        Q_JNI_NATIVE_SCOPED_METHOD(onConnectSignalListenerCalled, tst_qtquickview_signalqueue)
    });
}

void tst_qtquickview_signalqueue::onConnectSignalListenerCalled(JNIEnv *, jclass, String signalName,
                                                                QJniArray<jclass> argTypes,
                                                                QJniObject listener, jint id)
{
    Q_ASSERT(s_testInstance);
    emit s_testInstance->onConnectSignalListener(signalName.toString(), argTypes, listener, id);
}

SignalListenerInfo createInfo(int id = 1)
{
    return SignalListenerInfo::construct(QtSignalQueue::construct(), String(QString("signalName")),
                                         QJniArray<jclass>{}, QJniObject{}, id);
}

/*
    Verify that basic addition of elements works.
*/
void tst_qtquickview_signalqueue::pushEmpty()
{
    auto queue = QtSignalQueue::construct();
    queue.callMethod<void>("add", createInfo());
    auto array = queue.getField<Queue>("m_queuedSignalListeners");
    QCOMPARE(array.callMethod<jint>("size"), 1);
}

/*
    Verify that calling QtSignalQueue::remove() on an empty queue returns false.
*/
void tst_qtquickview_signalqueue::removeEmpty()
{
    auto queue = QtSignalQueue::construct();
    auto result = queue.callMethod<bool>("remove", 123);
    QCOMPARE(result, false);
}

/*
    Test basic add and removal of signals from the QtSignalQueue, by directly checking the size of
    the internal data object.
*/
void tst_qtquickview_signalqueue::pushRemove()
{
    auto queue = QtSignalQueue::construct();

    // Add 1 element, remove it and verify result
    const int id = 256;
    queue.callMethod<void>("add", createInfo(id));

    QCOMPARE(queue.callMethod<bool>("remove", id), true);

    auto array = queue.getField<Queue>("m_queuedSignalListeners");
    QCOMPARE(array.callMethod<jint>("size"), 0);

    // Add two elements, verify the size of the data container
    queue.callMethod<void>("add", createInfo(1));
    queue.callMethod<void>("add", createInfo(2));
    QCOMPARE(array.callMethod<jint>("size"), 2);

    // Remove 1 element, verify that result value is correct and the data container size is correct
    QCOMPARE(queue.callMethod<bool>("remove", 2), true);
    QCOMPARE(array.callMethod<jint>("size"), 1);

    // Remove a non-existing element, verify return value
    QCOMPARE(queue.callMethod<bool>("remove", 2), false);
}

/*
    QtSignalQueue::connectQueuedSignalListeners() must call QtQuickView::connectSignalListener()
    for all elements added to the queue. If there are no elements, no connectSignalListener() calls
    may happen. After connectQueuedSignalListeners() has been called, the queue must be empty.
*/
void tst_qtquickview_signalqueue::callConnect()
{
    auto queue = QtSignalQueue::construct();
    auto view = TestQtQuickView::construct(QNativeInterface::QAndroidApplication::context());
    QSignalSpy spy(s_testInstance, &tst_qtquickview_signalqueue::onConnectSignalListener);

    const int id = 256;
    queue.callMethod<void>("add", createInfo(id));

    queue.callMethod<void>("connectQueuedSignalListeners", view.object<QtQuickView>());

    spy.wait(200);
    QCOMPARE(spy.count(), 1);

    QVariantList args = spy.first();
    QCOMPARE(args.last().toInt(), id);

    auto array = queue.getField<Queue>("m_queuedSignalListeners");
    QCOMPARE(array.callMethod<jint>("size"), 0);

    spy.clear();
    queue.callMethod<void>("connectQueuedSignalListeners", view.object<QtQuickView>());
    spy.wait(200);
    QCOMPARE(spy.count(), 0);
}

QTEST_MAIN(tst_qtquickview_signalqueue)

#include "tst_qtquickview_signalqueue.moc"
