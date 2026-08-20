// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qcandbcfileparser.h"
#include "qcandbcfileparser_p.h"
#include "qcanmessagedescription.h"
#include "qcansignaldescription.h"
#include "qcanuniqueiddescription.h"
#include "private/qcanmessagedescription_p.h"
#include "private/qcansignaldescription_p.h"

#include <QtCore/QFile>
#include <QtCore/QRegularExpression>

#include <optional>

QT_BEGIN_NAMESPACE

/*!
    \class QCanDbcFileParser
    \inmodule QtSerialBus
    \since 6.5
    \preliminary

    \brief The QCanDbcFileParser class can be used to parse DBC files.

    A CAN database or CAN DBC file is an ASCII text file that contains
    information on how to decode and interpret raw CAN bus data. Some more
    details about the format can be found \l {CSSElectronics DBC Intro}{here}
    or \l {OpenVehicles DBC Intro}{here}.

    The QCanDbcFileParser class takes the input DBC file, parses it, and
    provides a list of \l {QCanMessageDescription}s as an output. These message
    descriptions can be forwarded to \l QCanFrameProcessor, and later used
    as rules to encode or decode \l {QCanBusFrame}s.

    Use one of \l parse() overloads to specify a file or a list of files that
    will be processed. Both overloads return \c true if the parsing completes
    successfully and \c false otherwise.

    Call the \l error() method to get the error which occurred during the
    parsing. If the parsing completes successfully, this method will return
    \l {QCanDbcFileParser::}{None}. Otherwise, you can use an
    \l errorString() method to get the string representation of an error.

    During the parsing some non-critical problems may occur as well. Such
    problems will be logged, but the parsing process will not be aborted. You
    can use the \l warnings() method to get the full list of such problems
    after the parsing is completed.

    If the parsing completes successfully, call \l messageDescriptions() to get
    a list of the message descriptions that were extracted during the last
    \l parse() call. Call \l messageValueDescriptions() to get the textual
    descriptions of signal raw values, if they are available.

    Use the static \l uniqueIdDescription() function to get a
    \l QCanUniqueIdDescription for the DBC format.

    \code
    QCanDbcFileParser fileParser;
    const bool result = fileParser.parse(u"path/to/file.dbc"_s);
    // Check result, call error() and warnings() if needed

    // Prepare a QCanFrameProcessor to decode or encode DBC frames
    QCanFrameProcessor frameProcessor;
    frameProcessor.setUniqueIdDescription(QCanDbcFileParser::uniqueIdDescription());
    frameProcessor.setMessageDescriptions(fileParser.messageDescriptions());
    \endcode

    \note The parser is stateful, which means that all the results (like
    extracted message descriptions, error code, or warnings) are reset once the
    next parsing starts.

    \section2 Supported Keywords

    The current implementation supports only a subset of keywords that you can
    find in a DBC file:

    \list
        \li \c {BO_} - message description.
        \li \c {SG_} - signal description.
        \li \c {SIG_VALTYPE_} - signal type description.
        \li \c {SG_MUL_VAL_} - extended multiplexing description.
        \li \c {CM_} - comments (only for message and signal descriptions).
        \li \c {VAL_} - textual descriptions for raw signal values.
    \endlist

    Lines starting from other keywords are simply ignored.

    \sa QCanMessageDescription, QCanFrameProcessor
*/

/*!
    \typealias QCanDbcFileParser::ValueDescriptions

    This is a type alias for \c {QHash<quint32, QString>}.

    The keys of the hash represent raw signal values, and the values of the
    hash represent corresponding string descriptions.
*/

/*!
    \typealias QCanDbcFileParser::SignalValueDescriptions

    This is a type alias for \c {QHash<QString, ValueDescriptions>}.

    The keys of the hash represent signal names, and the values of the
    hash contain the corresponding \l QCanDbcFileParser::ValueDescriptions
    entries.

    \sa QCanDbcFileParser::ValueDescriptions
*/

/*!
    \typealias QCanDbcFileParser::MessageValueDescriptions

    This is a type alias for
    \c {QHash<QtCanBus::UniqueId, SignalValueDescriptions>}.

    The keys of the hash represent message unique ids, and the values of the
    hash contain the corresponding \l QCanDbcFileParser::SignalValueDescriptions
    entries.

    \sa QCanDbcFileParser::SignalValueDescriptions
*/

/*!
    \enum QCanDbcFileParser::Error

    This enum represents the possible errors that can happen during the parsing
    of a DBC file.

    \value None No error occurred.
    \value FileReading An error occurred while opening or reading the file.
    \value Parsing An error occurred while parsing the content of the file.
*/

/*!
    Constructs a DBC file parser.
*/
QCanDbcFileParser::QCanDbcFileParser()
    : d(std::make_unique<QCanDbcFileParserPrivate>())
{
}

/*!
    Destroys this DBC file parser.
*/
QCanDbcFileParser::~QCanDbcFileParser() = default;

/*!
    Parses the file \a fileName. Returns \c true if the parsing completed
    successfully or \c false otherwise.

    If the parsing completed successfully, call the \l messageDescriptions()
    method to get the list of all extracted message descriptions.

    If the parsing failed, call the \l error() and \l errorString() methods
    to get the information about the error.

    Call the \l warnings() method to get the list of warnings that were
    logged during the parsing.

    \note This method expects the file contents to be encoded in UTF-8. If the
    file has a different encoding, decode it first, and use \l parseData()
    to extract the DBC information.

    \sa messageDescriptions(), error(), warnings(), parseData()
*/
bool QCanDbcFileParser::parse(const QString &fileName)
{
    d->reset();
    return d->parseFile(fileName);
}

/*!
    \overload

    Parses a list of files \a fileNames. Returns \c true if the parsing
    completed successfully or \c false otherwise.

    If the parsing completed successfully, call the \l messageDescriptions()
    method to get the list of all extracted message descriptions.

    The parsing stops at the first error. Call the \l error() and
    \l errorString() methods to get the information about the error.

    Call the \l warnings() method to get the list of warnings that were
    logged during the parsing.

    \note This method expects the file contents to be encoded in UTF-8. If the
    file has a different encoding, decode it first, and use \l parseData()
    to extract the DBC information.

    \sa messageDescriptions(), error(), warnings(), parseData()
*/
bool QCanDbcFileParser::parse(const QStringList &fileNames)
{
    d->reset();
    for (const auto &fileName : fileNames) {
        if (!d->parseFile(fileName))
            return false;
    }
    return true;
}

/*!
    \since 6.7

    Parses the input data \a data and returns \c true if the parsing completed
    successfully or \c false otherwise.

    If the parsing completed successfully, call the \l messageDescriptions()
    method to get the list of all extracted message descriptions.

    If the parsing failed, call the \l error() and \l errorString() methods
    to get the information about the error.

    Call the \l warnings() method to get the list of warnings that were
    logged during the parsing.

    The method expects that \a data is the content of a valid DBC file,
    properly converted to QStringView.

    Use this method when the input file has an encoding different from UTF-8.

    \code
    // Read the data from a DBC file with custom encoding
    const QByteArray initialData = ...;
    // Convert to UTF-16 using QStringDecoder or some other way
    const QString decodedData = ...;
    QCanDbcFileParser parser;
    const bool result = parser.parseData(decodedData);
    \endcode

    \sa messageDescriptions(), error(), warnings(), parse()
*/
bool QCanDbcFileParser::parseData(QStringView data)
{
    d->reset();
    return d->parseData(data);
}

/*!
    Returns the list of message descriptions that were extracted during the
    last \l parse() call.

    \sa parse(), error()
*/
QList<QCanMessageDescription> QCanDbcFileParser::messageDescriptions() const
{
    return d->getMessages();
}

/*!
    Returns the textual descriptions for signal raw values.

    DBC supports the possibility to provide textual descriptions to signal raw
    values. If such data exists in the parsed DBC file(s), it can be accessed
    using this function.

    The textual descriptions are unique for a certain signal within a specific
    message, so the returned structure contains the information about the
    message unique id and the signal name, as well as the actual value
    descriptions.

    \sa QCanDbcFileParser::MessageValueDescriptions,
    QCanDbcFileParser::SignalValueDescriptions,
    QCanDbcFileParser::ValueDescriptions
*/
QCanDbcFileParser::MessageValueDescriptions QCanDbcFileParser::messageValueDescriptions() const
{
    return d->m_valueDescriptions;
}

/*!
    Returns the last error which occurred during the parsing.

    \sa errorString(), parse()
*/
QCanDbcFileParser::Error QCanDbcFileParser::error() const
{
    return d->m_error;
}

/*!
    Returns the text representation of the last error which occurred during the
    parsing or an empty string if there was no error.

    \sa error()
*/
QString QCanDbcFileParser::errorString() const
{
    return d->m_errorString;
}

/*!
    Returns the list of non-critical problems which occurred during the parsing.

    A typical problem can be a malformed message or signal description. In such
    cases the malformed message or signal is skipped, but the rest of the file
    can be processed as usual.

    \sa error(), parse()
*/
QStringList QCanDbcFileParser::warnings() const
{
    return d->m_warnings;
}

/*!
    Returns a unique identifier description. DBC protocol always uses the
    Frame Id as an identifier, and therefore the unique identifier description
    is always the same.

    Use this method to get an instance of \l QCanUniqueIdDescription and pass
    it to \l QCanFrameProcessor.

    \sa QCanFrameProcessor::setUniqueIdDescription()
*/
QCanUniqueIdDescription QCanDbcFileParser::uniqueIdDescription()
{
    QCanUniqueIdDescription desc;
    desc.setSource(QtCanBus::DataSource::FrameId);
    desc.setEndian(QSysInfo::Endian::LittleEndian);
    desc.setStartBit(0);
    desc.setBitLength(29); // for both extended and normal frame id
    return desc;
}

/* QCanDbcFileParserPrivate implementation */

using namespace Qt::StringLiterals;

// signal name with whitespaces is invalid in DBC, so we can safely use it
// for internal purposes
static const auto kQtDummySignal = u"Qt Dummy Signal"_s;

static constexpr auto kMessageDef = "BO_ "_L1;
static constexpr auto kSignalDef = "SG_ "_L1;
static constexpr auto kSigValTypeDef = "SIG_VALTYPE_ "_L1;
static constexpr auto kCommentDef = "CM_ "_L1;
static constexpr auto kExtendedMuxDef = "SG_MUL_VAL_ "_L1;
static constexpr auto kValDef = "VAL_ "_L1;

static constexpr auto kUnsignedIntRegExp = "\\d+"_L1;
static constexpr auto kDoubleRegExp = "[+-]?\\d+((.\\d*)?([eE][+-]?\\d+)?)?"_L1;
static constexpr auto kDbcIdentRegExp = "[_[:alpha:]][_[:alnum:]]+"_L1;
static constexpr auto kOneOrMoreSpaceRegExp = "[ ]+"_L1;
static constexpr auto kMaybeSpaceRegExp = "[ ]*"_L1;
static constexpr auto kMuxIndicatorRegExp = "M|m\\d+M?"_L1;
static constexpr auto kByteOrderRegExp = "0|1"_L1;
static constexpr auto kValueTypeRegExp = "\\+|\\-"_L1;
// The pattern matches all printable characters, except double-quote (") and backslash (\).
static constexpr auto kCharStrRegExp = "((?![\\\"\\\\])\\P{Cc})*"_L1;

void QCanDbcFileParserPrivate::reset()
{
    m_fileName.clear();
    m_error = QCanDbcFileParser::Error::None;
    m_errorString.clear();
    m_warnings.clear();
    m_lineOffset = 0;
    m_isProcessingMessage = false;
    m_seenExtraData = false;
    m_currentMessage = {};
    m_messageDescriptions.clear();
    m_valueDescriptions.clear();
}

/*!
    \internal
    Returns \c false only in case of hard error. Returns \c true even if some
    warnings occurred during parsing.
*/
bool QCanDbcFileParserPrivate::parseFile(const QString &fileName)
{
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly)) {
        m_error = QCanDbcFileParser::Error::FileReading;
        m_errorString = f.errorString();
        return false;
    }
    m_fileName = fileName;
    m_seenExtraData = false;

    while (!f.atEnd()) {
        const QString str = QString::fromUtf8(f.readLine().trimmed());
        if (!processLine({str.constData(), str.size()})) // also sets the error properly
            return false;
    }
    addCurrentMessage(); // check if we need to add the message
    // now when we parsed the whole file, we can verify the signal multiplexing
    postProcessSignalMultiplexing();

    return true;
}

struct ReadData
{
    qsizetype index;
    QStringView result;
};

static ReadData readUntilNewline(QStringView in, qsizetype from)
{
    const qsizetype idx = in.indexOf('\n'_L1, from);

    return (idx == -1) ? ReadData{idx, in.sliced(from).trimmed()}
                       : ReadData{idx, in.sliced(from, idx - from).trimmed()};
}

/*!
    \internal
    The implementation basically copies parseFile(), including all
    post-processing. The only difference is in extracting the data.
*/
bool QCanDbcFileParserPrivate::parseData(QStringView data)
{
    if (data.isEmpty()) {
        m_error = QCanDbcFileParser::Error::Parsing;
        m_errorString = QObject::tr("Empty input data.");
        return false;
    }
    m_seenExtraData = false;
    qsizetype from = 0;
    while (true) {
        const auto [idx, sv] = readUntilNewline(data, from);
        if (!processLine(sv))
            return false;
        if (idx == -1) // reached the end of the string
            break;
        from = idx + 1;
    }
    addCurrentMessage();
    postProcessSignalMultiplexing();
    return true;
}

/*!
    \internal
    Returns \c false only in case of hard error. Returns \c true even if some
    warnings occurred during parsing.
*/
bool QCanDbcFileParserPrivate::processLine(const QStringView line)
{
    QStringView data = line;
    m_lineOffset = 0;

    auto handleParsingError = [this](QLatin1StringView section) {
        m_error = QCanDbcFileParser::Error::Parsing;
        m_errorString = !m_fileName.isEmpty()
                ? QObject::tr("Failed to parse file %1. Unexpected position "
                              "of %2 section.").arg(m_fileName, section)
                : QObject::tr("Failed to parse input data. Unexpected position "
                              "of %1 section.").arg(section);
    };

    if (data.startsWith(kMessageDef)) {
        if (m_seenExtraData) {
            // Unexpected position of message description
            handleParsingError(kMessageDef);
            return false;
        }
        addCurrentMessage();
        if (!parseMessage(data))
            return false;
    }
    // signal definitions can be on the same line as message definition,
    // or on a separate line
    data = data.sliced(m_lineOffset).trimmed();
    while (data.startsWith(kSignalDef)) {
        if (!m_isProcessingMessage || m_seenExtraData) {
            // Unexpected position of signal description
            handleParsingError(kSignalDef);
            return false;
        }
        if (!parseSignal(data))
            return false;
        data = data.sliced(m_lineOffset).trimmed();
    }
    // If we detect one of the following lines, then message description is
    // finished. We also assume that we can have only one key at each line.
    if (data.startsWith(kSigValTypeDef)) {
        m_seenExtraData = true;
        addCurrentMessage();
        parseSignalType(data);
    } else if (data.startsWith(kCommentDef)) {
        m_seenExtraData = true;
        addCurrentMessage();
        parseComment(data);
    } else if (data.startsWith(kExtendedMuxDef)) {
        m_seenExtraData = true;
        addCurrentMessage();
        parseExtendedMux(data);
    } else if (data.startsWith(kValDef)) {
        m_seenExtraData = true;
        addCurrentMessage();
        parseValueDescriptions(data);
    }
    return true;
}

static std::optional<QtCanBus::UniqueId> extractUniqueId(QStringView view)
{
    bool ok = false;
    const uint value = view.toUInt(&ok);
    if (ok)
        return QtCanBus::UniqueId{value & 0x1FFFFFFF};
    return std::nullopt;
}

/*!
    \internal
    Returns \c false only in case of hard error. Returns \c true even if some
    warnings occurred during parsing.
*/
bool QCanDbcFileParserPrivate::parseMessage(const QStringView data)
{
    // The regexp matches the following definition:
    // BO_ message_id message_name ':' message_size transmitter
    // also considering the fact that spaces around ':' seem to be optional, and
    // allowing more than one space between parts.

    // %1 - messageDef
    // %2 - maybeSpace
    // %3 - unsignedInt
    // %4 - oneOrMoreSpace
    // %5 - DbcIdentifier
    static const QString regExStr =
            "%1%2(?<messageId>%3)%4(?<name>%5)%2:%2(?<size>%3)%4(?<transmitter>%5)"_L1.
            arg(kMessageDef, kMaybeSpaceRegExp, kUnsignedIntRegExp, kOneOrMoreSpaceRegExp,
                kDbcIdentRegExp);
    static const QRegularExpression messageRegExp(regExStr);

    m_isProcessingMessage = false;
    const auto match = messageRegExp.matchView(data);
    if (match.hasMatch()) {
        m_currentMessage = extractMessage(match);
        // can't check for isValid() here, because demands signal descriptions
        if (!m_currentMessage.name().isEmpty()) {
            m_isProcessingMessage = true;
        } else {
            addWarning(QObject::tr("Failed to parse message description from "
                                   "string %1").arg(data));
        }
        m_lineOffset = match.capturedEnd(0);
    } else {
        addWarning(QObject::tr("Failed to find message description in string %1").arg(data));
        m_lineOffset = data.size(); // skip this string
    }
    return true;
}

QCanMessageDescription
QCanDbcFileParserPrivate::extractMessage(const QRegularExpressionMatch &match)
{
    Q_ASSERT(match.hasMatch());
    QCanMessageDescription desc;
    desc.setName(match.captured(u"name"_s));

    const auto id = extractUniqueId(match.capturedView(u"messageId"_s));
    if (id.has_value()) {
        desc.setUniqueId(id.value());
    } else {
        addWarning(QObject::tr("Failed to parse frame id for message %1").arg(desc.name()));
        return {};
    }

    bool ok = false;
    const auto size = match.capturedView(u"size"_s).toUInt(&ok);
    if (ok) {
        desc.setSize(size);
    } else {
        addWarning(QObject::tr("Failed to parse size for message %1").arg(desc.name()));
        return {};
    }

    desc.setTransmitter(match.captured(u"transmitter"_s));

    return desc;
}

/*!
    \internal
    Returns \c false only in case of hard error. Returns \c true even if some
    warnings occurred during parsing.
*/
bool QCanDbcFileParserPrivate::parseSignal(const QStringView data)
{
    // The regexp should match the following pattern:
    //      SG_ signal_name multiplexer_indicator : start_bit |
    //      signal_size @ byte_order value_type ( factor , offset )
    //      [ minimum | maximum ] unit receiver {, receiver}
    // We also need to consider the fact that some of the spaces might be
    // optional, and we can potentially allow more spaces between parts.
    // Note that the end of the signal description can contain multiple
    // receivers. The regexp is supposed to extract all of them, but we use
    // only the first one for now.

    // %1 - SignalDef
    // %2 - MaybeSpace
    // %3 - DbcIdentifier
    // %4 - OneOrMoreSpace
    // %5 - MuxIndicator
    // %6 - unsignedInt
    // %7 - byteOrder
    // %8 - valueType
    // %9 - double
    // %10 - charStr
    static const QString regExStr =
            "%1%2(?<name>%3)(%4(?<mux>%5))?%2:%2(?<startBit>%6)%2\\|%2(?<sigSize>%6)%2@%2"
            "(?<byteOrder>%7)%2(?<valueType>%8)%4\\(%2(?<factor>%9)%2,%2(?<offset>%9)%2\\)"
            "%4\\[%2(?<min>%9)%2\\|%2(?<max>%9)%2\\]%4\"(?<unit>%10)\""
            "%4(?<receiver>%3)(%2,%2%3)*"_L1.
            arg(kSignalDef, kMaybeSpaceRegExp, kDbcIdentRegExp, kOneOrMoreSpaceRegExp,
                kMuxIndicatorRegExp, kUnsignedIntRegExp, kByteOrderRegExp, kValueTypeRegExp,
                kDoubleRegExp, kCharStrRegExp);
    static const QRegularExpression signalRegExp(regExStr);

    const auto match = signalRegExp.matchView(data);
    if (match.hasMatch()) {
        QCanSignalDescription desc = extractSignal(match);

        if (desc.isValid())
            m_currentMessage.addSignalDescription(desc);
        else
            addWarning(QObject::tr("Failed to parse signal description from string %1").arg(data));

        m_lineOffset = match.capturedEnd(0);
    } else {
        addWarning(QObject::tr("Failed to find signal description in string %1").arg(data));
        m_lineOffset = data.size(); // skip this string
    }
    return true;
}

QCanSignalDescription QCanDbcFileParserPrivate::extractSignal(const QRegularExpressionMatch &match)
{
    Q_ASSERT(match.hasMatch());
    QCanSignalDescription desc;
    desc.setName(match.captured(u"name"_s));

    bool ok = false;

    if (match.hasCaptured(u"mux"_s)) {
        const auto muxStr = match.capturedView(u"mux"_s);
        if (muxStr == u"M"_s) {
            desc.setMultiplexState(QtCanBus::MultiplexState::MultiplexorSwitch);
        } else if (muxStr.endsWith(u"M"_s, Qt::CaseSensitive)) {
            desc.setMultiplexState(QtCanBus::MultiplexState::SwitchAndSignal);
            const auto val = muxStr.sliced(1, muxStr.size() - 2).toUInt(&ok);
            if (!ok) {
                addWarning(QObject::tr("Failed to parse multiplexor value for signal %1").
                           arg(desc.name()));
                return {};
            }
            // We have the value, but we do not really know the multiplexor
            // switch name. To know it, we potentially need to parse all signals
            // for the message. So for now we just create a dummy entry, and
            // the actual signal name will be updated later;
            desc.addMultiplexSignal(kQtDummySignal, val);
        } else {
            desc.setMultiplexState(QtCanBus::MultiplexState::MultiplexedSignal);
            const auto val = muxStr.sliced(1).toUInt(&ok);
            if (!ok) {
                addWarning(QObject::tr("Failed to parse multiplexor value for signal %1").
                           arg(desc.name()));
                return {};
            }
            // Same as above
            desc.addMultiplexSignal(kQtDummySignal, val);
        }
    }

    const uint startBit = match.capturedView(u"startBit"_s).toUInt(&ok);
    if (ok) {
        desc.setStartBit(startBit);
    } else {
        addWarning(QObject::tr("Failed to parse start bit for signal %1").arg(desc.name()));
        return {};
    }

    const uint bitLength = match.capturedView(u"sigSize"_s).toUInt(&ok);
    if (ok) {
        desc.setBitLength(bitLength);
    } else {
        addWarning(QObject::tr("Failed to parse bit length for signal %1").arg(desc.name()));
        return {};
    }

    // 0 = BE; 1 = LE
    const auto endian = match.capturedView(u"byteOrder"_s) == u"0"_s
            ? QSysInfo::Endian::BigEndian : QSysInfo::Endian::LittleEndian;
    desc.setDataEndian(endian);

    // + = unsigned; - = signed
    const auto dataFormat = match.capturedView(u"valueType"_s) == u"+"_s
            ? QtCanBus::DataFormat::UnsignedInteger : QtCanBus::DataFormat::SignedInteger;
    desc.setDataFormat(dataFormat);

    const double factor = match.capturedView(u"factor"_s).toDouble(&ok);
    if (ok) {
        desc.setFactor(factor);
    } else {
        addWarning(QObject::tr("Failed to parse factor for signal %1").arg(desc.name()));
        return {};
    }

    const double offset = match.capturedView(u"offset"_s).toDouble(&ok);
    if (ok) {
        desc.setOffset(offset);
    } else {
        addWarning(QObject::tr("Failed to parse offset for signal %1").arg(desc.name()));
        return {};
    }

    const double min = match.capturedView(u"min"_s).toDouble(&ok);
    if (ok) {
        const double max = match.capturedView(u"max"_s).toDouble(&ok);
        if (ok)
            desc.setRange(min, max);
    }
    if (!ok) {
        addWarning(QObject::tr("Failed to parse value range from signal %1").arg(desc.name()));
        return {};
    }

    desc.setPhysicalUnit(match.captured(u"unit"_s));
    desc.setReceiver(match.captured(u"receiver"_s));

    return desc;
}

void QCanDbcFileParserPrivate::parseSignalType(const QStringView data)
{
    // The regexp should match the following pattern:
    //      SIG_VALTYPE_ message_id signal_name signal_extended_value_type ;
    // We also need to consider the fact that we can potentially allow more
    // spaces between parts.

    // %1 sigValTypeDef
    // %2 maybeSpace
    // %3 unsignedInt
    // %4 oneOrMoreSpace
    // %5 DbcIdentifier
    const QString regExStr =
            "%1%2(?<messageId>%3)%4(?<sigName>%5)%2:%2(?<type>%3)%2;"_L1.
            arg(kSigValTypeDef, kMaybeSpaceRegExp, kUnsignedIntRegExp,
                kOneOrMoreSpaceRegExp, kDbcIdentRegExp);
    const QRegularExpression sigValTypeRegEx(regExStr);

    const auto match = sigValTypeRegEx.matchView(data);
    if (!match.hasMatch()) {
        m_lineOffset = data.size();
        addWarning(QObject::tr("Failed to find signal value type description in string %1").
                   arg(data));
        return;
    }

    m_lineOffset = match.capturedEnd(0);

    const auto uidOptional = extractUniqueId(match.capturedView(u"messageId"_s));
    if (!uidOptional) {
        addWarning(QObject::tr("Failed to parse frame id from string %1").arg(data));
        return;
    }

    const QtCanBus::UniqueId uid = uidOptional.value();
    auto msgDesc = m_messageDescriptions.value(uid);
    if (msgDesc.isValid()) {
        const QString sigName = match.captured(u"sigName"_s);
        auto sigDesc = msgDesc.signalDescriptionForName(sigName);
        if (sigDesc.isValid()) {
            bool ok = false;
            const auto type = match.capturedView(u"type").toUInt(&ok);
            if (ok) {
                bool sigDescChanged = false;
                switch (type) {
                case 0: /* signed or unsigned integer */
                    // do nothing, as we already have signed/unsinged integer
                    // based on "SG_ " string
                    break;
                case 1: /* 32-bit IEEE-float */
                    sigDesc.setDataFormat(QtCanBus::DataFormat::Float);
                    sigDesc.setBitLength(32);
                    sigDescChanged = true;
                    break;
                case 2: /* 64-bit IEEE-double */
                    sigDesc.setDataFormat(QtCanBus::DataFormat::Double);
                    sigDesc.setBitLength(64);
                    sigDescChanged = true;
                    break;
                default:
                    // invalid value
                    break;
                }
                if (sigDescChanged) {
                    msgDesc.addSignalDescription(sigDesc);
                    m_messageDescriptions.insert(msgDesc.uniqueId(), msgDesc);
                }
            } else {
                addWarning(QObject::tr("Failed to parse data type from string %1").arg(data));
            }
        } else {
            addWarning(QObject::tr("Failed to find signal description for signal %1. "
                                   "Skipping string %2").arg(sigName, data));
        }
    } else {
        addWarning(QObject::tr("Failed to find message description for unique id %1. "
                               "Skipping string %2").arg(qToUnderlying(uid)).arg(data));
    }
}

void QCanDbcFileParserPrivate::parseComment(const QStringView data)
{
    // The comment for message or signal description is represented by the
    // following pattern:
    //      CM_ (BO_ message_id char_string | SG_ message_id signal_name char_string);

    // %1 commentDef
    // %2 maybeSpace
    // %3 messageDef
    // %4 signalDef
    // %5 oneOrMoreSpace
    // %6 unsignedInt
    // %7 DbcIdentifier
    // %8 charStr
    const QString regExStr =
            "%1%2(?<type>(%3|%4))%2(?<messageId>%6)%5((?<sigName>%7)%5)?\"(?<comment>%8)\"%2;"_L1.
            arg(kCommentDef, kMaybeSpaceRegExp, kMessageDef, kSignalDef, kOneOrMoreSpaceRegExp,
                kUnsignedIntRegExp, kDbcIdentRegExp, kCharStrRegExp);
    const QRegularExpression commentRegExp(regExStr);

    const auto match = commentRegExp.matchView(data);
    if (!match.hasMatch()) {
        // no warning here, as we ignore some "general" comments, and parse only
        // comments related to messages and signals
        m_lineOffset = data.size();
        return;
    }

    m_lineOffset = match.capturedEnd(0);

    const auto type = match.capturedView(u"type"_s);

    const auto uidOptional = extractUniqueId(match.capturedView(u"messageId"_s));
    if (!uidOptional) {
        addWarning(QObject::tr("Failed to parse frame id from string %1").arg(data));
        return;
    }

    const QtCanBus::UniqueId uid = uidOptional.value();
    auto messageDesc = m_messageDescriptions.value(uid);
    if (!messageDesc.isValid()) {
        addWarning(QObject::tr("Failed to find message description for unique id %1. "
                               "Skipping string %2").arg(qToUnderlying(uid)).arg(data));
        return;
    }

    if (type == kMessageDef) {
        const QString comment = match.captured(u"comment"_s);
        messageDesc.setComment(comment);
        m_messageDescriptions.insert(uid, messageDesc);
    } else if (type == kSignalDef) {
        const QString sigName = match.captured(u"sigName"_s);
        auto signalDesc = messageDesc.signalDescriptionForName(sigName);
        if (signalDesc.isValid()) {
            const QString comment = match.captured(u"comment"_s);
            signalDesc.setComment(comment);
            messageDesc.addSignalDescription(signalDesc);
            m_messageDescriptions.insert(uid, messageDesc);
        } else {
            addWarning(QObject::tr("Failed to find signal description for signal %1. "
                                   "Skipping string %2").arg(sigName, data));
        }
    }
}

void QCanDbcFileParserPrivate::parseExtendedMux(const QStringView data)
{
    // The extended multiplexing is defined by the following pattern:
    //      SG_MUL_VAL_ message_id multiplexed_signal_name
    //      multiplexor_switch_name multiplexor_value_ranges ;
    // Here multiplexor_value_ranges consists of multiple ranges, separated
    // by a whitespace, and one range is defined as follows:
    //      multiplexor_value_range = unsigned_integer - unsigned_integer

    // %1 extendedMuxDef
    // %2 maybeSpace
    // %3 unsignedInt
    // %4 oneOrMoreSpace
    // %5 DbcIdentifier
    const QString regExStr =
            "%1%2(?<messageId>%3)%4(?<multiplexedSignal>%5)%4(?<multiplexorSwitch>%5)%4"
            "(?<firstRange>%3%2-%2%3)(%2,%2%3%2-%2%3)*%2;"_L1.
            arg(kExtendedMuxDef, kMaybeSpaceRegExp, kUnsignedIntRegExp, kOneOrMoreSpaceRegExp,
                kDbcIdentRegExp);
    const QRegularExpression extendedMuxRegExp(regExStr);

    const auto match = extendedMuxRegExp.matchView(data);
    if (!match.hasMatch()) {
        m_lineOffset = data.size();
        addWarning(QObject::tr("Failed to find extended multiplexing description in string %1").
                   arg(data));
        return;
    }

    m_lineOffset = match.capturedEnd(0);

    const auto uidOptional = extractUniqueId(match.capturedView(u"messageId"_s));
    if (!uidOptional) {
        addWarning(QObject::tr("Failed to parse frame id from string %1").arg(data));
        return;
    }

    const QtCanBus::UniqueId uid = uidOptional.value();
    auto messageDesc = m_messageDescriptions.value(uid);
    if (!messageDesc.isValid()) {
        addWarning(QObject::tr("Failed to find message description for unique id %1. "
                               "Skipping string %2").arg(qToUnderlying(uid)).arg(data));
        return;
    }

    const QString multiplexedSignalName = match.captured(u"multiplexedSignal"_s);
    const QString multiplexorSwitchName = match.captured(u"multiplexorSwitch"_s);

    auto multiplexedSignal = messageDesc.signalDescriptionForName(multiplexedSignalName);
    auto multiplexorSwitch = messageDesc.signalDescriptionForName(multiplexorSwitchName);

    if (!multiplexedSignal.isValid() || !multiplexorSwitch.isValid()) {
        const QString invalidName = multiplexedSignal.isValid() ? multiplexorSwitchName
                                                                : multiplexedSignalName;
        addWarning(QObject::tr("Failed to find signal description for signal %1. "
                               "Skipping string %2").arg(invalidName, data));
        return;
    }

    auto signalRanges = multiplexedSignal.multiplexSignals();
    signalRanges.remove(kQtDummySignal); // dummy signal not needed anymore

    QCanSignalDescription::MultiplexValues rangeValues;
    auto rangeView = match.capturedView(u"firstRange"_s);
    const auto sepIdx = rangeView.indexOf(u'-');
    if (sepIdx != -1) {
        const auto min = rangeView.first(sepIdx).trimmed().toUInt();
        const auto max = rangeView.sliced(sepIdx + 1).trimmed().toUInt();
        rangeValues.push_back({min, max});
    }

    // We can have an arbitrary amount of ranges, so we can't use capture groups
    // to capture them. But we know that they follow a specific pattern (because
    // the full string matched the regexp). So we need to parse the rest of the
    // matched string manually
    const auto totalEnd = match.capturedEnd(0); // including the ';'
    const auto firstRangeEnd = match.capturedEnd(u"firstRange"_s);
    const auto len = totalEnd - firstRangeEnd - 1;
    if (len > 0) {
        const auto otherRangesView = data.sliced(firstRangeEnd, len).trimmed();
        const QStringTokenizer parts = otherRangesView.tokenize(u',', Qt::SkipEmptyParts);
        for (const QStringView range : parts) {
            const auto sepIdx = range.indexOf(u'-');
            if (sepIdx != -1) {
                const auto min = range.first(sepIdx).trimmed().toUInt();
                const auto max = range.sliced(sepIdx + 1).trimmed().toUInt();
                rangeValues.push_back({min, max});
            }
        }
    }

    if (!rangeValues.isEmpty())
        signalRanges.insert(multiplexorSwitchName, rangeValues);
    else
        signalRanges.remove(multiplexorSwitchName);

    // update the value
    multiplexedSignal.setMultiplexSignals(signalRanges);
    messageDesc.addSignalDescription(multiplexedSignal);
    m_messageDescriptions.insert(uid, messageDesc);
}

void QCanDbcFileParserPrivate::parseValueDescriptions(const QStringView data)
{
    // The regexp should match the following pattern:
    //      VAL_ message_id signal_name { value_description };
    // Here the value_description is defined as follows
    //      value_description = unsigned_int char_string

    // %1 valDef
    // %2 maybeSpace
    // %3 unsignedInt
    // %4 oneOrMoreSpace
    // %5 DbcIdentifier
    // %6 charStr
    const QString regExStr =
            "%1%2(?<messageId>%3)%4(?<signalName>%5)(%4%3%4\"(%6)\")+%2;"_L1.
            arg(kValDef, kMaybeSpaceRegExp, kUnsignedIntRegExp, kOneOrMoreSpaceRegExp,
                kDbcIdentRegExp, kCharStrRegExp);

    const QRegularExpression valueDescRegExp(regExStr);

    const auto match = valueDescRegExp.matchView(data);
    if (!match.hasMatch()) {
        m_lineOffset = data.size();
        addWarning(QObject::tr("Failed to parse value description from string %1").arg(data));
        return;
    }

    m_lineOffset = match.capturedEnd(0);

    const auto uidOptional = extractUniqueId(match.capturedView(u"messageId"_s));
    if (!uidOptional) {
        addWarning(QObject::tr("Failed to parse value description from string %1").arg(data));
        return;
    }

    const QtCanBus::UniqueId uid = uidOptional.value();
    // Check if the message exists
    const auto messageDesc = m_messageDescriptions.value(uid);
    if (!messageDesc.isValid()) {
        addWarning(QObject::tr("Failed to find message description for unique id %1. "
                               "Skipping string %2").arg(qToUnderlying(uid)).arg(data));
        return;
    }

    // Check if the signal exists within the message
    const QString signalName = match.captured(u"signalName"_s);
    if (!messageDesc.signalDescriptionForName(signalName).isValid()) {
        addWarning(QObject::tr("Failed to find signal description for signal %1. "
                               "Skipping string %2").arg(signalName, data));
        return;
    }

    // We can have an arbitrary amount of value descriptions, so we can't use
    // capture groups to capture them. But we know that they follow a specific
    // pattern (because the full string matched the regexp). So we need to parse
    // the rest of the matched string manually
    const auto totalEnd = match.capturedEnd(0); // including the ';'
    const auto signalNameEnd = match.capturedEnd(u"signalName"_s);
    const auto len = totalEnd - signalNameEnd - 1;
    if (len > 0) {
        auto signalDescriptionsView = data.sliced(signalNameEnd, len).trimmed();
        while (signalDescriptionsView.size()) {
            const auto spacePos = signalDescriptionsView.indexOf(u' ');
            if (spacePos == -1)
                break;
            bool ok = false;
            const auto value = signalDescriptionsView.sliced(0, spacePos).toUInt(&ok);
            if (!ok)
                break;
            const auto firstQuotePos = signalDescriptionsView.indexOf(u'"', spacePos + 1);
            if (firstQuotePos == -1)
                break;
            const auto nextQuotePos = signalDescriptionsView.indexOf(u'"', firstQuotePos + 1);
            if (nextQuotePos == -1)
                break;
            const auto description = signalDescriptionsView.sliced(
                        firstQuotePos + 1, nextQuotePos - firstQuotePos - 1);

            m_valueDescriptions[uid][signalName].insert(value, description.toString());
            signalDescriptionsView = signalDescriptionsView.sliced(nextQuotePos + 1).trimmed();
        }
    }
}

void QCanDbcFileParserPrivate::postProcessSignalMultiplexing()
{
    // For the case of simple multiplexing we need to do the following for
    // every message description:
    // 1. Find the multiplexor signal
    // 2. Replace all kQtDummySignal entries with the name of the multiplexor
    // 3. While doing so, check if we have any signals with type
    //    SwitchAndSignal. This will mean that extended multiplexing is used
    // 4. Also detect conditions when we have more than one multiplexor signal.
    //    This is an error as well, and message description should be discarded.

    QList<QtCanBus::UniqueId> uidsToRemove;

    for (auto &messageDesc : m_messageDescriptions) {
        bool useExtendedMux = false;
        QString multiplexorSignal;
        auto &signalDescriptions = QCanMessageDescriptionPrivate::get(messageDesc)->messageSignals;
        for (const auto &signalDesc : std::as_const(signalDescriptions)) {
            if (signalDesc.multiplexState() == QtCanBus::MultiplexState::MultiplexorSwitch) {
                if (multiplexorSignal.isEmpty()) {
                    multiplexorSignal = signalDesc.name();
                } else {
                    // invalid config
                    multiplexorSignal.clear();
                    uidsToRemove.push_back(messageDesc.uniqueId());
                    break;
                }
            } else if (signalDesc.multiplexState() == QtCanBus::MultiplexState::SwitchAndSignal) {
                // extended multiplexing
                useExtendedMux = true;
            }
        }
        if (!useExtendedMux && !multiplexorSignal.isEmpty()) {
            // iterate through all signal descriptions and update kQtDummySignal
            for (auto &signalDesc : signalDescriptions) {
                if (signalDesc.multiplexState() == QtCanBus::MultiplexState::MultiplexedSignal) {
                    auto &muxValues = QCanSignalDescriptionPrivate::get(signalDesc)->muxSignals;
                    auto val = muxValues.value(kQtDummySignal);
                    muxValues.remove(kQtDummySignal);
                    muxValues.insert(multiplexorSignal, val);
                }
            }
        } else if (useExtendedMux) {
            // Iterate through all signal descriptions and check that we do
            // not have any kQtDummySignal entries. It such entry exists, this
            // means that there were errors while parsing extended multiplexing
            // table, and this message description is invalid
            for (const auto &signalDesc : std::as_const(signalDescriptions)) {
                const auto muxSignals = signalDesc.multiplexSignals();
                if (muxSignals.contains(kQtDummySignal)) {
                    uidsToRemove.push_back(messageDesc.uniqueId());
                    break;
                }
            }
        }
    }

    for (const auto &uid : std::as_const(uidsToRemove)) {
        m_messageDescriptions.remove(uid);
        addWarning(QObject::tr("Message description with unique id %1 is skipped because "
                               "it has invalid multiplexing description.").
                   arg(qToUnderlying(uid)));
    }
}

void QCanDbcFileParserPrivate::addWarning(QString &&warning)
{
    m_warnings.emplace_back(warning);
}

void QCanDbcFileParserPrivate::addCurrentMessage()
{
    if (m_isProcessingMessage) {
        auto uid = m_currentMessage.uniqueId();
        if (!m_currentMessage.isValid()) {
            addWarning(QObject::tr("Message description with unique id %1 is skipped "
                                   "because it's not valid.").arg(qToUnderlying(uid)));
        } else if (m_messageDescriptions.contains(uid)) {
            addWarning(QObject::tr("Message description with unique id %1 is skipped "
                                   "because such unique id is already used.").
                       arg(qToUnderlying(uid)));
        } else {
            m_messageDescriptions.insert(uid, m_currentMessage);
        }
        m_currentMessage = {};
        m_isProcessingMessage = false;
    }
}

QList<QCanMessageDescription> QCanDbcFileParserPrivate::getMessages() const
{
    return QList<QCanMessageDescription>(m_messageDescriptions.cbegin(),
                                         m_messageDescriptions.cend());
}

QT_END_NAMESPACE
