// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include <QtGrpc/private/qgrpcinterceptorchain_p.h>
#include <QtGrpc/private/qtgrpclogging_p.h>

QT_BEGIN_NAMESPACE

void QGrpcInterceptorChainPrivate::clear()
{
    m_combinedCapabilities = {};
    m_interceptors.clear();
}

bool QGrpcInterceptorChainPrivate::add(void *interceptor, DeleterFn deleter,
                                       QSpan<const QtGrpcPrivate::InterceptorCapabilityBinding>
                                           bindings)
{
    Q_ASSERT(interceptor);
    Q_ASSERT(!bindings.empty());

    // Entry takes ownership with RAII semantics.
    Entry e{
        interceptor,
        deleter,
        { bindings.cbegin(), bindings.cend() }
    };

    m_combinedCapabilities |= e.capabilities;
    m_interceptors.push_back(std::move(e));

    return true;
}

/*!
    \since 6.11
    \inmodule QtGrpc
    \ingroup qtgrpc-interceptors
    \class QGrpcInterceptorChain
    \brief Stores and manages a sequence of interceptors.

    QGrpcInterceptorChain stores interceptor instances that observe or modify
    different stages of an RPC. The chain is passed to a channel during
    construction, and the channel owns the chain for the remainder of its
    lifetime.

    As an RCP progresses through its lifecycle, the channel invokes the
    interceptors stored in the chain at the corresponding interception points.

    Interceptors are added to the chain in a defined order. The order is
    significant and determines how interceptors are invoked during outbound and
    inbound stages of an RPC. For details, see the \l{Direction and flow}
    section of the \l{Qt GRPC Interceptors Overview}.

    Interceptors can be added using two primary ownership models:

    \list
    \li \b{Owning}: Add interceptors using \c{std::unique_ptr<T>}. On success, the
        chain takes ownership and destroys the interceptors when the chain
        (and therefore the channel) is destroyed.
    \li \b{Non-owning}: Add interceptors using raw pointers \c{T*}. The chain does
        not take ownership. The caller must ensure that interceptor objects
        remain valid for as long as the channel may invoke them.
    \endlist

    Both models can be combined within the same chain. While supported,
    \b{mixing} owning and non-owning interceptors requires care to ensure that
    non-owning interceptors outlive all channels that may use them.

    \note QGrpcInterceptorChain is not copyable and can only be moved.

    \sa QtGrpc::InterceptorCapability
*/

/*!
    \fn void QGrpcInterceptorChain::swap(QGrpcInterceptorChain &other) noexcept

    \include qtgrpc-shared.qdocinc swap-desc
*/

/*!
    \fn template <typename T, QtGrpcPrivate::if_interceptor<T> = true> bool QGrpcInterceptorChain::add(std::unique_ptr<T> &&interceptor)

    Adds \a interceptor to the chain and takes ownership of it.

    Returns \c true if the interceptor was added; otherwise returns \c false.

    Ownership of \a interceptor is transferred to the chain only if the
    interceptor is successfully added. If the function returns \c false,
    ownership remains with the caller.

    //! [if_interceptor]
    Each interceptor type must implement at least one interceptor interface
    (for example, QGrpcStartInterceptor or QGrpcFinishedInterceptor) to provide
    interception capabilities.
    //! [if_interceptor]

    \sa set(), clear()
*/

/*!
    \fn template <typename... Ts, QtGrpcPrivate::if_interceptor<Ts...> = true> bool QGrpcInterceptorChain::set(std::unique_ptr<Ts> &&...interceptors)

    Replaces the contents of the chain with \a interceptors and takes
    ownership of them.

    Returns \c true if all interceptors were added successfully; otherwise
    returns \c false and leaves the chain unchanged.

    Interceptors are processed in the order in which they are passed to this
    function. If all additions succeed, the chain is replaced with the new
    interceptors in that order.

    Ownership is transferred incrementally as interceptors are processed.
    If the operation fails, any interceptors that were successfully added
    before the failure are destroyed, while the remaining (not yet processed)
    unique_ptrs remain valid.

    \include qgrpcinterceptorchain.cpp if_interceptor

    \sa add(), clear()
*/

/*!
    \fn template <typename T, QtGrpcPrivate::if_interceptor<T> = true> bool QGrpcInterceptorChain::add(T *interceptor)

    Adds \a interceptor to the chain without taking ownership.

    Returns \c true if the interceptor was added; otherwise returns \c false.

    //! [non_owning_add]
    When setting interceptors by raw pointer, the caller retains ownership.
    The QGrpcInterceptorChain stores non-owning references to the provided
    interceptors.

    The interceptor chain is transferred to and owned by the channel. As a
    result, all interceptor objects must remain valid for the entire lifetime
    of the channel that uses the chain.
    //! [non_owning_add]

    //! [non_owning_code_desc]
    The following code shows how to use a custom deleter to tie interceptor
    lifetime to a single channel:
    //! [non_owning_code_desc]

    \code
    auto *metricsInterceptor = new MetricsInterceptor();
    auto *authInterceptor = new AuthInterceptor();
    ~~~
    QGrpcInterceptorChain chain;
    if (!chain.add(metricsInterceptor))
        qFatal("Failed to add metrics interceptor");
    if (!chain.add(authInterceptor))
        qFatal("Failed to add auth interceptor");

    auto channel = std::shared_ptr<QGrpcHttp2Channel>(
        new QGrpcHttp2Channel(QUrl("you_uri:port"), std::move(chain)),
        [&metricsInterceptor, &authInterceptor](QGrpcHttp2Channel* ptr) {
            delete ptr;
            qDebug() << "Channel destroyed.";
            // If these interceptors are not shared with any other channel,
            // this is an appropriate place to delete them. Otherwise their
            // lifetime must be managed externally.
            delete metricsInterceptor;
            delete authInterceptor;
            metricsInterceptor = nullptr;
            authInterceptor = nullptr;
        }
    );
    \endcode

    \include qgrpcinterceptorchain.cpp if_interceptor

    \sa set(), clear()
*/

/*!
    \fn template <typename... Ts, QtGrpcPrivate::if_interceptor<Ts...> = true> bool QGrpcInterceptorChain::set(Ts *...interceptors)

    Replaces the contents of the chain with \a interceptors without taking
    ownership of them.

    Returns \c true if all interceptors were added successfully; otherwise
    returns \c false and leaves the chain unchanged.

    \include qgrpcinterceptorchain.cpp non_owning_add

    \include qgrpcinterceptorchain.cpp non_owning_code_desc

    \code
    auto *metricsInterceptor = new MetricsInterceptor();
    auto *authInterceptor = new AuthInterceptor();
    ~~~
    QGrpcInterceptorChain chain;
    if (!chain.set(metricsInterceptor, authInterceptor))
        qFatal("Failed to set interceptors to chain");

    auto channel = std::shared_ptr<QGrpcHttp2Channel>(
        new QGrpcHttp2Channel(QUrl("you_uri:port"), std::move(chain)),
        [&metricsInterceptor, &authInterceptor](QGrpcHttp2Channel* ptr) {
            delete ptr;
            qDebug() << "Channel destroyed.";
            // If these interceptors are not shared with any other channel,
            // this is an appropriate place to delete them. Otherwise their
            // lifetime must be managed externally.
            delete metricsInterceptor;
            delete authInterceptor;
            metricsInterceptor = nullptr;
            authInterceptor = nullptr;
        }
    );
    \endcode

    \include qgrpcinterceptorchain.cpp if_interceptor

    \sa add(), clear()
*/

/*!
    Constructs an empty interceptor chain.
*/
QGrpcInterceptorChain::QGrpcInterceptorChain() : d_ptr(new QGrpcInterceptorChainPrivate())
{
}

/*!
    Destroys the interceptor chain.
*/
QGrpcInterceptorChain::~QGrpcInterceptorChain()
{
    delete d_ptr;
}

/*!
    Clears the chain.

    Removes all interceptors from the chain. Any interceptors owned by the
    chain are destroyed.

    \sa add(), set()
*/
void QGrpcInterceptorChain::clear()
{
    Q_D(QGrpcInterceptorChain);
    d->clear();
}

/*!
    Returns the number of interceptors stored in the chain.

    \sa isEmpty()
*/
qsizetype QGrpcInterceptorChain::size() const noexcept
{
    Q_D(const QGrpcInterceptorChain);
    return d->m_interceptors.size();
}

/*!
    \fn bool QGrpcInterceptorChain::isEmpty() const noexcept

//! [isEmpty]
    Returns \c true if the chain contains no interceptors; otherwise returns \c false.
//! [isEmpty]

    \sa size(), empty()
*/

/*!
    \fn bool QGrpcInterceptorChain::empty() const noexcept

    \include qgrpcinterceptorchain.cpp isEmpty

    \sa size(), isEmpty()
*/

/*!
    Returns \c true if the chain contains at least one interceptor that provides
    a handler for capability \a cap; otherwise returns \c false.

    \sa capabilities(), capabilitiesAt()
*/
bool QGrpcInterceptorChain::hasHandlerFor(QtGrpc::InterceptorCapability cap) const noexcept
{
    Q_D(const QGrpcInterceptorChain);
    return d->hasHandlerFor(cap);
}

/*!
    Returns the combined capabilities of all interceptors stored in the chain.

    The returned value is the bitwise OR of capabilities provided by the interceptors.

    \sa hasHandlerFor(), capabilitiesAt(), QtGrpc::InterceptorCapability
*/
QtGrpc::InterceptorCapabilities QGrpcInterceptorChain::capabilities() const noexcept
{
    Q_D(const QGrpcInterceptorChain);
    return d->m_combinedCapabilities;
}

/*!
    Returns the capabilities provided by the interceptor at \a index.

    \warning Calling this function with an \a index that is not valid for
    the chain constitutes undefined behavior.

    \sa capabilities(), size()
*/
QtGrpc::InterceptorCapabilities
QGrpcInterceptorChain::capabilitiesAt(qsizetype index) const
{
    Q_D(const QGrpcInterceptorChain);
    Q_PRE(index >= 0 && index < size());
    return d->m_interceptors[index].capabilities;
}

bool QGrpcInterceptorChain::addImpl(void *interceptor, QGrpcInterceptorChain::DeleterFn deleter,
                                    QSpan<const QtGrpcPrivate::InterceptorCapabilityBinding>
                                        bindings)
{
    Q_D(QGrpcInterceptorChain);
    return d->add(interceptor, deleter, bindings);
}

void QGrpcInterceptorChain::warnNullInterceptor(qsizetype index, qsizetype total) const
{
    if (index == -1 && total == -1) {
        qGrpcWarning("Cannot add null interceptor");
    } else {
        qGrpcWarning("Cannot set null interceptor at index %lld of %lld",
                     static_cast<long long>(index), static_cast<long long>(total));
    }
}

QT_END_NAMESPACE
