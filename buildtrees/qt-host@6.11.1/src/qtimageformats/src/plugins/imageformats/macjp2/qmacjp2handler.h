// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QMACJP2HANDLER_H
#define QMACJP2HANDLER_H

#include <QScopedPointer>
#include <QImageIOHandler>

QT_BEGIN_NAMESPACE

class QImage;
class QByteArray;
class QIODevice;
class QVariant;
class QMacJp2HandlerPrivate;

class QMacJp2Handler : public QImageIOHandler
{
public:
    QMacJp2Handler();
    ~QMacJp2Handler();

    bool canRead() const override;
    bool read(QImage *image) override;
    bool write(const QImage &image) override;
    QVariant option(ImageOption option) const override;
    void setOption(ImageOption option, const QVariant &value) override;
    bool supportsOption(ImageOption option) const override;

    static bool canRead(QIODevice *iod);

private:
    Q_DECLARE_PRIVATE(QMacJp2Handler)
    QScopedPointer<QMacJp2HandlerPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QMACJP2HANDLER_P_H
