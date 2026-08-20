// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGRPCINTERCEPTORCHAIN_H
#define QGRPCINTERCEPTORCHAIN_H

#include <QtGrpc/qgrpcinterceptor.h>
#include <QtGrpc/qtgrpcglobal.h>
#include <QtGrpc/qtgrpcnamespace.h>

#include <QtCore/qcompilerdetection.h>
#include <QtCore/qspan.h>
#include <QtCore/qstring.h>

#include <memory>
#include <QtCore/q20type_traits.h>

QT_BEGIN_NAMESPACE

class QGrpcInterceptorChainPrivate;
class QGrpcInterceptorChain final
{
public:
    Q_GRPC_EXPORT QGrpcInterceptorChain();
    Q_GRPC_EXPORT ~QGrpcInterceptorChain();

    Q_DISABLE_COPY(QGrpcInterceptorChain)

    QGrpcInterceptorChain(QGrpcInterceptorChain &&other) noexcept
        : d_ptr(std::exchange(other.d_ptr, nullptr))
    {
    }
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_PURE_SWAP(QGrpcInterceptorChain)

    void swap(QGrpcInterceptorChain &other) noexcept { qt_ptr_swap(d_ptr, other.d_ptr); }

    template <typename T, QtGrpcPrivate::if_interceptor<T> = true>
    bool add(std::unique_ptr<T> &&interceptor)
    {
        return addHelper(std::move(interceptor));
    }

    template <typename... Ts, QtGrpcPrivate::if_interceptor<Ts...> = true>
    bool set(std::unique_ptr<Ts> &&...interceptors)
    {
        return setHelper(std::move(interceptors)...);
    }

    template <typename T, QtGrpcPrivate::if_interceptor<T> = true>
    bool add(T *interceptor)
    {
        return addHelper(interceptor);
    }

    template <typename... Ts, QtGrpcPrivate::if_interceptor<Ts...> = true>
    bool set(Ts *...interceptors)
    {
        return setHelper(interceptors...);
    }

    Q_GRPC_EXPORT void clear();

    [[nodiscard]] Q_GRPC_EXPORT qsizetype size() const noexcept;
    [[nodiscard]] bool isEmpty() const noexcept { return !size(); }
    [[nodiscard]] bool empty() const noexcept { return isEmpty(); }

    [[nodiscard]] Q_GRPC_EXPORT bool
    hasHandlerFor(QtGrpc::InterceptorCapability cap) const noexcept;
    [[nodiscard]] Q_GRPC_EXPORT QtGrpc::InterceptorCapabilities capabilities() const noexcept;
    [[nodiscard]] Q_GRPC_EXPORT QtGrpc::InterceptorCapabilities
    capabilitiesAt(qsizetype index) const;

private:
    using DeleterFn = void (*)(void *);

    template <typename T>
    struct InterceptorTraits
    {
        static constexpr bool is_supported = false;
    };

    template <typename T>
    struct InterceptorTraits<T *>
    {
        static constexpr bool is_supported = true;
        static constexpr bool is_owning = false;
        using element_type = T;

        static element_type *get(T *p) noexcept { return p; }
        static constexpr DeleterFn deleter() noexcept { return nullptr; }
    };

    template <typename T>
    struct InterceptorTraits<std::unique_ptr<T>>
    {
        static constexpr bool is_supported = true;
        static constexpr bool is_owning = true;
        using element_type = T;

        static_assert(!std::is_array_v<T>, "Arrays of interceptors are not supported.");

        static element_type *get(const std::unique_ptr<T> &p) noexcept { return p.get(); }
        static constexpr DeleterFn deleter() noexcept
        {
            return [](void *p) { delete static_cast<T *>(p); };
        }
    };

    template <typename T>
    [[nodiscard]] bool addHelper(T &&interceptor)
    {
        using namespace QtGrpcPrivate;

        using DecayedT = q20::remove_cvref_t<T>;
        using Traits = QGrpcInterceptorChain::InterceptorTraits<DecayedT>;

        static_assert(Traits::is_supported,
                      "Interceptor must be passed as T* or std::unique_ptr<T>.");

        if (!interceptor) {
            warnNullInterceptor();
            return false;
        }

        auto *ptr = Traits::get(interceptor);
        const auto deleter = Traits::deleter();
        auto bindings = InterceptorCapabilityBinding::extractFrom(ptr);

        if (!addImpl(ptr, deleter, bindings))
            return false;

        if constexpr (Traits::is_owning)
            interceptor.release(); // we take ownership on success

        return true;
    }

    template <typename... Ts>
    [[nodiscard]] bool setHelper(Ts &&...interceptors)
    {
        qsizetype failedIndex = -1;
        qsizetype idx = 0;
        auto checkNull = [&](const auto &p) {
            if (!p && failedIndex == -1)
                failedIndex = idx;
            ++idx;
        };
        (checkNull(interceptors), ...);

        if (failedIndex != -1) {
            warnNullInterceptor(failedIndex, sizeof...(Ts) - 1);
            return false;
        }

        QGrpcInterceptorChain tmp;
        bool ok = (... && tmp.addHelper(std::forward<Ts>(interceptors)));

        if (ok)
            *this = std::move(tmp);

        return ok;
    }

    [[nodiscard]] Q_GRPC_EXPORT bool
    addImpl(void *interceptor, DeleterFn deleter,
            QSpan<const QtGrpcPrivate::InterceptorCapabilityBinding> bindings);

    Q_DECL_COLD_FUNCTION Q_GRPC_EXPORT void warnNullInterceptor(qsizetype index = -1,
                                                                qsizetype total = -1) const;

    // prevent users from implementing their own, as the data-members don't qualify for it.
    friend bool operator==(const QGrpcInterceptorChain &,
                           const QGrpcInterceptorChain &) = delete;
    friend bool operator!=(const QGrpcInterceptorChain &,
                           const QGrpcInterceptorChain &) = delete;
    friend size_t qHash(const QGrpcInterceptorChain &) = delete;
    friend size_t qHash(const QGrpcInterceptorChain &, size_t) = delete;
#ifndef QT_NO_DEBUG_STREAM
    friend QDebug operator<<(QDebug debug, const QGrpcInterceptorChain &) = delete;
#endif
#ifndef QT_NO_DATASTREAM
    friend QDataStream &operator<<(QDataStream &, const QGrpcInterceptorChain &) = delete;
    friend QDataStream &operator>>(QDataStream &, QGrpcInterceptorChain &) = delete;
#endif

    QGrpcInterceptorChainPrivate *d_ptr;

    friend class QGrpcInterceptorEngine;
    Q_DECLARE_PRIVATE(QGrpcInterceptorChain)
};

QT_END_NAMESPACE

#endif // QGRPCINTERCEPTORCHAIN_H
