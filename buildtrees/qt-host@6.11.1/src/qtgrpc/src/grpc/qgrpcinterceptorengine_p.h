// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGRPCINTERCEPTORENGINE_P_H
#define QGRPCINTERCEPTORENGINE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGrpc/private/qgrpcinterceptorchain_p.h>
#include <QtGrpc/qgrpcinterceptor.h>
#include <QtGrpc/qgrpcoperationcontext.h>
#include <QtGrpc/qtgrpcnamespace.h>

#include <QtProtobuf/qprotobufmessage.h>

#include <QtCore/q20bit.h>
#include <QtCore/qspan.h>
#include <QtCore/qtconfigmacros.h>
#include <QtCore/qvarlengtharray.h>

#include <array>
#include <limits>
#include <type_traits>

QT_BEGIN_NAMESPACE

namespace QtGrpcPrivate {

inline constexpr std::underlying_type_t<QtGrpc::InterceptorCapability>
capabilityToIndex(QtGrpc::InterceptorCapability c) noexcept
{
    return q20::countr_zero(qToUnderlying(c));
}
inline constexpr QtGrpc::InterceptorCapability indexToCapability(uint8_t idx) noexcept
{
    Q_ASSERT(idx
             < std::numeric_limits<std::underlying_type_t<QtGrpc::InterceptorCapability>>::digits);
    return QtGrpc::InterceptorCapability(1u << idx);
}

static_assert(capabilityToIndex(QtGrpc::InterceptorCapability::Start) == 0);
static_assert(capabilityToIndex(QtGrpc::InterceptorCapability::InitialMetadata) == 1);
static_assert(capabilityToIndex(QtGrpc::InterceptorCapability::MessageReceived) == 2);
static_assert(capabilityToIndex(QtGrpc::InterceptorCapability::WriteMessage) == 3);
static_assert(capabilityToIndex(QtGrpc::InterceptorCapability::WritesDone) == 4);
static_assert(capabilityToIndex(QtGrpc::InterceptorCapability::TrailingMetadata) == 5);
static_assert(capabilityToIndex(QtGrpc::InterceptorCapability::Finished) == 6);
static_assert(capabilityToIndex(QtGrpc::InterceptorCapability::Cancel) == 7);
static_assert(InterceptorTypes::size == 8, "Please add assertions for all entries");

} // namespace QtGrpcPrivate

class QGrpcInterceptorEngine
{
public:
    explicit QGrpcInterceptorEngine(const QAbstractGrpcChannel &parentChannel) noexcept
        : m_parentChannel(parentChannel)
    {
    }

    [[nodiscard]] const QGrpcInterceptorChain &interceptorChain() const noexcept
    {
        return m_interceptorChain;
    }

    void setInterceptorChain(QGrpcInterceptorChain chain)
    {
        for (auto &handlers : m_capabilityHandlers)
            handlers.clear();

        for (const auto &i : chain.d_func()->m_interceptors)
            addInterceptor(i.bindings);

        m_interceptorChain = std::move(chain);
    }

    [[nodiscard]] bool hasHandlerFor(QtGrpc::InterceptorCapability capability) const noexcept
    {
        return m_interceptorChain.hasHandlerFor(capability);
    }

    [[nodiscard]]
    QGrpcStartInterceptor::Continuation onStart(QGrpcOperationContext &context,
                                                QProtobufMessage &message,
                                                QGrpcCallOptions &callOptions)
    {
        using Info = QtGrpcPrivate::InterceptorInfo<QGrpcStartInterceptor>;
        constexpr auto Idx = QtGrpcPrivate::capabilityToIndex(Info::capability);

        QGrpcInterceptionContext ctx(m_parentChannel, context);
        for (auto handler : m_capabilityHandlers[Idx]) {
            auto *iface = static_cast<QGrpcStartInterceptor *>(handler);
            auto result = (iface->*Info::method)(ctx, message, callOptions);
            if (result == QGrpcStartInterceptor::Continuation::Drop)
                return result;
        }

        return QGrpcStartInterceptor::Continuation::Proceed;
    }

    void onInitialMetadata(QGrpcOperationContext &context,
                           QMultiHash<QByteArray, QByteArray> &metadata)
    {
        invoke<QGrpcInitialMetadataInterceptor>(context, metadata);
    }

    void onMessageReceived(QGrpcOperationContext &context, QByteArray &messageData)
    {
        invoke<QGrpcMessageReceivedInterceptor>(context, messageData);
    }

    void onWriteMessage(QGrpcOperationContext &context, QProtobufMessage &message)
    {
        invoke<QGrpcWriteMessageInterceptor>(context, message);
    }

    void onWritesDone(QGrpcOperationContext &context)
    {
        invoke<QGrpcWritesDoneInterceptor>(context);
    }

    void onTrailingMetadata(QGrpcOperationContext &context,
                            QMultiHash<QByteArray, QByteArray> &metadata)
    {
        invoke<QGrpcTrailingMetadataInterceptor>(context, metadata);
    }

    void onFinished(QGrpcOperationContext &context, QGrpcStatus &status)
    {
        invoke<QGrpcFinishedInterceptor>(context, status);
    }

    void onCancel(QGrpcOperationContext &context) { invoke<QGrpcCancelInterceptor>(context); }

private:
    void addInterceptor(QSpan<const QtGrpcPrivate::InterceptorCapabilityBinding> bindings)
    {
        Q_ASSERT(!bindings.empty());
        for (auto binding : bindings) {
            const auto idx = QtGrpcPrivate::capabilityToIndex(binding.capability);
            m_capabilityHandlers[idx].push_back(binding.interface);
        }
    }

    template <typename Interface, typename... Args>
    void invoke(QGrpcOperationContext &context, Args &&...args)
    {
        using Info = QtGrpcPrivate::InterceptorInfo<Interface>;
        constexpr auto Idx = QtGrpcPrivate::capabilityToIndex(Info::capability);

        QGrpcInterceptionContext ctx(m_parentChannel, context);
        auto &handlers = m_capabilityHandlers[Idx];

        if constexpr (Info::flow == QtGrpcPrivate::InterceptionFlow::Outbound) {
            for (auto it = handlers.begin(); it != handlers.cend(); ++it) {
                auto *iface = static_cast<Interface *>(*it);
                (iface->*Info::method)(ctx, std::forward<Args>(args)...);
            }
        } else {
            for (auto rit = handlers.rbegin(); rit != handlers.crend(); ++rit) {
                auto *iface = static_cast<Interface *>(*rit);
                (iface->*Info::method)(ctx, std::forward<Args>(args)...);
            }
        }
    }

    const QAbstractGrpcChannel &m_parentChannel;

    // List of interceptor interfaces handling a single capability.
    using HandlerList = QVarLengthArray<void *, 5>;
    // Capability-indexed storage of interceptor handlers.
    std::array<HandlerList, QtGrpcPrivate::InterceptorTypes::size> m_capabilityHandlers;
    QGrpcInterceptorChain m_interceptorChain;
};

QT_END_NAMESPACE

#endif // QGRPCINTERCEPTORENGINE_P_H
