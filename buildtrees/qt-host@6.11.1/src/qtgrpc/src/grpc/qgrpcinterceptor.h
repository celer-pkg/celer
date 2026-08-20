// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGRPCINTERCEPTOR_H
#define QGRPCINTERCEPTOR_H

#if 0
#  pragma qt_class(QGrpcInterceptor)
#endif

#include <QtGrpc/qtgrpcglobal.h>
#include <QtGrpc/qtgrpcnamespace.h>

#include <QtCore/qcontainerfwd.h>
#include <QtCore/qflags.h>
#include <QtCore/qstringfwd.h>
#include <QtCore/qtclasshelpermacros.h>

#include <array>
#include <QtCore/q20type_traits.h>
#include <utility>

QT_BEGIN_NAMESPACE

class QGrpcStatus;
class QGrpcOperationContext;
class QGrpcCallOptions;
class QAbstractGrpcChannel;
class QProtobufMessage;

class QGrpcInterceptionContext
{
    const quint32 m_version = 1;

public:
    ~QGrpcInterceptionContext() = default;
    Q_DISABLE_COPY_MOVE(QGrpcInterceptionContext)

    [[nodiscard]] quint32 version() const noexcept { return m_version; }

    void channel() const && = delete;
    [[nodiscard]] const QAbstractGrpcChannel &channel() const & noexcept { return *m_channel; }

    [[nodiscard]] Q_GRPC_EXPORT QtGrpc::RpcDescriptor descriptor() const noexcept;

    void callOptions() const && = delete;
    [[nodiscard]] Q_GRPC_EXPORT const QGrpcCallOptions &callOptions() const & noexcept;

    [[nodiscard]] Q_GRPC_EXPORT quint64 operationId() const noexcept;

protected:
    const QAbstractGrpcChannel *m_channel;
    QGrpcOperationContext *m_operationContext;

private:
    explicit QGrpcInterceptionContext(const QAbstractGrpcChannel &channel,
                                      QGrpcOperationContext &context) noexcept
        : m_channel(&channel), m_operationContext(&context)
    {
    }

    // prevent users from implementing their own, as the data-members don't qualify for it.
    friend size_t qHash(const QGrpcInterceptionContext &) = delete;
    friend size_t qHash(const QGrpcInterceptionContext &, size_t) = delete;
    friend bool operator==(const QGrpcInterceptionContext &,
                           const QGrpcInterceptionContext &) = delete;
    friend bool operator!=(const QGrpcInterceptionContext &,
                           const QGrpcInterceptionContext &) = delete;

#ifndef QT_NO_DEBUG_STREAM
    friend QDebug operator<<(QDebug, const QGrpcInterceptionContext &) = delete;
#endif
#ifndef QT_NO_DATASTREAM
    friend QDataStream &operator<<(QDataStream &, const QGrpcInterceptionContext &) = delete;
    friend QDataStream &operator>>(QDataStream &, QGrpcInterceptionContext &) = delete;
#endif

    friend class QGrpcInterceptorEngine;
};

class Q_GRPC_EXPORT QGrpcStartInterceptor
{
public:
    enum class Continuation : uint8_t { Proceed, Drop };

    virtual ~QGrpcStartInterceptor();
    virtual Continuation onStart(QGrpcInterceptionContext &context, QProtobufMessage &message,
                                 QGrpcCallOptions &callOptions) = 0;
};

class Q_GRPC_EXPORT QGrpcInitialMetadataInterceptor
{
public:
    virtual ~QGrpcInitialMetadataInterceptor();
    virtual void onInitialMetadata(QGrpcInterceptionContext &context,
                                   QMultiHash<QByteArray, QByteArray> &metadata) = 0;
};

class Q_GRPC_EXPORT QGrpcMessageReceivedInterceptor
{
public:
    virtual ~QGrpcMessageReceivedInterceptor();
    virtual void onMessageReceived(QGrpcInterceptionContext &context, QByteArray &messageData) = 0;
};

class Q_GRPC_EXPORT QGrpcWriteMessageInterceptor
{
public:
    virtual ~QGrpcWriteMessageInterceptor();
    virtual void onWriteMessage(QGrpcInterceptionContext &context, QProtobufMessage &message) = 0;
};

class Q_GRPC_EXPORT QGrpcWritesDoneInterceptor
{
public:
    virtual ~QGrpcWritesDoneInterceptor();
    virtual void onWritesDone(QGrpcInterceptionContext &context) = 0;
};

class Q_GRPC_EXPORT QGrpcTrailingMetadataInterceptor
{
public:
    virtual ~QGrpcTrailingMetadataInterceptor();
    virtual void onTrailingMetadata(QGrpcInterceptionContext &context,
                                    QMultiHash<QByteArray, QByteArray> &metadata) = 0;
};

class Q_GRPC_EXPORT QGrpcFinishedInterceptor
{
public:
    virtual ~QGrpcFinishedInterceptor();
    virtual void onFinished(QGrpcInterceptionContext &context, QGrpcStatus &status) = 0;
};

class Q_GRPC_EXPORT QGrpcCancelInterceptor
{
public:
    virtual ~QGrpcCancelInterceptor();
    virtual void onCancel(QGrpcInterceptionContext &context) = 0;
};

namespace QtGrpc {
// clang-format off

// Stable mapping of interceptor types to bit flags.
enum class InterceptorCapability : quint64 {
    Start            = 0x01,
    InitialMetadata  = 0x02,
    MessageReceived  = 0x04,
    WriteMessage     = 0x08,
    WritesDone       = 0x10,
    TrailingMetadata = 0x20,
    Finished         = 0x40,
    Cancel           = 0x80,
    // WARNING: Do not reorder or modify existing values. Append only.
};
Q_DECLARE_FLAGS(InterceptorCapabilities, InterceptorCapability)
Q_DECLARE_OPERATORS_FOR_FLAGS(InterceptorCapabilities)

} // namespace QtGrpc

namespace QtGrpcPrivate {

template <typename... Ts>
struct TypeList
{
    static constexpr size_t size = sizeof...(Ts);

    template <typename T>
    static constexpr size_t countBaseOf() noexcept
    {
        return (... + (std::is_base_of_v<Ts, T> ? 1u : 0u));
    }

    template <typename T>
    using ContainsBase = std::disjunction<std::is_base_of<Ts, T>...>;

    template <typename T>
    using ContainsExact = std::disjunction<std::is_same<T, Ts>...>;

    template <typename Fn>
    static constexpr void forEach(Fn &&fn) noexcept
    {
        forEachImpl(std::forward<Fn>(fn), std::make_index_sequence<size>{});
    }

private:
    template <typename Fn, size_t... Is>
    static constexpr void forEachImpl(Fn &&fn, std::index_sequence<Is...>) noexcept
    {
        (std::forward<Fn>(fn)(std::integral_constant<size_t, Is>{},
                              q20::type_identity<Ts>{}), ...);
    }
};

using InterceptorTypes = TypeList<
    QGrpcStartInterceptor,
    QGrpcInitialMetadataInterceptor,
    QGrpcMessageReceivedInterceptor,
    QGrpcWriteMessageInterceptor,
    QGrpcWritesDoneInterceptor,
    QGrpcTrailingMetadataInterceptor,
    QGrpcFinishedInterceptor,
    QGrpcCancelInterceptor
>;

enum class InterceptionFlow : uint8_t {
    Outbound, // App -> Network
    Inbound   // Network -> App
};

// Maps each individual interceptor interface to its capability and method
template <typename T>
struct InterceptorInfo;

#define QGRPC_INTERCEPTOR_INFO(Type, Cap, Method, Flow)                        \
    template <>                                                                \
    struct InterceptorInfo<Type>                                               \
    {                                                                          \
        static constexpr auto capability = QtGrpc::InterceptorCapability::Cap; \
        static constexpr auto method = &Type::Method;                          \
        static constexpr auto flow = InterceptionFlow::Flow;                   \
    }

QGRPC_INTERCEPTOR_INFO(QGrpcStartInterceptor,
                       Start, onStart, Outbound);
QGRPC_INTERCEPTOR_INFO(QGrpcInitialMetadataInterceptor,
                       InitialMetadata, onInitialMetadata, Inbound);
QGRPC_INTERCEPTOR_INFO(QGrpcMessageReceivedInterceptor,
                       MessageReceived, onMessageReceived, Inbound);
QGRPC_INTERCEPTOR_INFO(QGrpcWriteMessageInterceptor,
                       WriteMessage, onWriteMessage, Outbound);
QGRPC_INTERCEPTOR_INFO(QGrpcWritesDoneInterceptor,
                       WritesDone, onWritesDone, Outbound);
QGRPC_INTERCEPTOR_INFO(QGrpcTrailingMetadataInterceptor,
                       TrailingMetadata, onTrailingMetadata, Inbound);
QGRPC_INTERCEPTOR_INFO(QGrpcFinishedInterceptor,
                       Finished, onFinished, Inbound);
QGRPC_INTERCEPTOR_INFO(QGrpcCancelInterceptor,
                       Cancel, onCancel, Outbound);

#undef QGRPC_INTERCEPTOR_INFO

struct InterceptorCapabilityBinding
{
    QtGrpc::InterceptorCapability capability;
    void *interface;

    // Returns the list of interfaces found in T and extracted from \a interceptor, bound
    // to their capability. A generic interceptor T probably uses multiple inheritance.
    template <typename T>
    static constexpr auto extractFrom(T *interceptor) noexcept
    {
        constexpr auto N = InterceptorTypes::countBaseOf<T>();
        std::array<InterceptorCapabilityBinding, N> bindings{};
        size_t i = 0;

        InterceptorTypes::forEach(
            [&](auto , auto interfaceType) constexpr {
                using Interface = typename decltype(interfaceType)::type;
                if constexpr (std::is_base_of_v<Interface, T>) {
                    bindings[i++] = {
                        InterceptorInfo<Interface>::capability,
                        static_cast<Interface *>(interceptor),
                    };
                }
            });

        return bindings;
    }
};

template <typename T>
using is_interceptor = std::conjunction<
    std::negation<std::is_const<T>>,
    typename InterceptorTypes::template ContainsBase<T>,
    std::negation<typename InterceptorTypes::template ContainsExact<T>>
>;

template <typename T, typename... Ts>
// A template alias doesn't work: they don't support pack expansion to
// non-pack parameters, as in
//   <typename...Ts, if_interceptor<Ts...> = true>
//                                   ^ error: T is not a pack
// [[citation needed]
constexpr bool is_interceptor_v = std::conjunction_v<
        is_interceptor<T>, // require at least one
        is_interceptor<Ts>...
    >;

template <typename...Ts>
using if_interceptor = std::enable_if_t<is_interceptor_v<Ts...>, bool>;

// clang-format on
} // namespace QtGrpcPrivate

QT_END_NAMESPACE

#endif // QGRPCINTERCEPTOR_H
