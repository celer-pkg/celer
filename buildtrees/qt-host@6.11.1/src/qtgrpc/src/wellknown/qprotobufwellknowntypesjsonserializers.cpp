// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "wrappers.qpb.h"
#include <QtProtobufWellKnownTypes/duration.qpb.h>
#include <QtProtobufWellKnownTypes/private/qprotobufwellknowntypesjsonserializers_p.h>
#include <QtProtobufWellKnownTypes/timestamp.qpb.h>

#include <QtProtobuf/private/protobufscalarjsonserializers_p.h>
#include <QtProtobuf/private/qprotobufjsonserializer_p.h>
#include <QtProtobuf/private/qtprotobuflogging_p.h>

#include <QtCore/qjsonvalue.h>
#include <QtCore/qtimezone.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace {
QJsonValue serializeProtobufWellKnownTimestamp(const QProtobufMessage *message)
{
    qint64 secs = 0;
    qint32 nanos = 0;

    if (const auto secondsValue = message->property("seconds"); secondsValue.canConvert<qint64>()) {
        secs = secondsValue.value<qint64>();
    } else {
        qProtoWarning("Failed to convert seconds");
    }

    if (const auto nanosValue = message->property("nanos"); nanosValue.canConvert<qint32>()) {
        nanos = nanosValue.value<qint32>();
    } else {
        qProtoWarning("Failed to convert nanos");
    }

    const auto datetime = QDateTime::fromMSecsSinceEpoch(secs * 1000 + nanos / 1000'000,
                                                         QTimeZone::UTC);
    return ProtobufScalarJsonSerializers::serializeCommon<
        QString>(datetime.toString(Qt::ISODateWithMs));
}

bool deserializeProtobufWellKnownTimestamp(QProtobufMessage *message, const QJsonValue &value)
{
    if (!value.isString())
        return false;

    const auto tsString = value.toString();
    // Protobuf requires upper-case letters in timestamp string be case sensitive.
    if (tsString.toUpper() != tsString)
        return false;

    if (tsString.contains(u' '))
        return false;

    //Ensure the field either ends with Z or a valid offset
    static const QRegularExpression TimeStampEnding(".+([\\+\\-]\\d{2}:\\d{2}|Z)$"_L1);
    if (!TimeStampEnding.match(tsString).hasMatch())
        return false;

    const auto datetime = QDateTime::fromString(tsString, Qt::ISODateWithMs);

    if (!datetime.isValid()) {
        qProtoWarning("Datetime is invalid");
        return false;
    }
    const auto msecs = datetime.toMSecsSinceEpoch();
    const qint64 seconds = msecs / 1000;
    const qint32 nanos = (msecs % 1000) * 1000'000;

    message->setProperty("seconds", QVariant::fromValue(seconds));
    message->setProperty("nanos", QVariant::fromValue(nanos));

    return true;
}

QJsonValue serializeProtobufWellKnownDuration(const QProtobufMessage *message)
{
    qint64 seconds = 0;
    qint32 nanos = 0;

    if (const auto secondsValue = message->property("seconds"); secondsValue.canConvert<qint64>()) {
        seconds = secondsValue.value<qint64>();
    } else {
        qProtoWarning("Failed to convert seconds");
        return {QJsonValue::Undefined};
    }

    if (const auto nanosValue = message->property("nanos"); nanosValue.canConvert<qint32>()) {
        nanos = nanosValue.value<qint32>();
    } else {
        qProtoWarning("Failed to convert nanos");
        return {QJsonValue::Undefined};
    }

    if (!google::protobuf::Duration::isValid(seconds, nanos))
        return {QJsonValue::Undefined};

    // Reserve the maximum possible number of bytes that can be used by duration string:
    //  - 9 symbols for nanoseconds
    //  - 12 symbols for seconds
    //  - 1 symbol for the sign
    //  - 1 symbol for the floating point preriod
    //  - 1 symbol for the trailing 's'
    static constexpr qsizetype ResultMaxSize = 24;
    QString result;
    result.reserve(ResultMaxSize);

    if (nanos < 0) {
        if (seconds == 0)
            result.append(u'-');
        nanos = qAbs(nanos);
    }
    result.append(QString::number(seconds));

    if (nanos != 0) {
        result.append(u'.');
        // Calculate the number of '0' we need to prepend before nanos and cut the traling zeros
        qint32 multiplier = 100'000'000;
        for (;multiplier != 0 && nanos % multiplier != 0; multiplier /= 10) {
            if (nanos / multiplier == 0)
                result.append(u'0');
        }
        nanos /= multiplier;
        result.append(QString::number(nanos));
    }
    result.append(u's');

    Q_ASSERT_X(result.size() <= ResultMaxSize, "serializeProtobufWellKnownDuration",
               "Serialized duration is malformed. It's an internal Qt issue please file a bugreport"
               " https://bugreports.qt.io");

    return ProtobufScalarJsonSerializers::serializeCommon<QString>(result);
}

bool deserializeProtobufWellKnownDuration(QProtobufMessage *message, const QJsonValue &value)
{
    if (!value.isString())
        return false;

    const static QRegularExpression DurationRegex("^(-?)(\\d{1,12})(\\.(\\d{1,9}))?s$"_L1);

    const auto valueString = value.toString();
    const auto match = DurationRegex.match(valueString);

    if (!match.hasMatch()) {
        qProtoWarning("The input string is malformed: %s", qPrintable(valueString));
        return false;
    }

    const auto sign = match.captured(1);
    auto nanosString = match.captured(4);
    auto seconds = match.captured(2).toLongLong();
    qint32 nanos = nanosString.toInt();

    if (!google::protobuf::Duration::isValid(seconds, nanos))
        return false;

    if (nanos != 0)
        nanos *= qint32(qPow(10, 9 - nanosString.size()));

    if (!sign.isEmpty()) {
        seconds *= -1;
        nanos *= -1;
    }

    message->setProperty("seconds", QVariant::fromValue(seconds));
    message->setProperty("nanos", QVariant::fromValue(nanos));

    return true;
}

template <typename T>
bool deserializeProtobufWellKnownValueType(QProtobufMessage *message, const QJsonValue &value)
{
    using ReturnType = std::invoke_result_t<decltype(&T::value), T>;

    bool ok = false;
    message->setProperty("value",
                         ProtobufScalarJsonSerializers::deserializeCommon<ReturnType>(value, ok));
    return ok;
}

template <typename T>
void registerProtobufWellKnownValueTypeJsonHandler()
{
    QtProtobufPrivate::registerCustomJsonHandler(QMetaType::fromType<T>(), nullptr,
                                                 deserializeProtobufWellKnownValueType<T>);
}

} // namespace

void QtProtobufWellKnownTypesPrivate::registerTimestampCustomJsonHandler()
{
    QtProtobufPrivate::registerCustomJsonHandler(QMetaType::fromType<google::protobuf::Timestamp>(),
                                                 serializeProtobufWellKnownTimestamp,
                                                 deserializeProtobufWellKnownTimestamp);
}

void QtProtobufWellKnownTypesPrivate::registerDurationCustomJsonHandler()
{
    QtProtobufPrivate::registerCustomJsonHandler(QMetaType::fromType<google::protobuf::Duration>(),
                                                 serializeProtobufWellKnownDuration,
                                                 deserializeProtobufWellKnownDuration);
}

void QtProtobufWellKnownTypesPrivate::registerBoolValueCustomJsonHandler()
{
    registerProtobufWellKnownValueTypeJsonHandler<google::protobuf::BoolValue>();
}

void QtProtobufWellKnownTypesPrivate::registerInt32ValueCustomJsonHandler()
{
    registerProtobufWellKnownValueTypeJsonHandler<google::protobuf::Int32Value>();
}

void QtProtobufWellKnownTypesPrivate::registerInt64ValueCustomJsonHandler()
{
    registerProtobufWellKnownValueTypeJsonHandler<google::protobuf::Int64Value>();
}

void QtProtobufWellKnownTypesPrivate::registerUInt32ValueCustomJsonHandler()
{
    registerProtobufWellKnownValueTypeJsonHandler<google::protobuf::UInt32Value>();
}

void QtProtobufWellKnownTypesPrivate::registerUInt64ValueCustomJsonHandler()
{
    registerProtobufWellKnownValueTypeJsonHandler<google::protobuf::UInt64Value>();
}

void QtProtobufWellKnownTypesPrivate::registerFloatValueCustomJsonHandler()
{
    registerProtobufWellKnownValueTypeJsonHandler<google::protobuf::FloatValue>();
}

void QtProtobufWellKnownTypesPrivate::registerDoubleValueCustomJsonHandler()
{
    registerProtobufWellKnownValueTypeJsonHandler<google::protobuf::DoubleValue>();
}

void QtProtobufWellKnownTypesPrivate::registerStringValueCustomJsonHandler()
{
    QtProtobufPrivate::
        registerCustomJsonHandler(QMetaType::fromType<google::protobuf::StringValue>(), nullptr,
                                  [](QProtobufMessage *message, const QJsonValue &value) -> bool {
                                      if (!value.isString())
                                          return false;
                                      message->setProperty("value",
                                                           QVariant::fromValue(value.toString()));
                                      return true;
                                  });
}

void QtProtobufWellKnownTypesPrivate::registerBytesValueCustomJsonHandler()
{
    QtProtobufPrivate::registerCustomJsonHandler(
        QMetaType::fromType<google::protobuf::BytesValue>(), nullptr,
        [](QProtobufMessage *message, const QJsonValue &value) -> bool {
            if (!value.isString())
                return false;
            message->setProperty("value",
                                 QVariant::fromValue(QByteArray::fromBase64(value.toString()
                                                                                .toLatin1())));
            return true;
        });
}

QT_END_NAMESPACE
