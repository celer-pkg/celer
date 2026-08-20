// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


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

#ifndef INSTANCEREPEATER_P_H
#define INSTANCEREPEATER_P_H

#include <QtQuick3DHelpers/qtquick3dhelpersexports.h>
#include <QtQuick3D/qquick3dinstancing.h>
#include <QtQuick3D/private/qquick3drepeater_p.h>
#include <QAbstractListModel>

QT_BEGIN_NAMESPACE

class Q_QUICK3DHELPERS_EXPORT InstanceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DInstancing *instancingTable READ instancing WRITE setInstancing NOTIFY instancingChanged)
    QML_NAMED_ELEMENT(InstanceModel)
    QML_ADDED_IN_VERSION(6, 4)

public:
    explicit InstanceModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;

    QQuick3DInstancing *instancing() const { return m_instancing; }
    void setInstancing(QQuick3DInstancing *instancing);

    const QQuick3DInstancing::InstanceTableEntry *instanceData(int index) const;

    enum Roles {
        PositionRole, RotationRole, ScaleRole, ColorRole, CustomDataRole
    };

    QHash<int, QByteArray> roleNames() const override {
        return {
            { ColorRole, "modelColor" },
            { PositionRole, "modelPosition" },
            { RotationRole, "modelRotation"},
            { ScaleRole, "modelScale"},
            { CustomDataRole, "modelData"}
        };
    }

private slots:
    void reset();

signals:
    void instancingChanged();

private:
    void ensureTable() const;

    QQuick3DInstancing *m_instancing = nullptr;

    QByteArray m_instanceData;
    int m_count = 0;
    QMetaObject::Connection m_tableConnection;
};

class Q_QUICK3DHELPERS_EXPORT InstanceRepeater : public QQuick3DRepeater
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DInstancing *instancingTable READ instancing WRITE setInstancing NOTIFY instancingChanged)
    QML_NAMED_ELEMENT(InstanceRepeater)
    QML_ADDED_IN_VERSION(6, 4)

public:
    explicit InstanceRepeater(QQuick3DNode *parent = nullptr);
    QQuick3DInstancing *instancing() const;
    void setInstancing(QQuick3DInstancing *instancing);
signals:
    void instancingChanged();
protected:
    void initDelegate(int index, QQuick3DNode *node) override;
private:
    InstanceModel *m_model = nullptr;
};
QT_END_NAMESPACE
#endif // INSTANCEREPEATER_P_H
