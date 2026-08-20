// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSCXMLDATAMODELPLUGIN_P_H
#define QSCXMLDATAMODELPLUGIN_P_H

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

#include <QtScxml/private/qscxmlglobals_p.h>
#include <QtScxml/qscxmldatamodel.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

#define QScxmlDataModelPluginInterface_iid "org.qt-project.qt.scxml.datamodel.plugin"

class Q_SCXML_EXPORT QScxmlDataModelPlugin : public QObject
{
    Q_OBJECT
public:
    virtual QScxmlDataModel *createScxmlDataModel() const;
};

Q_DECLARE_INTERFACE(QScxmlDataModelPlugin, QScxmlDataModelPluginInterface_iid)
QT_END_NAMESPACE

#endif // QSCXMLDATAMODELPLUGIN_P_H
