// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSG_RUNTIME_UTILITIES_H
#define QSSG_RUNTIME_UTILITIES_H

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

QT_BEGIN_NAMESPACE

class QQuick3DNode;
class QQuick3DObject;
class QObject;

namespace QSSGSceneDesc
{
struct Scene;
struct Node;
struct Property;
}

namespace QSSGRuntimeUtils
{
Q_QUICK3DASSETUTILS_EXPORT QQuick3DNode *createScene(QQuick3DNode &parent, const QSSGSceneDesc::Scene &scene);
Q_QUICK3DASSETUTILS_EXPORT void createGraphObject(QSSGSceneDesc::Node &node, QQuick3DObject &parent, bool traverseChildrenAndSetProperties = true);
Q_QUICK3DASSETUTILS_EXPORT void applyPropertyValue(const QSSGSceneDesc::Node *node, QObject *obj, QSSGSceneDesc::Property *property);
}

QT_END_NAMESPACE

#endif // QSSG_RUNTIME_UTILITIES_H
