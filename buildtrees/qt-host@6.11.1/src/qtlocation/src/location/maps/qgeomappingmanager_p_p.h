// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOMAPPINGMANAGER_P_H
#define QGEOMAPPINGMANAGER_P_H

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

class QGeoMappingManagerPrivate
{
public:
    QGeoMappingManagerPrivate();
    ~QGeoMappingManagerPrivate();

    QGeoMappingManagerEngine *engine = nullptr;

private:
    Q_DISABLE_COPY(QGeoMappingManagerPrivate)
};

QT_END_NAMESPACE

#endif
