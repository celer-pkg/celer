// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qcanbusframe.h"
#include "qcanframeprocessor.h"
#include "qcanframeprocessor_p.h"
#include "qcanmessagedescription.h"
#include "qcanmessagedescription_p.h"
#include "qcansignaldescription.h"
#include "qcansignaldescription_p.h"

#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtCore/QtEndian>

QT_BEGIN_NAMESPACE

// The initial revision of QCanFrameProcessor introduced the BE data processing
// logic which is different from what is normally done in CAN protocols.
// A later patch fixes the logic to be compliant with normal CAN approach
// (taking DBC as a reference), and introduces this define to disable the
// unused functions.
// We could completely remove the "dead code", but for now we want to get some
// feedback from the users to see if we need to have both approaches or not.
#define USE_DBC_COMPATIBLE_BE_HANDLING

// Helper method to extract the max bit number of the signal.
// Note that for BE it's not the last bit of the signal.
static quint16 extractMaxBitNum(quint16 startBit, quint16 bitLength, QSysInfo::Endian endian)
{
#ifdef USE_DBC_COMPATIBLE_BE_HANDLING
    if (endian == QSysInfo::Endian::LittleEndian) {
        return startBit + bitLength - 1;
    } else {
        const auto startByteNum = startBit / 8;
        const auto bitsInStartByte = startBit % 8 + 1;
        const auto leftBits = bitLength - bitsInStartByte;
        if (leftBits <= 0)
            return startBit; // so start bit is the largest

        const auto leftBytesRounded = (leftBits % 8 == 0) ? leftBits / 8 : leftBits / 8 + 1;
        return (startByteNum + leftBytesRounded + 1) * 8 - 1;
    }
#else
    return startBit + bitLength - 1;
#endif // USE_DBC_COMPATIBLE_BE_HANDLING
}

/*!
    \class QCanFrameProcessor
    \inmodule QtSerialBus
    \since 6.5
    \preliminary

    \brief The QCanFrameProcessor class can be used to decode
    a \l QCanBusFrame or to convert the input data into a \l QCanBusFrame that
    is ready to be sent to the receiver.

    The QCanFrameProcessor class operates on the CAN message descriptions
    (represented by the \l QCanMessageDescription and \l QCanSignalDescription
    classes) and a unique identifier description (represented by
    \l QCanUniqueIdDescription). It uses the descriptions to decode the
    incoming \l QCanBusFrame or to encode the user-specified data into the
    proper payload.

    Before doing any decoding or encoding, the QCanFrameProcessor instance
    \e must be initialized properly. The following data needs to be provided:

    \list
        \li A \l {QCanUniqueIdDescription::isValid}{valid} unique identifier
            description. Use the \l setUniqueIdDescription() method to provide
            a proper description.
        \li At least one message description. Use the
            \l addMessageDescriptions() or \l setMessageDescriptions() method
            to provide message descriptions.
            All message descriptions \e must have distinct unique identifiers.
            Each message can contain multiple signal descriptions, but signal
            names within one message \e must be unique as well.
    \endlist

    The \l parseFrame() method can be used to process the incoming
    \l QCanBusFrame. The method returns a \l {QCanFrameProcessor::}{ParseResult}
    structure which contains the \l {QCanFrameProcessor::ParseResult::uniqueId}
    {unique identifier} and the \l {QCanFrameProcessor::ParseResult::signalValues}
    {signal values} map. The keys of the map are the
    \l {QCanSignalDescription::name}{signal names}, and the values of the map
    are signal values.

    The \l prepareFrame() method can be used to generate a \l QCanBusFrame
    object for a specific unique identifier, using the provided signal names
    and desired values.

    Errors can occur during the encoding or decoding process. In such cases
    the \l error() and \l errorString() methods can be used to get the
    information about the error.

    Some non-critical problems may occur as well. Such problems will be logged,
    but the process will not be stopped. After the process is completed, the
    \l warnings() method can be used to access the list of all the warnings.

    \note The last error and error description, as well as the warnings, are
    reset once the decoding or encoding is started.

    \sa QCanMessageDescription, QCanSignalDescription
*/

/*!
    \enum QCanFrameProcessor::Error

    This enum represents the possible errors that can occur while
    encoding or decoding the \l QCanBusFrame.

    \value None No error occurred.
    \value InvalidFrame The received frame is invalid and cannot be parsed.
    \value UnsupportedFrameFormat The format of the received frame is not
                                  supported and cannot be parsed.
    \value Decoding An error occurred during decoding. Use
                         \l errorString() to get a string representation
                         of the error.
    \value Encoding An error occurred during encoding. Use
                         \l errorString() to get a string representation
                         of the error.
*/

/*!
    \struct QCanFrameProcessor::ParseResult
    \inmodule QtSerialBus
    \since 6.5

    \brief The struct is used as a return value for the
    \l QCanFrameProcessor::parseFrame() method.
*/

/*!
    \variable QCanFrameProcessor::ParseResult::uniqueId
    \brief the value of the unique identifier of the parsed frame.
*/

/*!
    \variable QCanFrameProcessor::ParseResult::signalValues
    \brief the map containing the extracted signals and their values.
    The keys of the map are the \l {QCanSignalDescription::name}{signal names},
    and the values of the map are signal values.
*/

/*!
    Creates a CAN frame processor.
*/
QCanFrameProcessor::QCanFrameProcessor()
    : d(std::make_unique<QCanFrameProcessorPrivate>())
{
}

/*!
    Destroys this frame processor.
*/
QCanFrameProcessor::~QCanFrameProcessor() = default;

/*!
    Constructs a CAN data frame, using \a uniqueId and \a signalValues
    and returns the constructed \l QCanBusFrame.

    The \a signalValues parameter \e must contain signal names as keys, and
    expected signal values as values.

    The process of creating the frame is as follows:

    \list 1
        \li The \a uniqueId is used to find an appropriate message
            description.
        \li If the message description is found, a \l QCanBusFrame with
            a payload of the specified size is created. All bytes of the
            payload, as well as the frame id, are initialized to zeros.
        \li The \l uniqueIdDescription() is used to encode the \a uniqueId into
            the appropriate part of the frame (frame id or payload).
        \li The selected message description is used to encode all the
            \a signalValues into the frame.
        \li The parts of the frame that are not covered by a unique id or
            existing signal descriptions are untouched (and so still contain
            zeros).
    \endlist

    If an error occurred during the encoding, an invalid \l QCanBusFrame is
    returned. In such cases, the \l error() and \l errorString() methods
    can be used to get information about the errors.

    \note Calling this method clears all previous errors and warnings.

    \sa addMessageDescriptions(), error(), errorString(), warnings()
*/
QCanBusFrame QCanFrameProcessor::prepareFrame(QtCanBus::UniqueId uniqueId,
                                              const QVariantMap &signalValues)
{
    d->resetErrors();

    if (!d->uidDescription.isValid()) {
        d->setError(Error::Encoding,
                    QObject::tr("No valid unique identifier description is specified."));
        return QCanBusFrame(QCanBusFrame::InvalidFrame);
    }

    if (!d->messages.contains(uniqueId)) {
        d->setError(Error::Encoding,
                    QObject::tr("Failed to find message description for unique id %1.").
                    arg(qToUnderlying(uniqueId)));
        return QCanBusFrame(QCanBusFrame::InvalidFrame);
    }

    const auto message = d->messages.value(uniqueId);
    QCanBusFrame::FrameId canFrameId = 0; // may be modified by the signal values
    QByteArray payload(message.size(), 0x00);

    // encode the uniqueId value into the frame on the proper position
    {
        const bool uidInPayload = d->uidDescription.source() == QtCanBus::DataSource::Payload;
        const quint16 bitsSize = uidInPayload ? payload.size() * 8 : 29;
        unsigned char *data = uidInPayload ? reinterpret_cast<unsigned char *>(payload.data())
                                           : reinterpret_cast<unsigned char *>(&canFrameId);
        if (!d->fillUniqueId(data, bitsSize, uniqueId)) {
            d->setError(Error::Encoding,
                        QObject::tr("Failed to encode unique id %1 into the frame").
                        arg(qToUnderlying(uniqueId)));
            return QCanBusFrame(QCanBusFrame::InvalidFrame);
        }
    }

    // helper function to check for multiplexor preconditions
    auto checkMuxValues = [](const QCanSignalDescription &desc,
                             const QVariantMap &signalValues) -> bool
    {
        const auto muxValues = desc.multiplexSignals();
        if (muxValues.isEmpty())
            return true;
        const auto *descPrivate = QCanSignalDescriptionPrivate::get(desc);
        for (auto it = muxValues.cbegin(); it != muxValues.cend(); ++it) {
            const auto &name = it.key();
            const auto &ranges = it.value();
            if (!signalValues.contains(name)
                    || !descPrivate->muxValueInRange(signalValues.value(name), ranges)) {
                return false;
            }
        }
        return true;
    };

    auto descriptionsHash = QCanMessageDescriptionPrivate::get(message)->messageSignals;
    for (auto it = signalValues.cbegin(); it != signalValues.cend(); ++it) {
        const QString &signalName = it.key();
        if (!descriptionsHash.contains(signalName)) {
            d->addWarning(QObject::tr("Skipping signal %1. It is not found in "
                                      "message description for unique id %2.").
                          arg(signalName, QString::number(qToUnderlying(uniqueId))));
            continue;
        }

        const auto &signalDesc = descriptionsHash.value(signalName);
        if (!signalDesc.isValid()) {
            d->addWarning(QObject::tr("Skipping signal %1. Its description is invalid.").
                          arg(signalName));
            continue;
        }

        // check for multiplexor prerequisites
        if (!checkMuxValues(signalDesc, signalValues)) {
            d->addWarning(QObject::tr("Skipping signal %1. Proper multiplexor values not found.").
                          arg(signalName));
            continue;
        }

        const bool dataInPayload = signalDesc.dataSource() == QtCanBus::DataSource::Payload;
        // For data in FrameId we consider max length == 29, because we do not
        // know if the frame is extended or not.
        const quint16 maxDataLength = dataInPayload ? payload.size() * 8 : 29;
        const auto signalDataEnd = extractMaxBitNum(signalDesc.startBit(), signalDesc.bitLength(),
                                                    signalDesc.dataEndian());
        if (signalDataEnd >= maxDataLength) {
            d->addWarning(QObject::tr("Skipping signal %1. Its length exceeds the expected "
                                      "message length.").arg(signalName));
            continue;
        }

        unsigned char *data = dataInPayload ? reinterpret_cast<unsigned char *>(payload.data())
                                            : reinterpret_cast<unsigned char *>(&canFrameId);
        d->encodeSignal(data, it.value(), signalDesc);
    }

    return QCanBusFrame(canFrameId, payload);
}

/*!
    Returns the last error.

    \sa errorString(), prepareFrame(), parseFrame()
*/
QCanFrameProcessor::Error QCanFrameProcessor::error() const
{
    return d->error;
}

/*!
    Returns the text description of the last error.

    \sa error(), prepareFrame(), parseFrame()
*/
QString QCanFrameProcessor::errorString() const
{
    return d->errorString;
}

/*!
    Returns the list of warnings generated during the last encoding or decoding
    call.

    \sa error(), errorString(), prepareFrame(), parseFrame()
*/
QStringList QCanFrameProcessor::warnings() const
{
    return d->warnings;
}

/*!
    Returns all the message descriptions that are currently used by this frame
    processor.

    \sa addMessageDescriptions(), setMessageDescriptions(),
    clearMessageDescriptions()
*/
QList<QCanMessageDescription> QCanFrameProcessor::messageDescriptions() const
{
    return QList<QCanMessageDescription>(d->messages.cbegin(), d->messages.cend());
}

/*!
    Adds new message descriptions \a descriptions to the available message
    descriptions.

    All message descriptions should have distinct unique ids.

    If some message descriptions have repeated unique ids, only the last
    description will be used.

    If the parser already had a message description with the same unique id, it
    will be overwritten.

    \sa messageDescriptions(), setMessageDescriptions(),
    clearMessageDescriptions()
*/
void QCanFrameProcessor::addMessageDescriptions(const QList<QCanMessageDescription> &descriptions)
{
    for (const auto &desc : descriptions)
        d->messages.insert(desc.uniqueId(), desc);
}

/*!
    Replaces current message descriptions used by this frame processor with the
    new message descriptions \a descriptions.

    \sa messageDescriptions(), addMessageDescriptions(),
    clearMessageDescriptions()
*/
void QCanFrameProcessor::setMessageDescriptions(const QList<QCanMessageDescription> &descriptions)
{
    d->messages.clear();
    addMessageDescriptions(descriptions);
}

/*!
    Removes all message descriptions for this frame processor.

    \sa messageDescriptions(), addMessageDescriptions(),
    setMessageDescriptions()
*/
void QCanFrameProcessor::clearMessageDescriptions()
{
    d->messages.clear();
}

/*!
    Returns the unique identifier description.

    The unique identifier description must be valid in order to encode or decode
    the CAN bus frames. See the \l QCanUniqueIdDescription class documentation
    for more details.

    \sa setUniqueIdDescription(), QCanUniqueIdDescription
*/
QCanUniqueIdDescription QCanFrameProcessor::uniqueIdDescription() const
{
    return d->uidDescription;
}

/*!
    Sets the unique identifier description to \a description.

    The unique identifier description must be valid in order to encode or decode
    the CAN bus frames. See the \l QCanUniqueIdDescription class documentation
    for more details.

    \sa uniqueIdDescription(), QCanUniqueIdDescription
*/
void QCanFrameProcessor::setUniqueIdDescription(const QCanUniqueIdDescription &description)
{
    d->uidDescription = description;
}

/*!
    Parses the frame \a frame using the specified message descriptions.

    The process of parsing is as follows:
    \list 1
        \li The \l uniqueIdDescription() is used to extract the unique
            identifier of the message.
        \li The extracted unique identifier is used to search for a suitable
            \l QCanMessageDescription from the list of all available
            \l messageDescriptions().
        \li The matching \l QCanMessageDescription is used to extract
            the signal values from the frame.
    \endlist

    This method returns a \l QCanFrameProcessor::ParseResult, which contains
    both the extracted unique identifier and a \l QVariantMap with the signals
    and their values. The keys of the map are the
    \l {QCanSignalDescription::name}{signal names}, and the values of the map
    are signal values.

    If an error occurred during the decoding, a result with empty
    \l {QCanFrameProcessor::ParseResult::}{signalValues} is returned.
    In such cases, the \l error() and \l errorString() methods can be used
    to get information about the errors.

    \note Calling this method clears all previous errors and warnings.

    \sa addMessageDescriptions(), error(), errorString(), warnings()
*/
QCanFrameProcessor::ParseResult QCanFrameProcessor::parseFrame(const QCanBusFrame &frame)
{
    d->resetErrors();

    if (!frame.isValid()) {
        d->setError(Error::InvalidFrame, QObject::tr("Invalid frame."));
        return {};
    }
    if (frame.frameType() != QCanBusFrame::DataFrame) {
        d->setError(Error::UnsupportedFrameFormat, QObject::tr("Unsupported frame format."));
        return {};
    }
    if (!d->uidDescription.isValid()) {
        d->setError(Error::Decoding,
                    QObject::tr("No valid unique identifier description is specified."));
        return {};
    }

    const auto uidOpt = d->extractUniqueId(frame);
    if (!uidOpt.has_value()) {
        d->setError(Error::Decoding,
                    QObject::tr("Failed to extract unique id from the frame."));
        return {};
    }

    const auto uniqueId = uidOpt.value();
    if (!d->messages.contains(uniqueId)) {
        d->setError(Error::Decoding,
                    QObject::tr("Could not find a message description for unique id %1.").
                    arg(qToUnderlying(uniqueId)));
        return {};
    }

    const auto message = d->messages.value(uniqueId);
    if (message.size() != frame.payload().size()) {
        d->setError(Error::Decoding,
                    QObject::tr("Payload size does not match message description. "
                                "Actual size = %1, expected size = %2.").
                    arg(frame.payload().size()).arg(message.size()));
        return {};
    }

    QVariantMap parsedSignals;
    // The multiplexor signals can form a complex dependency, so we can't
    // simply iterate through the signal descriptions in a natural order.
    // Instead, we first need to process all signals with no dependency on
    // other multiplexors, then handle the signals that have dependency on
    // already parsed signals, and so on, until we parse all signals.
    // One potential problem here is that the dependencies can be specified
    // incorrectly (for example, we can have circular dependencies, or
    // dependencies on non-existent signal), so we need to come up with a
    // reasonable condition to stop.

    auto seenNeededSignals = [](const QCanSignalDescription &desc,
                                const QVariantMap &parsedSignals) -> bool {
        const auto muxSignals = desc.multiplexSignals();
        if (muxSignals.isEmpty())
            return true;
        const auto *descPrivate = QCanSignalDescriptionPrivate::get(desc);
        for (auto it = muxSignals.cbegin(); it != muxSignals.cend(); ++it) {
            const auto &name = it.key();
            const auto &ranges = it.value();
            if (!parsedSignals.contains(name)
                    || !descPrivate->muxValueInRange(parsedSignals.value(name), ranges)) {
                return false;
            }
        }
        return true;
    };

    auto descriptionsHash = QCanMessageDescriptionPrivate::get(message)->messageSignals;
    while (true) {
        QList<QString> newNames;
        for (const auto &desc : std::as_const(descriptionsHash)) {
            if (seenNeededSignals(desc, parsedSignals)) {
                newNames.push_back(desc.name());
                if (!desc.isValid()) {
                    d->addWarning(QObject::tr("Skipping signal %1 in message with unique id %2"
                                              " because its description is invalid.").
                                  arg(desc.name(), QString::number(qToUnderlying(uniqueId))));
                    continue;
                }
                const QVariant value = d->decodeSignal(frame, desc);
                if (value.isValid())
                    parsedSignals.insert(desc.name(), value);
            }
        }
        for (const auto &name : std::as_const(newNames))
            descriptionsHash.remove(name);
        if (newNames.isEmpty() || descriptionsHash.isEmpty()) {
            // We either processed all signals, or failed to process more during
            // the last loop. The latter means that the multiplexor conditions
            // do not match for the rest of the signals, which is fine and will
            // always happen when multiplexing
            break;
        }
    }

    return {uniqueId, parsedSignals};
}

/* QCanFrameProcessorPrivate implementation */

void QCanFrameProcessorPrivate::resetErrors()
{
    error = QCanFrameProcessor::Error::None;
    errorString.clear();
    warnings.clear();
}

void QCanFrameProcessorPrivate::setError(QCanFrameProcessor::Error err, const QString &desc)
{
    error = err;
    errorString = desc;
}

void QCanFrameProcessorPrivate::addWarning(const QString &warning)
{
    warnings.push_back(warning);
}

QVariant QCanFrameProcessorPrivate::decodeSignal(const QCanBusFrame &frame,
                                                 const QCanSignalDescription &signalDesc)
{
    const auto signalDataEnd = extractMaxBitNum(signalDesc.startBit(), signalDesc.bitLength(),
                                                signalDesc.dataEndian());
    const bool dataFromPayload =
            signalDesc.dataSource() == QtCanBus::DataSource::Payload;

    const auto frameIdLength = frame.hasExtendedFrameFormat() ? 29 : 11;
    const auto maxDataLength = dataFromPayload ? frame.payload().size() * 8
                                               : frameIdLength;

    if (signalDataEnd >= maxDataLength) {
        addWarning(QObject::tr("Skipping signal %1 in message with unique id %2. "
                               "Its expected length exceeds the data length.").
                   arg(signalDesc.name(), QString::number(frame.frameId())));
        return QVariant();
    }

    const QByteArray payload = frame.payload();
    const auto frameId = frame.frameId();
    const unsigned char *data = dataFromPayload
            ? reinterpret_cast<const unsigned char *>(payload.data())
            : reinterpret_cast<const unsigned char *>(&frameId);

    return parseData(data, signalDesc);
}

static bool needValueConversion(const QCanSignalDescription &signalDesc)
{
    return !qIsNaN(signalDesc.factor()) || !qIsNaN(signalDesc.offset())
            || !qIsNaN(signalDesc.scaling());
}

template <typename T>
static double convertFromCanValue(T value, const QCanSignalDescription &signalDesc)
{
    double result = static_cast<double>(value);
    if (!qIsNaN(signalDesc.factor()))
        result *= signalDesc.factor();

    if (!qIsNaN(signalDesc.offset()))
        result += signalDesc.offset();

    if (!qIsNaN(signalDesc.scaling()))
        result *= signalDesc.scaling();

    return result;
}

static double convertToCanValue(const QVariant &value, const QCanSignalDescription &signalDesc)
{
    // Checks for 0 are done in the corresponding setters, so we can divide
    // safely.
    double result = value.toDouble();
    if (!qIsNaN(signalDesc.scaling()))
        result /= signalDesc.scaling();

    if (!qIsNaN(signalDesc.offset()))
        result -= signalDesc.offset();

    if (!qIsNaN(signalDesc.factor()))
        result /= signalDesc.factor();

    return result;
}

#ifdef USE_DBC_COMPATIBLE_BE_HANDLING

template <typename T>
static QVariant extractValue(const unsigned char *data, const QCanSignalDescription &signalDesc)
{
    constexpr auto tBitLength = sizeof(T) * 8;
    const auto length = signalDesc.bitLength();
    if constexpr (std::is_floating_point_v<T>)
        Q_ASSERT(tBitLength == length);
    else
        Q_ASSERT(tBitLength >= length);
    const auto maxBytesToRead = (length % 8 == 0) ? length / 8 : length / 8 + 1;
    const auto start = signalDesc.startBit();
    T value = {};
    const bool isBigEndian = signalDesc.dataEndian() == QSysInfo::Endian::BigEndian;
    if (isBigEndian) {
        // Big Endian - start bit is MSB
        if (start % 8 == 7 && length % 8 == 0) {
            // The data is aligned at byte offset, we can simply memcpy
            memcpy(&value, &data[(start - 7) / 8], maxBytesToRead);
        } else {
            // Data is not aligned at byte offset, we need to do some bit
            // shifting. We cannot perform bit operations on float or double
            // types, so we convert the value to uchar *.
            // Because of how BE data is organized, the indices for reading
            // would not be continuous. If we want to extract BE data from the
            // middle 12 bits of a 2-byte payload, we will need to read bits 5-0
            // and 15-10:
            // _________________________________________________________________
            // |7      |6      |5(MSB) |4      |3      |2      |1      |0      |
            // -----------------------------------------------------------------
            // |15     |14     |13     |12     |11     |10(LSB)|9      |8      |
            // -----------------------------------------------------------------
            unsigned char *valueData = reinterpret_cast<unsigned char *>(&value);
            qsizetype bitIdx = start;
            for (qsizetype processedBits = 0; processedBits < length; ++processedBits) {
                const auto dataByteIdx = bitIdx / 8;
                const auto dataBitIdx = bitIdx % 8;
                if (data[dataByteIdx] & (0x01 << dataBitIdx)) {
                    const auto byteIdx = processedBits / 8;
                    // start filling each byte from MSB
                    const auto bitIdx = 7 - (processedBits % 8);
                    valueData[byteIdx] |= (0x01 << bitIdx);
                }
                // handle jump like 0 -> 15 from the example above
                if (bitIdx % 8 == 0)
                    bitIdx += 15;
                else
                    --bitIdx;
            }
        }
    } else {
        // Little Endian - start bit is LSB
        if (start % 8 == 0 && length % 8 == 0) {
            // The data is aligned at byte offset, we can simply memcpy
            memcpy(&value, &data[start / 8], maxBytesToRead);
        } else {
            // Data is not aligned at byte offset, we need to do some bit
            // shifting. We cannot perform bit operations on float or double
            // types, so we convert the value to uchar *.
            unsigned char *valueData = reinterpret_cast<unsigned char *>(&value);
            quint16 valueIdx = 0;
            for (auto i = start; i < start + length; ++i, ++valueIdx) {
                const auto byteIdx = i / 8;
                const auto bitIdx = i % 8;
                if (data[byteIdx] & (0x01 << bitIdx))
                    valueData[valueIdx / 8] |= 0x01 << (valueIdx % 8);
            }
        }
    }
    // check and convert endian
    T convertedValue = {};
    if (isBigEndian)
        convertedValue = qFromBigEndian(value);
    else
        convertedValue = qFromLittleEndian(value);
    const bool endianChanged = convertedValue != value;
    value = convertedValue;
    // for signed & unsigned fill the most significant bits with proper values
    if constexpr (std::is_integral_v<T>) {
        if (tBitLength > length) {
            if (endianChanged) {
                // After endian conversion we have unneeded bits in the end,
                // so we need to cut them
                value = value >> (tBitLength - length);
            }
            // value has more bits than we could actually read, so we need to
            // fill the most significant bits properly
            const auto dataFormat = signalDesc.dataFormat();
            if (dataFormat == QtCanBus::DataFormat::SignedInteger) {
                if (value & (0x01ULL << (length - 1))) {
                    // msb = 1 -> negative value, fill the rest with 1's
                    for (auto i = length; i < tBitLength; ++i)
                        value |= (0x01ULL << i);
                } else {
                    // msb = 0 -> positive value, fill the rest with 0's
                    for (auto i = length; i < tBitLength; ++i)
                        value &= ~(0x01ULL << i);
                }
            } else if (dataFormat == QtCanBus::DataFormat::UnsignedInteger) {
                // simply fill most significant bits with 0's
                for (auto i = length; i < tBitLength; ++i)
                    value &= ~(0x01ULL << i);
            }
        }
    }
    // perform value conversions, if needed
    if (needValueConversion(signalDesc))
        return QVariant::fromValue(convertFromCanValue(value, signalDesc));

    return QVariant::fromValue(value);
}

#else

template <typename T>
static QVariant extractValue(const unsigned char *data, const QCanSignalDescription &signalDesc)
{
    constexpr auto tBitLength = sizeof(T) * 8;
    const auto length = signalDesc.bitLength();
    if constexpr (std::is_floating_point_v<T>)
        Q_ASSERT(tBitLength == length);
    else
        Q_ASSERT(tBitLength >= length);
    const auto maxBytesToRead = (length % 8 == 0) ? length / 8 : length / 8 + 1;
    const auto start = signalDesc.startBit();
    T value = {};
    if (start % 8 == 0 && length % 8 == 0) {
        // The data is aligned at byte offset, we can simply memcpy
        memcpy(&value, &data[start / 8], maxBytesToRead);
    } else {
        // Data is not aligned at byte offset, we need to do some bit shifting
        // We cannot perform bit operations on float or double types, so we
        // convert the value to uchar *.

        // If the data is in big endian, and data length % 8 != 0, then the
        // first byte is not full. So we need to read (8 - length % 8) bits
        // from it, and then complete it properly

        unsigned char *valueData = reinterpret_cast<unsigned char *>(&value);
        quint16 valueIdx = 0;
        quint16 startIdx = start;
        quint16 numToRead = length;
        if (signalDesc.dataEndian() == QSysInfo::Endian::BigEndian) {
            const auto readInFirstByte = length % 8;
            // else we have round number of bytes and all these tricks are not needed
            if (readInFirstByte) {
                const auto missingBits = 8 - readInFirstByte;
                bool lastBitIsOne = false;
                for (auto i = startIdx; i < startIdx + readInFirstByte; ++i, ++valueIdx) {
                    const auto byteIdx = i / 8;
                    const auto bitIdx = i % 8;
                    lastBitIsOne = data[byteIdx] & (0x01 << bitIdx);
                    if (lastBitIsOne)
                        valueData[valueIdx / 8] |= 0x01 << (valueIdx % 8);
                }
                if (lastBitIsOne) {
                    for (auto i = 0; i < missingBits; ++i, ++valueIdx)
                        valueData[valueIdx / 8] |= 0x01 << (valueIdx % 8);
                } else {
                    // We simply have zeros there, but still need to increase valueIdx
                    valueIdx += missingBits;
                }
                startIdx += readInFirstByte;
                numToRead -= readInFirstByte;
            }
        }
        for (auto i = startIdx; i < startIdx + numToRead; ++i, ++valueIdx) {
            const auto byteIdx = i / 8;
            const auto bitIdx = i % 8;
            if (data[byteIdx] & (0x01 << bitIdx))
                valueData[valueIdx / 8] |= 0x01 << (valueIdx % 8);
        }
    }
    // check and convert endian
    T convertedValue = {};
    if (signalDesc.dataEndian() == QSysInfo::Endian::LittleEndian)
        convertedValue = qFromLittleEndian(value);
    else
        convertedValue = qFromBigEndian(value);
    const bool endianChanged = convertedValue != value;
    value = convertedValue;
    // for signed & unsigned fill the most significant bits with proper values
    if constexpr (std::is_integral_v<T>) {
        if (tBitLength > length) {
            if (endianChanged) {
                // After endian conversion we have unneeded bits in the end,
                // so we need to cut them
                value = value >> (tBitLength - maxBytesToRead * 8);
            }
            // value has more bits than we could actually read, so we need to
            // fill the most significant bits properly
            const auto dataFormat = signalDesc.dataFormat();
            if (dataFormat == QtCanBus::DataFormat::SignedInteger) {
                if (value & (0x01ULL << (length - 1))) {
                    // msb = 1 -> negative value, fill the rest with 1's
                    for (auto i = length; i < tBitLength; ++i)
                        value |= (0x01ULL << i);
                } else {
                    // msb = 0 -> positive value, fill the rest with 0's
                    for (auto i = length; i < tBitLength; ++i)
                        value &= ~(0x01ULL << i);
                }
            } else if (dataFormat == QtCanBus::DataFormat::UnsignedInteger) {
                // simply fill most significant bits with 0's
                for (auto i = length; i < tBitLength; ++i)
                    value &= ~(0x01ULL << i);
            }
        }
    }
    // perform value conversions, if needed
    if (needValueConversion(signalDesc))
        return QVariant::fromValue(convertFromCanValue(value, signalDesc));

    return QVariant::fromValue(value);
}

#endif // USE_DBC_COMPATIBLE_BE_HANDLING

static QVariant parseAscii(const unsigned char *data, const QCanSignalDescription &signalDesc)
{
    Q_ASSERT(signalDesc.bitLength() % 8 == 0);

    const auto length = signalDesc.bitLength();
    const auto start = signalDesc.startBit();

    QByteArray value(length / 8, 0x00);

    char *valueData = value.data();
    quint16 valueIdx = 0;
    for (quint16 i = start; i < start + length; ++i, ++valueIdx) {
        const auto byteIdx = i / 8;
        const auto bitIdx = i % 8;
        if (data[byteIdx] & (0x01 << bitIdx))
            valueData[valueIdx / 8] |= 0x01 << (valueIdx % 8);
    }

    return QVariant(value);
}

QVariant QCanFrameProcessorPrivate::parseData(const unsigned char *data,
                                              const QCanSignalDescription &signalDesc)
{
    // We assume that signal's length does not exceed data size.
    // That is checked as a precondition to calling this method, so we do not
    // pass size for the data.
    switch (signalDesc.dataFormat()) {
    case QtCanBus::DataFormat::SignedInteger:
        return extractValue<qint64>(data, signalDesc);
    case QtCanBus::DataFormat::UnsignedInteger:
        return extractValue<quint64>(data, signalDesc);
    case QtCanBus::DataFormat::Float:
        return extractValue<float>(data, signalDesc);
    case QtCanBus::DataFormat::Double:
        return extractValue<double>(data, signalDesc);
    case QtCanBus::DataFormat::AsciiString:
        return parseAscii(data, signalDesc);
    }
    Q_UNREACHABLE();
}

#ifdef USE_DBC_COMPATIBLE_BE_HANDLING

template <typename T>
static void encodeValue(unsigned char *data, const QVariant &valueVar,
                        const QCanSignalDescription &signalDesc)
{
    constexpr auto tBitLength = sizeof(T) * 8;
    const auto length = signalDesc.bitLength();
    if constexpr (std::is_floating_point_v<T>)
        Q_ASSERT(tBitLength == length);
    else
        Q_ASSERT(tBitLength >= length);

    // Perform value conversion.
    T value = {};
    if (needValueConversion(signalDesc))
        value = static_cast<T>(std::round(convertToCanValue(valueVar, signalDesc)));
    else
        value = valueVar.value<T>();

    const bool dataLittleEndian = signalDesc.dataEndian() == QSysInfo::Endian::LittleEndian;

    const auto maxBytesToWrite = (length % 8 == 0) ? length / 8 : length / 8 + 1;

    // always treat the value-to-write as LE for simplicity
    value = qToLittleEndian(value);
    const quint16 start = signalDesc.startBit();
    if (dataLittleEndian) {
        // Little Endian
        if (start % 8 == 0 && length % 8 == 0) {
            // The data is aligned at byte offset, and has a round number of
            // bytes, so we can simply memcpy
            memcpy(&data[start / 8], &value, maxBytesToWrite);
        } else {
            const uchar *valueData = reinterpret_cast<const uchar *>(&value);
            for (quint16 i = 0; i < length; ++i) {
                const auto valueByteIdx = i / 8;
                const auto valueBitIdx = i % 8;
                const auto dataByteIdx = (start + i) / 8;
                const auto dataBitIdx = (start + i) % 8;

                if (valueData[valueByteIdx] & (0x01 << valueBitIdx))
                    data[dataByteIdx] |= (0x01 << dataBitIdx);
                else
                    data[dataByteIdx] &= ~(0x01 << dataBitIdx);
            }
        }
    } else {
        // Big Endian
        if (start % 8 == 7 && length % 8 == 0) {
            // The data is aligned at byte offset and has a round number of
            // bytes, so we can simply memcpy. Just need to convert to BE and
            // take the meaningful bytes (those will be the most significant
            // bytes after switching to BE).
            value = qToBigEndian(value);
            const uchar *valueData = reinterpret_cast<const uchar *>(&value);
            const auto byteIdx = sizeof(value) - maxBytesToWrite;
            memcpy(&data[(start - 7) / 8], &valueData[byteIdx], maxBytesToWrite);
        } else {
            // We need to start from the MSB of the valueToWrite
            // Because of how BE data is organized, the indices for writing
            // would not be continuous. If we want to write BE data to the
            // middle 12 bits of a 2-byte payload, we will need to write bits
            // 5-0 and 15-10:
            // _________________________________________________________________
            // |7      |6      |5(MSB) |4      |3      |2      |1      |0      |
            // -----------------------------------------------------------------
            // |15     |14     |13     |12     |11     |10(LSB)|9      |8      |
            // -----------------------------------------------------------------
            const uchar *valueData = reinterpret_cast<const uchar *>(&value);
            auto dataBit = signalDesc.startBit();
            for (auto valueBit = length - 1; valueBit >= 0; --valueBit) {
                const auto valueByteIdx = valueBit / 8;
                const auto valueBitIdx = valueBit % 8;
                const auto dataByteIdx = dataBit / 8;
                const auto dataBitIdx = dataBit % 8;
                if (valueData[valueByteIdx] & (0x01 << valueBitIdx))
                    data[dataByteIdx] |= (0x01 << dataBitIdx);
                else
                    data[dataByteIdx] &= ~(0x01 << dataBitIdx);
                // handle jumps like 0 -> 15
                if (dataBit % 8 == 0)
                    dataBit += 15;
                else
                    --dataBit;
            }
        }
    }
}

#else

static constexpr bool isNativeLittleEndian()
{
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    return true;
#else
    return false;
#endif
}

template <typename T>
static void encodeValue(unsigned char *data, const QVariant &valueVar,
                        const QCanSignalDescription &signalDesc)
{
    constexpr auto tBitLength = sizeof(T) * 8;
    const auto length = signalDesc.bitLength();
    if constexpr (std::is_floating_point_v<T>)
        Q_ASSERT(tBitLength == length);
    else
        Q_ASSERT(tBitLength >= length);
    const auto maxBytesToWrite = (length % 8 == 0) ? length / 8 : length / 8 + 1;

    // Perform value conversion.
    T value = {};
    if (needValueConversion(signalDesc))
        value = static_cast<T>(std::round(convertToCanValue(valueVar, signalDesc)));
    else
        value = valueVar.value<T>();

    // Check endian.
    // When doing endian-conversion for values with (bitSize % 8 != 0) we must
    // be very careful, because qTo{Little,Big}Endian swaps whole bytes.
    // After swapping the last byte, which could have less than 8 meaningful
    // bits, becomes the first byte. So we need to adjust it carefully, shifting
    // it in such a way that all meaningless bits are skipped.
    // We also need to consider that we operate on q{u}int64 values for
    // {un}signed integers, so we need to chop the unneeded bytes first.
    const bool dataLittleEndian =
            signalDesc.dataEndian() == QSysInfo::Endian::LittleEndian;

    T valueToWrite = value;
    quint16 writeOffset = 0;
    if (dataLittleEndian && !isNativeLittleEndian()) {
        valueToWrite = qToLittleEndian(valueToWrite);
    } else if (!dataLittleEndian && isNativeLittleEndian()) {
        valueToWrite = qToBigEndian(valueToWrite);
        // for floating point types we always pass the exact type, so no need
        // to shift extra/unneeded bits
        if constexpr (!std::is_floating_point_v<T>) {
            // get rid of the unneeded bytes
            valueToWrite = valueToWrite >> (tBitLength - maxBytesToWrite * 8);
            // skip meaningless bits in the first byte
            writeOffset = maxBytesToWrite * 8 - length;
            if (writeOffset > 0) {
                uchar *valueData = reinterpret_cast<uchar *>(&valueToWrite);
                valueData[0] = valueData[0] << writeOffset;
            }
        }
    }

    const quint16 start = signalDesc.startBit();
    if (start % 8 == 0 && length % 8 == 0) {
        // The data is aligned at byte offset, and has a round number of bytes,
        // so we can simply memcpy
        memcpy(&data[start / 8], &valueToWrite, maxBytesToWrite);
    } else {
        const uchar *valueData = reinterpret_cast<const uchar *>(&valueToWrite);
        for (quint16 i = 0; i < length; ++i) {
            const auto valueByteIdx = (i + writeOffset) / 8;
            const auto valueBitIdx = (i + writeOffset) % 8;
            const auto dataByteIdx = (start + i) / 8;
            const auto dataBitIdx = (start + i) % 8;

            if (valueData[valueByteIdx] & (0x01 << valueBitIdx))
                data[dataByteIdx] |= 0x01 << dataBitIdx;
            else
                data[dataByteIdx] &= ~(0x01 << dataBitIdx);
        }
    }
}

#endif // USE_DBC_COMPATIBLE_BE_HANDLING

static void encodeAscii(unsigned char *data, const QVariant &value,
                        const QCanSignalDescription &signalDesc)
{
    Q_ASSERT(signalDesc.bitLength() % 8 == 0);

    const QByteArray ascii = value.toByteArray();
    // The ascii array can have more or less bytes. Handle it.
    const auto length = std::min(ascii.size() * 8, static_cast<qsizetype>(signalDesc.bitLength()));

    const auto start = signalDesc.startBit();
    for (auto i = 0; i < length; ++i) {
        const auto dataByteIdx = (start + i) / 8;
        const auto dataBitIdx = (start + i) % 8;
        if (ascii.data()[i / 8] & (0x01 << (i % 8)))
            data[dataByteIdx] |= 0x01 << dataBitIdx;
        else
            data[dataByteIdx] &= ~(0x01 << dataBitIdx);
    }
    if (length < signalDesc.bitLength()) {
        // fill the rest of the bits with 0's
        for (auto i = length; i < signalDesc.bitLength(); ++i)
            data[i / 8] &= ~(0x01 << (i % 8));
    }
}

void QCanFrameProcessorPrivate::encodeSignal(unsigned char *data, const QVariant &value,
                                             const QCanSignalDescription &signalDesc)
{
    // We assume that signal's length does not exceed data size.
    // That is checked as a precondition to calling this method, so we do not
    // pass size for the data.
    switch (signalDesc.dataFormat()) {
    case QtCanBus::DataFormat::SignedInteger:
        encodeValue<qint64>(data, value, signalDesc);
        break;
    case QtCanBus::DataFormat::UnsignedInteger:
        encodeValue<quint64>(data, value, signalDesc);
        break;
    case QtCanBus::DataFormat::Float:
        encodeValue<float>(data, value, signalDesc);
        break;
    case QtCanBus::DataFormat::Double:
        encodeValue<double>(data, value, signalDesc);
        break;
    case QtCanBus::DataFormat::AsciiString:
        encodeAscii(data, value, signalDesc);
        break;
    }
}

std::optional<QtCanBus::UniqueId>
QCanFrameProcessorPrivate::extractUniqueId(const QCanBusFrame &frame) const
{
    const auto signalDataEnd = extractMaxBitNum(uidDescription.startBit(),
                                                uidDescription.bitLength(),
                                                uidDescription.endian());
    const bool dataFromPayload = uidDescription.source() == QtCanBus::DataSource::Payload;

    // For the FrameId case we do not really care if the frame id is extended
    // or not, because QCanBusFrame::FrameId is anyway 32-bit unsigned.
    const auto maxDataLength = dataFromPayload ? frame.payload().size() * 8 : 29;

    if (signalDataEnd >= maxDataLength)
        return {}; // add a more specific error description?

    const QByteArray payload = frame.payload();
    const auto frameId = frame.frameId();
    const unsigned char *data = dataFromPayload
            ? reinterpret_cast<const unsigned char *>(payload.data())
            : reinterpret_cast<const unsigned char *>(&frameId);

    // Now we need to do the same as when extracting a value for a signal, but
    // without additional value conversions. We have an extractValue() template
    // function, but it takes a QCanSignalDescription as an input parameter.
    // To reuse the code, we generate a dummy QCanSignalDescription based on the
    // values of uidDescription and call extractValue().
    // This approach introduces some unneeded checks and also result conversions
    // to/from QVariant. If this becomes a problem, we can copy-paste the code
    // from extractValue() and remove the unneeded parts.

    QCanSignalDescription dummyDesc;
    dummyDesc.setDataSource(uidDescription.source());
    dummyDesc.setDataEndian(uidDescription.endian());
    dummyDesc.setStartBit(uidDescription.startBit());
    dummyDesc.setBitLength(uidDescription.bitLength());
    dummyDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
    // other fields are unused, so default-initialized

    using UnderlyingType = std::underlying_type_t<QtCanBus::UniqueId>;
    const QVariant val = extractValue<UnderlyingType>(data, dummyDesc);
    return QtCanBus::UniqueId{val.value<UnderlyingType>()};
}

bool QCanFrameProcessorPrivate::fillUniqueId(unsigned char *data, quint16 sizeInBits,
                                             QtCanBus::UniqueId uniqueId)
{
    const auto uidDataEnd = extractMaxBitNum(uidDescription.startBit(),
                                             uidDescription.bitLength(),
                                             uidDescription.endian());
    if (uidDataEnd >= sizeInBits) {
        return false; // add a more specific error description?
    }

    // Now we need to do the same as when encoding signal value into the frame,
    // but without additional value conversions. We have encodeValue() template
    // function, but it takes QCanSignalDescription as an input parameter.
    // To reuse the code, we generate a dummy QCanSignalDescription based on the
    // values of uidDescription, and call encodeValue().
    // This approach introduces some unneeded checks and QVariant conversions.
    // If this becomes a problem, we can copy-paste the code from encodeValue()
    // and remove all the unneeded parts.

    QCanSignalDescription dummyDesc;
    dummyDesc.setDataSource(uidDescription.source());
    dummyDesc.setDataEndian(uidDescription.endian());
    dummyDesc.setStartBit(uidDescription.startBit());
    dummyDesc.setBitLength(uidDescription.bitLength());
    dummyDesc.setDataFormat(QtCanBus::DataFormat::UnsignedInteger);
    // other fields are unused, so default-initialized

    using UnderlyingType = std::underlying_type_t<QtCanBus::UniqueId>;
    encodeValue<UnderlyingType>(data, QVariant::fromValue(qToUnderlying(uniqueId)), dummyDesc);
    return true;
}

QCanFrameProcessorPrivate *QCanFrameProcessorPrivate::get(const QCanFrameProcessor &processor)
{
    return processor.d.get();
}

QT_END_NAMESPACE

#include "moc_qcanframeprocessor.cpp"
