// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#ifndef QAXBINDABLE_H
#define QAXBINDABLE_H

#include <QtWidgets/qwidget.h>

struct IUnknown;

QT_BEGIN_NAMESPACE

class QAxAggregated;
class QIODevice;
struct IAxServerBase;

class QAxBindable
{
    Q_DISABLE_COPY_MOVE(QAxBindable)
    friend class QAxServerBase;
public:
    QAxBindable();
    virtual ~QAxBindable();

    virtual QAxAggregated *createAggregate();
    void reportError(int code, const QString &src, const QString &desc, const QString &help = QString());

    virtual bool readData(QIODevice *source, const QString &format);
    virtual bool writeData(QIODevice *sink);

protected:
    bool requestPropertyChange(const char *property);
    void propertyChanged(const char *property);

    IUnknown *clientSite() const;

private:
    IAxServerBase *activex = nullptr;
};

QT_END_NAMESPACE

#endif // QAXBINDABLE_H
