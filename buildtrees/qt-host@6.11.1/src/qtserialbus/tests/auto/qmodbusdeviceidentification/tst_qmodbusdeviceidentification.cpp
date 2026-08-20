// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtSerialBus/qmodbusdeviceidentification.h>
#include <QtSerialBus/qmodbuspdu.h>

#include <QtTest/QtTest>

class tst_QModbusDeviceIdentification : public QObject
{
    Q_OBJECT

private slots:
    void testConstructor()
    {
        QModbusDeviceIdentification qmdi;
        QCOMPARE(qmdi.isValid(), false);
        QCOMPARE(qmdi.objectIds().isEmpty(), true);
        for (int i = QModbusDeviceIdentification::VendorNameObjectId;
            i < QModbusDeviceIdentification::UndefinedObjectId; ++i) {
            QCOMPARE(qmdi.contains(i), false);
            qmdi.remove(i); // force crash if the behavior changes
            QCOMPARE(qmdi.value(i), QByteArray());
        }
        QCOMPARE(qmdi.conformityLevel(), QModbusDeviceIdentification::BasicConformityLevel);
    }

    void testIsValid()
    {
        QModbusDeviceIdentification qmdi;
        QCOMPARE(qmdi.isValid(), false);
        QCOMPARE(qmdi.insert(QModbusDeviceIdentification::ReservedObjectId, "Reserved"), true);
        QCOMPARE(qmdi.isValid(), false);
        QCOMPARE(qmdi.insert(QModbusDeviceIdentification::VendorNameObjectId,
            "Company identification"), true);
        QCOMPARE(qmdi.isValid(), false);
        QCOMPARE(qmdi.insert(QModbusDeviceIdentification::ProductCodeObjectId, "Product code"),
            true);
        QCOMPARE(qmdi.isValid(), false);
        QCOMPARE(qmdi.insert(QModbusDeviceIdentification::MajorMinorRevisionObjectId, "V2.11"),
            true);
        QCOMPARE(qmdi.isValid(), true);
        QCOMPARE(qmdi.insert(QModbusDeviceIdentification::MajorMinorRevisionObjectId, ""), true);
        QCOMPARE(qmdi.isValid(), false);
        QCOMPARE(qmdi.insert(QModbusDeviceIdentification::MajorMinorRevisionObjectId, "V2.11"),
            true);
        QCOMPARE(qmdi.isValid(), true);
    }

    void testRemoveAndContains()
    {
        QModbusDeviceIdentification qmdi;
        QCOMPARE(qmdi.contains(QModbusDeviceIdentification::ReservedObjectId), false);
        QCOMPARE(qmdi.insert(QModbusDeviceIdentification::ReservedObjectId, "Reserved"), true);
        QCOMPARE(qmdi.contains(QModbusDeviceIdentification::ReservedObjectId), true);
        qmdi.remove(QModbusDeviceIdentification::ReservedObjectId);
        QCOMPARE(qmdi.contains(QModbusDeviceIdentification::ReservedObjectId), false);
    }

    void testInsertAndValue()
    {
        QModbusDeviceIdentification qmdi;
        QCOMPARE(qmdi.insert(QModbusDeviceIdentification::ProductDependentObjectId, "Test"), true);
        QCOMPARE(qmdi.value(QModbusDeviceIdentification::ProductDependentObjectId),
            QByteArray("Test"));
        QCOMPARE(qmdi.insert(QModbusDeviceIdentification::ProductDependentObjectId,
            QByteArray(246, '@')), false);
        QCOMPARE(qmdi.value(QModbusDeviceIdentification::ProductDependentObjectId),
            QByteArray("Test"));
        QCOMPARE(qmdi.insert(QModbusDeviceIdentification::ProductDependentObjectId,
            QByteArray(245, '@')), true);
        QCOMPARE(qmdi.value(QModbusDeviceIdentification::ProductDependentObjectId),
            QByteArray(245, '@'));
        QCOMPARE(qmdi.insert(QModbusDeviceIdentification::UndefinedObjectId, "Test"), false);
        QCOMPARE(qmdi.value(QModbusDeviceIdentification::UndefinedObjectId), QByteArray());
    }

    void testConformityLevel()
    {
        QModbusDeviceIdentification qmdi;
        QCOMPARE(qmdi.conformityLevel(), QModbusDeviceIdentification::BasicConformityLevel);
        qmdi.setConformityLevel(QModbusDeviceIdentification::BasicIndividualConformityLevel);
        QCOMPARE(qmdi.conformityLevel(), QModbusDeviceIdentification::BasicIndividualConformityLevel);
    }

    void testConstructFromByteArray()
    {
        const QByteArray vendorNameObject = "Company identification";
        QModbusResponse r(QModbusResponse::EncapsulatedInterfaceTransport,
            QByteArray::fromHex("0e01010000010016") + vendorNameObject);

        {
        QModbusDeviceIdentification qmdi = QModbusDeviceIdentification::fromByteArray(r.data());
        QCOMPARE(qmdi.isValid(), false);
        QCOMPARE(qmdi.objectIds(), QList<int>() << QModbusDeviceIdentification::VendorNameObjectId);
        QCOMPARE(qmdi.contains(QModbusDeviceIdentification::VendorNameObjectId), true);
        QCOMPARE(qmdi.value(QModbusDeviceIdentification::VendorNameObjectId), vendorNameObject);
        QCOMPARE(qmdi.conformityLevel(), QModbusDeviceIdentification::BasicConformityLevel);
        }

        const QByteArray productCodeObject = "Product code";
        r.setData(QByteArray::fromHex("0e01010000020016") + vendorNameObject
            + QByteArray::fromHex("010c") + productCodeObject);
        {
        QModbusDeviceIdentification qmdi = QModbusDeviceIdentification::fromByteArray(r.data());
        QCOMPARE(qmdi.isValid(), false);
        QCOMPARE(qmdi.objectIds(), QList<int>() << QModbusDeviceIdentification::VendorNameObjectId
             << QModbusDeviceIdentification::ProductCodeObjectId);
        QCOMPARE(qmdi.contains(QModbusDeviceIdentification::VendorNameObjectId), true);
        QCOMPARE(qmdi.contains(QModbusDeviceIdentification::ProductCodeObjectId), true);
        QCOMPARE(qmdi.value(QModbusDeviceIdentification::VendorNameObjectId), vendorNameObject);
        QCOMPARE(qmdi.value(QModbusDeviceIdentification::ProductCodeObjectId), productCodeObject);
        QCOMPARE(qmdi.conformityLevel(), QModbusDeviceIdentification::BasicConformityLevel);
        }

        const QByteArray majorMinorRevision = "V2.11";
        r.setData(QByteArray::fromHex("0e01010000030016") + vendorNameObject
            + QByteArray::fromHex("010c") + productCodeObject + QByteArray::fromHex("0205")
            + majorMinorRevision);
        {
        QModbusDeviceIdentification qmdi = QModbusDeviceIdentification::fromByteArray(r.data());
        QCOMPARE(qmdi.isValid(), true);
        QCOMPARE(qmdi.objectIds(), QList<int>() << QModbusDeviceIdentification::VendorNameObjectId
             << QModbusDeviceIdentification::ProductCodeObjectId
             << QModbusDeviceIdentification::MajorMinorRevisionObjectId);
        QCOMPARE(qmdi.contains(QModbusDeviceIdentification::VendorNameObjectId), true);
        QCOMPARE(qmdi.contains(QModbusDeviceIdentification::ProductCodeObjectId), true);
        QCOMPARE(qmdi.contains(QModbusDeviceIdentification::MajorMinorRevisionObjectId), true);
        QCOMPARE(qmdi.value(QModbusDeviceIdentification::VendorNameObjectId), vendorNameObject);
        QCOMPARE(qmdi.value(QModbusDeviceIdentification::ProductCodeObjectId), productCodeObject);
        QCOMPARE(qmdi.value(QModbusDeviceIdentification::MajorMinorRevisionObjectId), majorMinorRevision);
        QCOMPARE(qmdi.conformityLevel(), QModbusDeviceIdentification::BasicConformityLevel);
        }
    }
};

QTEST_MAIN(tst_QModbusDeviceIdentification)

#include "tst_qmodbusdeviceidentification.moc"
