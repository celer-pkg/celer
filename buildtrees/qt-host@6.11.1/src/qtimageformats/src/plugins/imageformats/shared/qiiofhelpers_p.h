// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QIIOFHELPERS_P_H
#define QIIOFHELPERS_P_H

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

#include <QImageIOPlugin>
#include <private/qcore_mac_p.h>
#include <ImageIO/ImageIO.h>
#include <QList>

QT_BEGIN_NAMESPACE

namespace NS_IIOF_HELPERS {

/*
Functions to utilize the native ImageIO Framework in OS X and iOS
*/

class QIIOFHelpers
{
public:
    static QImageIOPlugin::Capabilities systemCapabilities(const QString &uti);
    static bool readImage(QImageIOHandler *q_ptr, QImage *out);
    static bool writeImage(QImageIOHandler *q_ptr, const QImage &in, const QString &uti);
};

class QIIOFHelper
{
public:
    QIIOFHelper(QImageIOHandler *q);

    bool readImage(QImage *out);
    bool writeImage(const QImage &in, const QString &uti);
    QVariant imageProperty(QImageIOHandler::ImageOption option);
    void setOption(QImageIOHandler::ImageOption option, const QVariant &value);

protected:
    bool initRead();
    bool getIntProperty(CFStringRef property, int *value);

    QImageIOHandler *q_ptr = nullptr;
    QList<QVariant> writeOptions;
    QCFType<CGDataProviderRef> cgDataProvider = nullptr;
    QCFType<CGImageSourceRef> cgImageSource = nullptr;
    QCFType<CFDictionaryRef> cfImageDict = nullptr;
};

}

QT_END_NAMESPACE

#endif
