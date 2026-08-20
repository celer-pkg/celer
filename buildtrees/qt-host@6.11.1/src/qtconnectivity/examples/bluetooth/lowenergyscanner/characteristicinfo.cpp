// Copyright (C) 2013 BlackBerry Limited. All rights reserved.
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "characteristicinfo.h"

#include <QBluetoothUuid>

#include <QByteArray>

using namespace Qt::StringLiterals;

CharacteristicInfo::CharacteristicInfo(const QLowEnergyCharacteristic &characteristic):
    m_characteristic(characteristic)
{
}

void CharacteristicInfo::setCharacteristic(const QLowEnergyCharacteristic &characteristic)
{
    m_characteristic = characteristic;
    emit characteristicChanged();
}

QString CharacteristicInfo::getName() const
{
    //! [les-get-descriptors]
    QString name = m_characteristic.name();
    if (!name.isEmpty())
        return name;

    // find descriptor with CharacteristicUserDescription
    const QList<QLowEnergyDescriptor> descriptors = m_characteristic.descriptors();
    for (const QLowEnergyDescriptor &descriptor : descriptors) {
        if (descriptor.type() == QBluetoothUuid::DescriptorType::CharacteristicUserDescription) {
            name = descriptor.value();
            break;
        }
    }
    //! [les-get-descriptors]

    if (name.isEmpty())
        name = u"Unknown"_s;

    return name;
}

QString CharacteristicInfo::getUuid() const
{
    const QBluetoothUuid uuid = m_characteristic.uuid();
    bool success = false;
    quint16 result16 = uuid.toUInt16(&success);
    if (success)
        return u"0x"_s + QString::number(result16, 16);

    quint32 result32 = uuid.toUInt32(&success);
    if (success)
        return u"0x"_s + QString::number(result32, 16);

    return uuid.toString().remove('{'_L1).remove('}'_L1);
}

QString CharacteristicInfo::getValue() const
{
    // Show raw string first and hex value below
    QByteArray a = m_characteristic.value();
    QString result;
    if (a.isEmpty()) {
        result = u"<none>"_s;
        return result;
    }

    result = a;
    result += '\n'_L1;
    result += a.toHex();

    return result;
}

QString CharacteristicInfo::getPermission() const
{
    QString properties = u"( "_s;
    uint permission = m_characteristic.properties();
    if (permission & QLowEnergyCharacteristic::Read)
        properties += u" Read"_s;
    if (permission & QLowEnergyCharacteristic::Write)
        properties += u" Write"_s;
    if (permission & QLowEnergyCharacteristic::Notify)
        properties += u" Notify"_s;
    if (permission & QLowEnergyCharacteristic::Indicate)
        properties += u" Indicate"_s;
    if (permission & QLowEnergyCharacteristic::ExtendedProperty)
        properties += u" ExtendedProperty"_s;
    if (permission & QLowEnergyCharacteristic::Broadcasting)
        properties += u" Broadcast"_s;
    if (permission & QLowEnergyCharacteristic::WriteNoResponse)
        properties += u" WriteNoResp"_s;
    if (permission & QLowEnergyCharacteristic::WriteSigned)
        properties += u" WriteSigned"_s;
    properties += u" )"_s;
    return properties;
}

QLowEnergyCharacteristic CharacteristicInfo::getCharacteristic() const
{
    return m_characteristic;
}
