// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>, Viktor Kopp <vifactor@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:network-protocol

#include <QtGrpc/private/qtgrpclogging_p.h>
#include <QtGrpc/qgrpccalloptions.h>
#include <QtGrpc/qgrpcchanneloptions.h>
#include <QtGrpc/qgrpchttp2channel.h>
#include <QtGrpc/qgrpcinterceptorchain.h>
#include <QtGrpc/qgrpcoperationcontext.h>
#include <QtGrpc/qgrpcserializationformat.h>
#include <QtGrpc/qgrpcstatus.h>

#include <QtProtobuf/qprotobufjsonserializer.h>
#include <QtProtobuf/qprotobufserializer.h>

#include <QtNetwork/private/hpack_p.h>
#include <QtNetwork/private/http2protocol_p.h>
#include <QtNetwork/private/qdecompresshelper_p.h>
#include <QtNetwork/private/qhttp2connection_p.h>
#if QT_CONFIG(localserver)
#  include <QtNetwork/qlocalsocket.h>
#endif
#include <QtNetwork/qtcpsocket.h>
#if QT_CONFIG(ssl)
#  include <QtNetwork/qsslsocket.h>
#endif

#include <QtCore/private/qnoncontiguousbytedevice_p.h>
#include <QtCore/qalgorithms.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qendian.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qlist.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qpointer.h>
#include <QtCore/qqueue.h>
#include <QtCore/qtimer.h>
#include <QtCore/qvarlengtharray.h>

#include <QtCore/q20algorithm.h>

#include <functional>
#include <optional>
#include <utility>

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;
using namespace QtGrpc;

/*!
    \class QGrpcHttp2Channel
    \inmodule QtGrpc
    \brief The QGrpcHttp2Channel class provides a HTTP/2 transport layer
    for \gRPC communication.

    The QGrpcHttp2Channel class implements QAbstractGrpcChannel, enabling \gRPC
    communication carried over \l{https://datatracker.ietf.org/doc/html/rfc7540}
    {HTTP/2 framing}.

    HTTP/2 introduces several advantages over its predecessor, HTTP/1.1, making
    QGrpcHttp2Channel well-suited for high-performance, real-time applications
    that require efficient communication, without sacrificing security or
    reliability, by using multiplexed TCP connections.

    The channel can be customized with \l{Secure Sockets Layer (SSL)
    Classes}{SSL} support, a custom \l{QGrpcChannelOptions::}
    {serializationFormat}, or other options by constructing it with a
    QGrpcChannelOptions containing the required customizations.

    \note \l{QGrpcChannelOptions::filterServerMetadata} is enabled by default.

    \section2 Transportation scheme

    The QGrpcHttp2Channel implementation prefers different transportation
    methods based on the provided \c{hostUri}, \l{QUrl::}{scheme} and options.
    The following criteria applies:

    \table
    \header
        \li Scheme
        \li Description
        \li Default Port
        \li Requirements
        \li Example
    \row
        \li \c{http}
        \li Unencrypted HTTP/2 over TCP
        \li 80
        \li None
        \li \c{http://localhost}
    \row
        \li \c{https}
        \li TLS-encrypted HTTP/2 over TCP
        \li 443
        \li QSslSocket support \b{AND} (scheme \b{OR} \l{QGrpcChannelOptions::}{sslConfiguration})
        \li \c{https://localhost}
    \row
        \li \c{unix}
        \li Unix domain socket in filesystem path
        \li ✗
        \li QLocalSocket support \b{AND} scheme
        \li \c{unix:///tmp/grpc.socket}
    \row
        \li \c{unix-abstract}
        \li Unix domain socket in abstract namespace
        \li ✗
        \li QLocalSocket support \b{AND}
            \l{QLocalSocket::AbstractNamespaceOption}{AbstractNamespace}
            support \b{AND} scheme
        \li \c{unix-abstract:app_grpc_channel}
    \endtable

    \section2 Content-Type

    The \e{content-type} in \gRPC over HTTP/2 determines the message
    serialization format. It must start with \c{application/grpc} and can
    include a suffix. The format follows this scheme:

    \code
        "content-type": "application/grpc" [("+proto" / "+json" / {custom})]
    \endcode

    For example:
    \list
        \li \c{application/grpc+proto} specifies Protobuf encoding.
        \li \c{application/grpc+json} specifies JSON encoding.
    \endlist

    The serialization format can be configured either by specifying the \c
    {content-type} inside the metadata or by setting the \l{QGrpcChannelOptions::}
    {serializationFormat} directly. By default, the \c {application/grpc}
    content-type is used.

    To configure QGrpcHttp2Channel with the JSON serialization format using
    \c {content-type} metadata:

    \code
        auto jsonChannel = std::make_shared<QGrpcHttp2Channel>(
            QUrl("http://localhost:50051"_L1),
            QGrpcChannelOptions().setMetadata({
                { "content-type"_ba, "application/grpc+json"_ba },
            })
        );
    \endcode

    For a custom serializer and \c {content-type}, you can directly set the
    serialization format:

    \include qgrpcserializationformat.cpp custom-serializer-code

    \code
        auto dummyChannel = std::make_shared<QGrpcHttp2Channel>(
            QUrl("http://localhost:50051"_L1),
            QGrpcChannelOptions().setSerializationFormat(dummyFormat)
        );
    \endcode

    \include qgrpcserializationformat.cpp custom-serializer-desc

    \sa QAbstractGrpcChannel, QGrpcChannelOptions, QGrpcSerializationFormat
*/

namespace {

Q_STATIC_LOGGING_CATEGORY(lcChannel, "qt.grpc.channel.http2")
Q_STATIC_LOGGING_CATEGORY(lcStream, "qt.grpc.channel.http2.stream")

constexpr QLatin1String UnixScheme("unix");
constexpr QLatin1String UnixAbstractScheme("unix-abstract");
constexpr QLatin1String HttpScheme("http");
constexpr QLatin1String HttpsScheme("https");

const QByteArray HttpStatusHeader(":status");
const QByteArray ContentTypeHeader("content-type");
const QByteArray GrpcStatusHeader("grpc-status");
const QByteArray GrpcStatusMessageHeader("grpc-message");
const QByteArray DefaultContentType("application/grpc");
const QByteArray GrpcStatusDetailsHeader("grpc-status-details-bin");
const QByteArray GrpcAcceptEncodingHeader("grpc-accept-encoding");
const QByteArray GrpcAcceptEncodingValue("identity,deflate,gzip");
const QByteArray GrpcEncodingHeader("grpc-encoding");
constexpr qsizetype GrpcMessageSizeHeaderSize = 5;

// This HTTP/2 Error Codes to QGrpcStatus::StatusCode mapping should be kept in sync
// with the following docs:
//     https://www.rfc-editor.org/rfc/rfc7540#section-7
//     https://github.com/grpc/grpc/blob/master/doc/statuscodes.md
//     https://github.com/grpc/grpc/blob/master/doc/PROTOCOL-HTTP2.md
constexpr StatusCode http2ErrorToStatusCode(const quint32 http2Error)
{
    using Http2Error = Http2::Http2Error;

    switch (http2Error) {
    case Http2Error::HTTP2_NO_ERROR:
    case Http2Error::PROTOCOL_ERROR:
    case Http2Error::INTERNAL_ERROR:
    case Http2Error::FLOW_CONTROL_ERROR:
    case Http2Error::SETTINGS_TIMEOUT:
    case Http2Error::STREAM_CLOSED:
    case Http2Error::FRAME_SIZE_ERROR:
        return StatusCode::Internal;
    case Http2Error::REFUSE_STREAM:
        return StatusCode::Unavailable;
    case Http2Error::CANCEL:
        return StatusCode::Cancelled;
    case Http2Error::COMPRESSION_ERROR:
    case Http2Error::CONNECT_ERROR:
        return StatusCode::Internal;
    case Http2Error::ENHANCE_YOUR_CALM:
        return StatusCode::ResourceExhausted;
    case Http2Error::INADEQUATE_SECURITY:
        return StatusCode::PermissionDenied;
    case Http2Error::HTTP_1_1_REQUIRED:
        return StatusCode::Unknown;
    }
    return StatusCode::Internal;
}

// Ref: https://github.com/grpc/grpc/blob/master/doc/http-grpc-status-mapping.md
constexpr StatusCode http2StatusToStatusCode(const int status)
{
    switch (status) {
    case 200:
        return StatusCode::Ok;
    case 400:
        return StatusCode::Internal;
    case 401:
        return StatusCode::Unauthenticated;
    case 403:
        return StatusCode::PermissionDenied;
    case 404:
        return StatusCode::Unimplemented;
    case 429:
    case 502:
    case 503:
    case 504:
        return StatusCode::Unavailable;
    default:
        return StatusCode::Unknown;
    }
}

bool hasSslConfiguration(const QGrpcChannelOptions &opts)
{
#if QT_CONFIG(ssl)
    return opts.sslConfiguration().has_value();
#else
    Q_UNUSED(opts)
    return false;
#endif
}

} // namespace

class GrpcDataParser
{
public:
    struct Frame
    {
        Frame(QByteArray &&payload, bool isCompressed)
            : payload(std::move(payload)), isCompressed(isCompressed)
        {
        }
        QByteArray payload;
        bool isCompressed = false;
    };
    // Parses the next complete gRPC frame from the buffer. Removes the frame
    // on success, or returns std::nullopt if incomplete.
    std::optional<Frame> parseNextFrame()
    {
        static constexpr qsizetype FlagOffset = 0;
        static constexpr qsizetype LengthOffset = 1;

        std::optional<Frame> out;
        if (container.size() < GrpcMessageSizeHeaderSize)
            return out;

        // Parse length (big endian, 4 bytes after flag)
        const auto messageLength = qFromBigEndian<
            quint32>(reinterpret_cast<const uchar *>(container.constData() + LengthOffset));
        const qsizetype frameSize = GrpcMessageSizeHeaderSize + messageLength;

        if (container.size() < frameSize)
            return out; // Incomplete frame in buffer. Wait for more data

        out.emplace(container.mid(GrpcMessageSizeHeaderSize, messageLength),
                    container.at(FlagOffset) != 0);
        container.remove(0, frameSize);
        return out;
    }

    void feed(const QByteArray &data) { container.append(data); }
    qsizetype bytesAvailable() const { return container.size(); }

private:
    QByteArray container;
};

// The Http2Handler manages an individual RPC over the HTTP/2 channel.
// Each instance corresponds to an RPC initiated by the user.
class Http2Handler : public QObject
{
    Q_OBJECT

    enum class HeaderPhase { Invalid, Initial, Trailers, TrailersOnly };
    Q_ENUM(HeaderPhase);

public:
    enum class State : uint8_t {
        Idle,
        RequestHeadersSent,
        Active,
        // Endpoints
        Cancelled,
        Finished,
    };
    Q_ENUM(State);

    explicit Http2Handler(QGrpcHttp2ChannelPrivate *parent, QGrpcOperationContext *context,
                          QByteArray &&messageData, bool endStream);
    ~Http2Handler() override;

    void sendInitialRequest();
    void attachStream(QHttp2Stream *stream_);
    void processQueue();

    void finish(const QGrpcStatus &status);
    void asyncFinish(const QGrpcStatus &status);
    void cancelWithStatus(const QGrpcStatus &status);

    [[nodiscard]] bool expired() const { return !m_context; }

    [[nodiscard]] bool isStreamClosedForSending() const
    {
        // If stream pointer is nullptr this means we never opened it and should collect
        // the incoming messages in queue until the stream is opened or the error occurred.
        return m_stream != nullptr
            && (m_stream->state() == QHttp2Stream::State::HalfClosedLocal
                || m_stream->state() == QHttp2Stream::State::Closed);
    }

// context slot handlers:
    void cancel() { cancelWithStatus({ StatusCode::Cancelled, tr("Cancelled by client") }); }
    void writesDone();
    void writeMessage(QByteArrayView data);
    void deadlineTimeout()
    {
        cancelWithStatus({ StatusCode::DeadlineExceeded, tr("Deadline exceeded") });
    }

    void handleHeaders(const HPack::HttpHeader &headers, HeaderPhase phase);

private:
    [[nodiscard]] HPack::HttpHeader constructInitialHeaders() const;
    [[nodiscard]] bool constructFilterServerMetadata() const;
    [[nodiscard]] QGrpcHttp2ChannelPrivate *channelPriv() const;
    [[nodiscard]] QGrpcHttp2Channel *channel() const;
    [[nodiscard]] bool handleContextExpired();

    QPointer<QGrpcOperationContext> m_context;
    HPack::HttpHeader m_initialHeaders;
    QQueue<QByteArray> m_queue;
    QPointer<QHttp2Stream> m_stream;
    GrpcDataParser m_grpcDataParser;
    QByteArray m_negotiatedEncoding;
    std::unique_ptr<QDecompressHelper> m_decompressor;
    State m_state = State::Idle;
    const bool m_endStreamAtFirstData;
    bool m_writesDoneSent = false;
    bool m_filterServerMetadata;
    QTimer m_deadlineTimer;

    Q_DISABLE_COPY_MOVE(Http2Handler)
};

class QGrpcHttp2ChannelPrivate : public QObject
{
    Q_OBJECT
public:
    enum class SocketType : uint8_t { Tcp, Tls, Local, LocalAbstract };

    explicit QGrpcHttp2ChannelPrivate(const QUrl &uri, QGrpcHttp2Channel *q);
    ~QGrpcHttp2ChannelPrivate() override = default;

    void processOperation(QGrpcOperationContext *operationContext, QByteArray &&data,
                          bool endStream = false);

    QGrpcHttp2Channel *q_ptr = nullptr;
    const SocketType socketType;
    const QUrl hostUri;
    const QByteArray contentType;
    const QByteArray authorityHeader;
    const QByteArray schemeHeader;

private:
    enum ConnectionState { Connecting = 0, Connected, SettingsReceived, Error };

    static SocketType constructSocketType(const QUrl &rawUri, const QGrpcChannelOptions &chOpts);
    QUrl sanitizeHostUri(const QUrl &rawUri, const QGrpcChannelOptions &chOpts) const;
    QByteArray setupContentTypeNegotiation(QGrpcHttp2Channel *qPtr) const;
    static QByteArray constructAuthorityHeader(const QUrl &hostUri, SocketType socketType);

    bool createHttp2Stream(Http2Handler *handler);
    void createHttp2Connection();

#if QT_CONFIG(localserver)
    void handleLocalSocketError(QLocalSocket::LocalSocketError error)
    {
        handleSocketError(QDebug::toBytes(error));
    }
#endif
    void handleAbstractSocketError(QAbstractSocket::SocketError error)
    {
        handleSocketError(QDebug::toBytes(error));
    }
    void handleSocketError(const QByteArray &errorCode);

    template <typename Projection = q20::identity>
    void for_each_non_expired_handler(Projection proj)
    {
        QVarLengthArray<QObject *> expiredHandler;
        for (QObject *child : children()) {
            auto *handler = qobject_cast<Http2Handler *>(child);
            if (!handler)
                continue;
            if (handler->expired()) {
                expiredHandler.push_back(handler);
                continue;
            }
            std::invoke(std::forward<Projection>(proj), handler);
        }
        // Perform deletions after the loop to avoid modifying the children
        // list during iteration. Delete in reverse order to avoid
        // quadratic-time updates in QObject's children list.
        qDeleteAll(expiredHandler.crbegin(), expiredHandler.crend());
    }

private:
    std::unique_ptr<QIODevice> m_socket = nullptr;
    std::function<void()> m_reconnectFunction;

    bool m_isInsideSocketErrorOccurred = false;
    QHttp2Connection *m_connection = nullptr;
    ConnectionState m_state = Connecting;

    Q_DISABLE_COPY_MOVE(QGrpcHttp2ChannelPrivate)
};

///
/// ## Http2Handler Implementations
///

Http2Handler::Http2Handler(QGrpcHttp2ChannelPrivate *parent, QGrpcOperationContext *context,
                           QByteArray &&messageData, bool endStream)
    : QObject(parent), m_context(context), m_initialHeaders(constructInitialHeaders()),
      m_endStreamAtFirstData(endStream), m_filterServerMetadata(constructFilterServerMetadata())
{
    // If the context (lifetime bound to the user) is destroyed, this handler
    // can no longer perform any meaningful work. We allow it to be deleted;
    // QHttp2Stream will handle any outstanding cancellations appropriately.
    connect(context, &QGrpcOperationContext::destroyed, this, &Http2Handler::deleteLater);
    connect(context, &QGrpcOperationContext::cancelRequested, this, &Http2Handler::cancel);
    connect(context, &QGrpcOperationContext::writesDoneRequested, this, &Http2Handler::writesDone);
    if (!m_endStreamAtFirstData) {
        connect(context, &QGrpcOperationContext::writeMessageRequested, this,
                &Http2Handler::writeMessage);
    }

    m_deadlineTimer.setSingleShot(true);

    writeMessage(std::move(messageData));
}

Http2Handler::~Http2Handler()
{
    qCDebug(lcStream, "[%p] Destroying Http2Handler (state=%s, stream=%p)", this,
            QDebug::toBytes(m_state).constData(), m_stream.get());
    if (m_stream) {
        QHttp2Stream *streamPtr = m_stream.get();
        m_stream.clear();
        delete streamPtr;
    }
}

// Attaches the HTTP/2 stream and sets up the necessary connections and
// preconditions. The HTTP/2 connection is established, and the transport
// is now ready for communication.
void Http2Handler::attachStream(QHttp2Stream *stream_)
{
    Q_ASSERT(m_stream == nullptr);
    Q_ASSERT(stream_ != nullptr);

    m_stream = stream_;

    connect(m_stream.get(), &QHttp2Stream::headersReceived, this,
            [this](const HPack::HttpHeader &headers, bool endStream) mutable {
                if (m_state >= State::Cancelled) {
                    // In case we are Cancelled or Finished, a
                    // finished has been emitted already and the
                    // Handler should get deleted here.
                    qCDebug(lcStream, "[%p] Ignoring headers - already closed (state=%s)", this,
                            QDebug::toBytes(m_state).constData());
                    deleteLater();
                    return;
                }

                HeaderPhase phase = HeaderPhase::Invalid;
                if (m_state == State::RequestHeadersSent && endStream)
                    phase = HeaderPhase::TrailersOnly;
                else if (m_state == State::RequestHeadersSent && !endStream)
                    phase = HeaderPhase::Initial;
                else if (m_state == State::Active && endStream) {
                    phase = HeaderPhase::Trailers;
                } else {
                    qCWarning(lcStream,
                              "[%p] Received unexcpected %s HEADERS (state=%s, "
                              "endStream=%d)",
                              this, QDebug::toBytes(phase).constData(),
                              QDebug::toBytes(m_state).constData(), endStream);
                    return;
                }

                m_state = State::Active;
                handleHeaders(headers, phase);
            });

    connect(
        m_stream.get(), &QHttp2Stream::errorOccurred, this,
        [this](quint32 http2ErrorCode, const QString &errorString) {
            qCDebug(lcStream, "[%p] Stream errorOccurred (state=%s)", this,
                    QDebug::toBytes(m_state).constData());
            finish({ http2ErrorToStatusCode(http2ErrorCode), errorString });
        },
        Qt::SingleShotConnection);

    connect(m_stream.get(), &QHttp2Stream::dataReceived, m_context.get(),
            [this](const QByteArray &data, bool endStream) {
                if (m_state == State::Cancelled)
                    return;

                m_grpcDataParser.feed(data);
                while (auto frame = m_grpcDataParser.parseNextFrame()) {
                    QByteArray finalPayload;

                    if (frame->isCompressed) {
                        if (!m_decompressor || m_negotiatedEncoding.isEmpty()) {
                            finish({ QtGrpc::StatusCode::Internal,
                                     "Protocol error: received compressed message "
                                     "but no encoding was negotiated." });
                            return;
                        }
                        m_decompressor->feed(std::move(frame->payload));
                        // Read all decompressed data for this single message.
                        while (m_decompressor->hasData()) {
                            char buffer[4096];
                            qsizetype bytesRead = m_decompressor->read(buffer, sizeof(buffer));
                            if (bytesRead < 0) {
                                finish({ QtGrpc::StatusCode::Internal,
                                         "Decompression failed: %1"_L1
                                             .arg(m_decompressor->errorString()) });
                                return;
                            }
                            finalPayload.append(buffer, bytesRead);
                        }
                        m_decompressor->clear();
                        m_decompressor->setEncoding(m_negotiatedEncoding);
                    } else {
                        finalPayload = std::move(frame->payload);
                    }

                    qCDebug(lcStream,
                            "[%p] Processed gRPC message (compressed=%s, "
                            "payloadSize=%" PRIdQSIZETYPE ", bufferRemaining=%" PRIdQSIZETYPE ")",
                            this, frame->isCompressed ? "true" : "false", finalPayload.size(),
                            m_grpcDataParser.bytesAvailable());

                    emit m_context->messageReceived(finalPayload);
                }

                if (endStream) {
                    if (const auto bytes = m_grpcDataParser.bytesAvailable()) {
                        finish({ QtGrpc::StatusCode::DataLoss,
                                 "Unexcpected end of stream with %1 bytes remaining"_L1
                                     .arg(QString::number(bytes)) });
                        return;
                    }
                    finish({});
                }
            });

    connect(m_stream.get(), &QHttp2Stream::uploadFinished, this, &Http2Handler::processQueue);
}

// Builds HTTP/2 headers for the initial gRPC request.
// The headers are sent once the HTTP/2 connection is established.
HPack::HttpHeader Http2Handler::constructInitialHeaders() const
{
    const static QByteArray AuthorityHeader(":authority");
    const static QByteArray MethodHeader(":method");
    const static QByteArray MethodValue("POST");
    const static QByteArray PathHeader(":path");
    const static QByteArray SchemeHeader(":scheme");

    const static QByteArray TEHeader("te");
    const static QByteArray TEValue("trailers");
    const static QByteArray GrpcServiceNameHeader("service-name");
    const static QByteArray UserAgentHeader("user-agent");
    const static QByteArray UserAgentValue("grpc-c++-qtgrpc/"_ba + QT_VERSION_STR + " ("_ba
                                    + QSysInfo::productType().toUtf8() + '/'
                                    + QSysInfo::productVersion().toUtf8() + ')');

    const auto &channelOptions = channel()->channelOptions();
    const auto *channel = channelPriv();

    QByteArray service{ m_context->service() };
    QByteArray method{ m_context->method() };
    auto headers = HPack::HttpHeader{
        { AuthorityHeader,          channel->authorityHeader                 },
        { MethodHeader,             MethodValue                              },
        { PathHeader,               QByteArray('/' + service + '/' + method) },
        { SchemeHeader,             channel->schemeHeader                    },
        { ContentTypeHeader,        channel->contentType                     },
        { GrpcServiceNameHeader,    service                                  },
        { GrpcAcceptEncodingHeader, GrpcAcceptEncodingValue                  },
        { UserAgentHeader,          UserAgentValue                           },
        { TEHeader,                 TEValue                                  },
    };

    auto iterateMetadata = [&headers](const auto &metadata) {
        for (const auto &[key, value] : metadata.asKeyValueRange()) {
            const auto lowerKey = key.toLower();
            if (lowerKey == AuthorityHeader || lowerKey == MethodHeader || lowerKey == PathHeader
                || lowerKey == SchemeHeader || lowerKey == ContentTypeHeader) {
                continue;
            }
            headers.emplace_back(lowerKey, value);
        }
    };

    iterateMetadata(channelOptions.metadata(QtGrpc::MultiValue));
    iterateMetadata(m_context->callOptions().metadata(QtGrpc::MultiValue));

    return headers;
}

bool Http2Handler::constructFilterServerMetadata() const
{
    return m_context->callOptions()
        .filterServerMetadata()
        .value_or(channel()->channelOptions().filterServerMetadata().value_or(true));
}

QGrpcHttp2ChannelPrivate *Http2Handler::channelPriv() const
{
    return qobject_cast<QGrpcHttp2ChannelPrivate *>(this->parent());
}
QGrpcHttp2Channel *Http2Handler::channel() const
{
    return channelPriv()->q_ptr;
}

bool Http2Handler::handleContextExpired()
{
    if (m_context)
        return false;
    m_state = State::Cancelled;
    deleteLater(); // m_stream will sendRST_STREAM on destruction
    return true;
}

// Slot to enqueue a writeMessage request, either from the initial message
// or from the user in client/bidirectional streaming RPCs.
void Http2Handler::writeMessage(QByteArrayView data)
{
    if (m_writesDoneSent || m_state > State::Active || isStreamClosedForSending()) {
        qCDebug(lcStream, "[%p] Cannot write message (state=%s, writesDone=%d, streamClosed=%d)",
                this, QDebug::toBytes(m_state).constData(), m_writesDoneSent,
                isStreamClosedForSending());
        return;
    }

    QByteArray msg(GrpcMessageSizeHeaderSize + data.size(), '\0');
    // Args must be 4-byte unsigned int to fit into 4-byte big endian
    qToBigEndian(static_cast<quint32>(data.size()), msg.data() + 1);

    // protect against nullptr data.
    if (!data.isEmpty()) {
        std::memcpy(msg.begin() + GrpcMessageSizeHeaderSize, data.begin(),
                    static_cast<size_t>(data.size()));
    }

    m_queue.enqueue(msg);
    processQueue();
}

// Sends the initial headers and processes the message queue containing the
// initial message. At this point, the HTTP/2 connection is established, and
// the stream is attached.
void Http2Handler::sendInitialRequest()
{
    Q_ASSERT(!m_initialHeaders.empty());
    Q_ASSERT(m_stream);
    if (m_state >= State::Cancelled) {
        qCDebug(lcStream, "[%p] Stream finished before sending the initial request", this);
        return;
    }
    Q_ASSERT(m_state == State::Idle);

    if (!m_stream->sendHEADERS(m_initialHeaders, false)) {
        asyncFinish({ StatusCode::Unavailable,
                      tr("Unable to send initial headers to an HTTP/2 stream") });
        return;
    }
    m_state = State::RequestHeadersSent;
    m_initialHeaders.clear();
    processQueue();

    std::optional<std::chrono::milliseconds> deadline = m_context->callOptions().deadlineTimeout();
    if (!deadline)
        deadline = channel()->channelOptions().deadlineTimeout();
    if (deadline) {
        // We have an active stream, a deadline and the initial headers have
        // just been sent. It's time to start the timer.
        connect(&m_deadlineTimer, &QTimer::timeout, this, &Http2Handler::deadlineTimeout);
        m_deadlineTimer.start(*deadline);
    }
    qCDebug(lcStream, "[%p] Sending initial request (deadline=%s)", this,
            deadline ? qPrintable(QString::number(deadline->count()) + " ms"_L1) : "None");
}

// The core logic for sending the already serialized data through the HTTP/2 stream.
// This function is invoked either by the user via writeMessageRequested() or
// writesDoneRequested(), or it is continuously polled after the previous uploadFinished()
void Http2Handler::processQueue()
{
    if (!m_stream)
        return;

    if (m_stream->isUploadingDATA()) {
        qCDebug(lcStream, "[%p] Stream busy uploading (queue size=%" PRIdQSIZETYPE ")", this,
                m_queue.size());
        return;
    }

    if (m_queue.isEmpty())
        return;

    const auto nextMessage = m_queue.dequeue();
    const bool closeStream = nextMessage.isEmpty() || m_endStreamAtFirstData;
    m_stream->sendDATA(nextMessage, closeStream);
}

void Http2Handler::finish(const QGrpcStatus &status)
{
    if (handleContextExpired())
        return;
    if (m_state == State::Finished)
        return;
    if (m_state != State::Cancelled) // don't overwrite the Cancelled state
        m_state = State::Finished;
    m_deadlineTimer.stop();
    emit m_context->finished(status);
    deleteLater();
}
void Http2Handler::asyncFinish(const QGrpcStatus &status)
{
    if (handleContextExpired())
        return;
    QTimer::singleShot(0, m_context.get(), [this, status]() { finish(status); });
}

void Http2Handler::cancelWithStatus(const QGrpcStatus &status)
{
    if (m_state >= State::Cancelled) {
        qCWarning(lcStream, "[%p] Cannot cancel stream in state=%s", this,
                  QDebug::toBytes(m_state).constData());
        return;
    }
    qCDebug(lcStream, "[%p] Cancelling (state=%s)", this, QDebug::toBytes(m_state).constData());
    m_state = State::Cancelled;

    if (m_stream && m_stream->state() != QHttp2Stream::State::Idle) {
        // Immediate cancellation by sending the RST_STREAM frame.
        if (!m_stream->sendRST_STREAM(Http2::Http2Error::CANCEL)) {
            qCWarning(lcStream, "[%p] Failed cancellation (stream=%p, stream::state=%s)", this,
                      m_stream.get(), QDebug::toBytes(m_stream->state()).constData());
        }
    }

    finish(status);
}

void Http2Handler::writesDone()
{
    if (m_writesDoneSent || m_state > State::Active)
        return;
    m_writesDoneSent = true;

    qCDebug(lcStream, "[%p] Writes done received (streamClosed=%d)", this, isStreamClosedForSending());

    // Stream is already (half)closed, skip sending the DATA frame with the end-of-stream flag.
    if (isStreamClosedForSending())
        return;

    m_queue.enqueue({});
    processQueue();
}

void Http2Handler::handleHeaders(const HPack::HttpHeader &headers, HeaderPhase phase)
{
    // ABNF syntax: Rule, [Optional-Rule], *Variable-Repetition
    // Response-Headers → HTTPStatus [GrpcEncoding] [GrpcAcceptEncoding]
    //                    ContentType *Custom-Metadata
    // Trailers      → GrpcStatus [GrpcStatusMessage] [GrpcStatusDetails] *Custom-Metadata
    // Trailers-Only → HTTPStatus ContentType Trailers
    //
    // It's either Response-Headers + Trailers OR Trailers-Only for calls that
    // produce an immediate error. Any Trailers phase will finish the RPC.
    Q_ASSERT(phase != HeaderPhase::Invalid);
    struct HeaderValidation
    {
        const bool requireHttpStatus : 1;
        const bool requireContentType : 1;
        const bool requireGrpcStatus : 1;
        bool hasHttpStatus : 1;
        bool hasContentType : 1;
        bool hasGrpcStatus : 1;
    };

    if (handleContextExpired())
        return;

    HeaderValidation validation{
        (phase != HeaderPhase::Trailers),
        (phase != HeaderPhase::Trailers),
        (phase != HeaderPhase::Initial),
        false,
        false,
        false,
    };

    QMultiHash<QByteArray, QByteArray> metadata;
    std::optional<QtGrpc::StatusCode> statusCode;
    QString statusMessage;

    for (const auto &[k, v] : headers) {
        if (validation.requireHttpStatus && k == HttpStatusHeader) {
            if (const auto status = v.toInt(); status != 200) {
                finish({ http2StatusToStatusCode(status), "Received HTTP/2 status: %1"_L1.arg(v) });
                return;
            }
            validation.hasHttpStatus = true;
        } else if (validation.requireContentType && k == ContentTypeHeader) {
            if (!v.toLower().startsWith(DefaultContentType)) {
                finish({ StatusCode::Internal, "Unexpected content-type: %1"_L1.arg(v) });
                return;
            }
            validation.hasContentType = true;
        } else if (validation.requireGrpcStatus && k == GrpcStatusHeader) {
            bool ok;
            const auto parsed = v.toShort(&ok);
            if (!ok) {
                finish({ StatusCode::Internal, "Failed to parse gRPC-status: %1"_L1.arg(v) });
                return;
            }
            statusCode = static_cast<StatusCode>(parsed);
            validation.hasGrpcStatus = true;
        } else if (validation.requireGrpcStatus && k == GrpcStatusMessageHeader) {
            // Allowed optional headers
            statusMessage = QString::fromUtf8(v);
        } else if (validation.requireGrpcStatus && k == GrpcStatusDetailsHeader) {
            // Allowed optional headers
            // TODO: Implement status-details - QTBUG-138362
        } else if (phase == HeaderPhase::Initial && k == GrpcEncodingHeader) {
            // Allowed optional headers
            if (v == "identity"_ba)
                continue;
            if (!GrpcAcceptEncodingValue.contains(v)
                || !QDecompressHelper::isSupportedEncoding(v)) {
                finish({ StatusCode::Internal,
                         "Server responded with an unsupported compression algorithm: %1"_L1
                             .arg(v) });
                return;
            }
            // Create and configure the decompressor for this stream.
            m_decompressor = std::make_unique<QDecompressHelper>();
            if (!m_decompressor->setEncoding(v)) {
                finish({ StatusCode::Internal,
                         "Failed to initialize decompressor for algorithm: %1"_L1.arg(v) });
                return;
            }
            m_negotiatedEncoding = v;
        } else if (phase == HeaderPhase::Initial && k == GrpcAcceptEncodingHeader) {
            // Allowed optional headers
            // TODO: Implement client-side (request) compression handling - QTBUG-140235
        } else if (k.startsWith(':')) {
            qCWarning(lcStream,
                      "[%p] Received unhandled HTTP/2 pseudo-header: { key: '%s', value: '%s' } "
                      "in phase: %s",
                      this, k.data(), v.data(), QDebug::toBytes(phase).constData());
        } else if (k.startsWith("grpc-")) {
            qCWarning(lcStream,
                      "[%p] Received unexcpected gRPC-reserved header: { key: %s, value: %s } "
                      "in phase: %s",
                      this, k.data(), v.data(), QDebug::toBytes(phase).constData());
        } else { // Custom-Metadata
            metadata.insert(k, v);
            continue;
        }

        if (!m_filterServerMetadata)
            metadata.insert(k, v);
    }

    if (validation.requireHttpStatus && !validation.hasHttpStatus) {
        finish({ StatusCode::Internal, "Missing valid '%1' header"_L1.arg(HttpStatusHeader) });
        return;
    }

    if (validation.requireContentType && !validation.hasContentType) {
        finish({ StatusCode::Internal, "Missing valid '%1' header"_L1.arg(ContentTypeHeader) });
        return;
    }

    if (validation.requireGrpcStatus && !validation.hasGrpcStatus) {
        finish({ StatusCode::Internal, "Missing status code in trailers"_L1 });
        return;
    }

    switch (phase) {
    case HeaderPhase::Initial:
        m_context->setServerInitialMetadata(std::move(metadata));
        break;
    case HeaderPhase::TrailersOnly:
        [[fallthrough]];
    case HeaderPhase::Trailers:
        m_context->setServerTrailingMetadata(std::move(metadata));
        finish({ *statusCode, statusMessage });
        break;
    default:
        Q_UNREACHABLE();
    }
}

///
/// ## QGrpcHttp2ChannelPrivate Implementations
///

QGrpcHttp2ChannelPrivate::QGrpcHttp2ChannelPrivate(const QUrl &uri, QGrpcHttp2Channel *q)
    : q_ptr(q), socketType(constructSocketType(uri, q_ptr->channelOptions())),
      hostUri(sanitizeHostUri(uri, q_ptr->channelOptions())),
      contentType(setupContentTypeNegotiation(q_ptr)),
      authorityHeader(constructAuthorityHeader(hostUri, socketType)),
      schemeHeader(hostUri.scheme().toLatin1())
{
    switch (socketType) {
    case SocketType::Tcp: {
        auto socket = std::make_unique<QTcpSocket>();
        connect(socket.get(), &QAbstractSocket::connected, this,
                &QGrpcHttp2ChannelPrivate::createHttp2Connection);
        connect(socket.get(), &QAbstractSocket::errorOccurred, this,
                &QGrpcHttp2ChannelPrivate::handleAbstractSocketError);
        m_reconnectFunction = [this, socket = socket.get()] {
            qCDebug(lcChannel, "[%p] Connecting to TCP endpoint at: %s:%d", this,
                    qPrintable(hostUri.host()), hostUri.port());
            socket->connectToHost(hostUri.host(), hostUri.port());
        };
        m_socket = std::move(socket);
        break;
    }

    case SocketType::Tls: {
#if QT_CONFIG(ssl)
        auto socket = std::make_unique<QSslSocket>();
        if (const auto &sslConfig = q_ptr->channelOptions().sslConfiguration()) {
            socket->setSslConfiguration(*sslConfig);
        } else {
            static const QByteArray h2NexProtocol = "h2"_ba;
            auto defaultSslConfig = QSslConfiguration::defaultConfiguration();
            auto allowedNextProtocols = defaultSslConfig.allowedNextProtocols();
            if (!allowedNextProtocols.contains(h2NexProtocol)) {
                allowedNextProtocols.append(h2NexProtocol);
                defaultSslConfig.setAllowedNextProtocols(allowedNextProtocols);
            }
            socket->setSslConfiguration(defaultSslConfig);
        }
        connect(socket.get(), &QSslSocket::encrypted, this,
                &QGrpcHttp2ChannelPrivate::createHttp2Connection);
        connect(socket.get(), &QAbstractSocket::errorOccurred, this,
                &QGrpcHttp2ChannelPrivate::handleAbstractSocketError);
        m_reconnectFunction = [this, socket = socket.get()] {
            qCDebug(lcChannel, "[%p] Connecting to SSL endpoint at: %s:%d", this,
                    qPrintable(hostUri.host()), hostUri.port());
            socket->connectToHostEncrypted(hostUri.host(), hostUri.port());
        };
        m_socket = std::move(socket);
#else
        m_reconnectFunction = [this] {
            qCFatal(lcChannel, "[%p] QSslSocket support needed for TLS transportation", this);
        };
#endif
        break;
    }

    case SocketType::Local:
    case SocketType::LocalAbstract: {
#if QT_CONFIG(localserver)
        auto socket = std::make_unique<QLocalSocket>();
        if (socketType == SocketType::LocalAbstract)
            socket->setSocketOptions(QLocalSocket::AbstractNamespaceOption);
        connect(socket.get(), &QLocalSocket::connected, this,
                &QGrpcHttp2ChannelPrivate::createHttp2Connection);
        connect(socket.get(), &QLocalSocket::errorOccurred, this,
                &QGrpcHttp2ChannelPrivate::handleLocalSocketError);
        m_reconnectFunction = [this, socket = socket.get()] {
            const QString name = hostUri.host() + hostUri.path();
            qCDebug(lcChannel, "[%p] Connecting to local socket at: %s", this, qPrintable(name));
            socket->connectToServer(name);
        };
        m_socket = std::move(socket);
#else
        m_reconnectFunction = [this] {
            qCFatal(lcChannel,
                    "[%p] QLocalSocket support needed for 'unix' or 'unix-abstract' transportation",
                    this);
        };
#endif
        break;
    }

    } // switch (socketType)

    m_reconnectFunction();
}

void QGrpcHttp2ChannelPrivate::processOperation(QGrpcOperationContext *operationContext,
                                                QByteArray &&messageData, bool endStream)
{
    Q_ASSERT_X(operationContext != nullptr, "QGrpcHttp2ChannelPrivate::processOperation",
               "operation context is nullptr.");

    // Send the finished signals asynchronously, so user connections work correctly.
    if (!m_socket->isWritable() && m_state == ConnectionState::Connected) {
        qCWarning(lcChannel, "[%p] Socket not writable for operation to %s (error=%s)", this,
                  qPrintable(hostUri.toString()), qPrintable(m_socket->errorString()));
        QTimer::singleShot(0, operationContext,
                           [operationContext, err = m_socket->errorString()]() {
                               emit operationContext->finished({ StatusCode::Unavailable, err });
                           });
        return;
    }

    auto *handler = new Http2Handler(this, operationContext, std::move(messageData), endStream);
    if (m_connection && !createHttp2Stream(handler))
        return;

    if (m_state == ConnectionState::SettingsReceived)
        handler->sendInitialRequest();

    if (m_state == ConnectionState::Error) {
        Q_ASSERT_X(m_reconnectFunction, "QGrpcHttp2ChannelPrivate::processOperation",
                   "Socket reconnection function is not defined.");
        if (m_isInsideSocketErrorOccurred) {
            qCWarning(lcChannel,
                      "[%p] Inside socket error handler. Reconnect deferred to event loop.", this);
            QTimer::singleShot(0, this, [this] { m_reconnectFunction(); });
        } else {
            m_reconnectFunction();
        }
        m_state = ConnectionState::Connecting;
        qCDebug(lcChannel, "[%p] State changed to 'Connecting'. Reconnection initiated.", this);
    }
}

void QGrpcHttp2ChannelPrivate::createHttp2Connection()
{
    Q_ASSERT_X(m_connection == nullptr, "QGrpcHttp2ChannelPrivate::createHttp2Connection",
               "Attempt to create the HTTP/2 connection, but it already exists. This situation is "
               "exceptional.");

    // Nagle's algorithm slows down gRPC communication when frequently sending small utility
    // HTTP/2 frames. Since an ACK is not sent until a predefined timeout if the TCP frame is
    // not full enough, communication hangs. In our case, this results in a 40ms delay when
    // WINDOW_UPDATE or PING frames are sent in a separate TCP frame.
    //
    // TODO: We should probably allow users to opt out of this using QGrpcChannelOptions,
    // see QTBUG-134428.
    if (QAbstractSocket *abstractSocket = qobject_cast<QAbstractSocket *>(m_socket.get()))
        abstractSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    m_connection = QHttp2Connection::createDirectConnection(m_socket.get(), {});

    Q_ASSERT_X(m_connection, "QGrpcHttp2ChannelPrivate", "Unable to create the HTTP/2 connection");
    connect(m_socket.get(), &QAbstractSocket::readyRead, m_connection,
            &QHttp2Connection::handleReadyRead);

    m_state = ConnectionState::Connected;
    qCDebug(lcChannel, "[%p] Created new HTTP/2 connection to %s", this,
            qPrintable(hostUri.toString()));

    connect(m_connection, &QHttp2Connection::settingsFrameReceived, this, [this] {
        if (m_state == ConnectionState::SettingsReceived)
            return;
        m_state = ConnectionState::SettingsReceived;
        qCDebug(lcChannel, "[%p] SETTINGS frame received. Connection ready for use.", this);
        for_each_non_expired_handler([](Http2Handler *handler) { handler->sendInitialRequest(); });
    });

    for_each_non_expired_handler([this](Http2Handler *handler) { createHttp2Stream(handler); });
}

void QGrpcHttp2ChannelPrivate::handleSocketError(const QByteArray &errorCode)
{
    for_each_non_expired_handler([this, &errorCode](Http2Handler *handler) {
        if (m_isInsideSocketErrorOccurred) {
            qCCritical(lcChannel,
                        "[%p] Socket errorOccurred signal triggered while "
                        "already handling an error",
                        this);
            return;
        }
        m_isInsideSocketErrorOccurred = true;
        auto reset = qScopeGuard([this]() { m_isInsideSocketErrorOccurred = false; });
        emit handler->finish({ StatusCode::Unavailable,
                                tr("Network error occurred: %1").arg(errorCode) });
    });

    qCDebug(lcChannel, "[%p] Socket error occurred (code=%s, details=%s, hostUri=%s)", this,
            errorCode.constData(), qPrintable(m_socket->errorString()),
            qPrintable(hostUri.toString()));
    delete m_connection;
    m_connection = nullptr;
    m_state = ConnectionState::Error;
}

QUrl QGrpcHttp2ChannelPrivate::sanitizeHostUri(const QUrl &rawUri,
                                               const QGrpcChannelOptions &chOpts) const
{
    QUrl sanitizedUri(rawUri);
    auto check = [&](QLatin1StringView expected) {
        if (rawUri.scheme() != expected) {
            qCWarning(lcChannel,
                      "[%p] Unsupported transport protocol scheme '%s'. Fall back to '%s'.", this,
                      qPrintable(rawUri.scheme()), qPrintable(expected));
            sanitizedUri.setScheme(expected);
        }
    };
    const auto scheme = rawUri.scheme();
    if (scheme == UnixScheme || scheme == UnixAbstractScheme) {
        sanitizedUri.setScheme(HttpScheme);
    } else if (scheme == HttpsScheme || hasSslConfiguration(chOpts)) {
        check(HttpsScheme);
        if (rawUri.port() < 0)
            sanitizedUri.setPort(443);
    } else {
        check(HttpScheme);
        if (rawUri.port() < 0)
            sanitizedUri.setPort(80);
    }
    return sanitizedUri;
}

QGrpcHttp2ChannelPrivate::SocketType
QGrpcHttp2ChannelPrivate::constructSocketType(const QUrl &rawUri, const QGrpcChannelOptions &chOpts)
{
    const auto scheme = rawUri.scheme();
    if (scheme == UnixScheme)
        return SocketType::Local;
    if (scheme == UnixAbstractScheme)
        return SocketType::LocalAbstract;
    if (scheme == HttpsScheme || hasSslConfiguration(chOpts))
        return SocketType::Tls;
    return SocketType::Tcp;
}

QByteArray QGrpcHttp2ChannelPrivate::setupContentTypeNegotiation(QGrpcHttp2Channel *qPtr) const
{
    auto channelOptions = qPtr->channelOptions();
    auto formatSuffix = channelOptions.serializationFormat().suffix();
    const QByteArray defaultContentType = DefaultContentType;
    const QByteArray contentTypeFromOptions = !formatSuffix.isEmpty()
        ? defaultContentType + '+' + formatSuffix
        : defaultContentType;

    bool warnAboutFormatConflict = !formatSuffix.isEmpty();
    QByteArray finalContentType = contentTypeFromOptions;

    const auto it = channelOptions.metadata(QtGrpc::MultiValue).constFind(ContentTypeHeader.data());
    if (it != channelOptions.metadata(QtGrpc::MultiValue).cend()) {
        if (formatSuffix.isEmpty() && it.value() != DefaultContentType) {
            // Auto-detect format from content-type header
            if (it.value() == "application/grpc+json") {
                channelOptions.setSerializationFormat(SerializationFormat::Json);
            } else if (it.value() == "application/grpc+proto" || it.value() == DefaultContentType) {
                channelOptions.setSerializationFormat(SerializationFormat::Protobuf);
            } else {
                qCWarning(lcChannel,
                          "[%p] Unable to determine serializer for entry { key: %s, value: %s }. "
                          "Defaulting to format '%s'",
                          this, it.key().data(), it.value().data(),
                          QDebug::toBytes(SerializationFormat::Default).constData());
                channelOptions.setSerializationFormat(SerializationFormat::Default);
            }
            qPtr->setChannelOptions(channelOptions);
            warnAboutFormatConflict = false;
        } else if (it.value() != contentTypeFromOptions) {
            warnAboutFormatConflict = true;
        } else {
            warnAboutFormatConflict = false;
        }
    } else {
        warnAboutFormatConflict = false;
    }

    // Update final content type if format changed
    if (formatSuffix != channelOptions.serializationFormat().suffix()) {
        finalContentType = !channelOptions.serializationFormat().suffix().isEmpty()
            ? defaultContentType + '+' + channelOptions.serializationFormat().suffix()
            : defaultContentType;
    }

    if (warnAboutFormatConflict) {
        qCWarning(lcChannel,
                  "[%p] Manually specified serialization format '%s' does not "
                  "match metadata entry { key: %s, value: %s }",
                  this, contentTypeFromOptions.data(), it.key().data(), it.value().data());
    }

    return finalContentType;
}

QByteArray QGrpcHttp2ChannelPrivate::constructAuthorityHeader(const QUrl &hostUri,
                                                              SocketType socketType)
{
    auto authority = hostUri.authority(QUrl::FullyEncoded | QUrl::RemoveUserInfo | QUrl::RemovePort)
                         .toLatin1();
    const int port = hostUri.port();
    if ((socketType == SocketType::Tcp && port != 80)
        || (socketType == SocketType::Tls && port != 443)) {
        authority += ':';
        authority += QByteArray::number(port);
    }

    return authority;
}

bool QGrpcHttp2ChannelPrivate::createHttp2Stream(Http2Handler *handler)
{
    Q_ASSERT(handler != nullptr);
    Q_ASSERT(m_connection);
    constexpr QHttp2Stream::Configuration StreamConfiguration = {
        false, // useDownloadBuffer
    };
    const auto streamAttempt = m_connection->createStream(StreamConfiguration);
    if (!streamAttempt.ok()) {
        handler->asyncFinish({ StatusCode::Unavailable,
                               tr("Unable to create an HTTP/2 stream (%1)")
                                   .arg(QDebug::toString(streamAttempt.error())) });
        return false;
    }
    handler->attachStream(streamAttempt.unwrap());
    return true;
}

///
/// ## QGrpcHttp2Channel Implementations
///

/*!
    Constructs QGrpcHttp2Channel with \a hostUri. Please see the
    \l{Transportation scheme} section for more information.
*/
QGrpcHttp2Channel::QGrpcHttp2Channel(const QUrl &hostUri)
    : d_ptr(std::make_unique<QGrpcHttp2ChannelPrivate>(hostUri, this))
{
}

/*!
    Constructs QGrpcHttp2Channel with \a hostUri and \a options. Please see the
    \l{Transportation scheme} section for more information.
*/
QGrpcHttp2Channel::QGrpcHttp2Channel(const QUrl &hostUri, const QGrpcChannelOptions &options)
    : QAbstractGrpcChannel(options),
      d_ptr(std::make_unique<QGrpcHttp2ChannelPrivate>(hostUri, this))
{
}

QGrpcHttp2Channel::QGrpcHttp2Channel(const QUrl &hostUri, QGrpcInterceptorChain interceptorChain)
    : QAbstractGrpcChannel(std::move(interceptorChain)),
      d_ptr(std::make_unique<QGrpcHttp2ChannelPrivate>(hostUri, this))
{
}

QGrpcHttp2Channel::QGrpcHttp2Channel(const QUrl &hostUri, const QGrpcChannelOptions &options,
                                     QGrpcInterceptorChain interceptorChain)
    : QAbstractGrpcChannel(options, std::move(interceptorChain)),
      d_ptr(std::make_unique<QGrpcHttp2ChannelPrivate>(hostUri, this))
{
}

/*!
    Destroys the QGrpcHttp2Channel object.
*/
QGrpcHttp2Channel::~QGrpcHttp2Channel() = default;

/*!
    Returns the host URI for this channel.
*/
QUrl QGrpcHttp2Channel::hostUri() const
{
    return d_ptr->hostUri;
}

/*!
    \internal
    Initiates a unary \gRPC call.
*/
void QGrpcHttp2Channel::call(QGrpcOperationContext *operationContext, QByteArray &&messageData)
{
    d_ptr->processOperation(operationContext, std::move(messageData), true);
}

/*!
    \internal
    Initiates a server-side \gRPC stream.
*/
void QGrpcHttp2Channel::serverStream(QGrpcOperationContext *operationContext,
                                     QByteArray &&messageData)
{
    d_ptr->processOperation(operationContext, std::move(messageData), true);
}

/*!
    \internal
    Initiates a client-side \gRPC stream.
*/
void QGrpcHttp2Channel::clientStream(QGrpcOperationContext *operationContext,
                                     QByteArray &&messageData)
{
    d_ptr->processOperation(operationContext, std::move(messageData), false);
}

/*!
    \internal
    Initiates a bidirectional \gRPC stream.
*/
void QGrpcHttp2Channel::bidiStream(QGrpcOperationContext *operationContext,
                                   QByteArray &&messageData)
{
    d_ptr->processOperation(operationContext, std::move(messageData), false);
}

/*!
    \internal
    Returns the serializer of the channel.
*/
std::shared_ptr<QAbstractProtobufSerializer> QGrpcHttp2Channel::serializer() const
{
    return channelOptions().serializationFormat().serializer();
}

QT_END_NAMESPACE

#include "qgrpchttp2channel.moc"
