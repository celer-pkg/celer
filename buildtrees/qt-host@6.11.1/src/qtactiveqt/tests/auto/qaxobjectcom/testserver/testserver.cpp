// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QString>
#include <QUuid>
#include <QtCore/qassert.h>
#include <QtCore/qdebug.h>
#include "testserverlib.h"
#include "testserver.h"

extern "C" IMAGE_DOS_HEADER __ImageBase; // Trickery to get path to current dll
static long s_serverLock = 0;

static const std::array<wchar_t, MAX_PATH> GetModuleFilename()
{
    std::array<wchar_t, MAX_PATH> filename;
    const DWORD length = GetModuleFileName(reinterpret_cast<HMODULE>(&__ImageBase), filename.data(),
                                           static_cast<DWORD>(filename.size()));

    if (0 == length || filename.size() == length)
        return {};

    return filename;
}

// Create a dispatcher that implements IDispatch for the input owner
static ComPtr<IUnknown> CreateDispatcher(const QUuid &guid, IUnknown *owner)
{
    ComPtr<ITypeLib> typeLib;
    if (LoadTypeLib(GetModuleFilename().data(), &typeLib) != S_OK)
    {
        qCritical("Failed to load type library");
        return {};
    }

    ComPtr<ITypeInfo> typeInfo;
    if (typeLib->GetTypeInfoOfGuid(guid, &typeInfo)
        != S_OK) {
        qCritical("Failed to get type info");
        return {};
    }

    ComPtr<IUnknown> dispatcher;
    if (CreateStdDispatch(owner, owner, typeInfo.Get(), &dispatcher) != S_OK) {
        qCritical("Failed to create standard dispatch");
        return {};
    }

    return dispatcher;
}

TestServer::TestServer()
{
    m_unkDisp = CreateDispatcher(__uuidof(IComServer), this);
    ++s_serverLock;
}

HRESULT __stdcall TestServer::QueryInterface(IID const &id, void **result)
{
    Q_ASSERT(result);
    if (id == __uuidof(IDispatch)) {
        ComPtr<IDispatch> disp;
        m_unkDisp.As(&disp);
        *result = disp.Detach();
        return S_OK;
    }
    return QComObject::QueryInterface(id, result);
}

HRESULT TestServer::SetObserver(IUnknown* observer)
{
    const ComPtr<IUnknown> rcv{ observer };
    return rcv.As(&m_receiver);
}

HRESULT TestServer::VariantIn(VARIANT v)
{
    return m_receiver->VariantIn(v);
}

struct Factory : IClassFactory
{
    ULONG __stdcall AddRef() override { return 2; }
    ULONG __stdcall Release() override { return 1; }
    HRESULT __stdcall QueryInterface(IID const &id, void **result) override
    {
        Q_ASSERT(result);

        if (id == __uuidof(IClassFactory) || id == __uuidof(IUnknown)) {
            *result = static_cast<IClassFactory *>(this);
            return S_OK;
        }

        *result = nullptr;
        return E_NOINTERFACE;
    }

    HRESULT __stdcall CreateInstance(IUnknown *outer, IID const &iid, void **result) override
    {
        Q_ASSERT(result);
        Q_ASSERT(!outer);

        const ComPtr<TestServer> server{ new (std::nothrow) TestServer };
        return server->QueryInterface(iid, result);
    }

    HRESULT __stdcall LockServer(BOOL lock) override
    {
        if (lock)
            ++s_serverLock;
        else
            --s_serverLock;

        return S_OK;
    }
};

HRESULT __stdcall DllGetClassObject(CLSID const &clsid, IID const &iid, void **result)
{
    if (__uuidof(TestServer) == clsid) {
        static Factory farm;

        return farm.QueryInterface(iid, result);
    }

    return CLASS_E_CLASSNOTAVAILABLE;
}

HRESULT __stdcall DllCanUnloadNow()
{
    return s_serverLock ? S_FALSE : S_OK;
}
