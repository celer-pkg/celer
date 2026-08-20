// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGIBLBAKERIMPORTER_P_H
#define QSSGIBLBAKERIMPORTER_P_H

#include "qtquick3diblbaker_p.h"
#include <QDir>

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

QT_BEGIN_NAMESPACE

class Q_QUICK3DIBLBAKER_EXPORT QSSGIblBaker
{
public:
    const QStringList inputExtensions() const;
    const QString outputExtension() const;
    const QString import(const QString &sourceFile, const QDir &savePath, QStringList *generatedFiles);

private:
    QByteArray createBsdfMipLevel(QByteArray &previousMipLevel, uint width, uint height);
    QStringList m_generatedFiles;
};

QT_END_NAMESPACE

#endif // QSSGIBLBAKERIMPORTER_P_H
