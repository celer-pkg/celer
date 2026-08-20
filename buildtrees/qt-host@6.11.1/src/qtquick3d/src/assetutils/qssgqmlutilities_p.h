// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGQMLUTILITIES_P_H
#define QSSGQMLUTILITIES_P_H

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

#include <QString>
#include <QColor>
#include <QVariant>
#include <QHash>
#include <QTextStream>
#include <QtCore/QJsonObject>

QT_BEGIN_NAMESPACE

namespace QSSGSceneDesc {
struct Scene;
struct Node;
struct Animation;
}

namespace QSSGMesh {
class Mesh;
}

class QDir;

namespace QSSGQmlUtilities {

QString Q_QUICK3DASSETUTILS_EXPORT qmlComponentName(const QString &name);
QString Q_QUICK3DASSETUTILS_EXPORT colorToQml(const QColor &color);
QString Q_QUICK3DASSETUTILS_EXPORT variantToQml(const QVariant &variant);
QString Q_QUICK3DASSETUTILS_EXPORT sanitizeQmlId(const QString &id);
QString Q_QUICK3DASSETUTILS_EXPORT sanitizeQmlSourcePath(const QString &source, bool removeParentDirectory = false);
QString Q_QUICK3DASSETUTILS_EXPORT stripParentDirectory(const QString &filePath);

void Q_QUICK3DASSETUTILS_EXPORT writeQml(const QSSGSceneDesc::Scene &scene, QTextStream &stream, const QDir &outdir, const QJsonObject &optionsObject = QJsonObject());
void Q_QUICK3DASSETUTILS_EXPORT writeQmlComponent(const QSSGSceneDesc::Node &node, QTextStream &stream, const QDir &outDir);

Q_REQUIRED_RESULT QString Q_QUICK3DASSETUTILS_EXPORT getMeshSourceName(const QByteArrayView &name);

void Q_QUICK3DASSETUTILS_EXPORT createTimelineAnimation(const QSSGSceneDesc::Animation &anim, QObject *parent, bool isEnabled = true, bool useBinaryKeyframe = true);

}


QT_END_NAMESPACE

#endif // QSSGQMLUTILITIES_P_H
