// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "QtScxml/qscxmldatamodel.h"
#include "qscxmlecmascriptdatamodel_p.h"
#include "qscxmlecmascriptdatamodelplugin_p.h"

QT_BEGIN_NAMESPACE

QScxmlDataModel *QScxmlEcmaScriptDataModelPlugin::createScxmlDataModel() const
{
    return new QScxmlEcmaScriptDataModel;
}

QT_END_NAMESPACE
