// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//TESTED_COMPONENT=src/location

#include <QtPositioning/qgeopositioninfo.h>

#include <QMetaType>
#include <QObject>
#include <QDebug>
#include <QTest>
#include <QtCore/QtNumeric>

#include <float.h>

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QGeoPositionInfo::Attribute)

QByteArray tst_qgeopositioninfo_debug;

void tst_qgeopositioninfo_messageHandler(QtMsgType type, const QMessageLogContext&, const QString &msg)
{
    switch (type) {
        case QtDebugMsg :
            tst_qgeopositioninfo_debug = msg.toLocal8Bit();
            break;
        default:
            break;
    }
}

QList<qreal> tst_qgeopositioninfo_qrealTestValues()
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

QList<QGeoPositionInfo::Attribute> tst_qgeopositioninfo_getAttributes()
{
    QList<QGeoPositionInfo::Attribute> attributes;
    attributes << QGeoPositionInfo::Direction
            << QGeoPositionInfo::GroundSpeed
            << QGeoPositionInfo::VerticalSpeed
            << QGeoPositionInfo::MagneticVariation
            << QGeoPositionInfo::HorizontalAccuracy
            << QGeoPositionInfo::VerticalAccuracy;
    return attributes;
}


class tst_QGeoPositionInfo : public QObject
{
    Q_OBJECT

private:
    QGeoPositionInfo infoWithAttribute(QGeoPositionInfo::Attribute attribute, qreal value)
    {
        QGeoPositionInfo info;
        info.setAttribute(attribute, value);
        return info;
    }

    void addTestData_info()
    {
        QTest::addColumn<QGeoPositionInfo>("info");

        QTest::newRow("invalid") << QGeoPositionInfo();

        QTest::newRow("coord") << QGeoPositionInfo(QGeoCoordinate(-27.3422,150.2342), QDateTime());
        QTest::newRow("datetime") << QGeoPositionInfo(QGeoCoordinate(), QDateTime::currentDateTime());

        QList<QGeoPositionInfo::Attribute> attributes = tst_qgeopositioninfo_getAttributes();
        QList<qreal> values = tst_qgeopositioninfo_qrealTestValues();
        for (int i=0; i<attributes.size(); i++) {
            for (int j=0; j<values.size(); j++) {
                QTest::newRow(qPrintable(QString("Attribute %1 = %2").arg(attributes[i]).arg(values[j])))
                        << infoWithAttribute(attributes[i], values[j]);
            }
        }
    }

private slots:
    void constructor()
    {
        QGeoPositionInfo info;
        QVERIFY(!info.isValid());
        QVERIFY(!info.coordinate().isValid());
        QVERIFY(info.timestamp().isNull());
    }

    void constructor_coord_dateTime()
    {
        QFETCH(QGeoCoordinate, coord);
        QFETCH(QDateTime, dateTime);
        QFETCH(bool, valid);

        QGeoPositionInfo info(coord, dateTime);
        QCOMPARE(info.coordinate(), coord);
        QCOMPARE(info.timestamp(), dateTime);
        QCOMPARE(info.isValid(), valid);
    }

    void constructor_coord_dateTime_data()
    {
        QTest::addColumn<QGeoCoordinate>("coord");
        QTest::addColumn<QDateTime>("dateTime");
        QTest::addColumn<bool>("valid");

        QTest::newRow("both null") << QGeoCoordinate() << QDateTime() << false;
        QTest::newRow("both valid") << QGeoCoordinate(1,1) << QDateTime::currentDateTime() << true;
        QTest::newRow("valid coord") << QGeoCoordinate(1,1) << QDateTime() << false;
        QTest::newRow("valid datetime") << QGeoCoordinate() << QDateTime::currentDateTime() << false;
        QTest::newRow("valid time but not date == invalid")
                << QGeoCoordinate() << QDateTime(QDate(), QTime::currentTime()) << false;
        QTest::newRow("valid date but not time == valid due to QDateTime constructor")
                << QGeoCoordinate() << QDateTime(QDate::currentDate(), QTime()) << false;
    }

    void constructor_copy()
    {
        QFETCH(QGeoPositionInfo, info);

        QCOMPARE(QGeoPositionInfo(info), info);
    }

    void constructor_copy_data()
    {
        addTestData_info();
    }

    void constructor_move()
    {
        QFETCH(QGeoPositionInfo, info);
        QGeoPositionInfo infoCopy = info;
        QCOMPARE(QGeoPositionInfo(std::move(info)), infoCopy);
        // The moved-from object will go out of scope and  will be destroyed
        // here, so we also implicitly check that moved-from object's destructor
        // is called without any issues.
    }

    void constructor_move_data()
    {
        addTestData_info();
    }

    void operator_assign()
    {
        QFETCH(QGeoPositionInfo, info);

        QGeoPositionInfo info2;
        info2 = info;
        QCOMPARE(info2, info);
    }

    void operator_assign_data()
    {
        addTestData_info();
    }

    void operator_move_assign()
    {
        QFETCH(QGeoPositionInfo, info);
        QGeoPositionInfo infoCopy = info;

        QGeoPositionInfo obj;
        obj = std::move(info);
        QCOMPARE(obj, infoCopy);

        // check that (move)assigning to the moved-from object is ok
        info = std::move(infoCopy);
        QCOMPARE(info, obj);
    }

    void operator_move_assign_data()
    {
        addTestData_info();
    }

    void operator_equals()
    {
        QFETCH(QGeoPositionInfo, info);

        QVERIFY(info == info);
        if (info.isValid())
            QCOMPARE(info == QGeoPositionInfo(), false);
    }

    void operator_equals_data()
    {
        addTestData_info();
    }

    void operator_notEquals()
    {
        QFETCH(QGeoPositionInfo, info);

        QCOMPARE(info != info, false);
        if (info.isValid())
            QCOMPARE(info != QGeoPositionInfo(), true);
    }

    void operator_notEquals_data()
    {
        addTestData_info();
    }

    void setDateTime()
    {
        QFETCH(QDateTime, dateTime);

        QGeoPositionInfo info;
        info.setTimestamp(dateTime);
        QCOMPARE(info.timestamp(), dateTime);
    }

    void setDateTime_data()
    {
        QTest::addColumn<QDateTime>("dateTime");
        QTest::newRow("invalid") << QDateTime();
        QTest::newRow("now") << QDateTime::currentDateTime();
    }

    void dateTime()
    {
        QGeoPositionInfo info;
        QVERIFY(info.timestamp().isNull());
    }

    void setCoordinate()
    {

        QFETCH(QGeoCoordinate, coord);

        QGeoPositionInfo info;
        info.setCoordinate(coord);
        QCOMPARE(info.coordinate(), coord);
    }

    void setCoordinate_data()
    {
        QTest::addColumn<QGeoCoordinate>("coord");

        QTest::newRow("invalid") << QGeoCoordinate();
        QTest::newRow("valid") << QGeoCoordinate(30,30);
    }

    void attribute()
    {
        QFETCH(QGeoPositionInfo::Attribute, attribute);
        QFETCH(qreal, value);

        QGeoPositionInfo info;
        QVERIFY(qIsNaN(info.attribute(attribute)));

        info.setAttribute(attribute, value);
        QCOMPARE(info.attribute(attribute), value);

        info.removeAttribute(attribute);
        QVERIFY(qIsNaN(info.attribute(attribute)));
    }

    void attribute_data()
    {
        QTest::addColumn<QGeoPositionInfo::Attribute>("attribute");
        QTest::addColumn<qreal>("value");

        QList<QGeoPositionInfo::Attribute> attributes = tst_qgeopositioninfo_getAttributes();
        QList<qreal> values = tst_qgeopositioninfo_qrealTestValues();
        for (int i=0; i<attributes.size(); i++) {
            for (int j=0; j<values.size(); j++) {
                QTest::newRow(qPrintable(QString("Attribute %1 = %2").arg(attributes[i]).arg(values[j])))
                        << attributes[i] << values[j];
            }
        }
    }

    void hasAttribute()
    {
        QFETCH(QGeoPositionInfo::Attribute, attribute);
        QFETCH(qreal, value);

        QGeoPositionInfo info;
        QVERIFY(!info.hasAttribute(attribute));

        info.setAttribute(attribute, value);
        QVERIFY(info.hasAttribute(attribute));

        info.removeAttribute(attribute);
        QVERIFY(!info.hasAttribute(attribute));
    }

    void hasAttribute_data()
    {
        attribute_data();
    }

    void removeAttribute()
    {
        QFETCH(QGeoPositionInfo::Attribute, attribute);
        QFETCH(qreal, value);

        QGeoPositionInfo info;
        QVERIFY(!info.hasAttribute(attribute));

        info.setAttribute(attribute, value);
        QVERIFY(info.hasAttribute(attribute));

        info.removeAttribute(attribute);
        QVERIFY(!info.hasAttribute(attribute));

        info.setAttribute(attribute, value);
        QVERIFY(info.hasAttribute(attribute));
    }

    void removeAttribute_data()
    {
        attribute_data();
    }

    void datastream()
    {
        QFETCH(QGeoPositionInfo, info);

        QByteArray ba;
        QDataStream out(&ba, QIODevice::WriteOnly);
        out << info;

        QDataStream in(&ba, QIODevice::ReadOnly);
        QGeoPositionInfo inInfo;
        in >> inInfo;
        QCOMPARE(inInfo, info);
    }

    void datastream_data()
    {
        addTestData_info();
    }

    void debug()
    {
        QFETCH(QGeoPositionInfo, info);
        QFETCH(int, nextValue);
        QFETCH(QByteArray, debugStringEnd);

        qInstallMessageHandler(tst_qgeopositioninfo_messageHandler);
        qDebug() << info << nextValue;
        qInstallMessageHandler(0);

        // use endsWith() so we don't depend on QDateTime's debug() implementation
        QVERIFY2(tst_qgeopositioninfo_debug.endsWith(debugStringEnd),
                 qPrintable(QString::fromLatin1("'%1' does not end with '%2'").
                            arg(QLatin1String(tst_qgeopositioninfo_debug),
                                QLatin1String(debugStringEnd))));
    }

    void debug_data()
    {
        QTest::addColumn<QGeoPositionInfo>("info");
        QTest::addColumn<int>("nextValue");
        QTest::addColumn<QByteArray>("debugStringEnd");

        QTest::newRow("no values") << QGeoPositionInfo() << 40
                << QString("QGeoCoordinate(?, ?)) 40").toLatin1();

        QGeoCoordinate coord(1, 1);
        QTest::newRow("coord, time") << QGeoPositionInfo(coord, QDateTime::currentDateTime())
                << 40 << QByteArray("QGeoCoordinate(1, 1)) 40");

        QGeoPositionInfo info;
        info.setAttribute(QGeoPositionInfo::Direction, 1.1);
        info.setAttribute(QGeoPositionInfo::GroundSpeed, 2.1);
        info.setAttribute(QGeoPositionInfo::VerticalSpeed, 3.1);
        info.setAttribute(QGeoPositionInfo::MagneticVariation, 4.1);
        info.setAttribute(QGeoPositionInfo::HorizontalAccuracy, 5.1);
        info.setAttribute(QGeoPositionInfo::VerticalAccuracy, 6.1);
        QTest::newRow("all attributes") << info << 40
                << QByteArray("QGeoCoordinate(?, ?), Direction=1.1, GroundSpeed=2.1, VerticalSpeed=3.1, MagneticVariation=4.1, HorizontalAccuracy=5.1, VerticalAccuracy=6.1) 40");
    }
};


QTEST_APPLESS_MAIN(tst_QGeoPositionInfo)
#include "tst_qgeopositioninfo.moc"
