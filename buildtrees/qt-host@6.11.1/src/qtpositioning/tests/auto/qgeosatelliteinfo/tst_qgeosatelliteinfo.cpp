// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//TESTED_COMPONENT=src/location

#include <QtPositioning/qgeosatelliteinfo.h>

#include <QMetaType>
#include <QObject>
#include <QDebug>
#include <QTest>

#include <float.h>
#include <limits.h>

QT_USE_NAMESPACE
Q_DECLARE_METATYPE(QGeoSatelliteInfo::Attribute)

QByteArray tst_qgeosatelliteinfo_debug;

void tst_qgeosatelliteinfo_messageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    switch (type) {
        case QtDebugMsg :
            tst_qgeosatelliteinfo_debug = msg.toLocal8Bit();
            break;
        default:
            break;
    }
}


QList<qreal> tst_qgeosatelliteinfo_qrealTestValues()
{
    QList<qreal> values;

    if (qreal(DBL_MIN) == DBL_MIN)
        values << DBL_MIN;

    values << FLT_MIN;
    values << -1.0 << 0.0 << 1.0;
    values << FLT_MAX;

    if (qreal(DBL_MAX) == DBL_MAX)
        values << DBL_MAX;

    return values;
}

QList<int> tst_qgeosatelliteinfo_intTestValues()
{
    QList<int> values;
    values << INT_MIN << -100 << 0 << 100 << INT_MAX;
    return values;
}

QList<QGeoSatelliteInfo::Attribute> tst_qgeosatelliteinfo_getAttributes()
{
    QList<QGeoSatelliteInfo::Attribute> attributes;
    attributes << QGeoSatelliteInfo::Elevation
            << QGeoSatelliteInfo::Azimuth;
    return attributes;
}


class tst_QGeoSatelliteInfo : public QObject
{
    Q_OBJECT

private:
    QGeoSatelliteInfo updateWithAttribute(QGeoSatelliteInfo::Attribute attribute, qreal value)
    {
        QGeoSatelliteInfo info;
        info.setAttribute(attribute, value);
        return info;
    }

    void addTestData_update()
    {
        QTest::addColumn<QGeoSatelliteInfo>("info");

        QList<int> intValues = tst_qgeosatelliteinfo_intTestValues();

        for (int rssi : intValues) {
            QGeoSatelliteInfo info;
            info.setSignalStrength(rssi);
            QTest::addRow("signal_strength_%d", rssi) << info;
        }

        for (int satId : intValues) {
            QGeoSatelliteInfo info;
            info.setSatelliteIdentifier(satId);
            QTest::addRow("satellite_identifier_%d", satId) << info;
        }

        auto satSystemMetaEnum = QMetaEnum::fromType<QGeoSatelliteInfo::SatelliteSystem>();
        for (auto system : {QGeoSatelliteInfo::GPS, QGeoSatelliteInfo::GLONASS}) {
            QGeoSatelliteInfo info;
            info.setSatelliteSystem(system);
            QTest::addRow("satellite_system_%s", satSystemMetaEnum.valueToKey(system)) << info;
        }

        QList<QGeoSatelliteInfo::Attribute> attributes = tst_qgeosatelliteinfo_getAttributes();
        QList<qreal> qrealValues = tst_qgeosatelliteinfo_qrealTestValues();
        QCOMPARE_GE(qrealValues.size(), attributes.size()); // we need to have enough values
        auto attributesMetaEnum = QMetaEnum::fromType<QGeoSatelliteInfo::Attribute>();
        for (qsizetype i = 0; i < attributes.size(); ++i) {
            QTest::addRow("%s=%g", attributesMetaEnum.valueToKey(attributes[i]), qrealValues[i])
                    << updateWithAttribute(attributes[i], qrealValues[i]);
        }
    }

    void testIntData()
    {
        QTest::addColumn<int>("value");

        QList<int> intValues = tst_qgeosatelliteinfo_intTestValues();
        for (int val : intValues)
            QTest::addRow("%d", val) << val;
    }

private slots:
    void constructor()
    {
        QGeoSatelliteInfo info;
        QCOMPARE(info.signalStrength(), -1);
        QCOMPARE(info.satelliteIdentifier(), -1);
        QCOMPARE(info.satelliteSystem(), QGeoSatelliteInfo::Undefined);
        for (auto attr : tst_qgeosatelliteinfo_getAttributes())
            QCOMPARE(info.attribute(attr), qreal(-1.0));
    }
    void constructor_copy()
    {
        QFETCH(QGeoSatelliteInfo, info);

        QCOMPARE(QGeoSatelliteInfo(info), info);
    }

    void constructor_copy_data()
    {
        addTestData_update();
    }

    void constructor_move()
    {
        QFETCH(QGeoSatelliteInfo, info);
        QGeoSatelliteInfo infoCopy = info;
        QCOMPARE(QGeoSatelliteInfo(std::move(info)), infoCopy);
        // The moved-from object will go out of scope and  will be destroyed
        // here, so we also implicitly check that moved-from object's destructor
        // is called without any issues.
    }

    void constructor_move_data()
    {
        addTestData_update();
    }

    void operator_comparison()
    {
        QFETCH(QGeoSatelliteInfo, info);

        QVERIFY(info == info);
        QCOMPARE(info != info, false);
        QCOMPARE(info == QGeoSatelliteInfo(), false);
        QCOMPARE(info != QGeoSatelliteInfo(), true);

        QVERIFY(QGeoSatelliteInfo() == QGeoSatelliteInfo());
    }

    void operator_comparison_data()
    {
        addTestData_update();
    }

    void operator_assign()
    {
        QFETCH(QGeoSatelliteInfo, info);

        QGeoSatelliteInfo info2 = info;
        QCOMPARE(info2, info);
    }

    void operator_assign_data()
    {
        addTestData_update();
    }

    void operator_move_assign()
    {
        QFETCH(QGeoSatelliteInfo, info);
        QGeoSatelliteInfo infoCopy = info;

        QGeoSatelliteInfo obj;
        obj = std::move(info);
        QCOMPARE(obj, infoCopy);

        // check that (move)assigning to the moved-from object is ok
        info = std::move(infoCopy);
        QCOMPARE(info, obj);
    }

    void operator_move_assign_data()
    {
        addTestData_update();
    }

    void setSignalStrength()
    {
        QFETCH(int, value);

        QGeoSatelliteInfo info;
        QCOMPARE(info.signalStrength(), -1);

        info.setSignalStrength(value);
        QCOMPARE(info.signalStrength(), value);
    }

    void setSignalStrength_data()
    {
        testIntData();
    }

    void setSatelliteIdentifier()
    {
        QFETCH(int, value);

        QGeoSatelliteInfo info;
        QCOMPARE(info.satelliteIdentifier(), -1);

        info.setSatelliteIdentifier(value);
        QCOMPARE(info.satelliteIdentifier(), value);
    }

    void setSatelliteIdentifier_data()
    {
        testIntData();
    }

    void setSatelliteSystem()
    {
        QFETCH(QGeoSatelliteInfo::SatelliteSystem, system);

        QGeoSatelliteInfo info;
        QCOMPARE(info.satelliteSystem(), QGeoSatelliteInfo::Undefined);

        info.setSatelliteSystem(static_cast<QGeoSatelliteInfo::SatelliteSystem>(system));
        QCOMPARE(info.satelliteSystem(), static_cast<QGeoSatelliteInfo::SatelliteSystem>(system));
    }

    void setSatelliteSystem_data()
    {
        QTest::addColumn<QGeoSatelliteInfo::SatelliteSystem>("system");

        const auto metaEnum = QMetaEnum::fromType<QGeoSatelliteInfo::SatelliteSystem>();
        for (int i = 0; i < metaEnum.keyCount(); ++i) {
            QTest::newRow(metaEnum.key(i))
                    << static_cast<QGeoSatelliteInfo::SatelliteSystem>(metaEnum.value(i));
        }
    }

    void attribute()
    {
        QFETCH(QGeoSatelliteInfo::Attribute, attribute);
        QFETCH(qreal, value);

        QGeoSatelliteInfo u;
        QCOMPARE(u.attribute(attribute), qreal(-1.0));

        u.setAttribute(attribute, value);
        QCOMPARE(u.attribute(attribute), value);
        u.removeAttribute(attribute);
        QCOMPARE(u.attribute(attribute), qreal(-1.0));
    }

    void attribute_data()
    {
        QTest::addColumn<QGeoSatelliteInfo::Attribute>("attribute");
        QTest::addColumn<qreal>("value");

        const auto metaEnum = QMetaEnum::fromType<QGeoSatelliteInfo::Attribute>();
        for (int i = 0; i < metaEnum.keyCount(); ++i) {
            for (qreal val : tst_qgeosatelliteinfo_qrealTestValues()) {
                QTest::addRow("%s=%g", metaEnum.key(i), val)
                        << static_cast<QGeoSatelliteInfo::Attribute>(metaEnum.value(i)) << val;
            }
        }
    }

    void hasAttribute()
    {
        QFETCH(QGeoSatelliteInfo::Attribute, attribute);
        QFETCH(qreal, value);

        QGeoSatelliteInfo u;
        QVERIFY(!u.hasAttribute(attribute));

        u.setAttribute(attribute, value);
        QVERIFY(u.hasAttribute(attribute));

        u.removeAttribute(attribute);
        QVERIFY(!u.hasAttribute(attribute));
    }

    void hasAttribute_data()
    {
        attribute_data();
    }

    void removeAttribute()
    {
        QFETCH(QGeoSatelliteInfo::Attribute, attribute);
        QFETCH(qreal, value);

        QGeoSatelliteInfo u;
        QVERIFY(!u.hasAttribute(attribute));

        u.setAttribute(attribute, value);
        QVERIFY(u.hasAttribute(attribute));

        u.removeAttribute(attribute);
        QVERIFY(!u.hasAttribute(attribute));

        u.setAttribute(attribute, value);
        QVERIFY(u.hasAttribute(attribute));
    }

    void removeAttribute_data()
    {
        attribute_data();
    }

    void datastream()
    {
        QFETCH(QGeoSatelliteInfo, info);

        QByteArray ba;
        QDataStream out(&ba, QIODevice::WriteOnly);
        out << info;

        QDataStream in(&ba, QIODevice::ReadOnly);
        QGeoSatelliteInfo inInfo;
        in >> inInfo;
        QCOMPARE(inInfo, info);
    }

    void datastream_data()
    {
        addTestData_update();
    }

    void debug()
    {
        QFETCH(QGeoSatelliteInfo, info);
        QFETCH(int, nextValue);
        QFETCH(QByteArray, debugString);

        qInstallMessageHandler(tst_qgeosatelliteinfo_messageHandler);
        qDebug() << info << nextValue;
        qInstallMessageHandler(0);
        QCOMPARE(QString(tst_qgeosatelliteinfo_debug), QString(debugString));
    }

    void debug_data()
    {
        QTest::addColumn<QGeoSatelliteInfo>("info");
        QTest::addColumn<int>("nextValue");
        QTest::addColumn<QByteArray>("debugString");

        QGeoSatelliteInfo info;

        QTest::newRow("uninitialized") << info << 45
                << QByteArray("QGeoSatelliteInfo(system=QGeoSatelliteInfo::Undefined, "
                              "satId=-1, signal-strength=-1) 45");

        info = QGeoSatelliteInfo();
        info.setSignalStrength(1);
        QTest::newRow("with_SignalStrength") << info << 60
                << QByteArray("QGeoSatelliteInfo(system=QGeoSatelliteInfo::Undefined, "
                              "satId=-1, signal-strength=1) 60");

        info = QGeoSatelliteInfo();
        info.setSatelliteIdentifier(1);
        QTest::newRow("with_SatelliteIdentifier") << info << -1
                << QByteArray("QGeoSatelliteInfo(system=QGeoSatelliteInfo::Undefined, "
                              "satId=1, signal-strength=-1) -1");

        info = QGeoSatelliteInfo();
        info.setSatelliteSystem(QGeoSatelliteInfo::GPS);
        QTest::newRow("with_System_GPS") << info << 1
                << QByteArray("QGeoSatelliteInfo(system=QGeoSatelliteInfo::GPS, "
                              "satId=-1, signal-strength=-1) 1");

        info = QGeoSatelliteInfo();
        info.setSatelliteSystem(QGeoSatelliteInfo::GLONASS);
        QTest::newRow("with_System_GLONASS") << info << 56
                << QByteArray("QGeoSatelliteInfo(system=QGeoSatelliteInfo::GLONASS, "
                              "satId=-1, signal-strength=-1) 56");

        info = QGeoSatelliteInfo();
        info.setAttribute(QGeoSatelliteInfo::Elevation, 1.1);
        QTest::newRow("with_Elevation") << info << 0
                << QByteArray("QGeoSatelliteInfo(system=QGeoSatelliteInfo::Undefined, "
                              "satId=-1, signal-strength=-1, Elevation=1.1) 0");

        info = QGeoSatelliteInfo();
        info.setAttribute(QGeoSatelliteInfo::Azimuth, 1.1);
        QTest::newRow("with_Azimuth") << info << 45
                << QByteArray("QGeoSatelliteInfo(system=QGeoSatelliteInfo::Undefined, "
                              "satId=-1, signal-strength=-1, Azimuth=1.1) 45");
    }
};


QTEST_APPLESS_MAIN(tst_QGeoSatelliteInfo)
#include "tst_qgeosatelliteinfo.moc"
