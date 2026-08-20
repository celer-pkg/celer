// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>
#include <QtSerialBus/QModbusDataUnit>

class tst_QModbusDataUnit : public QObject
{
    Q_OBJECT
public:
    explicit tst_QModbusDataUnit();

private slots:
    void constructors();
    void setters();
    void testAPI();
};

tst_QModbusDataUnit::tst_QModbusDataUnit()
{
}

void tst_QModbusDataUnit::constructors()
{
    QModbusDataUnit unit1(QModbusDataUnit::Coils);

    QCOMPARE(unit1.registerType(), QModbusDataUnit::Coils);
    QCOMPARE(unit1.startAddress(), 0);
    QCOMPARE(unit1.valueCount(), 0);
    QVERIFY(unit1.values().isEmpty());

    QModbusDataUnit coils(QModbusDataUnit::Coils, 15, 20);
    QCOMPARE(coils.registerType(), QModbusDataUnit::Coils);
    QCOMPARE(coils.startAddress(), 15);
    const auto values = coils.values();
    QCOMPARE(values.size(), 20);
    QCOMPARE(values, QList<quint16>(20));
    for (auto val : values) {
        QCOMPARE(val, quint16(0));
    }
    QCOMPARE(coils.valueCount(), 20);

    QModbusDataUnit unit2(QModbusDataUnit::HoldingRegisters, 3, QList<quint16> { 9 });
    QCOMPARE(unit2.registerType(), QModbusDataUnit::HoldingRegisters);
    QCOMPARE(unit2.startAddress(), 3);
    QCOMPARE(unit2.values().size(), 1);
    QCOMPARE(unit2.values().at(0), (quint16) 9);
    QCOMPARE(unit2.valueCount(), 1);

    QList<quint16> data;
    data.append(5);
    data.append(6);
    data.append(7);

    QModbusDataUnit unit3(QModbusDataUnit::InputRegisters, 2, data);
    QCOMPARE(unit3.registerType(), QModbusDataUnit::InputRegisters);
    QCOMPARE(unit3.startAddress(), 2);
    QCOMPARE(unit3.values().size(), 3);
    QCOMPARE(unit3.values().at(0), (quint16) 5);
    QCOMPARE(unit3.values().at(1), (quint16) 6);
    QCOMPARE(unit3.values().at(2), (quint16) 7);
    QCOMPARE(unit3.valueCount(), 3);
}

void tst_QModbusDataUnit::setters()
{
    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters, 3, QList<quint16> { 9 });
    QCOMPARE(unit.valueCount(), 1);

    unit.setRegisterType(QModbusDataUnit::InputRegisters);
    unit.setStartAddress(2);
    QList<quint16> data;
    data.append(9u);
    data.append(5u);
    unit.setValues(data);

    QCOMPARE(unit.registerType(), QModbusDataUnit::InputRegisters);
    QCOMPARE(unit.startAddress(), 2);
    QCOMPARE(unit.valueCount(), 2);
    QCOMPARE(unit.values().size(), 2);
    QCOMPARE(unit.values().at(0), (quint16) 9);
    QCOMPARE(unit.values().at(1), (quint16) 5);

    //valueCount can be adjusted but values() stays the same
    unit.setValueCount(1);
    QCOMPARE(unit.valueCount(), 1);
    QCOMPARE(unit.values().size(), 2);
    QCOMPARE(unit.values().at(0), (quint16) 9);
    QCOMPARE(unit.values().at(1), (quint16) 5);
}

void tst_QModbusDataUnit::testAPI()
{
    {
        QModbusDataUnit unit;
        QCOMPARE(unit.isValid(), false);
        unit.setStartAddress(15);
        QCOMPARE(unit.isValid(), false);
        unit.setRegisterType(QModbusDataUnit::Coils);
        QCOMPARE(unit.isValid(), true);
    }

    {
        QModbusDataUnit unit;
        QCOMPARE(unit.isValid(), false);
        unit.setRegisterType(QModbusDataUnit::Coils);
        QCOMPARE(unit.isValid(), false);
        unit.setStartAddress(15);
        QCOMPARE(unit.isValid(), true);
    }

    QModbusDataUnit unit;
    QCOMPARE(unit.isValid(), false);
    QCOMPARE(unit.registerType(), QModbusDataUnit::Invalid);
    QCOMPARE(unit.startAddress(), -1);
    QCOMPARE(unit.valueCount(), 0);
    QCOMPARE(unit.values(), QList<quint16>());

    unit.setStartAddress(15);
    unit.setRegisterType(QModbusDataUnit::Coils);
    QCOMPARE(unit.isValid(), true);

    QCOMPARE(unit.values(), QList<quint16>());
    unit.setValues(QList<quint16> { 1, 2, 3, 4 });
    QCOMPARE(unit.values(), QList<quint16>({ 1, 2, 3, 4 }));

    QCOMPARE(unit.valueCount(), 4);
    unit.setValueCount(25);
    QCOMPARE(unit.valueCount(), 25);

    QCOMPARE(unit.value(0), quint16(1));
    unit.setValue(0, 25);
    QCOMPARE(unit.value(0), quint16(25));
}

QTEST_MAIN(tst_QModbusDataUnit)

#include "tst_qmodbusdataunit.moc"
