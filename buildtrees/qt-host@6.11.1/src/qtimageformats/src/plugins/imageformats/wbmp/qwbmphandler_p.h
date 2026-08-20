// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QWBMPHANDLER_P_H
#define QWBMPHANDLER_P_H

#include <qimageiohandler.h>

QT_BEGIN_NAMESPACE

class WBMPReader;

class QWbmpHandler : public QImageIOHandler
{
public:
    QWbmpHandler(QIODevice *device);
    ~QWbmpHandler();

    bool canRead() const override;
    bool read(QImage *image) override;
    bool write(const QImage &image) override;

    QVariant option(ImageOption option) const override;
    bool supportsOption(ImageOption option) const override;

    static bool canRead(QIODevice *device);

private:
    WBMPReader *m_reader;
};

QT_END_NAMESPACE

#endif /* QWBMPHANDLER_P_H */
