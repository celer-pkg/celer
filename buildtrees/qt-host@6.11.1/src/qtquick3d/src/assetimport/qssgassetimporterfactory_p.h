// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGASSETIMPORTERFACTORY_P_H
#define QSSGASSETIMPORTERFACTORY_P_H

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

#include <QtQuick3DAssetImport/private/qtquick3dassetimportglobal_p.h>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QSSGAssetImporter;

class Q_QUICK3DASSETIMPORT_EXPORT QSSGAssetImporterFactory
{
public:
    static QStringList keys();
    static QSSGAssetImporter *create(const QString &name, const QStringList &args);
};

QT_END_NAMESPACE

#endif // QSSGASSETIMPORTERFACTORY_P_H
