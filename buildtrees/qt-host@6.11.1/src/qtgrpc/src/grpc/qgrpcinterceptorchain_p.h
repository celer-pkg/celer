// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:insignifcant reason:default

#ifndef QGRPCINTERCEPTORCHAIN_P_H
#define QGRPCINTERCEPTORCHAIN_P_H

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

#include <QtGrpc/qgrpcinterceptorchain.h>

#include <QtCore/qspan.h>

#include <utility>
#include <vector>

QT_BEGIN_NAMESPACE

class QGrpcInterceptorChainPrivate
{
    using DeleterFn = QGrpcInterceptorChain::DeleterFn;

public:
    void clear();

    bool add(void *interceptor, DeleterFn deleter,
             QSpan<const QtGrpcPrivate::InterceptorCapabilityBinding> bindings);

    [[nodiscard]] bool hasHandlerFor(QtGrpc::InterceptorCapability cap) const noexcept
    {
        return m_combinedCapabilities.testFlag(cap);
    }

    struct Entry
    {
        void *storage = nullptr;
        DeleterFn deleter = nullptr;
        std::vector<QtGrpcPrivate::InterceptorCapabilityBinding> bindings;
        QtGrpc::InterceptorCapabilities capabilities;

        Entry() = default;
        ~Entry()
        {
            if (storage && deleter)
                deleter(storage);
        }
        Entry(void *s, DeleterFn d, std::vector<QtGrpcPrivate::InterceptorCapabilityBinding> b)
            : storage(s), deleter(d), bindings(std::move(b))
        {
            for (auto binding : bindings)
                capabilities.setFlag(binding.capability);
        }

        Q_DISABLE_COPY(Entry)

        Entry(Entry &&other) noexcept
            : storage(std::exchange(other.storage, nullptr)),
              deleter(std::exchange(other.deleter, nullptr)), bindings(std::move(other.bindings)),
              capabilities(other.capabilities)
        {
        }
        QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(Entry)

        void swap(Entry &other) noexcept
        {
            qt_ptr_swap(storage, other.storage);
            qt_ptr_swap(deleter, other.deleter);
            bindings.swap(other.bindings);
            std::swap(capabilities, other.capabilities);
        }
    };

    QtGrpc::InterceptorCapabilities m_combinedCapabilities;
    std::vector<Entry> m_interceptors;
};

QT_END_NAMESPACE

#endif // QGRPCINTERCEPTORCHAIN_P_H
