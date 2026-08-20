// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef UNIFORMMODEL_H
#define UNIFORMMODEL_H

#include <QAbstractTableModel>
#include <QUrl>
#include <QVector2D>
#include <QList>
#include <QImage>
#include <QtQml/qqmlregistration.h>

class EffectManager;
class UniformModel : public QAbstractListModel
{
    Q_OBJECT
public:
    struct Uniform
    {
        enum class Type
        {
            Bool,
            Int,
            Float,
            Vec2,
            Vec3,
            Vec4,
            Color,
            Sampler,
            Define
        };

        Type type;
        QVariant value;
        QVariant defaultValue;
        QVariant minValue;
        QVariant maxValue;
        QByteArray name;
        QString description;
        QString customValue;
        bool useCustomValue = false;
        bool visible = true;
        bool exportProperty = true;
        bool canMoveUp = false;
        bool canMoveDown = false;
        bool enableMipmap = false;
        bool exportImage = true;
        // The effect node which owns this uniform
        int nodeId = -1;
        bool operator==(const Uniform& rhs) const noexcept
        {
           return this->name == rhs.name;
        }

    };
    using UniformTable = QList<Uniform>;

    enum UniformModelRoles {
        Type = Qt::UserRole + 1,
        NodeId,
        Name,
        Value,
        DefaultValue,
        Description,
        CustomValue,
        UseCustomValue,
        MinValue,
        MaxValue,
        Visible,
        ExportProperty,
        CanMoveUp,
        CanMoveDown,
        EnableMipmap,
        ExportImage
    };

    explicit UniformModel(QObject *parent = nullptr);

    void setModelData(UniformTable *data);
    int rowCount(const QModelIndex & = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role) const final;
    QHash<int, QByteArray> roleNames() const final;
    bool setData(const QModelIndex &index, const QVariant &value, int role) final;

    static QVariant getInitializedVariant(Uniform::Type type, bool maxValue = false);
    static QString getImageElementName(const Uniform &uniform);
    static QString typeToProperty(Uniform::Type type);
    QString typeToUniform(Uniform::Type type);
    QString valueAsString(const Uniform &uniform);
    QString valueAsVariable(const Uniform &uniform);
    QString valueAsBinding(const Uniform &uniform);
    QString variantAsDataString(const Uniform::Type type, const QVariant &variant);
    QVariant valueStringToVariant(const Uniform::Type type, const QString &value);

    Q_INVOKABLE bool updateRow(int nodeId, int rowIndex, int type, const QString &id,
                               const QVariant &defaultValue, const QString &description, const QString &customValue,
                               bool useCustomValue, const QVariant &minValue, const QVariant &maxValue,
                               bool enableMipmap, bool exportImage);
    Q_INVOKABLE void removeRow(int rowIndex, int rows = 1);
    Q_INVOKABLE bool resetValue(int rowIndex);
    Q_INVOKABLE bool setImage(int rowIndex, const QVariant &value);
    Q_INVOKABLE void moveIndex(int rowIndex, int direction);

    void appendUniform(Uniform uniform);

    void setUniformValueData(Uniform *uniform, const QString &value, const QString &defaultValue, const QString &minValue, const QString &maxValue);
    // These convert between uniform type & string name in JSON
    UniformModel::Uniform::Type typeFromString(const QString &typeString) const;
    QString stringFromType(Uniform::Type type);

Q_SIGNALS:
    void uniformsChanged();
    void qmlComponentChanged();
    void addFSCode(const QString &code);

private:
    friend class EffectManager;
    bool validateUniformName(const QString &uniformName);
    void updateCanMoveStatus();
    UniformTable *m_uniformTable = nullptr;
    EffectManager *m_effectManager = nullptr;
};

#endif // UNIFORMMODEL_H
