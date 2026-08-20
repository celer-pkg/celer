// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DBAKEDLIGHTMAP_P_H
#define QQUICK3DBAKEDLIGHTMAP_P_H

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

#include <QtQuick3D/private/qquick3dnode_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3D_EXPORT QQuick3DBakedLightmap : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString key READ key WRITE setKey NOTIFY keyChanged)
    Q_PROPERTY(QString loadPrefix READ loadPrefix WRITE setLoadPrefix NOTIFY loadPrefixChanged)

    QML_NAMED_ELEMENT(BakedLightmap)

public:
    bool isEnabled() const;
    QString key() const;
    QString loadPrefix() const;

public Q_SLOTS:
    void setEnabled(bool enabled);
    void setKey(const QString &key);
    void setLoadPrefix(const QString &loadPrefix);

Q_SIGNALS:
    void changed();
    void enabledChanged();
    void keyChanged();
    void loadPrefixChanged();

private:
    bool m_enabled = true;
    QString m_key;
    QString m_loadPrefix;
};

QT_END_NAMESPACE

#endif // QQUICK3DBAKEDLIGHTMAP_P_H
