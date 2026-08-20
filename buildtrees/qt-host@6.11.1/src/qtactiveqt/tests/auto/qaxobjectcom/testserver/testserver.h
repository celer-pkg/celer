// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TEST_SERVER_H
#define TEST_SERVER_H

#include <qt_windows.h>
#include "testserverlib.h"
#include <QtCore/private/qcomobject_p.h>
#include <QtCore/private/qcomptr_p.h>
#include "../../conversion/comutil_p.h"

class __declspec(uuid("af732aba-95cf-4ee7-bd59-8f946b7f82e3"))
TestServer : public QComObject<IComServer>
{
public:
    TestServer();

    HRESULT __stdcall QueryInterface(IID const &id, void **result) override;
    HRESULT __stdcall SetObserver(IUnknown *observer) override;
    HRESULT __stdcall VariantIn(VARIANT v) override;

private:
    ComPtr<IUnknown> m_unkDisp;
    ComPtr<IComServer> m_receiver;
};

struct Receiver : public QComObject<IComServer>
{
    HRESULT SetObserver(IUnknown *observer) override { return E_NOTIMPL; }

    HRESULT VariantIn(VARIANT arg) override
    {
        lastArg = ComVariant{ arg };
        return S_OK;
    }

    ComVariant lastArg{};
};

#endif
