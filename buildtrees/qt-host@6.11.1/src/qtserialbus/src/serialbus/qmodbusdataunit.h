// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QMODBUSDATAUNIT_H
#define QMODBUSDATAUNIT_H

#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qmetatype.h>

QT_BEGIN_NAMESPACE

class QModbusDataUnit
{
public:
    enum RegisterType {
        Invalid,
        DiscreteInputs,
        Coils,
        InputRegisters,
        HoldingRegisters
    };

    QModbusDataUnit() = default;

    constexpr explicit QModbusDataUnit(RegisterType type) noexcept
        : m_type(type)
        , m_startAddress(0)
    {}

    QModbusDataUnit(RegisterType type, int newStartAddress, quint16 newValueCount)
        : QModbusDataUnit(type, newStartAddress, QList<quint16>(newValueCount))
    {}

    QModbusDataUnit(RegisterType type, int newStartAddress, const QList<quint16> &newValues)
        : m_type(type)
        , m_startAddress(newStartAddress)
        , m_values(newValues)
        , m_valueCount(newValues.size())
    {}

    RegisterType registerType() const { return m_type; }
    void setRegisterType(RegisterType type) { m_type = type; }

    inline int startAddress() const { return m_startAddress; }
    inline void setStartAddress(int newAddress) { m_startAddress = newAddress; }

    inline QList<quint16> values() const { return m_values; }
    inline void setValues(const QList<quint16> &newValues)
    {
        m_values = newValues;
        m_valueCount = newValues.size();
    }

    inline qsizetype valueCount() const { return m_valueCount; }
    inline void setValueCount(qsizetype newCount) { m_valueCount = newCount; }

    inline void setValue(qsizetype index, quint16 newValue)
    {
        if (m_values.isEmpty() || index >= m_values.size())
            return;
        m_values[index] = newValue;
    }
    inline quint16 value(qsizetype index) const { return m_values.value(index); }

    bool isValid() const { return m_type != Invalid && m_startAddress != -1; }

private:
    RegisterType m_type = Invalid;
    int m_startAddress = -1;
    QList<quint16> m_values;
    qsizetype m_valueCount = 0;
};
typedef QMap<QModbusDataUnit::RegisterType, QModbusDataUnit> QModbusDataUnitMap;

Q_DECLARE_TYPEINFO(QModbusDataUnit, Q_RELOCATABLE_TYPE);
Q_DECLARE_TYPEINFO(QModbusDataUnit::RegisterType, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QModbusDataUnit::RegisterType)

#endif // QMODBUSDATAUNIT_H
