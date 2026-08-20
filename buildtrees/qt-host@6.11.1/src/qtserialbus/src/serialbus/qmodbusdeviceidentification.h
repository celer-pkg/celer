// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QMODBUSDEVICEIDENTIFICATION_P_H
#define QMODBUSDEVICEIDENTIFICATION_P_H

#include <QtCore/qmap.h>
#include <QtCore/qmetatype.h>
#include <QtSerialBus/qtserialbusglobal.h>

QT_BEGIN_NAMESPACE

class QModbusDeviceIdentification
{
public:
    enum ObjectId {
        /* Basic mandatory */
        VendorNameObjectId = 0x00,
        ProductCodeObjectId = 0x01,
        MajorMinorRevisionObjectId = 0x02,

        /* Regular optional */
        VendorUrlObjectId = 0x03,
        ProductNameObjectId = 0x04,
        ModelNameObjectId = 0x05,
        UserApplicationNameObjectId = 0x06,
        ReservedObjectId = 0x07,

        /* Extended optional */
        ProductDependentObjectId = 0x80,

        UndefinedObjectId = 0x100
    };

    enum ReadDeviceIdCode {
        BasicReadDeviceIdCode = 0x01,
        RegularReadDeviceIdCode = 0x02,
        ExtendedReadDeviceIdCode = 0x03,
        IndividualReadDeviceIdCode = 0x04
    };

    enum ConformityLevel {
        BasicConformityLevel = 0x01,
        RegularConformityLevel = 0x02,
        ExtendedConformityLevel = 0x03,
        BasicIndividualConformityLevel = 0x81,
        RegularIndividualConformityLevel = 0x82,
        ExtendedIndividualConformityLevel = 0x83
    };

    QModbusDeviceIdentification() = default;

    bool isValid() const {
        return !m_objects.value(VendorNameObjectId).isEmpty()
            && !m_objects.value(ProductCodeObjectId).isEmpty()
            && !m_objects.value(MajorMinorRevisionObjectId).isEmpty();
    }

    QList<int> objectIds() const { return m_objects.keys(); }
    void remove(uint objectId) { m_objects.remove(objectId); }
    bool contains(uint objectId) const { return m_objects.contains(objectId); }

    bool insert(uint objectId, const QByteArray &data) {
        if ((data.size() > 245) || (objectId >= ObjectId::UndefinedObjectId))
            return false;
        m_objects[objectId] = data;
        return true;
    }
    QByteArray value(uint objectId) const { return m_objects.value(objectId); }

    ConformityLevel conformityLevel() const { return m_conformityLevel; }
    void setConformityLevel(ConformityLevel level) { m_conformityLevel = level; }

    static Q_SERIALBUS_EXPORT QModbusDeviceIdentification fromByteArray(const QByteArray &ba);

private:
    QMap<int, QByteArray> m_objects;
    ConformityLevel m_conformityLevel = BasicConformityLevel;
};
Q_DECLARE_TYPEINFO(QModbusDeviceIdentification, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(QModbusDeviceIdentification::ObjectId, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QModbusDeviceIdentification::ReadDeviceIdCode, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QModbusDeviceIdentification::ConformityLevel, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QModbusDeviceIdentification)

#endif // QMODBUSDEVICEIDENTIFICATION_P_H
