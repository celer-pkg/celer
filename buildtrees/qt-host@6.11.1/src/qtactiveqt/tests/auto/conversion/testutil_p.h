// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TESTUTIL_P_H
#define TESTUTIL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtcore/qt_windows.h>
#include <qtcore/private/qcomobject_p.h>
#include <qtcore/qtglobal>
#include <QtCore/private/qcomptr_p.h>
#include <atomic>

template <typename T>
ULONG refCount(const ComPtr<T> &p)
{
    p->AddRef();
    return p->Release();
}

struct IUnknownStub : QComObject<IUnknown>
{
};

struct IDispatchStub : QComObject<IDispatch>
{
    HRESULT GetTypeInfoCount(UINT * /*pctinfo*/) override { return E_NOTIMPL; }

    HRESULT GetTypeInfo(UINT /*iTInfo*/, LCID /*lcid*/, ITypeInfo ** /*ppTInfo*/) override
    {
        return E_NOTIMPL;
    }

    HRESULT GetIDsOfNames(const IID & /*riid*/, LPOLESTR * /*rgszNames*/, UINT /*cNames*/,
                          LCID /*lcid*/, DISPID * /*rgDispId*/) override
    {
        return E_NOTIMPL;
    }

    HRESULT Invoke(DISPID /*dispIdMember*/, const IID & /*riid*/, LCID /*lcid*/, WORD /*wFlags*/,
                   DISPPARAMS * /*pDispParams*/, VARIANT * /*pVarResult*/,
                   EXCEPINFO * /*pExcepInfo*/, UINT * /*puArgErr*/) override
    {
        return E_NOTIMPL;
    }
};

#endif
