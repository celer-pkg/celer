// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#ifndef QAXAGGREGATED_H
#define QAXAGGREGATED_H

#include <QtCore/qobject.h>

struct IUnknown;

QT_BEGIN_NAMESPACE

class QWidget;

class QUuid;

class QAxAggregated
{
    friend class QAxServerBase;
    friend class QAxClientSite;
    Q_DISABLE_COPY_MOVE(QAxAggregated)
public:
    virtual long queryInterface(const QUuid &iid, void **iface) = 0;

protected:
    QAxAggregated() = default;
    virtual ~QAxAggregated() = default;

    inline IUnknown *controllingUnknown() const
    { return controlling_unknown; }
    QWidget *widget() const;
    inline QObject *object() const { return the_object; }

private:
    IUnknown *controlling_unknown = nullptr;
    QObject *the_object = nullptr;
};

#define QAXAGG_IUNKNOWN \
    HRESULT WINAPI QueryInterface(REFIID iid, LPVOID *iface) override \
        { return controllingUnknown()->QueryInterface(iid, iface); } \
    ULONG WINAPI AddRef() override { return controllingUnknown()->AddRef(); } \
    ULONG WINAPI Release() override { return controllingUnknown()->Release(); } \

QT_END_NAMESPACE

#endif // QAXAGGREGATED_H
