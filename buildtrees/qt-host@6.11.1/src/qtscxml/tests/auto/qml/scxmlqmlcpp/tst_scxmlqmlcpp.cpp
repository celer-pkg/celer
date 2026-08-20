// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../../shared/util.h"

#include <QtTest>
#include <QtTest/private/qpropertytesthelper_p.h>
#include <QtScxml/QScxmlStateMachine>
#include <QtScxml/QScxmlNullDataModel>
#include <QtScxmlQml/private/eventconnection_p.h>
#include <QtScxmlQml/private/invokedservices_p.h>
#include <QtScxmlQml/private/statemachineloader_p.h>
#include "topmachine.h"
#include <functional>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlComponent>
#include <memory>

class tst_scxmlqmlcpp : public QQmlDataTest
{
    Q_OBJECT

private slots:
    void initTestCase() override;

    void eventConnectionStateMachineBinding();
    void eventConnectionEventsBinding();

    void invokedServicesStateMachineBinding();
    void invokedServicesChildrenBinding();

    void stateMachineLoaderInitialValuesBinding();
    void stateMachineLoaderSourceStateMachineBinding();
    void stateMachineLoaderDatamodelBinding();

private:
    QScxmlEventConnection m_eventConnection;
    QScxmlInvokedServices m_invokedServices;
    QScxmlStateMachineLoader m_stateMachineLoader;
    std::unique_ptr<QScxmlStateMachine> m_stateMachine1;
    std::unique_ptr<QScxmlStateMachine> m_stateMachine2;
};

void tst_scxmlqmlcpp::initTestCase()
{
    QQmlDataTest::initTestCase();
    m_stateMachine1.reset(QScxmlStateMachine::fromFile("no_real_file_needed"));
    m_stateMachine2.reset(QScxmlStateMachine::fromFile("no_real_file_needed"));
}

void tst_scxmlqmlcpp::eventConnectionStateMachineBinding()
{
    QCOMPARE(m_eventConnection.bindableStateMachine().value(), nullptr);
    QTestPrivate::testReadWritePropertyBasics<QScxmlEventConnection, QScxmlStateMachine*>(
               m_eventConnection, m_stateMachine1.get(), m_stateMachine2.get(),  "stateMachine");
    m_eventConnection.setStateMachine(nullptr); // tidy up
}

void tst_scxmlqmlcpp::eventConnectionEventsBinding()
{
    QStringList eventList1{{"event1"},{"event2"}};
    QStringList eventList2{{"event3"},{"event4"}};
    QCOMPARE(m_eventConnection.events(), QStringList());
    QTestPrivate::testReadWritePropertyBasics<QScxmlEventConnection, QStringList>(
                m_eventConnection, eventList1, eventList2, "events");
    m_eventConnection.setEvents(QStringList()); // tidy up
}

void tst_scxmlqmlcpp::invokedServicesStateMachineBinding()
{
    QCOMPARE(m_invokedServices.stateMachine(), nullptr);
    QTestPrivate::testReadWritePropertyBasics<QScxmlInvokedServices, QScxmlStateMachine*>(
                m_invokedServices, m_stateMachine1.get(), m_stateMachine2.get(), "stateMachine");
    m_invokedServices.setStateMachine(nullptr); // tidy up
}

void tst_scxmlqmlcpp::invokedServicesChildrenBinding()
{
    TopMachine topSm;
    QScxmlInvokedServices invokedServices;
    invokedServices.setStateMachine(&topSm);
    QCOMPARE(invokedServices.children().size(), 0);
    QCOMPARE(topSm.invokedServices().size(), 0);
    // at some point during the topSm execution there are 3 invoked services
    // of the same name ('3' filters out as '1' at QML binding)
    topSm.start();
    QTRY_COMPARE(topSm.invokedServices().size(), 3);
    QCOMPARE(invokedServices.children().size(), 1);
    // after completion invoked services drop back to 0
    QTRY_COMPARE(topSm.invokedServices().size(), 0);
    QCOMPARE(invokedServices.children().size(), 0);
    // bind *to* the invokedservices property and check that we observe same changes
    // during the topSm execution
    QProperty<qsizetype> serviceCounter;
    serviceCounter.setBinding([&](){ return invokedServices.children().size(); });
    QCOMPARE(serviceCounter, 0);
    topSm.start();
    QTRY_COMPARE(serviceCounter, 1);
    QCOMPARE(topSm.invokedServices().size(), 3);
}

void tst_scxmlqmlcpp::stateMachineLoaderInitialValuesBinding()
{
    QVariantMap values1{{"key1","value1"}, {"key2","value2"}};
    QVariantMap values2{{"key3","value3"}, {"key4","value4"}};
    QTestPrivate::testReadWritePropertyBasics<QScxmlStateMachineLoader, QVariantMap>(
                m_stateMachineLoader, values1, values2, "initialValues");
    m_stateMachineLoader.setInitialValues(QVariantMap()); // tidy up
}

void tst_scxmlqmlcpp::stateMachineLoaderSourceStateMachineBinding()
{
    // Test source and stateMachine together as they interact with each other

    QUrl source1(testFileUrl("submachineA.scxml"));
    QUrl source2(testFileUrl("submachineB.scxml"));
    // The 'setSource' of the statemachineloader assumes a valid qml context
    QQmlEngine engine;
    const QUrl smlUrl = testFileUrl("stateMachineLoader.qml");
    QQmlComponent component(&engine, smlUrl);
    QVERIFY2(component.isReady(), qPrintable(component.errorString()));
    std::unique_ptr<QObject> root(component.create());
    QScxmlStateMachineLoader *sml =
            qobject_cast<QScxmlStateMachineLoader*>(root->findChild<QObject*>("sml"));
    QVERIFY(sml != nullptr);

    QQmlComponent otherComponent(&engine, testFileUrl("smlHelper.qml"));

    // -- StateMachineLoader::source
    QTestPrivate::testReadWritePropertyBasics<QScxmlStateMachineLoader, QUrl>(
                *sml, source1, source2, "source",
                [&otherComponent]() {
                    return std::unique_ptr<QScxmlStateMachineLoader>(
                            qobject_cast<QScxmlStateMachineLoader*>(otherComponent.create()));
                });
    if (QTest::currentTestFailed()) {
        qWarning() << "QScxmlStateMachineLoader::source property testing failed";
        return;
    }

    // -- StateMachineLoader::stateMachine
    // The statemachine can be set indirectly by setting the 'source'
    QSignalSpy smSpy(sml, &QScxmlStateMachineLoader::stateMachineChanged);
    QUrl sourceNonexistent(testFileUrl("file_doesnt_exist.scxml"));
    QUrl sourceBroken(testFileUrl("brokenstatemachine.scxml"));

    QVERIFY(sml->stateMachine() != nullptr);
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression(
                        smlUrl.toString() + ":5:5: QML StateMachineLoader: " +
                        "Cannot open '" + testFile("file_doesnt_exist.scxml") +
                        "' for reading: .*"));
    sml->setSource(sourceNonexistent);
    QVERIFY(sml->stateMachine() == nullptr);
    QCOMPARE(smSpy.size(), 1);

    QString sourceBrokenNoScheme;
    if (sourceBroken.scheme() == QStringLiteral("qrc"))
        sourceBrokenNoScheme = QStringLiteral(":") + sourceBroken.path();
    else
        sourceBrokenNoScheme = sourceBroken.toLocalFile();
    QTest::ignoreMessage(QtWarningMsg,
                         qPrintable(smlUrl.toString() + ":5:5: QML StateMachineLoader: " +
                        sourceBrokenNoScheme + ":12:1: error: initial state 'working' " +
                        "not found for <scxml> element"));

    QTest::ignoreMessage(QtWarningMsg,
                        "SCXML document has errors");
    QTest::ignoreMessage(QtWarningMsg,
                         qPrintable(smlUrl.toString() + ":5:5: QML StateMachineLoader: Something " +
                        "went wrong while parsing '" + sourceBroken.toString() + "':\n"));
    sml->setSource(sourceBroken);
    QVERIFY(sml->stateMachine() == nullptr);
    QCOMPARE(smSpy.size(), 1);

    QProperty<bool> hasStateMachine([&](){ return sml->stateMachine() ? true : false; });
    QVERIFY(hasStateMachine == false);
    sml->setSource(source1);
    QCOMPARE(smSpy.size(), 2);
    QVERIFY(hasStateMachine == true);
}

void tst_scxmlqmlcpp::stateMachineLoaderDatamodelBinding()
{
    QScxmlNullDataModel model1;
    QScxmlNullDataModel model2;
    QTestPrivate::testReadWritePropertyBasics<QScxmlStateMachineLoader,QScxmlDataModel*>
            (m_stateMachineLoader, &model1, &model2, "dataModel");
    m_stateMachineLoader.setDataModel(nullptr); // tidy up
}

QTEST_MAIN(tst_scxmlqmlcpp)
#include "tst_scxmlqmlcpp.moc"
