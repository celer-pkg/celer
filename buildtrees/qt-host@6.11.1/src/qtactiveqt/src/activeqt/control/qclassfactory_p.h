// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#ifndef QCLASSFACTORY_P_H
#define QCLASSFACTORY_P_H

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

#include <qcoreapplication.h>
#include <ocidl.h>
#include <QtCore/private/qcomobject_p.h>

QT_BEGIN_NAMESPACE

namespace QtPrivate {

template <>
struct QComObjectTraits<IClassFactory2>
{
    static constexpr bool isGuidOf(REFIID riid) noexcept
    {
        return QComObjectTraits<IClassFactory2, IClassFactory>::isGuidOf(riid);
    }
};

} // namespace QtPrivate

// COM Factory class, mapping COM requests to ActiveQt requests.
// One instance of this class for each ActiveX the server can provide.
class QClassFactory : public QComObject<IClassFactory2>
{
    Q_DISABLE_COPY_MOVE(QClassFactory)
public:
    QClassFactory(CLSID clsid);

    virtual ~QClassFactory();

    HRESULT WINAPI CreateInstanceHelper(IUnknown *pUnkOuter, REFIID iid, void **ppObject);

    // IClassFactory
    HRESULT WINAPI CreateInstance(IUnknown *pUnkOuter, REFIID iid, void **ppObject) override;

    HRESULT WINAPI LockServer(BOOL fLock) override;
    // IClassFactory2
    HRESULT WINAPI RequestLicKey(DWORD, BSTR *pKey) override;

    HRESULT WINAPI GetLicInfo(LICINFO *pLicInfo) override;

    HRESULT WINAPI CreateInstanceLic(IUnknown *pUnkOuter, IUnknown *pUnkReserved,
                                     REFIID iid, BSTR bKey, PVOID *ppObject) override;

    static void cleanupCreatedApplication(QCoreApplication &app);

    QString className;

protected:
    CRITICAL_SECTION refCountSection;
    bool licensed = false;
    QString classKey;
};

QT_END_NAMESPACE

#endif // QCLASSFACTORY_P_H
