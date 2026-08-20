// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qpcsccard_p.h"
#include "ndef/qnfctagtype4ndeffsm_p.h"
#include "qapduutils_p.h"
#include <QtCore/QLoggingCategory>
#include <QtCore/QTimer>

#if defined(Q_OS_DARWIN)
#    define SCARD_ATTR_MAXINPUT 0x0007A007
#elif !defined(Q_OS_WIN)
#    include <reader.h>
#endif

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_NFC_PCSC)

/*
    Windows SCardBeginTransaction() documentation says the following:

        If a transaction is held on the card for more than five seconds with no
        operations happening on that card, then the card is reset. Calling any
        of the Smart Card and Reader Access Functions or Direct Card Access
        Functions on the card that is transacted results in the timer being
        reset to continue allowing the transaction to be used.

    We use a timer to ensure that transactions do not timeout unexpectedly.
*/
static constexpr int KeepAliveIntervalMs = 2500;

/*
    Start a temporary transaction if a persistent transaction was not already
    started due to call to onSendCommandRequest().

    If a temporary transaction was initiated, it is ended when this object is
    destroyed.
*/
QPcscCard::Transaction::Transaction(QPcscCard *card) : m_card(card)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;
    if (!m_card->isValid() || m_card->m_inAutoTransaction)
        return;

    auto ret = SCardBeginTransaction(m_card->m_handle);
    if (ret != SCARD_S_SUCCESS) {
        qCWarning(QT_NFC_PCSC) << "SCardBeginTransaction failed:" << QPcsc::errorMessage(ret);
        m_card->invalidate();
        return;
    }

    m_initiated = true;
}

QPcscCard::Transaction::~Transaction()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;
    if (!m_initiated || !m_card->isValid())
        return;

    auto ret = SCardEndTransaction(m_card->m_handle, SCARD_LEAVE_CARD);

    if (ret != SCARD_S_SUCCESS) {
        qCWarning(QT_NFC_PCSC) << "SCardEndTransaction failed:" << QPcsc::errorMessage(ret);
        m_card->invalidate();
    }
}

QPcscCard::QPcscCard(SCARDHANDLE handle, DWORD protocol, QObject *parent)
    : QObject(parent), m_handle(handle)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    m_keepAliveTimer = new QTimer(this);
    m_keepAliveTimer->setInterval(KeepAliveIntervalMs);
    connect(m_keepAliveTimer, &QTimer::timeout, this, &QPcscCard::onKeepAliveTimeout);

    m_ioPci.dwProtocol = protocol;
    m_ioPci.cbPciLength = sizeof(m_ioPci);

    // Assume that everything is NFC Tag Type 4 for now
    m_tagDetectionFsm = std::make_unique<QNfcTagType4NdefFsm>();

    performNdefDetection();
}

QPcscCard::~QPcscCard()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;
    invalidate();
}

void QPcscCard::performNdefDetection()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    if (!m_isValid)
        return;

    Transaction transaction(this);

    auto action = m_tagDetectionFsm->detectNdefSupport();

    while (action == QNdefAccessFsm::SendCommand) {
        auto command = m_tagDetectionFsm->getCommand(action);

        if (action == QNdefAccessFsm::ProvideResponse) {
            auto result = sendCommand(command, NoAutoTransaction);
            action = m_tagDetectionFsm->provideResponse(result.response);
        }
    }

    qCDebug(QT_NFC_PCSC) << "NDEF detection result" << action;

    m_supportsNdef = action == QNdefAccessFsm::Done;
    qCDebug(QT_NFC_PCSC) << "NDEF supported:" << m_supportsNdef;
}

/*
    Release the resource associated with the card and notify the NFC target
    about disconnection. The card is deleted when automatic deletion is enabled.
*/
void QPcscCard::invalidate()
{
    if (!m_isValid)
        return;

    SCardDisconnect(m_handle, m_inAutoTransaction ? SCARD_RESET_CARD : SCARD_LEAVE_CARD);

    m_isValid = false;
    m_inAutoTransaction = false;

    Q_EMIT disconnected();
    Q_EMIT invalidated();

    if (m_autodelete)
        deleteLater();
}

/*
    Send the given command to the card.

    Start an automatic transaction if autoTransaction is StartAutoTransaction
    and it is not already started. This automatic transaction lasts until the
    invalidate() or onDisconnectRequest() is called.

    The automatic transaction is used to ensure that other applications do not
    interfere with commands sent by the user application.

    If autoTransaction is NoAutoTransaction, then the calling code should ensure
    that either the command is atomic or that a temporary transaction is started
    using Transaction object.
*/
QPcsc::RawCommandResult QPcscCard::sendCommand(const QByteArray &command,
                                               QPcscCard::AutoTransaction autoTransaction)
{
    if (!m_isValid)
        return {};

    if (!m_inAutoTransaction && autoTransaction == StartAutoTransaction) {
        qCDebug(QT_NFC_PCSC) << "Starting transaction";

        // FIXME: Is there a timeout on this?
        auto ret = SCardBeginTransaction(m_handle);
        if (ret != SCARD_S_SUCCESS) {
            qCWarning(QT_NFC_PCSC) << "SCardBeginTransaction failed:" << QPcsc::errorMessage(ret);
            invalidate();
            return {};
        }
        m_inAutoTransaction = true;
        m_keepAliveTimer->start();
    }

    QPcsc::RawCommandResult result;
    result.response.resize(0xFFFF + 2);
    DWORD recvLength = result.response.size();

    qCDebug(QT_NFC_PCSC) << "TX:" << command.toHex(':');

    result.ret = SCardTransmit(m_handle, &m_ioPci, reinterpret_cast<LPCBYTE>(command.constData()),
                               command.size(), nullptr,
                               reinterpret_cast<LPBYTE>(result.response.data()), &recvLength);
    if (result.ret != SCARD_S_SUCCESS) {
        qCWarning(QT_NFC_PCSC) << "SCardTransmit failed:" << QPcsc::errorMessage(result.ret);
        result.response.clear();
        invalidate();
    } else {
        result.response.resize(recvLength);
        qCDebug(QT_NFC_PCSC) << "RX:" << result.response.toHex(':');
    }

    return result;
}

void QPcscCard::onKeepAliveTimeout()
{
    if (!m_isValid || !m_inAutoTransaction) {
        m_keepAliveTimer->stop();
        return;
    }

    checkCardPresent();
}

QByteArray QPcscCard::readUid()
{
    QByteArray command = QCommandApdu::build(0xFF, QCommandApdu::GetData, 0x00, 0x00, {}, 256);

    // Atomic command, no need for transaction.
    QResponseApdu res(sendCommand(command, NoAutoTransaction).response);
    if (!res.isOk())
        return {};
    return res.data();
}

void QPcscCard::onReadNdefMessagesRequest(const QNearFieldTarget::RequestId &request)
{
    if (!m_isValid) {
        Q_EMIT requestCompleted(request, QNearFieldTarget::ConnectionError, {});
        return;
    }

    if (!m_supportsNdef) {
        Q_EMIT requestCompleted(request, QNearFieldTarget::UnsupportedError, {});
        return;
    }

    Transaction transaction(this);

    auto nextState = m_tagDetectionFsm->readMessages();

    while (true) {
        if (nextState == QNdefAccessFsm::SendCommand) {
            auto command = m_tagDetectionFsm->getCommand(nextState);

            if (nextState == QNdefAccessFsm::ProvideResponse) {
                auto result = sendCommand(command, NoAutoTransaction);
                nextState = m_tagDetectionFsm->provideResponse(result.response);
            }
        } else if (nextState == QNdefAccessFsm::GetMessage) {
            auto message = m_tagDetectionFsm->getMessage(nextState);
            Q_EMIT ndefMessageRead(message);
        } else {
            break;
        }
    }

    qCDebug(QT_NFC_PCSC) << "Final state:" << nextState;
    auto errorCode = (nextState == QNdefAccessFsm::Done) ? QNearFieldTarget::NoError
                                                         : QNearFieldTarget::NdefReadError;
    Q_EMIT requestCompleted(request, errorCode, {});
}

/*
    Ends the persistent transaction and resets the card if sendCommand() was
    used by user.

    Resetting the card ensures that the current state of the card does not get
    shared with other processes.
*/
void QPcscCard::onDisconnectRequest()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    if (!m_isValid)
        return;

    LONG ret;
    if (m_inAutoTransaction) {
        // NOTE: PCSCLite does not automatically release transaction in
        // SCardReconnect(): https://salsa.debian.org/rousseau/PCSC/-/issues/11
        ret = SCardEndTransaction(m_handle, SCARD_RESET_CARD);
        if (ret != SCARD_S_SUCCESS) {
            qCWarning(QT_NFC_PCSC) << "SCardEndTransaction failed:" << QPcsc::errorMessage(ret);
            invalidate();
            return;
        }

        m_inAutoTransaction = false;
    }

    DWORD activeProtocol;
    ret = SCardReconnect(m_handle, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                         SCARD_LEAVE_CARD, &activeProtocol);
    if (ret != SCARD_S_SUCCESS) {
        qCWarning(QT_NFC_PCSC) << "SCardReconnect failed:" << QPcsc::errorMessage(ret);
        invalidate();
        return;
    }

    Q_EMIT disconnected();
}

void QPcscCard::onTargetDestroyed()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;
    invalidate();
}

void QPcscCard::onSendCommandRequest(const QNearFieldTarget::RequestId &request,
                                     const QByteArray &command)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    if (!m_isValid) {
        Q_EMIT requestCompleted(request, QNearFieldTarget::ConnectionError, {});
        return;
    }

    auto result = sendCommand(command, StartAutoTransaction);
    if (result.isOk())
        Q_EMIT requestCompleted(request, QNearFieldTarget::NoError, result.response);
    else
        Q_EMIT requestCompleted(request, QNearFieldTarget::CommandError, {});
}

void QPcscCard::onWriteNdefMessagesRequest(const QNearFieldTarget::RequestId &request,
                                           const QList<QNdefMessage> &messages)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    if (!m_isValid) {
        Q_EMIT requestCompleted(request, QNearFieldTarget::ConnectionError, {});
        return;
    }

    if (!m_supportsNdef) {
        Q_EMIT requestCompleted(request, QNearFieldTarget::UnsupportedError, {});
        return;
    }

    Transaction transaction(this);

    auto nextState = m_tagDetectionFsm->writeMessages(messages);

    while (nextState == QNdefAccessFsm::SendCommand) {
        auto command = m_tagDetectionFsm->getCommand(nextState);
        if (nextState == QNdefAccessFsm::ProvideResponse) {
            auto result = sendCommand(command, NoAutoTransaction);
            nextState = m_tagDetectionFsm->provideResponse(result.response);
        }
    }

    auto errorCode = (nextState == QNdefAccessFsm::Done) ? QNearFieldTarget::NoError
                                                         : QNearFieldTarget::NdefWriteError;
    Q_EMIT requestCompleted(request, errorCode, {});
}

/*
    Enable automatic card deletion when the connection is closed by the user
    or the card otherwise becomes unavailable.

    The automatic deletion is prevented initially so that
    QNearFieldManagerPrivate can safely establish connections between this
    object and a QNearFieldTargetPrivate proxy.
*/
void QPcscCard::enableAutodelete()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    m_autodelete = true;
    if (!m_isValid)
        deleteLater();
}

/*
    Check if the card is still present in the reader.

    Invalidates the object if card is not present.
*/
bool QPcscCard::checkCardPresent()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    if (!m_isValid)
        return false;

    DWORD state;
    auto ret = SCardStatus(m_handle, nullptr, nullptr, &state, nullptr, nullptr, nullptr);
    if (ret != SCARD_S_SUCCESS) {
        qCWarning(QT_NFC_PCSC) << "SCardStatus failed:" << QPcsc::errorMessage(ret);
        invalidate();
        return false;
    }

    qCDebug(QT_NFC_PCSC) << "State:" << Qt::hex << state;

    return (state & SCARD_PRESENT) != 0;
}

int QPcscCard::readMaxInputLength()
{
    if (!m_isValid)
        return 0;

    // Maximum standard APDU length
    static constexpr int DefaultMaxInputLength = 261;

    uint32_t maxInput;
    DWORD attrSize = sizeof(maxInput);
    auto ret = SCardGetAttrib(m_handle, SCARD_ATTR_MAXINPUT, reinterpret_cast<LPBYTE>(&maxInput),
                              &attrSize);
    if (ret != SCARD_S_SUCCESS) {
        qCDebug(QT_NFC_PCSC) << "SCardGetAttrib failed:" << QPcsc::errorMessage(ret);
        return DefaultMaxInputLength;
    }

    if (attrSize != sizeof(maxInput)) {
        qCWarning(QT_NFC_PCSC) << "Unexpected attribute size for SCARD_ATTR_MAXINPUT:" << attrSize;
        return DefaultMaxInputLength;
    }

    return static_cast<int>(maxInput);
}

QT_END_NAMESPACE
