// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//TESTED_COMPONENT=src/location

#include <QTest>
#include <QMetaType>
#include <QSignalSpy>

#include <limits.h>
#include <float.h>

#include <QDebug>
#include <QDataStream>
#include <QFile>

#include <QtPositioning/qgeoareamonitorinfo.h>
#include <QtPositioning/qgeoareamonitorsource.h>
#include <QtPositioning/qgeopositioninfo.h>
#include <QtPositioning/qgeopositioninfosource.h>
#include <QtPositioning/qnmeapositioninfosource.h>
#include <QtPositioning/qgeocircle.h>
#include <QtPositioning/qgeorectangle.h>

#include "logfilepositionsource.h"
#include "positionconsumerthread.h"

#include <algorithm>

QT_USE_NAMESPACE
#define UPDATE_INTERVAL 50

QString tst_qgeoareamonitorinfo_debug;

void tst_qgeoareamonitorinfo_messageHandler(QtMsgType type,
                                            const QMessageLogContext &,
                                            const QString &msg)
{
    switch (type) {
        case QtDebugMsg :
            tst_qgeoareamonitorinfo_debug = msg;
            break;
        default:
            break;
    }
}

static QList<QByteArray> readFileData(const QString &fileName)
{
    QList<QByteArray> data;
    QFile logFile(fileName);
    if (logFile.open(QIODevice::ReadOnly)) {
        data = logFile.readAll().split('\n');
        logFile.close();
    } else {
        qWarning() << "Error: cannot open source file" << logFile.fileName();
    }
    return data;
}

class DummyMonitorSource : public QGeoAreaMonitorSource
{
    Q_OBJECT
public:
    static const QString kTestProperty;

    DummyMonitorSource(QObject *parent = nullptr) : QGeoAreaMonitorSource(parent)
    {}

    Error error() const override
    {
        return NoError;
    }
    AreaMonitorFeatures supportedAreaMonitorFeatures() const override
    {
        return AnyAreaMonitorFeature;
    }

    bool startMonitoring(const QGeoAreaMonitorInfo &monitor) override
    {
        Q_UNUSED(monitor);
        return false;
    }
    bool stopMonitoring(const QGeoAreaMonitorInfo &monitor) override
    {
        Q_UNUSED(monitor);
        return false;
    }
    bool requestUpdate(const QGeoAreaMonitorInfo &monitor, const char *signal) override
    {
        Q_UNUSED(monitor);
        Q_UNUSED(signal);
        return false;
    }

    QList<QGeoAreaMonitorInfo> activeMonitors() const override
    {
        return {};
    }
    QList<QGeoAreaMonitorInfo> activeMonitors(const QGeoShape &lookupArea) const override
    {
        Q_UNUSED(lookupArea);
        return {};
    }

    bool setBackendProperty(const QString &name, const QVariant &value) override
    {
        if (name == kTestProperty) {
            m_testPropertyValue = value.toInt();
            return true;
        }
        return false;
    }
    QVariant backendProperty(const QString &name) const override
    {
        if (name == kTestProperty)
            return m_testPropertyValue;
        return QVariant();
    }

private:
    int m_testPropertyValue = 0;
};

const QString DummyMonitorSource::kTestProperty = "TestProperty";

class tst_QGeoAreaMonitorSource : public QObject
{
    Q_OBJECT

private:
    QList<QByteArray> m_fileData;

private slots:
    void initTestCase()
    {
#if QT_CONFIG(library)
        /*
         * Set custom path since CI doesn't install plugins
         */
#ifdef Q_OS_WIN
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() +
                                     QStringLiteral("/../../../../plugins"));
#else
        QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath()
                                         + QStringLiteral("/../../../plugins"));
#endif
#endif
        qRegisterMetaType<QGeoAreaMonitorInfo>();
        m_fileData = readFileData(QFINDTESTDATA("simplelog.txt"));
    }

    void init()
    {
    }

    void cleanup()
    {
        std::unique_ptr<QGeoAreaMonitorSource> obj(
                QGeoAreaMonitorSource::createSource(QStringLiteral("positionpoll"), 0));
        QVERIFY(obj != nullptr);
        QCOMPARE(obj->sourceName(), QStringLiteral("positionpoll"));

        const QList<QGeoAreaMonitorInfo> list = obj->activeMonitors();
        if (list.size() > 0) {
            //cleanup installed monitors
            for (const QGeoAreaMonitorInfo& info : list) {
                QVERIFY(obj->stopMonitoring(info));
            }
        }
        QVERIFY(obj->activeMonitors().size() == 0);
    }

    void cleanupTestCase()
    {
    }

    void tst_monitor()
    {
        QGeoAreaMonitorInfo defaultMonitor;
        QVERIFY(defaultMonitor.name().isEmpty());
        QVERIFY(!defaultMonitor.identifier().isEmpty());
        QCOMPARE(defaultMonitor.isPersistent(), false);
        QVERIFY(!defaultMonitor.area().isValid());
        QVERIFY(!defaultMonitor.isValid());
        QCOMPARE(defaultMonitor.expiration(), QDateTime());
        QCOMPARE(defaultMonitor.notificationParameters(), QVariantMap());

        std::unique_ptr<QGeoAreaMonitorSource> obj(
                QGeoAreaMonitorSource::createSource(QStringLiteral("positionpoll"), 0));
        QVERIFY(obj != nullptr);
        QCOMPARE(obj->sourceName(), QStringLiteral("positionpoll"));
        QVERIFY(!obj->startMonitoring(defaultMonitor));
        QCOMPARE(obj->activeMonitors().size(), 0);
        QVERIFY(!obj->requestUpdate(defaultMonitor,
                                    SIGNAL(areaEntered(QGeoMonitorInfo,QGeoAreaPositionInfo))));

        //copy constructor based
        QGeoAreaMonitorInfo copy(defaultMonitor);
        QVERIFY(copy.name().isEmpty());
        QCOMPARE(copy.identifier(), defaultMonitor.identifier());
        QVERIFY(copy == defaultMonitor);
        QVERIFY(!(copy != defaultMonitor));
        QCOMPARE(copy.isPersistent(), false);

        copy.setName(QString("my name"));
        QCOMPARE(copy.name(), QString("my name"));


        QDateTime now = QDateTime::currentDateTime().addSecs(1000); //little bit in the future
        copy.setExpiration(now);
        QVERIFY(copy != defaultMonitor);
        QCOMPARE(copy.expiration(), now);

        QCOMPARE(copy.isPersistent(), defaultMonitor.isPersistent());
        copy.setPersistent(true);
        QCOMPARE(copy.isPersistent(), true);
        QCOMPARE(defaultMonitor.isPersistent(), false);
        copy.setPersistent(false);

        QVERIFY(copy.area() == defaultMonitor.area());
        QVERIFY(!copy.area().isValid());
        copy.setArea(QGeoCircle(QGeoCoordinate(1, 2), 4));
        QVERIFY(copy.area().isValid());
        QVERIFY(copy.area() != defaultMonitor.area());
        QVERIFY(copy.area().contains(QGeoCoordinate(1, 2)));

        QVERIFY(copy.notificationParameters().isEmpty());
        QVariantMap map;
        map.insert(QString("MyKey"), QVariant(123));
        copy.setNotificationParameters(map);
        QVERIFY(!copy.notificationParameters().isEmpty());
        QCOMPARE(copy.notificationParameters().value(QString("MyKey")).toInt(), 123);
        QCOMPARE(defaultMonitor.notificationParameters().value(QString("MyKey")).toInt(), 0);

        QCOMPARE(defaultMonitor.identifier(), copy.identifier());

        //assignment operator based
        QGeoAreaMonitorInfo assignmentCopy;
        assignmentCopy = copy;
        QVERIFY(copy == assignmentCopy);
        QVERIFY(assignmentCopy != defaultMonitor);

        QVERIFY(assignmentCopy.area().contains(QGeoCoordinate(1, 2)));
        QCOMPARE(assignmentCopy.expiration(), now);
        QCOMPARE(assignmentCopy.isPersistent(), false);
        QCOMPARE(assignmentCopy.notificationParameters().value(QString("MyKey")).toInt(), 123);
        QCOMPARE(defaultMonitor.identifier(), assignmentCopy.identifier());
        QCOMPARE(assignmentCopy.name(), QString("my name"));

        //validity checks for requestUpdate()
        obj.reset(QGeoAreaMonitorSource::createSource(QStringLiteral("positionpoll"), 0));
        QVERIFY(obj != nullptr);
        QCOMPARE(obj->sourceName(), QStringLiteral("positionpoll"));
        QCOMPARE(obj->activeMonitors().size(), 0);
        //reference -> should work
        QVERIFY(obj->requestUpdate(copy, SIGNAL(areaEntered(QGeoAreaMonitorInfo,QGeoPositionInfo))));
        QCOMPARE(obj->activeMonitors().size(), 1);
        //replaces areaEntered single shot
        QVERIFY(obj->requestUpdate(copy, SIGNAL(areaExited(QGeoAreaMonitorInfo,QGeoPositionInfo))));
        QCOMPARE(obj->activeMonitors().size(), 1);
        //replaces areaExited single shot
        QVERIFY(obj->startMonitoring(copy));
        QCOMPARE(obj->activeMonitors().size(), 1);


        //invalid signal
        QVERIFY(!obj->requestUpdate(copy, 0));
        QCOMPARE(obj->activeMonitors().size(), 1);

        //signal that doesn't exist
        QVERIFY(!obj->requestUpdate(copy, SIGNAL(areaEntered(QGeoMonitor))));
        QCOMPARE(obj->activeMonitors().size(), 1);

        QVERIFY(!obj->requestUpdate(copy, "SIGNAL(areaEntered(QGeoMonitor))"));
        QCOMPARE(obj->activeMonitors().size(), 1);

        //ensure that we cannot add a persistent monitor to a source
        //that doesn't support persistence
        QGeoAreaMonitorInfo persistenceMonitor(copy);
        persistenceMonitor.setPersistent(obj->supportedAreaMonitorFeatures() & QGeoAreaMonitorSource::PersistentAreaMonitorFeature);
        persistenceMonitor.setPersistent(!persistenceMonitor.isPersistent());

        QVERIFY(!obj->requestUpdate(persistenceMonitor, SIGNAL(areaEntered(QGeoAreaMonitorInfo,QGeoPositionInfo))));
        QCOMPARE(obj->activeMonitors().size(), 1);
        QVERIFY(!obj->startMonitoring(persistenceMonitor));
        QCOMPARE(obj->activeMonitors().size(), 1);

        //ensure that persistence was only reason for rejection
        persistenceMonitor.setPersistent(!persistenceMonitor.isPersistent());
        QVERIFY(obj->startMonitoring(persistenceMonitor));
        //persistenceMonitor is copy of already added monitor
        //the last call was an update
        QCOMPARE(obj->activeMonitors().size(), 1);
    }

    void tst_monitor_move_semantics()
    {
        QGeoAreaMonitorInfo info1("test");
        info1.setArea(QGeoCircle(QGeoCoordinate(1.0, 1.0), 100));
        info1.setExpiration(QDateTime::currentDateTimeUtc());
        QGeoAreaMonitorInfo infoCopy(info1);

        QGeoAreaMonitorInfo info2(std::move(info1));
        QCOMPARE(info2, infoCopy);

        QGeoAreaMonitorInfo info3;
        info3.setName("name");
        info3.setArea(QGeoRectangle(QGeoCoordinate(1, 2), QGeoCoordinate(2, 1)));
        info3.setPersistent(true);
        infoCopy = info3;

        // check that (move)assigning to the moved-from object is ok
        info1 = std::move(info3);
        QCOMPARE(info1, infoCopy);

        // The moved-from object info3 will go out of scope and  will be
        // destroyed here, so we also implicitly check that moved-from object's
        // destructor is called without any issues.
    }

    void tst_monitorValid()
    {
        QGeoAreaMonitorInfo mon;
        QVERIFY(!mon.isValid());
        QCOMPARE(mon.name(), QString());
        QCOMPARE(mon.area().isValid(), false);

        QGeoAreaMonitorInfo mon2 = mon;
        QVERIFY(!mon2.isValid());

        QGeoShape invalidShape;
        QGeoCircle emptyCircle(QGeoCoordinate(0,1), 0);
        QGeoCircle validCircle(QGeoCoordinate(0,1), 1);

        //all invalid since no name set yet
        mon2.setArea(invalidShape);
        QVERIFY(mon2.area() == invalidShape);
        QVERIFY(!mon2.isValid());

        mon2.setArea(emptyCircle);
        QVERIFY(mon2.area() == emptyCircle);
        QVERIFY(!mon2.isValid());

        mon2.setArea(validCircle);
        QVERIFY(mon2.area() == validCircle);
        QVERIFY(!mon2.isValid());

        //valid since name and non-empy shape has been set
        QGeoAreaMonitorInfo validMonitor("TestMonitor");
        QVERIFY(validMonitor.name() == QString("TestMonitor"));
        QVERIFY(!validMonitor.isValid());

        validMonitor.setArea(invalidShape);
        QVERIFY(validMonitor.area() == invalidShape);
        QVERIFY(!validMonitor.isValid());

        validMonitor.setArea(emptyCircle);
        QVERIFY(validMonitor.area() == emptyCircle);
        QVERIFY(!validMonitor.isValid());

        validMonitor.setArea(validCircle);
        QVERIFY(validCircle == validMonitor.area());
        QVERIFY(validMonitor.isValid());
    }

    void tst_monitorStreaming()
    {
        QByteArray container;
        QDataStream stream(&container, QIODevice::ReadWrite);

        QGeoAreaMonitorInfo monitor("someName");
        monitor.setArea(QGeoCircle(QGeoCoordinate(1,3), 5.4));
        const QDateTime expirationTime = QDateTime::currentDateTime().addSecs(60);
        monitor.setExpiration(expirationTime);
        monitor.setPersistent(true);
        const QVariantMap params { {"string_param", "some string"},
                                   {"int_param", 1}, {"double_param", 3.5} };
        monitor.setNotificationParameters(params);
        QVERIFY(monitor.isValid());
        QCOMPARE(monitor.name(), QString("someName"));
        QCOMPARE(monitor.expiration(), expirationTime);
        QVERIFY(monitor.isPersistent());
        QCOMPARE(monitor.notificationParameters(), params);

        QGeoAreaMonitorInfo target;
        QVERIFY(!target.isValid());
        QVERIFY(target.name().isEmpty());

        QVERIFY(target != monitor);

        stream << monitor;
        stream.device()->seek(0);
        stream >> target;

        QVERIFY(target == monitor);
        QVERIFY(target.isValid());
        QCOMPARE(target.name(), QString("someName"));
        QVERIFY(target.area() == QGeoCircle(QGeoCoordinate(1,3), 5.4));
        QCOMPARE(target.expiration(), expirationTime);
        QVERIFY(target.isPersistent());
        QCOMPARE(target.notificationParameters(), params);
    }

    void tst_createDefaultSource()
    {
        std::unique_ptr<QObject> parent(new QObject);

        // Have to use a raw pointer here, because otherwise we'd end up
        // deleting the obj twice when deleting the parent
        QGeoAreaMonitorSource *obj = QGeoAreaMonitorSource::createDefaultSource(parent.get());
        QVERIFY(obj != nullptr);
        QVERIFY(obj->parent() == parent.get());
        delete obj;

        const QStringList monitors = QGeoAreaMonitorSource::availableSources();
        QVERIFY(!monitors.isEmpty());
        QVERIFY(monitors.contains(QStringLiteral("positionpoll")));

        obj = QGeoAreaMonitorSource::createSource(QStringLiteral("positionpoll"), parent.get());
        QVERIFY(obj != nullptr);
        QCOMPARE(obj->sourceName(), QStringLiteral("positionpoll"));
        parent.reset();

        // using a smart pointer will cause a double delete here
        obj = QGeoAreaMonitorSource::createSource(QStringLiteral("randomNonExistingName"), 0);
        QVERIFY(obj == nullptr);
    }

    void tst_activeMonitors()
    {
        std::unique_ptr<QGeoAreaMonitorSource> obj(
                QGeoAreaMonitorSource::createSource(QStringLiteral("positionpoll"), 0));
        QVERIFY(obj != nullptr);
        QCOMPARE(obj->sourceName(), QStringLiteral("positionpoll"));

        // using this -> no need for smart pointer
        LogFilePositionSource *source = new LogFilePositionSource(m_fileData, this);
        source->setUpdateInterval(UPDATE_INTERVAL);
        obj->setPositionInfoSource(source);
        QCOMPARE(obj->positionInfoSource(), source);


        QVERIFY(obj->activeMonitors().isEmpty());

        QGeoAreaMonitorInfo mon("Monitor_Circle");
        mon.setArea(QGeoCircle(QGeoCoordinate(1,1), 1000));
        QVERIFY(obj->startMonitoring(mon));

        QGeoAreaMonitorInfo mon2("Monitor_rectangle_below");
        QGeoRectangle r_below(QGeoCoordinate(1,1),2,2);
        mon2.setArea(r_below);
        QVERIFY(obj->startMonitoring(mon2));

        QGeoAreaMonitorInfo mon3("Monitor_rectangle_above");
        QGeoRectangle r_above(QGeoCoordinate(2,1),2,2);
        mon3.setArea(r_above);
        QVERIFY(obj->startMonitoring(mon3));

#define CHECK(o, args, ...) do { \
            const QList<QGeoAreaMonitorInfo> results = o ->activeMonitors args; \
            const std::initializer_list<QGeoAreaMonitorInfo> expected = __VA_ARGS__ ; \
            QCOMPARE(results.size(), int(expected.size())); \
            QVERIFY(std::is_permutation(results.begin(), results.end(), \
                                        expected.begin(), expected.end())); \
        } while (false)

        CHECK(obj, (), {mon, mon2, mon3});
        CHECK(obj, (QGeoShape{}), {});
        CHECK(obj, (QGeoRectangle{QGeoCoordinate{1, 1}, 0.2, 0.2}), {mon, mon2});
        CHECK(obj, (QGeoCircle{QGeoCoordinate{1, 1}, 1000}), {mon, mon2});
        CHECK(obj, (QGeoCircle{QGeoCoordinate{2, 1},1000}), {mon3});

        //same as above except that we use a different monitor source object instance
        //all monitor objects of same type share same active monitors
        std::unique_ptr<QGeoAreaMonitorSource> secondObj(
                QGeoAreaMonitorSource::createSource(QStringLiteral("positionpoll"), 0));
        QVERIFY(secondObj != nullptr);
        QCOMPARE(secondObj->sourceName(), QStringLiteral("positionpoll"));

        CHECK(secondObj, (), {mon, mon2, mon3});
        CHECK(secondObj, (QGeoShape{}), {});
        CHECK(secondObj, (QGeoRectangle{QGeoCoordinate{1, 1}, 0.2, 0.2}), {mon, mon2});
        CHECK(secondObj, (QGeoCircle{QGeoCoordinate{1, 1}, 1000}), {mon, mon2});
        CHECK(secondObj, (QGeoCircle{QGeoCoordinate{2, 1}, 1000}), {mon3});

#undef CHECK
    }

    void tst_testExpiryTimeout()
    {
        std::unique_ptr<QGeoAreaMonitorSource> obj(
                QGeoAreaMonitorSource::createSource(QStringLiteral("positionpoll"), 0));
        QVERIFY(obj != nullptr);
        QCOMPARE(obj->sourceName(), QStringLiteral("positionpoll"));

        std::unique_ptr<QGeoAreaMonitorSource> secondObj(
                QGeoAreaMonitorSource::createSource(QStringLiteral("positionpoll"), 0));
        QVERIFY(secondObj != nullptr);
        QCOMPARE(secondObj->sourceName(), QStringLiteral("positionpoll"));

        // using this -> no need for smart pointer
        LogFilePositionSource *source = new LogFilePositionSource(m_fileData, this);
        source->setUpdateInterval(UPDATE_INTERVAL);
        obj->setPositionInfoSource(source);

        //Singleton pattern behind QGeoAreaMonitorSource ensures same position info source
        QCOMPARE(obj->positionInfoSource(), source);
        QCOMPARE(secondObj->positionInfoSource(), source);

        QSignalSpy expirySpy(obj.get(), SIGNAL(monitorExpired(QGeoAreaMonitorInfo)));
        QSignalSpy expirySpy2(secondObj.get(), SIGNAL(monitorExpired(QGeoAreaMonitorInfo)));

        QDateTime now = QDateTime::currentDateTime();

        const int monitorCount = 4;
        for (int i = 1; i <= monitorCount; i++) {
            QGeoAreaMonitorInfo mon(QString::number(i));
            mon.setArea(QGeoRectangle(QGeoCoordinate(i,i), i, i));
            mon.setExpiration(now.addSecs(i*2));
            QVERIFY(mon.isValid());
            QVERIFY(obj->startMonitoring(mon));
        }



        QCOMPARE(obj->activeMonitors().size(), monitorCount);
        QCOMPARE(secondObj->activeMonitors().size(), monitorCount);

        QGeoAreaMonitorInfo info("InvalidExpiry");
        info.setArea(QGeoRectangle(QGeoCoordinate(10,10), 1, 1 ));
        QVERIFY(info.isValid());
        info.setExpiration(now.addSecs(-1000));
        QVERIFY(info.expiration() < now);
        QVERIFY(!obj->startMonitoring(info));
        QCOMPARE(obj->activeMonitors().size(), monitorCount);
        QVERIFY(!obj->requestUpdate(info, SIGNAL(areaEntered(QGeoAreaMonitorInfo,QGeoPositionInfo))));
        QCOMPARE(obj->activeMonitors().size(), monitorCount);

        for (int i = 1; i <= monitorCount; i++) {
            QTRY_VERIFY_WITH_TIMEOUT(expirySpy.size() == 1, 3000); //each expiry within 2 s
            QGeoAreaMonitorInfo mon = expirySpy.takeFirst().at(0).value<QGeoAreaMonitorInfo>();
            QCOMPARE(obj->activeMonitors().size(), monitorCount-i);
            QCOMPARE(mon.name(), QString::number(i));
        }

        QCOMPARE(expirySpy2.size(), monitorCount);
        QCOMPARE(secondObj->activeMonitors().size(), 0); //all monitors expired
        for (int i = 1; i <= monitorCount; i++) {
            QGeoAreaMonitorInfo mon = expirySpy2.takeFirst().at(0).value<QGeoAreaMonitorInfo>();
            QCOMPARE(mon.name(), QString::number(i));
        }
    }

    void tst_enteredExitedSignal()
    {
        std::unique_ptr<QGeoAreaMonitorSource> obj(
                QGeoAreaMonitorSource::createSource(QStringLiteral("positionpoll"), 0));
        QVERIFY(obj != nullptr);
        QCOMPARE(obj->sourceName(), QStringLiteral("positionpoll"));
        obj->setObjectName("firstObject");
        QSignalSpy enteredSpy(obj.get(),
                              SIGNAL(areaEntered(QGeoAreaMonitorInfo,QGeoPositionInfo)));
        QSignalSpy exitedSpy(obj.get(), SIGNAL(areaExited(QGeoAreaMonitorInfo,QGeoPositionInfo)));

        // using this -> no need for smart pointer
        LogFilePositionSource *source = new LogFilePositionSource(m_fileData, this);
        source->setUpdateInterval(UPDATE_INTERVAL);
        obj->setPositionInfoSource(source);
        QCOMPARE(obj->positionInfoSource(), source);

        std::unique_ptr<QGeoAreaMonitorSource> secondObj(
                QGeoAreaMonitorSource::createSource(QStringLiteral("positionpoll"), 0));
        QVERIFY(secondObj != nullptr);
        QCOMPARE(secondObj->sourceName(), QStringLiteral("positionpoll"));
        QSignalSpy enteredSpy2(secondObj.get(),
                               SIGNAL(areaEntered(QGeoAreaMonitorInfo,QGeoPositionInfo)));
        QSignalSpy exitedSpy2(secondObj.get(),
                              SIGNAL(areaExited(QGeoAreaMonitorInfo,QGeoPositionInfo)));
        secondObj->setObjectName("secondObject");

        QGeoAreaMonitorInfo infoRectangle("Rectangle");
        infoRectangle.setArea(QGeoRectangle(QGeoCoordinate(-27.65, 153.093), 0.2, 0.2));
        QVERIFY(infoRectangle.isValid());
        QVERIFY(obj->startMonitoring(infoRectangle));

        QGeoAreaMonitorInfo infoCircle("Circle");
        infoCircle.setArea(QGeoCircle(QGeoCoordinate(-27.70, 153.093),10000));
        QVERIFY(infoCircle.isValid());
        QVERIFY(obj->startMonitoring(infoCircle));

        QGeoAreaMonitorInfo singleShot_enter("SingleShot_on_Entered");
        singleShot_enter.setArea(QGeoRectangle(QGeoCoordinate(-27.67, 153.093), 0.2, 0.2));
        QVERIFY(singleShot_enter.isValid());
        QVERIFY(obj->requestUpdate(singleShot_enter,
                                   SIGNAL(areaEntered(QGeoAreaMonitorInfo,QGeoPositionInfo))));

        QGeoAreaMonitorInfo singleShot_exit("SingleShot_on_Exited");
        singleShot_exit.setArea(QGeoRectangle(QGeoCoordinate(-27.70, 153.093), 0.2, 0.2));
        QVERIFY(singleShot_exit.isValid());
        QVERIFY(obj->requestUpdate(singleShot_exit,
                                   SIGNAL(areaExited(QGeoAreaMonitorInfo,QGeoPositionInfo))));

        QVERIFY(obj->activeMonitors().size() == 4); //all monitors active
        QVERIFY(secondObj->activeMonitors().size() == 4); //all monitors active

        static const int Number_Of_Entered_Events = 6;
        static const int Number_Of_Exited_Events = 5;
        //takes 87 (lines)*50(timeout)/1000 seconds to finish
        QTRY_VERIFY_WITH_TIMEOUT(enteredSpy.size() == Number_Of_Entered_Events, 5000);
        QTRY_VERIFY_WITH_TIMEOUT(exitedSpy.size() == Number_Of_Exited_Events, 5000);
        QCOMPARE(enteredSpy.size(), Number_Of_Entered_Events);
        QCOMPARE(exitedSpy.size(), Number_Of_Exited_Events);

        QList<QGeoAreaMonitorInfo> monitorsInExpectedEnteredEventOrder;
        monitorsInExpectedEnteredEventOrder << infoRectangle << singleShot_enter << singleShot_exit
                                            << infoCircle << infoCircle << infoRectangle;

        QList<QGeoAreaMonitorInfo> monitorsInExpectedExitedEventOrder;
        monitorsInExpectedExitedEventOrder << infoRectangle << infoCircle
                                            << singleShot_exit << infoCircle << infoRectangle;

        QList<QGeoCoordinate> enteredEventCoordinateOrder;
        enteredEventCoordinateOrder << QGeoCoordinate(-27.55, 153.090718) //infoRectangle
                                    << QGeoCoordinate(-27.57, 153.090718) //singleshot_enter
                                    << QGeoCoordinate(-27.60, 153.090908) //singleshot_exit
                                    << QGeoCoordinate(-27.62, 153.091036) //infoCircle
                                    << QGeoCoordinate(-27.78, 153.093647) //infoCircle
                                    << QGeoCoordinate(-27.75, 153.093896);//infoRectangle
        QCOMPARE(enteredEventCoordinateOrder.size(), Number_Of_Entered_Events);
        QCOMPARE(monitorsInExpectedEnteredEventOrder.size(), Number_Of_Entered_Events);

        QList<QGeoCoordinate> exitedEventCoordinateOrder;
        exitedEventCoordinateOrder  << QGeoCoordinate(-27.78, 153.092218) //infoRectangle
                                    << QGeoCoordinate(-27.79, 153.092308) //infoCircle
                                    << QGeoCoordinate(-27.81, 153.092530) //singleshot_exit
                                    << QGeoCoordinate(-27.61, 153.095231) //infoCircle
                                    << QGeoCoordinate(-27.54, 153.095995);//infoCircle
        QCOMPARE(exitedEventCoordinateOrder.size(), Number_Of_Exited_Events);
        QCOMPARE(monitorsInExpectedExitedEventOrder.size(), Number_Of_Exited_Events);

        //verify that both sources got the same signals
        for (int i = 0; i < Number_Of_Entered_Events; i++) {
            //first source
            QGeoAreaMonitorInfo monInfo = enteredSpy.first().at(0).value<QGeoAreaMonitorInfo>();
            QGeoPositionInfo posInfo = enteredSpy.takeFirst().at(1).value<QGeoPositionInfo>();
            QVERIFY2(monInfo == monitorsInExpectedEnteredEventOrder.at(i),
                     qPrintable(QString::number(i) + ": " + monInfo.name()));
            QVERIFY2(posInfo.coordinate() == enteredEventCoordinateOrder.at(i),
                     qPrintable(QString::number(i) + ". posInfo"));

            //reset info objects to avoid comparing the same
            monInfo = QGeoAreaMonitorInfo();
            posInfo = QGeoPositionInfo();

            //second source
            monInfo = enteredSpy2.first().at(0).value<QGeoAreaMonitorInfo>();
            posInfo = enteredSpy2.takeFirst().at(1).value<QGeoPositionInfo>();
            QVERIFY2(monInfo == monitorsInExpectedEnteredEventOrder.at(i),
                     qPrintable(QString::number(i) + ": " + monInfo.name()));
            QVERIFY2(posInfo.coordinate() == enteredEventCoordinateOrder.at(i),
                     qPrintable(QString::number(i) + ". posInfo"));
        }

        for (int i = 0; i < Number_Of_Exited_Events; i++) {
            //first source
            QGeoAreaMonitorInfo monInfo = exitedSpy.first().at(0).value<QGeoAreaMonitorInfo>();
            QGeoPositionInfo posInfo = exitedSpy.takeFirst().at(1).value<QGeoPositionInfo>();
            QVERIFY2(monInfo == monitorsInExpectedExitedEventOrder.at(i),
                     qPrintable(QString::number(i) + ": " + monInfo.name()));
            QVERIFY2(posInfo.coordinate() == exitedEventCoordinateOrder.at(i),
                     qPrintable(QString::number(i) + ". posInfo"));

            //reset info objects to avoid comparing the same
            monInfo = QGeoAreaMonitorInfo();
            posInfo = QGeoPositionInfo();

            //second source
            monInfo = exitedSpy2.first().at(0).value<QGeoAreaMonitorInfo>();
            posInfo = exitedSpy2.takeFirst().at(1).value<QGeoPositionInfo>();
            QVERIFY2(monInfo == monitorsInExpectedExitedEventOrder.at(i),
                     qPrintable(QString::number(i) + ": " + monInfo.name()));
            QVERIFY2(posInfo.coordinate() == exitedEventCoordinateOrder.at(i),
                     qPrintable(QString::number(i) + ". posInfo"));
        }

        QCOMPARE(obj->activeMonitors().size(), 2); //single shot monitors have been removed
        QCOMPARE(secondObj->activeMonitors().size(), 2);
    }

    void tst_swapOfPositionSource()
    {
        std::unique_ptr<QGeoAreaMonitorSource> obj(
                QGeoAreaMonitorSource::createSource(QStringLiteral("positionpoll"), 0));
        QVERIFY(obj != nullptr);
        QCOMPARE(obj->sourceName(), QStringLiteral("positionpoll"));
        obj->setObjectName("firstObject");
        QSignalSpy enteredSpy(obj.get(),
                              SIGNAL(areaEntered(QGeoAreaMonitorInfo,QGeoPositionInfo)));
        QSignalSpy exitedSpy(obj.get(), SIGNAL(areaExited(QGeoAreaMonitorInfo,QGeoPositionInfo)));

        std::unique_ptr<QGeoAreaMonitorSource> obj2(
                QGeoAreaMonitorSource::createSource(QStringLiteral("positionpoll"), 0));
        QVERIFY(obj2 != nullptr);
        QCOMPARE(obj2->sourceName(), QStringLiteral("positionpoll"));
        obj2->setObjectName("secondObject");
        QSignalSpy enteredSpy2(obj2.get(),
                               SIGNAL(areaEntered(QGeoAreaMonitorInfo,QGeoPositionInfo)));
        QSignalSpy exitedSpy2(obj2.get(),
                              SIGNAL(areaExited(QGeoAreaMonitorInfo,QGeoPositionInfo)));

        // using this -> no need for smart pointer
        LogFilePositionSource *source = new LogFilePositionSource(m_fileData, this);
        source->setUpdateInterval(UPDATE_INTERVAL);
        source->setObjectName("FirstLogFileSource");

        // using this -> no need for smart pointer
        LogFilePositionSource *source2 = new LogFilePositionSource(m_fileData, this);
        source2->setUpdateInterval(UPDATE_INTERVAL);
        source2->setObjectName("SecondLogFileSource");

        obj->setPositionInfoSource(source);
        QCOMPARE(obj->positionInfoSource(), obj2->positionInfoSource());
        QCOMPARE(obj2->positionInfoSource(), source);

        QGeoAreaMonitorInfo infoRectangle("Rectangle");
        infoRectangle.setArea(QGeoRectangle(QGeoCoordinate(-27.70, 153.092), 0.2, 0.2));
        QVERIFY(infoRectangle.isValid());
        QVERIFY(obj->startMonitoring(infoRectangle));

        QCOMPARE(obj->activeMonitors().size(), 1);
        QCOMPARE(obj2->activeMonitors().size(), 1);

        QGeoCoordinate firstBorder(-27.6, 153.090908);
        QGeoCoordinate secondBorder(-27.81, 153.092530);

        /***********************************/
        //1. trigger events on source (until areaExit
        QTRY_VERIFY_WITH_TIMEOUT(exitedSpy.size() == 1, 5000);
        QCOMPARE(enteredSpy.size(), enteredSpy2.size());
        QCOMPARE(exitedSpy.size(), exitedSpy2.size());

        //compare entered event
        QVERIFY(enteredSpy.first().at(0).value<QGeoAreaMonitorInfo>() ==
                enteredSpy2.first().at(0).value<QGeoAreaMonitorInfo>());
        QGeoPositionInfo info = enteredSpy.takeFirst().at(1).value<QGeoPositionInfo>();
        QVERIFY(info == enteredSpy2.takeFirst().at(1).value<QGeoPositionInfo>());
        QVERIFY(info.coordinate() == firstBorder);
        //compare exit event
        QVERIFY(exitedSpy.first().at(0).value<QGeoAreaMonitorInfo>() ==
                exitedSpy2.first().at(0).value<QGeoAreaMonitorInfo>());
        info = exitedSpy.takeFirst().at(1).value<QGeoPositionInfo>();
        QVERIFY(info == exitedSpy2.takeFirst().at(1).value<QGeoPositionInfo>());
        QVERIFY(info.coordinate() == secondBorder);

        QCOMPARE(exitedSpy.size(), 0);
        QCOMPARE(enteredSpy.size(), 0);
        QCOMPARE(exitedSpy2.size(), 0);
        QCOMPARE(enteredSpy2.size(), 0);

        /***********************************/
        //2. change position source -> which restarts at beginning again
        obj2->setPositionInfoSource(source2);
        QCOMPARE(obj->positionInfoSource(), obj2->positionInfoSource());
        QCOMPARE(obj2->positionInfoSource(), source2);

        QTRY_VERIFY_WITH_TIMEOUT(exitedSpy.size() == 1, 5000);
        QCOMPARE(enteredSpy.size(), enteredSpy2.size());
        QCOMPARE(exitedSpy.size(), exitedSpy2.size());

        //compare entered event
        QVERIFY(enteredSpy.first().at(0).value<QGeoAreaMonitorInfo>() ==
                enteredSpy2.first().at(0).value<QGeoAreaMonitorInfo>());
        info = enteredSpy.takeFirst().at(1).value<QGeoPositionInfo>();
        QVERIFY(info == enteredSpy2.takeFirst().at(1).value<QGeoPositionInfo>());
        QVERIFY(info.coordinate() == firstBorder);
        //compare exit event
        QVERIFY(exitedSpy.first().at(0).value<QGeoAreaMonitorInfo>() ==
                exitedSpy2.first().at(0).value<QGeoAreaMonitorInfo>());
        info = exitedSpy.takeFirst().at(1).value<QGeoPositionInfo>();
        QVERIFY(info == exitedSpy2.takeFirst().at(1).value<QGeoPositionInfo>());
        QVERIFY(info.coordinate() == secondBorder);
    }

    void debug_data()
    {
        QTest::addColumn<QGeoAreaMonitorInfo>("info");
        QTest::addColumn<int>("nextValue");
        QTest::addColumn<QString>("debugString");

        QGeoAreaMonitorInfo info;
        QTest::newRow("uninitialized") << info << 45
                << QString("QGeoAreaMonitorInfo(\"\", QGeoShape(Unknown), "
                              "persistent: false, expiry: QDateTime(Invalid)) 45");

        info.setArea(QGeoRectangle());
        info.setPersistent(true);
        info.setName("RectangleAreaMonitor");
        QTest::newRow("Rectangle Test") << info  << 45
                << QString("QGeoAreaMonitorInfo(\"RectangleAreaMonitor\", QGeoShape(Rectangle), "
                              "persistent: true, expiry: QDateTime(Invalid)) 45");

        info = QGeoAreaMonitorInfo();
        info.setArea(QGeoCircle());
        info.setPersistent(false);
        info.setName("CircleAreaMonitor");
        QVariantMap map;
        map.insert(QString("foobarKey"), QVariant(45)); //should be ignored
        info.setNotificationParameters(map);
        QTest::newRow("Circle Test") << info  << 45
                << QString("QGeoAreaMonitorInfo(\"CircleAreaMonitor\", QGeoShape(Circle), "
                              "persistent: false, expiry: QDateTime(Invalid)) 45");

        // we ignore any further QDateTime related changes to avoid depending on QDateTime related
        // failures in case its QDebug string changes
    }

    void debug()
    {
        QFETCH(QGeoAreaMonitorInfo, info);
        QFETCH(int, nextValue);
        QFETCH(QString, debugString);

        qInstallMessageHandler(tst_qgeoareamonitorinfo_messageHandler);
        qDebug() << info << nextValue;
        qInstallMessageHandler(0);
        QCOMPARE(tst_qgeoareamonitorinfo_debug, debugString);
    }

    void multipleThreads()
    {
#if defined(Q_OS_MACOS) && defined(Q_PROCESSOR_ARM)
        QSKIP("This test randomly hangs on macOS ARM (QTBUG-110931)");
#endif
        std::unique_ptr<QGeoAreaMonitorSource> obj(
                QGeoAreaMonitorSource::createSource(QStringLiteral("positionpoll"), 0));
        QVERIFY(obj != nullptr);
        QCOMPARE(obj->sourceName(), QStringLiteral("positionpoll"));

        QVERIFY(obj->activeMonitors().isEmpty());

        LogFilePositionSource *source = new LogFilePositionSource(m_fileData, this);
        source->setUpdateInterval(UPDATE_INTERVAL);
        obj->setPositionInfoSource(source);
        QCOMPARE(obj->positionInfoSource(), source);

        QSignalSpy noDataSpy(source, &LogFilePositionSource::noDataLeft);
        QSignalSpy updatesStartedSpy(source, &LogFilePositionSource::updatesStarted);
        QSignalSpy updatesStoppedSpy(source, &LogFilePositionSource::updatesStopped);

        // generate threads
        const int threadCount = 10;
        QList<PositionConsumerThread *> threads;
        for (int i = 0; i < threadCount; ++i) {
            auto threadObj = new PositionConsumerThread(obj.get(), this);
            threadObj->start();
            threads.push_back(threadObj);
        }

        // generate objects to monitor
        QGeoAreaMonitorInfo infoRectangle("Rectangle");
        infoRectangle.setArea(QGeoRectangle(QGeoCoordinate(-27.65, 153.093), 0.2, 0.2));
        QVERIFY(infoRectangle.isValid());
        QVERIFY(obj->startMonitoring(infoRectangle));

        QGeoAreaMonitorInfo infoCircle("Circle");
        infoCircle.setArea(QGeoCircle(QGeoCoordinate(-27.70, 153.093), 10000));
        QVERIFY(infoCircle.isValid());
        QVERIFY(obj->startMonitoring(infoCircle));

        QGeoAreaMonitorInfo singleShot_enter("SingleShot_on_Entered");
        singleShot_enter.setArea(QGeoRectangle(QGeoCoordinate(-27.67, 153.093), 0.2, 0.2));
        QVERIFY(singleShot_enter.isValid());
        QVERIFY(obj->requestUpdate(singleShot_enter,
                                   SIGNAL(areaEntered(QGeoAreaMonitorInfo,QGeoPositionInfo))));

        QGeoAreaMonitorInfo singleShot_exit("SingleShot_on_Exited");
        singleShot_exit.setArea(QGeoRectangle(QGeoCoordinate(-27.70, 153.093), 0.2, 0.2));
        QVERIFY(singleShot_exit.isValid());
        QVERIFY(obj->requestUpdate(singleShot_exit,
                                   SIGNAL(areaExited(QGeoAreaMonitorInfo,QGeoPositionInfo))));

        // wait until we read all data
        QTRY_COMPARE_WITH_TIMEOUT(noDataSpy.size(), 1, 5000);

        // first request all the threads to terminate
        for (int i = 0; i < threadCount; ++i)
            threads[i]->stopProcessing();

        static const int Number_Of_Entered_Events = 6;
        static const int Number_Of_Exited_Events = 5;
        // wait until each thread is stopped, and compare the result values
        for (int i = 0; i < threadCount; ++i) {
            threads[i]->wait();
            QCOMPARE(threads[i]->detectedEnterCount(), Number_Of_Entered_Events);
            QCOMPARE(threads[i]->detectedExitCount(), Number_Of_Exited_Events);
        }

        // Verify that the source started and stopped updates only once.
        // This is needed to check that the connection tracking logic in
        // connectNotify()/disconnectNotify() is working properly.
        QCOMPARE(updatesStartedSpy.size(), 1);
        QCOMPARE(updatesStoppedSpy.size(), 1);
    }

    void backendProperties()
    {
        std::unique_ptr<QGeoAreaMonitorSource> obj = std::make_unique<DummyMonitorSource>();

        const QString invalidProperty = "SomePropertyName";

        QCOMPARE(obj->backendProperty(DummyMonitorSource::kTestProperty), 0);
        QCOMPARE(obj->backendProperty(invalidProperty), QVariant());

        QVERIFY(obj->setBackendProperty(DummyMonitorSource::kTestProperty, 10));
        QVERIFY(!obj->setBackendProperty(invalidProperty, 15));

        QCOMPARE(obj->backendProperty(DummyMonitorSource::kTestProperty), 10);
        QCOMPARE(obj->backendProperty(invalidProperty), QVariant());
    }

    void randomSignalName()
    {
        auto src = std::unique_ptr<QGeoAreaMonitorSource>(
                QGeoAreaMonitorSource::createSource(QStringLiteral("positionpoll"), nullptr));
        QVERIFY(src);

        QGeoAreaMonitorInfo infoCircle("Circle");
        infoCircle.setArea(QGeoCircle(QGeoCoordinate(-27.70, 153.093), 10000));
        QVERIFY(infoCircle.isValid());

        // empty signal -> invalid
        QVERIFY(!src->requestUpdate(infoCircle, ""));

        // non-existent single-char signal -> invalid
        QVERIFY(!src->requestUpdate(infoCircle, "a"));

        // non-existent multi-char signal -> invalid
        QVERIFY(!src->requestUpdate(infoCircle, "abcd"));
    }
};


QTEST_GUILESS_MAIN(tst_QGeoAreaMonitorSource)
#include "tst_qgeoareamonitor.moc"
