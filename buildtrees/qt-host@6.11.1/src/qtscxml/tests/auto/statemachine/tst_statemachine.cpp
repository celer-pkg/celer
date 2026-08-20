// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest>
#include <QtTest/private/qpropertytesthelper_p.h>
#include <QObject>
#include <QXmlStreamReader>
#include <QtScxml/qscxmlcompiler.h>
#include <QtScxml/qscxmlstatemachine.h>
#include <QtScxml/qscxmlinvokableservice.h>
#include <QtScxml/private/qscxmlstatemachine_p.h>
#include <QtScxml/QScxmlNullDataModel>

#include "topmachine.h"

enum { SpyWaitTime = 8000 };

class tst_StateMachine: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void stateNames_data();
    void stateNames();
    void activeStateNames_data();
    void activeStateNames();
    void connections();
    void historyState();
    void onExit();
    void eventOccurred();

    void doneDotStateEvent();
    void running();
    void restart();

    void invokeStateMachine();

    void multipleInvokableServices(); // QTBUG-61484
    void logWithoutExpr();

    void foreachLoop();

    void bindings();

    void setTableDataUpdatesObjectNames();

    void finalizeRunsOnDoneInvokeEvents();
    void errorsFromNestedStatemachine(); // QTBUG-135396
};

void tst_StateMachine::errorsFromNestedStatemachine()
{
    QScopedPointer<QScxmlStateMachine> machine(QScxmlStateMachine::fromFile(":/tst_statemachine/nestedinvoker.scxml"));
    auto error = machine->parseErrors().front();
    QCOMPARE(error.fileName(), "nestederror.scxml");
}

void tst_StateMachine::stateNames_data()
{
    QTest::addColumn<QString>("scxmlFileName");
    QTest::addColumn<bool>("compressed");
    QTest::addColumn<QStringList>("expectedStates");

    QTest::newRow("stateNames-compressed") << QString(":/tst_statemachine/statenames.scxml")
                                      << true
                                      << (QStringList() << QString("a1") << QString("a2") << QString("final"));
    QTest::newRow("stateNames-notCompressed") << QString(":/tst_statemachine/statenames.scxml")
                                      << false
                                      << (QStringList() << QString("top") << QString("a") << QString("a1") <<  QString("a2") << QString("b") << QString("final"));
    QTest::newRow("stateNamesNested-compressed") << QString(":/tst_statemachine/statenamesnested.scxml")
                                      << true
                                      << (QStringList() << QString("a") << QString("b"));
    QTest::newRow("stateNamesNested-notCompressed") << QString(":/tst_statemachine/statenamesnested.scxml")
                                      << false
                                      << (QStringList() << QString("super_top") << QString("a") << QString("b"));

    QTest::newRow("ids1") << QString(":/tst_statemachine/ids1.scxml")
                          << false
                          << (QStringList() << QString("foo.bar") << QString("foo-bar")
                              << QString("foo_bar") << QString("_"));
}

void tst_StateMachine::stateNames()
{
    QFETCH(QString, scxmlFileName);
    QFETCH(bool, compressed);
    QFETCH(QStringList, expectedStates);

    QScopedPointer<QScxmlStateMachine> stateMachine(QScxmlStateMachine::fromFile(scxmlFileName));
    QVERIFY(!stateMachine.isNull());
    QCOMPARE(stateMachine->parseErrors().size(), 0);

    QCOMPARE(stateMachine->stateNames(compressed), expectedStates);
}

void tst_StateMachine::activeStateNames_data()
{
    QTest::addColumn<QString>("scxmlFileName");
    QTest::addColumn<bool>("compressed");
    QTest::addColumn<QStringList>("expectedStates");

    QTest::newRow("stateNames-compressed") << QString(":/tst_statemachine/statenames.scxml")
                                      << true
                                      << (QStringList() << QString("a1") << QString("final"));
    QTest::newRow("stateNames-notCompressed") << QString(":/tst_statemachine/statenames.scxml")
                                      << false
                                      << (QStringList() << QString("top") << QString("a") << QString("a1") << QString("b") << QString("final"));
    QTest::newRow("stateNamesNested-compressed") << QString(":/tst_statemachine/statenamesnested.scxml")
                                      << true
                                      << (QStringList() << QString("a") << QString("b"));
    QTest::newRow("stateNamesNested-notCompressed") << QString(":/tst_statemachine/statenamesnested.scxml")
                                      << false
                                      << (QStringList() << QString("super_top") << QString("a") << QString("b"));
}

void tst_StateMachine::activeStateNames()
{
    QFETCH(QString, scxmlFileName);
    QFETCH(bool, compressed);
    QFETCH(QStringList, expectedStates);

    QScopedPointer<QScxmlStateMachine> stateMachine(QScxmlStateMachine::fromFile(scxmlFileName));
    QVERIFY(!stateMachine.isNull());

    QSignalSpy stableStateSpy(stateMachine.data(), SIGNAL(reachedStableState()));

    stateMachine->start();

    stableStateSpy.wait(5000);

    QCOMPARE(stateMachine->activeStateNames(compressed), expectedStates);
}

class Receiver : public QObject {
    Q_OBJECT
public slots:
    void a(bool enabled)
    {
        aReached = aReached || enabled;
    }

    void b(bool enabled)
    {
        bReached = bReached || enabled;
    }

    void aEnter()
    {
        aEntered = true;
    }

    void aExit()
    {
        aExited = true;
    }

public:
    bool aReached = false;
    bool bReached = false;
    bool aEntered = false;
    bool aExited = false;
};

void tst_StateMachine::connections()
{
    QScopedPointer<QScxmlStateMachine> stateMachine(
                QScxmlStateMachine::fromFile(QString(":/tst_statemachine/statenames.scxml")));
    QVERIFY(!stateMachine.isNull());
    Receiver receiver;

    bool a1Reached = false;
    bool finalReached = false;
    QMetaObject::Connection a = stateMachine->connectToState("a", &receiver, &Receiver::a);
    QVERIFY(a);
    QMetaObject::Connection b = stateMachine->connectToState("b", &receiver, SLOT(b(bool)));
    QVERIFY(b);
    QMetaObject::Connection a1 = stateMachine->connectToState("a1", &receiver,
                                                              [&a1Reached](bool enabled) {
        a1Reached = a1Reached || enabled;
    });
    QVERIFY(a1);
    QMetaObject::Connection final = stateMachine->connectToState("final",
                                                                 [&finalReached](bool enabled) {
        finalReached = finalReached || enabled;
    });
    QVERIFY(final);

    bool a1Entered = false;
    bool a1Exited = false;
    bool finalEntered = false;
    bool finalExited = false;
    typedef QScxmlStateMachine QXSM;

    QMetaObject::Connection aEntry = stateMachine->connectToState(
                "a", QXSM::onEntry(&receiver, &Receiver::aEnter));
    QVERIFY(aEntry);
    QMetaObject::Connection aExit = stateMachine->connectToState(
                "a", QXSM::onExit(&receiver, &Receiver::aExit));
    QVERIFY(aExit);
    QMetaObject::Connection a1Entry = stateMachine->connectToState("a1", &receiver,
                                                                   QXSM::onEntry([&a1Entered]() {
        a1Entered = true;
    }));
    QVERIFY(a1Entry);
    QMetaObject::Connection a1Exit = stateMachine->connectToState("a1", &receiver,
                                                                   QXSM::onExit([&a1Exited]() {
        a1Exited = true;
    }));
    QVERIFY(a1Exit);

    QMetaObject::Connection finalEntry = stateMachine->connectToState(
                "final", QXSM::onEntry([&finalEntered]() {
        finalEntered = true;
    }));
    QVERIFY(finalEntry);

    QMetaObject::Connection finalExit = stateMachine->connectToState(
                "final", QXSM::onExit([&finalExited]() {
        finalExited = true;
    }));
    QVERIFY(finalExit);

    stateMachine->start();

    QTRY_VERIFY(a1Reached);
    QTRY_VERIFY(finalReached);
    QTRY_VERIFY(receiver.aReached);
    QTRY_VERIFY(receiver.bReached);

    QVERIFY(disconnect(a));
    QVERIFY(disconnect(b));
    QVERIFY(disconnect(a1));
    QVERIFY(disconnect(final));

#if defined(__cpp_return_type_deduction) && __cpp_return_type_deduction == 201304
    QVERIFY(receiver.aEntered);
    QVERIFY(!receiver.aExited);
    QVERIFY(a1Entered);
    QVERIFY(!a1Exited);
    QVERIFY(finalEntered);
    QVERIFY(!finalExited);

    QVERIFY(disconnect(aEntry));
    QVERIFY(disconnect(aExit));
    QVERIFY(disconnect(a1Entry));
    QVERIFY(disconnect(a1Exit));
    QVERIFY(disconnect(finalEntry));
    QVERIFY(disconnect(finalExit));
#endif
}

void tst_StateMachine::historyState()
{
    QScopedPointer<QScxmlStateMachine> stateMachine(
                QScxmlStateMachine::fromFile(QString(":/tst_statemachine/historystate.scxml")));
    QVERIFY(!stateMachine.isNull());

    bool state2Reached = false;
    QMetaObject::Connection state2Connection = stateMachine->connectToState("State2",
                            [&state2Reached](bool enabled) {
        state2Reached = state2Reached || enabled;
    });
    QVERIFY(state2Connection);

    stateMachine->start();

    QTRY_VERIFY(state2Reached);
}

void tst_StateMachine::onExit()
{
#if defined(__cpp_return_type_deduction) && __cpp_return_type_deduction == 201304
    // Test onExit being actually called

    typedef QScxmlStateMachine QXSM;
    QScopedPointer<QXSM> stateMachine(QXSM::fromFile(QString(":/tst_statemachine/eventoccurred.scxml")));

    Receiver receiver;
    bool aExited1 = false;

    stateMachine->connectToState("a", QXSM::onExit([&aExited1]() { aExited1 = true; }));
    stateMachine->connectToState("a", QXSM::onExit(&receiver, &Receiver::aExit));
    stateMachine->connectToState("a", QXSM::onExit(&receiver, "aEnter"));
    {
        // Should not crash
        Receiver receiver2;
        stateMachine->connectToState("a", QXSM::onEntry(&receiver2, &Receiver::aEnter));
        stateMachine->connectToState("a", QXSM::onEntry(&receiver2, "aExit"));
        stateMachine->connectToState("a", QXSM::onExit(&receiver2, &Receiver::aExit));
        stateMachine->connectToState("a", QXSM::onExit(&receiver2, "aEnter"));
    }

    stateMachine->start();
    QTRY_VERIFY(receiver.aEntered);
    QTRY_VERIFY(receiver.aExited);
    QTRY_VERIFY(aExited1);
#endif
}

bool hasChildEventRouters(QScxmlStateMachine *stateMachine)
{
    // Cast to QObject, to avoid ambigous "children" member.
    const QObject &parentRouter = QScxmlStateMachinePrivate::get(stateMachine)->m_router;
    return !parentRouter.children().isEmpty();
}

void tst_StateMachine::eventOccurred()
{
    QScopedPointer<QScxmlStateMachine> stateMachine(QScxmlStateMachine::fromFile(QString(":/tst_statemachine/eventoccurred.scxml")));
    QVERIFY(!stateMachine.isNull());

    qRegisterMetaType<QScxmlEvent>();
    QSignalSpy finishedSpy(stateMachine.data(), SIGNAL(finished()));

    int events = 0;
    auto con1 = stateMachine->connectToEvent("internalEvent2", [&events](const QScxmlEvent &event) {
        QCOMPARE(++events, 1);
        QCOMPARE(event.name(), QString("internalEvent2"));
        QCOMPARE(event.eventType(), QScxmlEvent::ExternalEvent);
    });
    QVERIFY(con1);

    auto con2 = stateMachine->connectToEvent("externalEvent", [&events](const QScxmlEvent &event) {
        QCOMPARE(++events, 2);
        QCOMPARE(event.name(), QString("externalEvent"));
        QCOMPARE(event.eventType(), QScxmlEvent::ExternalEvent);
    });
    QVERIFY(con2);

    auto con3 = stateMachine->connectToEvent("timeout", [&events](const QScxmlEvent &event) {
        QCOMPARE(++events, 3);
        QCOMPARE(event.name(), QString("timeout"));
        QCOMPARE(event.eventType(), QScxmlEvent::ExternalEvent);
    });
    QVERIFY(con3);

    auto con4 = stateMachine->connectToEvent("done.*", [&events](const QScxmlEvent &event) {
        QCOMPARE(++events, 4);
        QCOMPARE(event.name(), QString("done.state.top"));
        QCOMPARE(event.eventType(), QScxmlEvent::ExternalEvent);
    });
    QVERIFY(con4);

    auto con5 = stateMachine->connectToEvent("done.state", [&events](const QScxmlEvent &event) {
        QCOMPARE(++events, 5);
        QCOMPARE(event.name(), QString("done.state.top"));
        QCOMPARE(event.eventType(), QScxmlEvent::ExternalEvent);
    });
    QVERIFY(con5);

    auto con6 = stateMachine->connectToEvent("done.state.top", [&events](const QScxmlEvent &event) {
        QCOMPARE(++events, 6);
        QCOMPARE(event.name(), QString("done.state.top"));
        QCOMPARE(event.eventType(), QScxmlEvent::ExternalEvent);
    });
    QVERIFY(con6);

    stateMachine->start();

    finishedSpy.wait(5000);
    QCOMPARE(events, 6);

    QVERIFY(disconnect(con1));
    QVERIFY(disconnect(con2));
    QVERIFY(disconnect(con3));
    QVERIFY(disconnect(con4));
    QVERIFY(disconnect(con5));
    QVERIFY(disconnect(con6));

    QTRY_VERIFY(!hasChildEventRouters(stateMachine.data()));
}

void tst_StateMachine::doneDotStateEvent()
{
    QScopedPointer<QScxmlStateMachine> stateMachine(QScxmlStateMachine::fromFile(QString(":/tst_statemachine/stateDotDoneEvent.scxml")));
    QVERIFY(!stateMachine.isNull());

    QSignalSpy finishedSpy(stateMachine.data(), SIGNAL(finished()));

    stateMachine->start();
    finishedSpy.wait(5000);
    QCOMPARE(finishedSpy.size(), 1);
    QCOMPARE(stateMachine->activeStateNames(true).size(), 1);
    QVERIFY(stateMachine->activeStateNames(true).contains(QLatin1String("success")));
}

void tst_StateMachine::running()
{
    QScopedPointer<QScxmlStateMachine> stateMachine(
                QScxmlStateMachine::fromFile(QString(":/tst_statemachine/statenames.scxml")));
    QVERIFY(!stateMachine.isNull());

    QSignalSpy runningChangedSpy(stateMachine.data(), SIGNAL(runningChanged(bool)));

    QCOMPARE(stateMachine->isRunning(), false);

    stateMachine->start();

    QCOMPARE(runningChangedSpy.size(), 1);
    QCOMPARE(stateMachine->isRunning(), true);

    stateMachine->stop();

    QCOMPARE(runningChangedSpy.size(), 2);
    QCOMPARE(stateMachine->isRunning(), false);
}

void tst_StateMachine::restart()
{
    QScopedPointer<QScxmlStateMachine> stateMachine(
            QScxmlStateMachine::fromFile(QString(":/tst_statemachine/stateDotDoneEvent.scxml")));
    QVERIFY(!stateMachine.isNull());

    QSignalSpy finishedSpy(stateMachine.data(), SIGNAL(finished()));

    stateMachine->start();
    finishedSpy.wait(5000);
    QCOMPARE(finishedSpy.size(), 1);
    QCOMPARE(stateMachine->activeStateNames(true).size(), 1);
    QVERIFY(stateMachine->activeStateNames(true).contains(QLatin1String("success")));

    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("(.*)Can't start finished machine"));
    stateMachine->start();
}

void tst_StateMachine::invokeStateMachine()
{
    QScopedPointer<QScxmlStateMachine> stateMachine(
                QScxmlStateMachine::fromFile(QString(":/tst_statemachine/invoke.scxml")));
    QVERIFY(!stateMachine.isNull());

    stateMachine->start();
    QCOMPARE(stateMachine->isRunning(), true);
    QTRY_VERIFY(stateMachine->activeStateNames().contains(QString("anyplace")));

    QList<QScxmlInvokableService *> services = stateMachine->invokedServices();
    QCOMPARE(services.size(), 1);
    QVariant subMachineVariant = services[0]->property("stateMachine");
    QVERIFY(subMachineVariant.isValid());
    QScxmlStateMachine *subMachine = qvariant_cast<QScxmlStateMachine *>(subMachineVariant);
    QVERIFY(subMachine);
    QTRY_VERIFY(subMachine->activeStateNames().contains("here"));
}

void tst_StateMachine::multipleInvokableServices()
{
    QScopedPointer<QScxmlStateMachine> stateMachine(
                QScxmlStateMachine::fromFile(QString(":/tst_statemachine/multipleinvokableservices.scxml")));
    QVERIFY(!stateMachine.isNull());

    QSignalSpy finishedSpy(stateMachine.data(), SIGNAL(finished()));
    stateMachine->start();
    QCOMPARE(stateMachine->isRunning(), true);

    finishedSpy.wait(5000);
    QCOMPARE(finishedSpy.size(), 1);
    QCOMPARE(stateMachine->activeStateNames(true).size(), 1);
    QVERIFY(stateMachine->activeStateNames(true).contains(QLatin1String("success")));
}

void tst_StateMachine::logWithoutExpr()
{
    QScopedPointer<QScxmlStateMachine> stateMachine(
                QScxmlStateMachine::fromFile(QString(":/tst_statemachine/emptylog.scxml")));
    QVERIFY(!stateMachine.isNull());
    QTest::ignoreMessage(QtDebugMsg, "\"Hi2\" : \"\"");
    stateMachine->start();
    QSignalSpy logSpy(stateMachine.data(), SIGNAL(log(QString,QString)));
    QTRY_COMPARE(logSpy.size(), 1);
}

void tst_StateMachine::foreachLoop()
{
    QScopedPointer<QScxmlStateMachine> stateMachine(
                QScxmlStateMachine::fromFile(QString(":/tst_statemachine/foreach.scxml")));
    QVERIFY(!stateMachine.isNull());

    qRegisterMetaType<QScxmlEvent>();
    QSignalSpy finishedSpy(stateMachine.data(), SIGNAL(finished()));

    int events1 = 0;
    int events2 = 0;
    int events3 = 0;

    auto con1 = stateMachine->connectToEvent("internalEvent1", [&events1](const QScxmlEvent &event) {
        ++events1;
        QCOMPARE(event.name(), QString("internalEvent1"));
    });
    QVERIFY(con1);

    auto con2 = stateMachine->connectToEvent("internalEvent2", [&events2](const QScxmlEvent &event) {
        ++events2;
        QCOMPARE(event.name(), QString("internalEvent2"));
    });
    QVERIFY(con2);

    auto con3 = stateMachine->connectToEvent("internalEvent3", [&events3](const QScxmlEvent &event) {
        ++events3;
        QCOMPARE(event.name(), QString("internalEvent3"));
    });
    QVERIFY(con3);

    stateMachine->start();

    finishedSpy.wait(5000);
    QCOMPARE(events1, 3);
    QCOMPARE(events2, 3);
    QCOMPARE(events3, 3);
}

void tst_StateMachine::bindings()
{
    // -- QScxmlStateMachine::initialized
    std::unique_ptr<QScxmlStateMachine> stateMachine1(
                QScxmlStateMachine::fromFile(QString(":/tst_statemachine/invoke.scxml")));
    QVERIFY(stateMachine1.get());
    QTestPrivate::testReadOnlyPropertyBasics<QScxmlStateMachine, bool>(
                *stateMachine1, false, true, "initialized", [&](){ stateMachine1.get()->start(); });
    if (QTest::currentTestFailed()) {
        qWarning() << "QScxmlStateMachine::initialized bindable test failed.";
        return;
    }

    using StateMachinePtr = std::unique_ptr<QScxmlStateMachine>;

    // -- QScxmlStateMachine::initialValues
    QVariantMap map1{{"map", 1}};
    QVariantMap map2{{"map", 2}};
    QTestPrivate::testReadWritePropertyBasics<QScxmlStateMachine, QVariantMap>(
            *stateMachine1, map1, map2, "initialValues",
            []() {
                return StateMachinePtr{QScxmlStateMachine::fromFile(QString("not_a_real_file"))};
            });
    if (QTest::currentTestFailed()) {
        qWarning() << "QScxmlStateMachine::initialValues bindable test failed.";
        return;
    }

    // -- QScxmlStateMachine::loader
    class MockLoader: public QScxmlCompiler::Loader
    {
    public:
        QByteArray load(const QString&, const QString&, QStringList*) override { return QByteArray(); }
    };
    MockLoader loader1;
    MockLoader loader2;
    QTestPrivate::testReadWritePropertyBasics<QScxmlStateMachine, QScxmlCompiler::Loader*>(
            *stateMachine1, &loader1, &loader2, "loader",
            []() {
                return StateMachinePtr{QScxmlStateMachine::fromFile(QString("not_a_real_file"))};
            });
    if (QTest::currentTestFailed()) {
        qWarning() << "QScxmlStateMachine::loader bindable test failed.";
        return;
    }

    // -- QScxmlStateMachine::dataModel
    // Use non-existent file below, as valid file would initialize the model
    std::unique_ptr<QScxmlStateMachine> stateMachine2(
                QScxmlStateMachine::fromFile(QString("not_a_real_file")));
    std::unique_ptr<QScxmlStateMachine> stateMachine3(
                QScxmlStateMachine::fromFile(QString("not_a_real_file")));
    QScxmlNullDataModel model1;
    // data can only change once
    QTestPrivate::testWriteOncePropertyBasics<QScxmlStateMachine, QScxmlDataModel*>(
            *stateMachine2, nullptr, &model1, "dataModel", true,
            []() {
                return StateMachinePtr{QScxmlStateMachine::fromFile(QString("not_a_real_file"))};
            });
    if (QTest::currentTestFailed()) {
        qWarning() << "QScxmlStateMachine::dataModel bindable test failed.";
        return;
    }

    // -- QScxmlStateMachine::tableData
    // Use the statemachine to generate the tableData for testing
    std::unique_ptr<QScxmlStateMachine> stateMachine4(
                QScxmlStateMachine::fromFile(QString(":/tst_statemachine/invoke.scxml")));
    QTestPrivate::testReadWritePropertyBasics<QScxmlStateMachine, QScxmlTableData*>(
            *stateMachine2, stateMachine1.get()->tableData(), stateMachine4.get()->tableData(),
            "tableData",
            []() {
                return StateMachinePtr{QScxmlStateMachine::fromFile(QString("not_a_real_file"))};
            });
    if (QTest::currentTestFailed()) {
        qWarning() << "QScxmlStateMachine::tableData bindable test failed.";
        return;
    }

    // -- QScxmlStateMachine::invokedServices
    // Test executes statemachine and observes as the invoked services change
    TopMachine topSm;
    QSignalSpy invokedSpy(&topSm, SIGNAL(invokedServicesChanged(QList<QScxmlInvokableService*>)));
    QCOMPARE(topSm.invokedServices().size(), 0);
    // at some point during the topSm execution there are 3 invoked services
    topSm.start();
    QTRY_COMPARE(topSm.invokedServices().size(), 3);
    QCOMPARE(invokedSpy.size(), 1);
    // after completion invoked services drop back to 0
    QTRY_COMPARE(topSm.isRunning(), false);
    QCOMPARE(topSm.invokedServices().size(), 0);
    QCOMPARE(invokedSpy.size(), 2);
    // bind *to* the invokedservices property and check that we observe same changes
    // during the topSm execution
    QProperty<qsizetype> invokedServicesObserver;
    invokedServicesObserver.setBinding([&](){ return topSm.invokedServices().size(); });
    QCOMPARE(invokedServicesObserver, 0);
    topSm.start();
    QTRY_COMPARE(invokedServicesObserver, 3);
    QCOMPARE(topSm.invokedServices().size(), 3);
    QCOMPARE(invokedSpy.size(), 3);

    // -- QScxmlDataModel::stateMachine
    QScxmlNullDataModel dataModel1;
    std::unique_ptr<QScxmlStateMachine> stateMachine5(
                QScxmlStateMachine::fromFile(QString("not_a_real_file")));
    // data can only change once
    QTestPrivate::testWriteOncePropertyBasics<QScxmlNullDataModel, QScxmlStateMachine*>(
                dataModel1, nullptr, stateMachine5.get(), "stateMachine");
    if (QTest::currentTestFailed()) {
        qWarning() << "QScxmlDataModel::stateMachine bindable test failed.";
        return;
    }
}

void tst_StateMachine::setTableDataUpdatesObjectNames()
{
    std::unique_ptr<QScxmlStateMachine> stateMachine1(
            QScxmlStateMachine::fromFile(QString(":/tst_statemachine/emptylog.scxml")));
    const QString sm1ObjectName = stateMachine1->objectName();
    std::unique_ptr<QScxmlStateMachine> stateMachine2(
            QScxmlStateMachine::fromFile(QString(":/tst_statemachine/eventoccurred.scxml")));
    const QString sm2ObjectName = stateMachine2->objectName();
    QCOMPARE_NE(sm1ObjectName, sm2ObjectName);

    std::unique_ptr<QScxmlStateMachine> sm(
            QScxmlStateMachine::fromFile(QString("not_a_real_file")));
    QVERIFY(sm->objectName().isEmpty());
    // no name set, so update object name
    sm->setTableData(stateMachine1->tableData());
    QCOMPARE_EQ(sm->objectName(), sm1ObjectName);

    // object name already set, so do not update
    sm->setTableData(stateMachine2->tableData());
    QCOMPARE_EQ(sm->objectName(), sm1ObjectName); // did not change
}

void tst_StateMachine::finalizeRunsOnDoneInvokeEvents()
{
    QScopedPointer<QScxmlStateMachine> stateMachine(
                QScxmlStateMachine::fromFile(QString(":/tst_statemachine/finalizeOnDoneInvokeEvent.scxml")));
    QVERIFY(!stateMachine.isNull());

    QSignalSpy logSpy(stateMachine.data(), SIGNAL(log(QString,QString)));

    stateMachine->start();

    QTRY_COMPARE(logSpy.size(), 1);

    QList<QVariant> arguments = logSpy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QString(u"finalize"));
    QVERIFY(arguments.at(1).toString().startsWith("done.invoke.s0"));
}

QTEST_MAIN(tst_StateMachine)

#include "tst_statemachine.moc"
