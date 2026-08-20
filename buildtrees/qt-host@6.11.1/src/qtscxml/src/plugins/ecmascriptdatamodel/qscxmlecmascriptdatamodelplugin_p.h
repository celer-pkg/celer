// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QSCXMLECMASCRIPTDATAMODELPLUGIN_P_H
#define QSCXMLECMASCRIPTDATAMODELPLUGIN_P_H

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

#include <QtScxml/private/qscxmldatamodelplugin_p.h>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QScxmlEcmaScriptDataModelPlugin : public QScxmlDataModelPlugin
{
    Q_OBJECT
    Q_INTERFACES(QScxmlDataModelPlugin)
    Q_PLUGIN_METADATA(IID QScxmlDataModelPluginInterface_iid FILE "ecmascriptdatamodelplugin.json")

public:
    QScxmlDataModel *createScxmlDataModel() const override;
};

QT_END_NAMESPACE

#endif // QSCXMLECMASCRIPTDATAMODELPLUGIN_P_H
