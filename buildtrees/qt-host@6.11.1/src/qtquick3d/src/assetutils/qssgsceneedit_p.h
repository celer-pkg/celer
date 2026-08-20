// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGSCENEEDIT_P_H
#define QSSGSCENEEDIT_P_H

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

#include <QtQuick3DAssetUtils/private/qtquick3dassetutilsglobal_p.h>
#include <QtCore/QStringView>

QT_BEGIN_NAMESPACE

namespace QSSGSceneDesc {
    struct Scene;
    struct Node;
    struct Property;
}

class QJsonObject;
class QJsonValue;

namespace QSSGQmlUtilities {

void Q_QUICK3DASSETUTILS_EXPORT applyEdit(QSSGSceneDesc::Scene *scene, const QJsonObject &changes);
void Q_QUICK3DASSETUTILS_EXPORT setProperty(QSSGSceneDesc::Node *node, const QStringView propertyName, const QJsonValue &value);
QSSGSceneDesc::Node Q_QUICK3DASSETUTILS_EXPORT *addResource(QSSGSceneDesc::Scene *scene, const QJsonObject &addition);
}
QT_END_NAMESPACE

#endif // QSSGSCENEEDIT_P_H
