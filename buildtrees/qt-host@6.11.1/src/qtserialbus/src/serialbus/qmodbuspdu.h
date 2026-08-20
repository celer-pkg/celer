// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
#ifndef QMODBUSPDU_H
#define QMODBUSPDU_H

#include <QtCore/qdatastream.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qlist.h>
#include <QtCore/qmetatype.h>
#include <QtSerialBus/qtserialbusglobal.h>

QT_BEGIN_NAMESPACE

class Q_SERIALBUS_EXPORT QModbusPdu
{
public:
    enum ExceptionCode {
        IllegalFunction = 0x01,
        IllegalDataAddress = 0x02,
        IllegalDataValue = 0x03,
        ServerDeviceFailure = 0x04,
        Acknowledge = 0x05,
        ServerDeviceBusy = 0x06,
        NegativeAcknowledge = 0x07,
        MemoryParityError = 0x08,
        GatewayPathUnavailable = 0x0A,
        GatewayTargetDeviceFailedToRespond = 0x0B,
        ExtendedException = 0xFF,
    };

    enum FunctionCode {
        Invalid = 0x00,
        ReadCoils = 0x01,
        ReadDiscreteInputs = 0x02,
        ReadHoldingRegisters = 0x03,
        ReadInputRegisters = 0x04,
        WriteSingleCoil = 0x05,
        WriteSingleRegister = 0x06,
        ReadExceptionStatus = 0x07,
        Diagnostics = 0x08,
        GetCommEventCounter = 0x0B,
        GetCommEventLog = 0x0C,
        WriteMultipleCoils = 0x0F,
        WriteMultipleRegisters = 0x10,
        ReportServerId = 0x11,
        ReadFileRecord = 0x14,
        WriteFileRecord = 0x15,
        MaskWriteRegister = 0x16,
        ReadWriteMultipleRegisters = 0x17,
        ReadFifoQueue = 0x18,
        EncapsulatedInterfaceTransport = 0x2B,
        UndefinedFunctionCode = 0x100
    };

    QModbusPdu() = default;
    virtual ~QModbusPdu();

    bool isValid() const {
        return (m_code >= ReadCoils && m_code < UndefinedFunctionCode)
                && (m_data.size() < 253);
    }

    static const quint8 ExceptionByte = 0x80;
    ExceptionCode exceptionCode() const {
        if (!m_data.size() || !isException())
            return ExtendedException;
        return static_cast<ExceptionCode>(m_data.at(0));
    }
    bool isException() const { return m_code & ExceptionByte; }

    qint16 size() const { return dataSize() + 1; }
    qint16 dataSize() const { return qint16(m_data.size()); }

    FunctionCode functionCode() const {
        return FunctionCode(quint8(m_code) &~ ExceptionByte);
    }
    virtual void setFunctionCode(FunctionCode code) { m_code = code; }

    QByteArray data() const { return m_data; }
    void setData(const QByteArray &newData) { m_data = newData; }

    template <typename ... Args> void encodeData(Args ... newData) {
        encode(std::forward<Args>(newData)...);
    }

    template <typename ... Args> void decodeData(Args && ... newData) const {
        decode(std::forward<Args>(newData)...);
    }

protected:
    QModbusPdu(FunctionCode code, const QByteArray &newData)
        : m_code(code)
        , m_data(newData)
    {}

    QModbusPdu(const QModbusPdu &) = default;
    QModbusPdu &operator=(const QModbusPdu &) = default;

    template <typename ... Args>
    QModbusPdu(FunctionCode code, Args ... newData)
        : m_code(code)
    {
        encode(std::forward<Args>(newData)...);
    }

private:
    template <typename T, typename ... Ts> struct IsType { enum { value = false }; };
    template <typename T, typename T1, typename ... Ts> struct IsType<T, T1, Ts...> {
        enum { value = std::is_same<T, T1>::value || IsType<T, Ts...>::value };
    };

    template <typename T>
    using is_pod = std::conjunction<
            std::is_trivially_copyable<T>,
            std::is_trivially_default_constructible<T>,
            std::is_standard_layout<T>
        >;

    template <typename T> void encode(QDataStream *stream, const T &t) {
        static_assert(is_pod<T>::value, "Only POD types supported.");
        static_assert(IsType<T, quint8, quint16>::value, "Only quint8 and quint16 supported.");
        (*stream) << t;
    }
    template <typename T> void decode(QDataStream *stream, T &t) const {
        static_assert(is_pod<T>::value, "Only POD types supported.");
        static_assert(IsType<T, quint8 *, quint16 *>::value, "Only quint8* and quint16* supported.");
        (*stream) >> *t;
    }
    template<typename T>
    void encode(QDataStream *stream, const QList<T> &vector)
    {
        static_assert(is_pod<T>::value, "Only POD types supported.");
        static_assert(IsType<T, quint8, quint16>::value, "Only quint8 and quint16 supported.");
        for (int i = 0; i < vector.size(); ++i)
            (*stream) << vector[i];
    }

    template<typename ... Args> void encode(Args ... newData) {
        m_data.clear();
        constexpr size_t argCount = sizeof...(Args);
        if constexpr (argCount > 0) {
            QDataStream stream(&m_data, QIODevice::WriteOnly);
            char tmp[argCount] = { (encode(&stream, newData), void(), '0')... };
            Q_UNUSED(tmp);
        }
    }
    template<typename ... Args> void decode(Args ... newData) const {
        constexpr size_t argCount = sizeof...(Args);
        if constexpr (argCount > 0) {
            if (!m_data.isEmpty()) {
                QDataStream stream(m_data);
                char tmp[argCount] = { (decode(&stream, newData), void(), '0')... };
                Q_UNUSED(tmp);
            }
        }
    }

private:
    FunctionCode m_code = Invalid;
    QByteArray m_data;
    friend class QModbusSerialAdu;
    friend struct QModbusPduPrivate;
};
Q_SERIALBUS_EXPORT QDebug operator<<(QDebug debug, const QModbusPdu &pdu);
Q_SERIALBUS_EXPORT QDataStream &operator<<(QDataStream &stream, const QModbusPdu &pdu);
Q_SERIALBUS_EXPORT QDataStream &operator>>(QDataStream &stream, QModbusPdu::FunctionCode &code);

class Q_SERIALBUS_EXPORT QModbusRequest : public QModbusPdu
{
public:
    QModbusRequest() = default;
    QModbusRequest(const QModbusPdu &pdu)
        : QModbusPdu(pdu)
    {}

    explicit QModbusRequest(FunctionCode code, const QByteArray &newData = QByteArray())
        : QModbusPdu(code, newData)
    {}
    ~QModbusRequest() override;

    static int minimumDataSize(const QModbusRequest &pdu);
    static int calculateDataSize(const QModbusRequest &pdu);

    using CalcFuncPtr = decltype(&calculateDataSize);
    static void registerDataSizeCalculator(FunctionCode fc, CalcFuncPtr func);

    template <typename ... Args>
    QModbusRequest(FunctionCode code, Args ... newData)
        : QModbusPdu(code, newData...)
    {}
};
Q_SERIALBUS_EXPORT QDataStream &operator>>(QDataStream &stream, QModbusRequest &pdu);
inline QDataStream &operator<<(QDataStream &stream, const QModbusRequest &pdu)
{ return stream << static_cast<const QModbusPdu &>(pdu); }

class Q_SERIALBUS_EXPORT QModbusResponse : public QModbusPdu
{
public:
    QModbusResponse() = default;
    QModbusResponse(const QModbusPdu &pdu)
        : QModbusPdu(pdu)
    {}

    explicit QModbusResponse(FunctionCode code, const QByteArray &newData = QByteArray())
        : QModbusPdu(code, newData)
    {}
    ~QModbusResponse() override;

    static int minimumDataSize(const QModbusResponse &pdu);
    static int calculateDataSize(const QModbusResponse &pdu);

    using CalcFuncPtr = decltype(&calculateDataSize);
    static void registerDataSizeCalculator(FunctionCode fc, CalcFuncPtr func);

    template <typename ... Args>
    QModbusResponse(FunctionCode code, Args ... newData)
        : QModbusPdu(code, newData...)
    {}
};

class Q_SERIALBUS_EXPORT QModbusExceptionResponse : public QModbusResponse
{
public:
    QModbusExceptionResponse() = default;
    QModbusExceptionResponse(const QModbusPdu &pdu)
        : QModbusResponse(pdu)
    {}
    QModbusExceptionResponse(FunctionCode fc, ExceptionCode ec)
        : QModbusResponse(FunctionCode(quint8(fc) | ExceptionByte), static_cast<quint8> (ec))
    {}
    ~QModbusExceptionResponse() override;

    void setFunctionCode(FunctionCode c) override {
        QModbusPdu::setFunctionCode(FunctionCode(quint8(c) | ExceptionByte));
    }
    void setExceptionCode(ExceptionCode ec) { QModbusPdu::encodeData(quint8(ec)); }
};
Q_SERIALBUS_EXPORT QDataStream &operator>>(QDataStream &stream, QModbusResponse &pdu);
inline QDataStream &operator<<(QDataStream &stream, const QModbusResponse &pdu)
{ return stream << static_cast<const QModbusPdu &>(pdu); }

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QModbusPdu::ExceptionCode)
Q_DECLARE_METATYPE(QModbusPdu::FunctionCode)

#endif // QMODBUSPDU_H
