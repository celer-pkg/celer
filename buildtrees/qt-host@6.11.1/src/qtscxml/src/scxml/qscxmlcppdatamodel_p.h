// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSCXMLCPPDATAMODEL_P_H
#define QSCXMLCPPDATAMODEL_P_H

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

#include <private/qscxmldatamodel_p.h>
#include <QtScxml/qscxmlcppdatamodel.h>
#include <QtScxml/qscxmlevent.h>

QT_BEGIN_NAMESPACE

class Q_SCXML_EXPORT QScxmlCppDataModelPrivate : public QScxmlDataModelPrivate
{
public:
    QScxmlEvent event;
};

QT_END_NAMESPACE

#endif // QSCXMLCPPDATAMODEL_P_H
