// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef UNIFORMMODEL_H
#define UNIFORMMODEL_H

#include <QAbstractTableModel>
#include <QtQml/qqmlregistration.h>
#include "custommaterial.h"

QT_BEGIN_NAMESPACE

class UniformModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
public:
    using UniformTable = CustomMaterial::UniformTable;
    // This enum is used in QML to get the type of the uniform
    // but should map to CustomMaterial::Uniform::Type
    enum UniformType {
        Bool,
        Int,
        Float,
        Vec2,
        Vec3,
        Vec4,
        Mat44,
        Sampler,
    };
    Q_ENUM(UniformType)

    enum UniformModelRoles {
        Type = Qt::UserRole + 1,
        Name,
        Value
    };

    explicit UniformModel(QObject *parent = nullptr);

    void setModelData(UniformTable *data);
    int rowCount(const QModelIndex & = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role) const final;
    QHash<int, QByteArray> roleNames() const final;
    bool setData(const QModelIndex &index, const QVariant &value, int role) final;

    Q_INVOKABLE bool insertRow(int rowIndex, int type, const QString &id);
    Q_INVOKABLE void removeRow(int rowIndex, int rows = 1);

private:
    bool validateUniformName(const QString &uniformName);
    UniformTable *m_uniformTable = nullptr;
};

QT_END_NAMESPACE

#endif // UNIFORMMODEL_H
