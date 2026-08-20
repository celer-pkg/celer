// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qnfctagtype4ndeffsm_p.h"
#include <QtCore/QtEndian>
#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_STATIC_LOGGING_CATEGORY(QT_NFC_T4T, "qt.nfc.t4t")

/*
    NDEF support for NFC Type 4 tags.

    Based on Type 4 Tag Operation Specification, Version 2.0 (T4TOP 2.0).
*/

QByteArray QNfcTagType4NdefFsm::getCommand(QNdefAccessFsm::Action &nextAction)
{
    // ID of the NDEF Tag Application
    static constexpr uint8_t NtagApplicationIdV2[] { 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01 };
    // Capability container file ID.
    static constexpr uint8_t CapabilityContainerId[] { 0xE1, 0x03 };
    // Shortcut for specifying zero length of NDEF message data.
    static constexpr uint8_t ZeroLength[] { 0x00, 0x00 };

    nextAction = ProvideResponse;

    switch (m_currentState) {
    case SelectApplicationForProbe:
    case SelectApplicationForRead:
    case SelectApplicationForWrite:
        return QCommandApdu::build(0x00, QCommandApdu::Select, 0x04, 0x00,
                                   QByteArrayView::fromArray(NtagApplicationIdV2), 256);
    case SelectCCFile:
        return QCommandApdu::build(0x00, QCommandApdu::Select, 0x00, 0x0C,
                                   QByteArrayView::fromArray(CapabilityContainerId));
    case ReadCCFile:
        return QCommandApdu::build(0x00, QCommandApdu::ReadBinary, 0x00, 0x00, {}, 15);
    case SelectNdefFileForRead:
    case SelectNdefFileForWrite:
        return QCommandApdu::build(0x00, QCommandApdu::Select, 0x00, 0x0C, m_ndefFileId);
    case ReadNdefMessageLength:
        return QCommandApdu::build(0x00, QCommandApdu::ReadBinary, 0x00, 0x00, {}, 2);
    case ReadNdefMessage: {
        uint16_t readSize = qMin(m_fileSize, m_maxReadSize);

        return QCommandApdu::build(0x00, QCommandApdu::ReadBinary, m_fileOffset >> 8,
                                   m_fileOffset & 0xFF, {}, readSize);
    }
    case ClearNdefLength:
        m_fileOffset = 2;
        m_fileSize = m_ndefData.size();
        return QCommandApdu::build(0x00, QCommandApdu::UpdateBinary, 0x00, 0x00,
                                   QByteArrayView::fromArray(ZeroLength));
    case WriteNdefFile: {
        uint16_t updateSize = qMin(m_fileSize, m_maxUpdateSize);
        uint16_t fileOffset = m_fileOffset;

        m_fileOffset += updateSize;
        m_fileSize -= updateSize;

        return QCommandApdu::build(0x00, QCommandApdu::UpdateBinary, fileOffset >> 8,
                                   fileOffset & 0xFF, m_ndefData.mid(fileOffset - 2, updateSize));
    }
    case WriteNdefLength: {
        QByteArray data(2, Qt::Uninitialized);
        qToUnaligned(qToBigEndian<uint16_t>(m_ndefData.size()), data.data());

        return QCommandApdu::build(0x00, QCommandApdu::UpdateBinary, 0x00, 0x00, data);
    }
    default:
        nextAction = Unexpected;
        return {};
    }
}

QNdefMessage QNfcTagType4NdefFsm::getMessage(QNdefAccessFsm::Action &nextAction)
{
    if (m_currentState == NdefMessageRead) {
        auto message = QNdefMessage::fromByteArray(m_ndefData);
        m_ndefData.clear();
        m_currentState = NdefSupportDetected;
        nextAction = Done;
        return message;
    }

    nextAction = Unexpected;
    return {};
}

QNdefAccessFsm::Action QNfcTagType4NdefFsm::detectNdefSupport()
{
    switch (m_currentState) {
    case SelectApplicationForProbe:
        m_targetState = NdefSupportDetected;
        return SendCommand;
    case NdefSupportDetected:
        return Done;
    case NdefNotSupported:
        return Failed;
    default:
        return Unexpected;
    }
}

QNdefAccessFsm::Action QNfcTagType4NdefFsm::readMessages()
{
    switch (m_currentState) {
    case SelectApplicationForProbe:
        m_targetState = NdefMessageRead;
        return SendCommand;
    case NdefSupportDetected:
        m_currentState = SelectApplicationForRead;
        m_targetState = NdefMessageRead;
        return SendCommand;
    case NdefNotSupported:
        return Failed;
    default:
        return Unexpected;
    }
}

QNdefAccessFsm::Action QNfcTagType4NdefFsm::writeMessages(const QList<QNdefMessage> &messages)
{
    // Only one message per tag is supported
    if (messages.isEmpty() || messages.size() > 1)
        return Failed;

    auto messageData = messages.first().toByteArray();
    if (messageData.size() > m_maxNdefSize - 2)
        return Failed;

    m_ndefData = messageData;

    m_targetState = NdefMessageWritten;

    switch (m_currentState) {
    case SelectApplicationForProbe:
        return SendCommand;

    case NdefNotSupported:
        return Failed;

    case NdefSupportDetected:
        if (!m_writable)
            return Failed;

        m_currentState = SelectApplicationForWrite;
        return SendCommand;

    default:
        return Unexpected;
    };
}

QNdefAccessFsm::Action QNfcTagType4NdefFsm::provideResponse(const QByteArray &response)
{
    QResponseApdu apdu(response);

    switch (m_currentState) {
    case SelectApplicationForProbe:
        return handleSimpleResponse(apdu, SelectCCFile, NdefNotSupported);
    case SelectCCFile:
        return handleSimpleResponse(apdu, ReadCCFile, NdefNotSupported);
    case ReadCCFile:
        return handleReadCCResponse(apdu);

    case SelectApplicationForRead:
        return handleSimpleResponse(apdu, SelectNdefFileForRead, NdefSupportDetected);
    case SelectNdefFileForRead:
        return handleSimpleResponse(apdu, ReadNdefMessageLength, NdefSupportDetected);
    case ReadNdefMessageLength:
        return handleReadFileLengthResponse(apdu);
    case ReadNdefMessage:
        return handleReadFileResponse(apdu);

    case SelectApplicationForWrite:
        return handleSimpleResponse(apdu, SelectNdefFileForWrite, NdefSupportDetected);
    case SelectNdefFileForWrite:
        return handleSimpleResponse(apdu, ClearNdefLength, NdefSupportDetected);
    case ClearNdefLength:
        return handleSimpleResponse(apdu, WriteNdefFile, NdefSupportDetected);
    case WriteNdefFile:
        return handleWriteNdefFileResponse(apdu);
    case WriteNdefLength:
        return handleSimpleResponse(apdu, NdefSupportDetected, NdefSupportDetected, Done);

    default:
        return Unexpected;
    }
}

QNdefAccessFsm::Action QNfcTagType4NdefFsm::handleSimpleResponse(
        const QResponseApdu &response, QNfcTagType4NdefFsm::State okState,
        QNfcTagType4NdefFsm::State failedState, QNdefAccessFsm::Action okAction)
{
    if (!response.isOk()) {
        m_currentState = failedState;
        return Failed;
    }

    m_currentState = okState;
    return okAction;
}

QNdefAccessFsm::Action QNfcTagType4NdefFsm::handleReadCCResponse(const QResponseApdu &response)
{
    m_currentState = NdefNotSupported;

    if (!response.isOk())
        return Failed;

    if (response.data().size() < 15) {
        qCDebug(QT_NFC_T4T) << "Invalid response size";
        return Failed;
    }

    qsizetype idx = 0;
    auto readU8 = [&data = response.data(), &idx]() {
        return static_cast<uint8_t>(data.at(idx++));
    };
    auto readU16 = [&data = response.data(), &idx]() {
        Q_ASSERT(idx >= 0 && idx <= data.size() - 2);
        uint16_t res = qFromBigEndian(qFromUnaligned<uint16_t>(data.constData() + idx));
        idx += 2;
        return res;
    };
    auto readBytes = [&data = response.data(), &idx](qsizetype count) {
        auto res = data.sliced(idx, count);
        idx += count;
        return res;
    };
    auto ccLen = readU16();
    if (ccLen < 15) {
        qCDebug(QT_NFC_T4T) << "CC length is too small";
        return Failed;
    }
    auto mapping = readU8();
    if ((mapping & 0xF0) != 0x20) {
        qCDebug(QT_NFC_T4T) << "Unsupported mapping:" << Qt::hex << mapping;
        return Failed;
    }
    m_maxReadSize = readU16();
    if (m_maxReadSize < 0xF) {
        qCDebug(QT_NFC_T4T) << "Invalid maxReadSize" << m_maxReadSize;
        return Failed;
    }

    m_maxUpdateSize = readU16();
    auto tlvTag = readU8();
    if (tlvTag != 0x04) {
        qCDebug(QT_NFC_T4T) << "Invalid TLV tag";
        return Failed;
    }
    auto tlvSize = readU8();
    if (tlvSize == 0xFF || tlvSize < 6) {
        qCDebug(QT_NFC_T4T) << "Invalid TLV size";
        return Failed;
    }
    m_ndefFileId = readBytes(2);

    m_maxNdefSize = readU16();
    if (m_maxNdefSize < 2) {
        qCDebug(QT_NFC_T4T) << "No space for NDEF file length";
        return Failed;
    }

    /*
        The specification defined value 0 for read access and write access
        fields to mean that access is granted without any security, all
        other values are either reserved, proprietary, or indicate than no
        access is granted at all. Here all non-zero value are handled as
        no access is granted.
    */
    auto readAccess = readU8();
    if (readAccess != 0) {
        qCDebug(QT_NFC_T4T) << "No read access";
        return Failed;
    }
    auto writeAccess = readU8();
    // It's not possible to atomically clear the length field if update
    // size is < 2, so handle such tags as read-only. This also simplifies
    // the update logic (no need to ever split ClearNdefLength/WriteNdefLength
    // states)
    m_writable = writeAccess == 0 && m_maxUpdateSize >= 2;

    m_currentState = NdefSupportDetected;

    if (m_targetState == NdefSupportDetected) {
        return Done;
    } else if (m_targetState == NdefMessageRead) {
        // Skip extra application select
        m_currentState = SelectNdefFileForRead;
        return SendCommand;
    } else if (m_targetState == NdefMessageWritten) {
        if (m_writable) {
            if (m_ndefData.size() > m_maxNdefSize - 2) {
                qCDebug(QT_NFC_T4T) << "Message is too large";
                return Failed;
            }

            // Skip extra application select
            m_currentState = SelectNdefFileForWrite;
            return SendCommand;
        } else {
            return Failed;
        }
    }

    return Unexpected;
}

QNdefAccessFsm::Action
QNfcTagType4NdefFsm::handleReadFileLengthResponse(const QResponseApdu &response)
{
    if (!response.isOk() || response.data().size() < 2) {
        m_currentState = NdefSupportDetected;
        return Failed;
    }

    m_fileSize = qFromBigEndian(qFromUnaligned<uint16_t>(response.data().constData()));
    if (m_fileSize > m_maxNdefSize - 2) {
        m_currentState = NdefSupportDetected;
        return Failed;
    }

    m_fileOffset = 2;
    m_ndefData.clear();

    if (m_fileSize == 0) {
        m_currentState = NdefMessageRead;
        return GetMessage;
    }

    m_currentState = ReadNdefMessage;
    return SendCommand;
}

QNdefAccessFsm::Action QNfcTagType4NdefFsm::handleReadFileResponse(const QResponseApdu &response)
{
    if (!response.isOk() || response.data().size() == 0) {
        m_currentState = NdefSupportDetected;
        return Failed;
    }

    auto readSize = qMin<qsizetype>(m_fileSize, response.data().size());
    m_ndefData.append(response.data().first(readSize));
    m_fileOffset += readSize;
    m_fileSize -= readSize;

    if (m_fileSize == 0) {
        m_currentState = NdefMessageRead;
        return GetMessage;
    }

    return SendCommand;
}

QNdefAccessFsm::Action
QNfcTagType4NdefFsm::handleWriteNdefFileResponse(const QResponseApdu &response)
{
    if (!response.isOk()) {
        m_currentState = NdefSupportDetected;
        return Failed;
    }

    if (m_fileSize == 0)
        m_currentState = WriteNdefLength;

    return SendCommand;
}

QT_END_NAMESPACE
