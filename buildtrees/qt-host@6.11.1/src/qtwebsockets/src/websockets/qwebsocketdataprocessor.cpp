// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:network-protocol
/*!
    \class QWebSocketDataProcessor
    The class QWebSocketDataProcessor is responsible for reading, validating and
    interpreting data from a WebSocket.
    It reads data from a QIODevice, validates it against \l{RFC 6455}, and parses it into
    frames (data, control).
    It emits signals that correspond to the type of the frame: textFrameReceived(),
    binaryFrameReceived(), textMessageReceived(), binaryMessageReceived(), pingReceived(),
    pongReceived() and closeReceived().
    Whenever an error is detected, the errorEncountered() signal is emitted.
    QWebSocketDataProcessor also checks if a frame is allowed in a sequence of frames
    (e.g. a continuation frame cannot follow a final frame).
    This class is an internal class used by QWebSocketInternal for data processing and validation.

    \sa Frame()

    \internal
*/
#include "qwebsocketdataprocessor_p.h"
#include "qwebsocketprotocol.h"
#include "qwebsocketprotocol_p.h"
#include "qwebsocketframe_p.h"

#include <QtCore/QtEndian>
#include <QtCore/QDebug>
#include <QtCore/QIODevice>
#include <QtCore/QStringDecoder>

#include <limits.h>

QT_BEGIN_NAMESPACE

/*!
    \internal
 */
QWebSocketDataProcessor::QWebSocketDataProcessor(QObject *parent) :
    QObject(parent),
    m_processingState(PS_READ_HEADER),
    m_isFinalFrame(false),
    m_isFragmented(false),
    m_opCode(QWebSocketProtocol::OpCodeClose),
    m_isControlFrame(false),
    m_hasMask(false),
    m_mask(0),
    m_payloadLength(0),
    m_decoder(QStringDecoder(QStringDecoder::Utf8, QStringDecoder::Flag::ConvertInvalidToNull)),
    m_waitTimer(new QChronoTimer(this))
{
    clear();
    // initialize the internal timeout timer
    using namespace std::chrono_literals;
    m_waitTimer->setInterval(5s);
    m_waitTimer->setSingleShot(true);
    m_waitTimer->callOnTimeout(this, &QWebSocketDataProcessor::timeout);
}

/*!
    \internal
 */
QWebSocketDataProcessor::~QWebSocketDataProcessor()
{
    clear();
}

void QWebSocketDataProcessor::setMaxAllowedFrameSize(quint64 maxAllowedFrameSize)
{
    frame.setMaxAllowedFrameSize(maxAllowedFrameSize);
}

quint64 QWebSocketDataProcessor::maxAllowedFrameSize() const
{
    return frame.maxAllowedFrameSize();
}

/*!
    \internal
 */
void QWebSocketDataProcessor::setMaxAllowedMessageSize(quint64 maxAllowedMessageSize)
{
    if (maxAllowedMessageSize <= maxMessageSize())
        m_maxAllowedMessageSize = maxAllowedMessageSize;
}

/*!
    \internal
 */
quint64 QWebSocketDataProcessor::maxAllowedMessageSize() const
{
    return m_maxAllowedMessageSize;
}

/*!
    \internal
 */
quint64 QWebSocketDataProcessor::maxMessageSize()
{
    return MAX_MESSAGE_SIZE_IN_BYTES;   //COV_NF_LINE
}

/*!
    \internal
 */
quint64 QWebSocketDataProcessor::maxFrameSize()
{
   return QWebSocketFrame::maxFrameSize();
}

/*!
    \internal
*/
void QWebSocketDataProcessor::setIdleTimeout(std::chrono::milliseconds timeout)
{
    Q_ASSERT(!m_waitTimer->isActive());
    m_waitTimer->setInterval(timeout);
}

/*!
    \internal
*/
std::chrono::milliseconds QWebSocketDataProcessor::idleTimeout() const
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(m_waitTimer->interval());
}

/*!
    \internal

    Returns \c true if a complete websocket frame has been processed;
    otherwise returns \c false.
 */
bool QWebSocketDataProcessor::process(QIODevice *pIoDevice)
{
    bool isDone = false;

    while (!isDone) {
        frame.readFrame(pIoDevice);
        if (!frame.isDone()) {
            // waiting for more data available
            QObject::connect(pIoDevice, &QIODevice::readyRead,
                             m_waitTimer, &QChronoTimer::stop, Qt::UniqueConnection);
            m_waitTimer->start();
            return false;
        } else if (Q_LIKELY(frame.isValid())) {
            if (frame.isControlFrame()) {
                isDone = processControlFrame(frame);
            } else {
                //we have a dataframe; opcode can be OC_CONTINUE, OC_TEXT or OC_BINARY
                if (Q_UNLIKELY(!m_isFragmented && frame.isContinuationFrame())) {
                    clear();
                    Q_EMIT errorEncountered(QWebSocketProtocol::CloseCodeProtocolError,
                                            tr("Received Continuation frame, while there is " \
                                               "nothing to continue."));
                    return true;
                }
                if (Q_UNLIKELY(m_isFragmented && frame.isDataFrame() &&
                               !frame.isContinuationFrame())) {
                    clear();
                    Q_EMIT errorEncountered(QWebSocketProtocol::CloseCodeProtocolError,
                                            tr("All data frames after the initial data frame " \
                                               "must have opcode 0 (continuation)."));
                    return true;
                }
                if (!frame.isContinuationFrame()) {
                    m_opCode = frame.opCode();
                    m_isFragmented = !frame.isFinalFrame();
                }
                quint64 messageLength = m_opCode == QWebSocketProtocol::OpCodeText
                        ? quint64(m_textMessage.size())
                        : quint64(m_binaryMessage.size());
                if (Q_UNLIKELY((messageLength + quint64(frame.payload().size())) >
                               maxAllowedMessageSize())) {
                    clear();
                    Q_EMIT errorEncountered(QWebSocketProtocol::CloseCodeTooMuchData,
                                            tr("Received message is too big."));
                    return true;
                }

                bool isFinalFrame = frame.isFinalFrame();
                if (m_opCode == QWebSocketProtocol::OpCodeText) {
                    QString frameTxt = m_decoder(frame.payload());
                    bool decoderHadError = m_decoder.hasError();
                    if (!decoderHadError && isFinalFrame) {
                        auto r = m_decoder.finalize();
                        decoderHadError = r.error != QStringDecoder::FinalizeResult::Error::NoError;
                    }
                    if (Q_UNLIKELY(decoderHadError)) {
                        clear();
                        Q_EMIT errorEncountered(QWebSocketProtocol::CloseCodeWrongDatatype,
                                                tr("Invalid UTF-8 code encountered."));
                        return true;
                    } else {
                        m_textMessage.append(frameTxt);
                        frame.clear();
                        Q_EMIT textFrameReceived(frameTxt, isFinalFrame);
                    }
                } else {
                    m_binaryMessage.append(frame.payload());
                    QByteArray payload = frame.payload();
                    frame.clear();
                    Q_EMIT binaryFrameReceived(payload, isFinalFrame);
                }

                if (isFinalFrame) {
                    isDone = true;
                    if (m_opCode == QWebSocketProtocol::OpCodeText) {
                        const QString textMessage(m_textMessage);
                        clear();
                        Q_EMIT textMessageReceived(textMessage);
                    } else {
                        const QByteArray binaryMessage(m_binaryMessage);
                        clear();
                        Q_EMIT binaryMessageReceived(binaryMessage);
                    }
                }
            }
        } else {
            Q_EMIT errorEncountered(frame.closeCode(), frame.closeReason());
            clear();
            isDone = true;
        }
        frame.clear();
    }
    return true;
}

/*!
    \internal
 */
void QWebSocketDataProcessor::clear()
{
    m_processingState = PS_READ_HEADER;
    m_isFinalFrame = false;
    m_isFragmented = false;
    m_opCode = QWebSocketProtocol::OpCodeClose;
    m_hasMask = false;
    m_mask = 0;
    m_binaryMessage.clear();
    m_textMessage.clear();
    m_payloadLength = 0;
    m_decoder.resetState();
    frame.clear();
}

/*!
    \internal
 */
bool QWebSocketDataProcessor::processControlFrame(const QWebSocketFrame &frame)
{
    bool mustStopProcessing = true; //control frames never expect additional frames to be processed
    switch (frame.opCode()) {
    case QWebSocketProtocol::OpCodePing:
        Q_EMIT pingReceived(frame.payload());
        break;

    case QWebSocketProtocol::OpCodePong:
        Q_EMIT pongReceived(frame.payload());
        break;

    case QWebSocketProtocol::OpCodeClose:
    {
        quint16 closeCode = QWebSocketProtocol::CloseCodeNormal;
        QString closeReason;
        QByteArray payload = frame.payload();
        if (Q_UNLIKELY(payload.size() == 1)) {
            //size is either 0 (no close code and no reason)
            //or >= 2 (at least a close code of 2 bytes)
            closeCode = QWebSocketProtocol::CloseCodeProtocolError;
            closeReason = tr("Payload of close frame is too small.");
        } else if (Q_LIKELY(payload.size() > 1)) {
            //close frame can have a close code and reason
            closeCode = qFromBigEndian<quint16>(reinterpret_cast<const uchar *>(payload.constData()));
            if (Q_UNLIKELY(!QWebSocketProtocol::isCloseCodeValid(closeCode))) {
                closeCode = QWebSocketProtocol::CloseCodeProtocolError;
                closeReason = tr("Invalid close code %1 detected.").arg(closeCode);
            } else {
                if (payload.size() > 2) {
                    auto toUtf16 = QStringDecoder(QStringDecoder::Utf8,
                        QStringDecoder::Flag::Stateless | QStringDecoder::Flag::ConvertInvalidToNull);
                    closeReason = toUtf16(QByteArrayView(payload).sliced(2));
                    if (Q_UNLIKELY(toUtf16.hasError())) {
                        closeCode = QWebSocketProtocol::CloseCodeWrongDatatype;
                        closeReason = tr("Invalid UTF-8 code encountered.");
                    }
                }
            }
        }
        Q_EMIT closeReceived(static_cast<QWebSocketProtocol::CloseCode>(closeCode), closeReason);
        break;
    }

    case QWebSocketProtocol::OpCodeContinue:
    case QWebSocketProtocol::OpCodeBinary:
    case QWebSocketProtocol::OpCodeText:
    case QWebSocketProtocol::OpCodeReserved3:
    case QWebSocketProtocol::OpCodeReserved4:
    case QWebSocketProtocol::OpCodeReserved5:
    case QWebSocketProtocol::OpCodeReserved6:
    case QWebSocketProtocol::OpCodeReserved7:
    case QWebSocketProtocol::OpCodeReservedC:
    case QWebSocketProtocol::OpCodeReservedB:
    case QWebSocketProtocol::OpCodeReservedD:
    case QWebSocketProtocol::OpCodeReservedE:
    case QWebSocketProtocol::OpCodeReservedF:
        //do nothing
        //case statements added to make C++ compiler happy
        break;

    default:
        Q_EMIT errorEncountered(QWebSocketProtocol::CloseCodeProtocolError,
                                tr("Invalid opcode detected: %1").arg(int(frame.opCode())));
        //do nothing
        break;
    }
    return mustStopProcessing;
}

/*!
    \internal
 */
void QWebSocketDataProcessor::timeout()
{
    clear();
    Q_EMIT errorEncountered(QWebSocketProtocol::CloseCodeGoingAway,
                            tr("Timeout when reading data from socket."));
}

QT_END_NAMESPACE
