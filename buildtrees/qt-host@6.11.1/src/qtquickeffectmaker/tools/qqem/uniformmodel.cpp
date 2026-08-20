// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "uniformmodel.h"
#include "propertyhandler.h"
#include "effectmanager.h"

UniformModel::UniformModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

void UniformModel::setModelData(UniformTable *data)
{
    beginResetModel();
    m_uniformTable = data;
    endResetModel();
    updateCanMoveStatus();
    Q_EMIT uniformsChanged();
}

int UniformModel::rowCount(const QModelIndex &) const
{
    return int((m_uniformTable != nullptr) ? m_uniformTable->size() : 0);
}

QVariant UniformModel::data(const QModelIndex &index, int role) const
{
    if (!m_uniformTable || !index.isValid())
        return QVariant();

    if (index.row() >= m_uniformTable->size())
        return false;

    const auto &uniform = (*m_uniformTable)[index.row()];

    if (role == Type) {
        return QVariant::fromValue(uniform.type);
    } else if (role == NodeId) {
        return QVariant::fromValue(uniform.nodeId);
    } else if (role == Name) {
        return QVariant::fromValue(QString::fromLatin1(uniform.name));
    } else if (role == Value) {
        switch (uniform.type) {
        case Uniform::Type::Sampler:
            return uniform.value.value<QString>();
        case Uniform::Type::Bool:
            return uniform.value.value<bool>();
        case Uniform::Type::Int:
            return uniform.value.value<int>();
        case Uniform::Type::Float:
            return uniform.value.value<double>();
        case Uniform::Type::Vec2:
            return uniform.value.value<QVector2D>();
        case Uniform::Type::Vec3:
            return uniform.value.value<QVector3D>();
        case Uniform::Type::Vec4:
            return uniform.value.value<QVector4D>();
        case Uniform::Type::Color:
            return uniform.value.value<QColor>();
        case Uniform::Type::Define:
            return uniform.value.value<QString>();
        }
    } else if (role == DefaultValue) {
        switch (uniform.type) {
        case Uniform::Type::Sampler:
            return uniform.defaultValue.value<QString>();
        case Uniform::Type::Bool:
            return uniform.defaultValue.value<bool>();
        case Uniform::Type::Int:
            return uniform.defaultValue.value<int>();
        case Uniform::Type::Float:
            return uniform.defaultValue.value<double>();
        case Uniform::Type::Vec2:
            return uniform.defaultValue.value<QVector2D>();
        case Uniform::Type::Vec3:
            return uniform.defaultValue.value<QVector3D>();
        case Uniform::Type::Vec4:
            return uniform.defaultValue.value<QVector4D>();
        case Uniform::Type::Color:
            return uniform.defaultValue.value<QColor>();
        case Uniform::Type::Define:
            return uniform.defaultValue.value<QString>();
        }
    } else if (role == MinValue) {
        switch (uniform.type) {
        case Uniform::Type::Sampler:
            return uniform.minValue.value<QString>();
        case Uniform::Type::Bool:
            return uniform.minValue.value<bool>();
        case Uniform::Type::Int:
            return uniform.minValue.value<int>();
        case Uniform::Type::Float:
            return uniform.minValue.value<double>();
        case Uniform::Type::Vec2:
            return uniform.minValue.value<QVector2D>();
        case Uniform::Type::Vec3:
            return uniform.minValue.value<QVector3D>();
        case Uniform::Type::Vec4:
            return uniform.minValue.value<QVector4D>();
        case Uniform::Type::Color:
            return uniform.minValue.value<QColor>();
        case Uniform::Type::Define:
            return uniform.minValue.value<QString>();
        }
    } else if (role == MaxValue) {
        switch (uniform.type) {
        case Uniform::Type::Sampler:
            return uniform.maxValue.value<QString>();
        case Uniform::Type::Bool:
            return uniform.maxValue.value<bool>();
        case Uniform::Type::Int:
            return uniform.maxValue.value<int>();
        case Uniform::Type::Float:
            return uniform.maxValue.value<double>();
        case Uniform::Type::Vec2:
            return uniform.maxValue.value<QVector2D>();
        case Uniform::Type::Vec3:
            return uniform.maxValue.value<QVector3D>();
        case Uniform::Type::Vec4:
            return uniform.maxValue.value<QVector4D>();
        case Uniform::Type::Color:
            return uniform.maxValue.value<QColor>();
        case Uniform::Type::Define:
            return uniform.maxValue.value<QString>();
        }
    } else if (role == Description) {
        return QVariant::fromValue(uniform.description);
    } else if (role == CustomValue) {
        return QVariant::fromValue(uniform.customValue);
    } else if (role == UseCustomValue) {
        return QVariant::fromValue(uniform.useCustomValue);
    } else if (role == Visible) {
        return QVariant::fromValue(uniform.visible);
    } else if (role == ExportProperty) {
        return QVariant::fromValue(uniform.exportProperty);
    } else if (role == CanMoveUp) {
        return QVariant::fromValue(uniform.canMoveUp);
    } else if (role == CanMoveDown) {
        return QVariant::fromValue(uniform.canMoveDown);
    } else if (role == EnableMipmap) {
        return QVariant::fromValue(uniform.enableMipmap);
    } else if (role == ExportImage) {
        return QVariant::fromValue(uniform.exportImage);
    }

    return QVariant();
}

QHash<int, QByteArray> UniformModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Type] = "type";
    roles[Name] = "name";
    roles[Value] = "value";
    roles[DefaultValue] = "defaultValue";
    roles[MinValue] = "minValue";
    roles[MaxValue] = "maxValue";
    roles[NodeId] = "nodeId";
    roles[Description] = "description";
    roles[CustomValue] = "customValue";
    roles[UseCustomValue] = "useCustomValue";
    roles[Visible] = "visible";
    roles[ExportProperty] = "exportProperty";
    roles[CanMoveUp] = "canMoveUp";
    roles[CanMoveDown] = "canMoveDown";
    roles[EnableMipmap] = "enableMipmap";
    roles[ExportImage] = "exportImage";
    return roles;
}

bool UniformModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || !m_uniformTable)
        return false;

    if (index.row() >= m_uniformTable->size())
        return false;

    auto &uniform = (*m_uniformTable)[index.row()];

    bool ok = true;

    if (role == Type) {
        const auto v = value.toInt(&ok);
        if (ok)
            uniform.type = static_cast<Uniform::Type>(v);
    } else if (role == Name) {
        uniform.name = value.toString().toUtf8();
    } else if (role == Value) {
        switch (uniform.type) {
        case Uniform::Type::Bool:
        case Uniform::Type::Int:
        case Uniform::Type::Float:
        case Uniform::Type::Vec2:
        case Uniform::Type::Vec3:
        case Uniform::Type::Vec4:
        case Uniform::Type::Color:
        case Uniform::Type::Sampler:
        case Uniform::Type::Define:
            uniform.value = value;
            break;
        }
    } else if (role == DefaultValue) {
        switch (uniform.type) {
        case Uniform::Type::Bool:
        case Uniform::Type::Int:
        case Uniform::Type::Float:
        case Uniform::Type::Vec2:
        case Uniform::Type::Vec3:
        case Uniform::Type::Vec4:
        case Uniform::Type::Color:
        case Uniform::Type::Sampler:
        case Uniform::Type::Define:
            uniform.defaultValue = value;
            break;
        }
    } else if (role == MinValue) {
        switch (uniform.type) {
        case Uniform::Type::Bool:
        case Uniform::Type::Int:
        case Uniform::Type::Float:
        case Uniform::Type::Vec2:
        case Uniform::Type::Vec3:
        case Uniform::Type::Vec4:
        case Uniform::Type::Color:
        case Uniform::Type::Sampler:
        case Uniform::Type::Define:
            uniform.minValue = value;
            break;
        }
    } else if (role == MaxValue) {
        switch (uniform.type) {
        case Uniform::Type::Bool:
        case Uniform::Type::Int:
        case Uniform::Type::Float:
        case Uniform::Type::Vec2:
        case Uniform::Type::Vec3:
        case Uniform::Type::Vec4:
        case Uniform::Type::Color:
        case Uniform::Type::Sampler:
        case Uniform::Type::Define:
            uniform.maxValue = value;
            break;
        }
    } else if (role == Description) {
        uniform.description = value.toString();
    } else if (role == CustomValue) {
        uniform.customValue = value.toString();
    } else if (role == UseCustomValue) {
        uniform.useCustomValue = value.toBool();
    } else if (role == ExportProperty) {
        uniform.exportProperty = value.toBool();
    } else if (role == CanMoveUp) {
        uniform.canMoveUp = value.toBool();
    } else if (role == CanMoveDown) {
        uniform.canMoveDown = value.toBool();
    } else if (role == EnableMipmap) {
        uniform.enableMipmap = value.toBool();
    } else if (role == ExportImage) {
        uniform.exportImage = value.toBool();
    }

    if (role == Value) {
        // Don't update uniforms when value is changed, as those
        // go through g_propertyData
        g_propertyData.insert(uniform.name, value);
        if (m_effectManager)
            m_effectManager->setUnsavedChanges(true);
    }
    if (role != Value || !uniform.exportProperty) {
        // But non-exported (const value) properties need update
        Q_EMIT uniformsChanged();
    }

    if (ok)
        emit dataChanged(index, index, {role});

    return ok;
}

// Initialize the value variant with correct type
QVariant UniformModel::getInitializedVariant(Uniform::Type type, bool maxValue)
{
    switch (type) {
    case Uniform::Type::Sampler:
        return QString();
    case Uniform::Type::Bool:
        return maxValue ? true : false;
    case Uniform::Type::Int:
        return maxValue ? 100 : 0;
    case Uniform::Type::Float:
        return maxValue ? 1.0 : 0.0;
    case Uniform::Type::Vec2:
        return maxValue ? QVector2D(1.0, 1.0) : QVector2D(0.0, 0.0);
    case Uniform::Type::Vec3:
        return maxValue ? QVector3D(1.0, 1.0, 1.0) : QVector3D(0.0, 0.0, 0.0);
    case Uniform::Type::Vec4:
        return maxValue ? QVector4D(1.0, 1.0, 1.0, 1.0) : QVector4D(0.0, 0.0, 0.0, 0.0);
    case Uniform::Type::Color:
        return maxValue ? QColor::fromRgbF(1.0f, 1.0f, 1.0f, 1.0f) : QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f);
    case Uniform::Type::Define:
        return QString();
    }
    return QVariant();
}

// Return name for the image property Image element
QString UniformModel::getImageElementName(const Uniform &uniform)
{
    if (uniform.value.toString().isEmpty())
        return QStringLiteral("null");
    QString simplifiedName = uniform.name.simplified();
    simplifiedName = simplifiedName.remove(' ');
    return QStringLiteral("imageItem") + simplifiedName;
}

// This will update the row content at rowIndex
// When rowIndex = -1, new row will be appended
bool UniformModel::updateRow(int nodeId, int rowIndex, int type, const QString &id,
                             const QVariant &defaultValue, const QString &description, const QString &customValue,
                             bool useCustomValue, const QVariant &minValue, const QVariant &maxValue,
                             bool enableMipmap, bool exportImage)
{
    if (m_uniformTable == nullptr)
        return false;

    bool addNewRow = false;
    if (rowIndex == -1) {
        addNewRow = true;
        // Add new row after the last of its node so uniforms are
        // in continuous groups per node.
        rowIndex = m_uniformTable->size();
        for (int i = m_uniformTable->size() - 1; i >= 0; i--) {
            auto &uniform = (*m_uniformTable)[i];
            if (uniform.nodeId == nodeId) {
                rowIndex = i + 1;
                break;
            }
        }
    }

    if (addNewRow && !validateUniformName(id))
        return false;

    if (addNewRow)
        beginInsertRows(QModelIndex(), rowIndex, rowIndex);

    Uniform newUniform = { };
    auto &uniform = addNewRow ? newUniform : (*m_uniformTable)[rowIndex];

    uniform.type = Uniform::Type(type);
    uniform.name = id.toLocal8Bit();
    uniform.description = description;
    uniform.customValue = customValue;
    uniform.useCustomValue = useCustomValue;
    uniform.enableMipmap = enableMipmap;
    uniform.exportImage = exportImage;
    switch (uniform.type) {
    case Uniform::Type::Bool:
        uniform.defaultValue = defaultValue;
        break;
    case Uniform::Type::Int:
    case Uniform::Type::Float:
    case Uniform::Type::Vec2:
    case Uniform::Type::Vec3:
    case Uniform::Type::Vec4:
    case Uniform::Type::Color:
    case Uniform::Type::Define:
        uniform.defaultValue = defaultValue;
        uniform.minValue = minValue;
        uniform.maxValue = maxValue;
        break;
    case Uniform::Type::Sampler:
        if (defaultValue.metaType().id() == QMetaType::QUrl)
            uniform.defaultValue = defaultValue.toString();
        else
            uniform.defaultValue = "";
        // Update the mipmap property
        QString mipmapProperty = m_effectManager->mipmapPropertyName(uniform.name);
        g_propertyData[mipmapProperty] = enableMipmap;
        break;
    }

    if (addNewRow) {
        // When adding a property, value is default value
        uniform.value = uniform.defaultValue;
        g_propertyData.insert(uniform.name, uniform.value);
        // Note: NodeId is only updated when adding new property
        // Properties can't be moved to other nodes
        uniform.nodeId = nodeId;
        m_uniformTable->insert(rowIndex, uniform);
        endInsertRows();
        updateCanMoveStatus();
        if (uniform.type == Uniform::Type::Sampler) {
            QString fs = "\n";
            fs += "// *** Autogenerated - Remove if not needed ***\n";
            fs += "// vec4 " + uniform.name + "Tex = texture(" + uniform.name + ", texCoord);\n";
            Q_EMIT addFSCode(fs);
        }
    } else {
        if (!uniform.useCustomValue)
            g_propertyData.insert(uniform.name, uniform.value);
        Q_EMIT dataChanged(QAbstractItemModel::createIndex(rowIndex, 0),
                           QAbstractItemModel::createIndex(rowIndex, 0));
    }
    // Update QML component as useCustomValue, customValue etc. might have changed
    Q_EMIT qmlComponentChanged();
    Q_EMIT uniformsChanged();
    return true;
}

bool UniformModel::resetValue(int rowIndex)
{
    auto &uniform = (*m_uniformTable)[rowIndex];
    auto index = QAbstractItemModel::createIndex(rowIndex, 0);
    setData(index, uniform.defaultValue, Value);

    g_propertyData.insert(uniform.name, uniform.defaultValue);

    emit dataChanged(index, index, {Value});

    // Image element may change, so update component
    if (uniform.type == Uniform::Type::Sampler)
        Q_EMIT qmlComponentChanged();
    Q_EMIT uniformsChanged();

    return true;
}

// Updates the image data at rowIndex
bool UniformModel::setImage(int rowIndex, const QVariant &value)
{
    auto &uniform = (*m_uniformTable)[rowIndex];
    uniform.value = value.toString();

    g_propertyData.insert(uniform.name, value);

    emit dataChanged(QAbstractItemModel::createIndex(0, 0),
                     QAbstractItemModel::createIndex(rowIndex, 0));

    // Image element may change, so update component
    if (uniform.type == Uniform::Type::Sampler)
        Q_EMIT qmlComponentChanged();
    Q_EMIT uniformsChanged();
    return true;
}

// Move uniform up/down in the list
void UniformModel::moveIndex(int rowIndex, int direction)
{
    if (m_uniformTable == nullptr)
        return;

    // Note: beginMoveRows index behaves differently than QList move index.
    int destinationRowIndex = rowIndex + 2;
    int destinationListIndex = rowIndex + 1;
    if (direction == -1) {
        destinationRowIndex = rowIndex - 1;
        destinationListIndex = rowIndex - 1;
    }

    if (destinationListIndex >= 0 && m_uniformTable->size() > destinationListIndex) {
        beginMoveRows(QModelIndex(), rowIndex, rowIndex, QModelIndex(), destinationRowIndex);
        m_uniformTable->move(rowIndex, destinationListIndex);
        endMoveRows();
        updateCanMoveStatus();
    }
}

void UniformModel::removeRow(int rowIndex, int rows)
{
    if (m_uniformTable == nullptr)
        return;

    if (m_uniformTable->size() > rowIndex) {
        rows = qBound(1, rows, m_uniformTable->size());
        beginRemoveRows(QModelIndex(), rowIndex, rowIndex + rows - 1);
        m_uniformTable->remove(rowIndex, rows);
        endRemoveRows();
        updateCanMoveStatus();

        emit dataChanged(QAbstractItemModel::createIndex(0, 0),
                         QAbstractItemModel::createIndex(rowIndex, 0));
        Q_EMIT uniformsChanged();
    }
}

bool UniformModel::validateUniformName(const QString &uniformName)
{
    if (!m_uniformTable)
        return false;

    // must be unique
    for (const auto &uniform : *m_uniformTable) {
        if (uniform.name == uniformName)
            return false;
    }

    return true;
}

QString UniformModel::typeToProperty(Uniform::Type type)
{
    QString property;
    switch (type) {
    case Uniform::Type::Bool:
        property = "bool";
        break;
    case Uniform::Type::Int:
        property = "int";
        break;
    case Uniform::Type::Float:
        property = "real";
        break;
    case Uniform::Type::Vec2:
        property = "point";
        break;
    case Uniform::Type::Vec3:
        property = "vector3d";
        break;
    case Uniform::Type::Vec4:
        property = "vector4d";
        break;
    case Uniform::Type::Color:
        property = "color";
        break;
    case Uniform::Type::Sampler:
        property = "var";
        break;
    case Uniform::Type::Define:
        property = "var";
        break;
    }
    return property;
}

QString UniformModel::typeToUniform(Uniform::Type type)
{
    QString property;
    switch (type) {
    case Uniform::Type::Bool:
        property = "bool";
        break;
    case Uniform::Type::Int:
        property = "int";
        break;
    case Uniform::Type::Float:
        property = "float";
        break;
    case Uniform::Type::Vec2:
        property = "vec2";
        break;
    case Uniform::Type::Vec3:
        property = "vec3";
        break;
    case Uniform::Type::Vec4:
    case Uniform::Type::Color:
        property = "vec4";
        break;
    case Uniform::Type::Sampler:
        property = "sampler2D";
        break;
    case Uniform::Type::Define:
        property = "define";
        break;
    }
    return property;
}

// Get value in QML format that used for exports
QString UniformModel::valueAsString(const Uniform &uniform)
{
    QString s;
    switch (uniform.type) {
    case Uniform::Type::Bool:
        s = uniform.value.toBool() ? "true" : "false";
        break;
    case Uniform::Type::Int:
        s = QString::number(uniform.value.toInt());
        break;
    case Uniform::Type::Float:
        s = QString::number(uniform.value.toDouble());
        break;
    case Uniform::Type::Vec2:
    {
        QVector2D v2 = uniform.value.value<QVector2D>();
        s = QString("Qt.point(%1, %2)").arg(v2.x()).arg(v2.y());
        break;
    }
    case Uniform::Type::Vec3:
    {
        QVector3D v3 = uniform.value.value<QVector3D>();
        s = QString("Qt.vector3d(%1, %2, %3)").arg(v3.x()).arg(v3.y()).arg(v3.z());
        break;
    }
    case Uniform::Type::Vec4:
    {
        QVector4D v4 = uniform.value.value<QVector4D>();
        s = QString("Qt.vector4d(%1, %2, %3, %4)").arg(v4.x()).arg(v4.y()).arg(v4.z()).arg(v4.w());
        break;
    }
    case Uniform::Type::Color:
    {
        QColor c = uniform.value.value<QColor>();
        s = QString("Qt.rgba(%1, %2, %3, %4)").arg(c.redF()).arg(c.greenF()).arg(c.blueF()).arg(c.alphaF());
        break;
    }
    case Uniform::Type::Sampler:
    {
        s = getImageElementName(uniform);
        break;
    }
    case Uniform::Type::Define:
    {
        s = uniform.value.toString();
        break;
    }
    }
    return s;
}

// Get value in GLSL format that is used for non-exported const properties
QString UniformModel::valueAsVariable(const Uniform &uniform)
{
    QString s;
    switch (uniform.type) {
    case Uniform::Type::Bool:
        s = uniform.value.toBool() ? "true" : "false";
        break;
    case Uniform::Type::Int:
        s = QString::number(uniform.value.toInt());
        break;
    case Uniform::Type::Float:
        s = QString::number(uniform.value.toDouble());
        break;
    case Uniform::Type::Vec2:
    {
        QVector2D v2 = uniform.value.value<QVector2D>();
        s = QString("vec2(%1, %2)").arg(v2.x()).arg(v2.y());
        break;
    }
    case Uniform::Type::Vec3:
    {
        QVector3D v3 = uniform.value.value<QVector3D>();
        s = QString("vec3(%1, %2, %3)").arg(v3.x()).arg(v3.y()).arg(v3.z());
        break;
    }
    case Uniform::Type::Vec4:
    {
        QVector4D v4 = uniform.value.value<QVector4D>();
        s = QString("vec4(%1, %2, %3, %4)").arg(v4.x()).arg(v4.y()).arg(v4.z()).arg(v4.w());
        break;
    }
    case Uniform::Type::Color:
    {
        QColor c = uniform.value.value<QColor>();
        s = QString("vec4(%1, %2, %3, %4)").arg(c.redF()).arg(c.greenF()).arg(c.blueF()).arg(c.alphaF());
        break;
    }
    default:
        qWarning() << QString("Unhandled const variable type: %1").arg(int(uniform.type)).toLatin1();
        break;
    }
    return s;
}

// Get value in QML binding that used for previews
QString UniformModel::valueAsBinding(const Uniform &uniform)
{
    QString s;
    switch (uniform.type) {
    case Uniform::Type::Bool:
    case Uniform::Type::Int:
    case Uniform::Type::Float:
    case Uniform::Type::Define:
        s = "g_propertyData." + uniform.name;
        break;
    case Uniform::Type::Vec2:
    {
        QString sx = QString("g_propertyData.%1.x").arg(uniform.name);
        QString sy = QString("g_propertyData.%1.y").arg(uniform.name);
        s = QString("Qt.point(%1, %2)").arg(sx, sy);
        break;
    }
    case Uniform::Type::Vec3:
    {
        QString sx = QString("g_propertyData.%1.x").arg(uniform.name);
        QString sy = QString("g_propertyData.%1.y").arg(uniform.name);
        QString sz = QString("g_propertyData.%1.z").arg(uniform.name);
        s = QString("Qt.vector3d(%1, %2, %3)").arg(sx, sy, sz);
        break;
    }
    case Uniform::Type::Vec4:
    {
        QString sx = QString("g_propertyData.%1.x").arg(uniform.name);
        QString sy = QString("g_propertyData.%1.y").arg(uniform.name);
        QString sz = QString("g_propertyData.%1.z").arg(uniform.name);
        QString sw = QString("g_propertyData.%1.w").arg(uniform.name);
        s = QString("Qt.vector4d(%1, %2, %3, %4)").arg(sx, sy, sz, sw);
        break;
    }
    case Uniform::Type::Color:
    {
        QString sr = QString("g_propertyData.%1.r").arg(uniform.name);
        QString sg = QString("g_propertyData.%1.g").arg(uniform.name);
        QString sb = QString("g_propertyData.%1.b").arg(uniform.name);
        QString sa = QString("g_propertyData.%1.a").arg(uniform.name);
        s = QString("Qt.rgba(%1, %2, %3, %4)").arg(sr, sg, sb, sa);
        break;
    }
    case Uniform::Type::Sampler:
    {
        s = getImageElementName(uniform);
        break;
    }
    }
    return s;
}

QString UniformModel::variantAsDataString(const Uniform::Type type, const QVariant &variant)
{
    QString s;
    switch (type) {
    case Uniform::Type::Bool:
        s = variant.toBool() ? "true" : "false";
        break;
    case Uniform::Type::Int:
        s = QString::number(variant.toInt());
        break;
    case Uniform::Type::Float:
        s = QString::number(variant.toDouble());
        break;
    case Uniform::Type::Vec2:
    {
        QStringList list;
        QVector2D v2 = variant.value<QVector2D>();
        list << QString::number(v2.x());
        list << QString::number(v2.y());
        s = list.join(", ");
        break;
    }
    case Uniform::Type::Vec3:
    {
        QStringList list;
        QVector3D v3 = variant.value<QVector3D>();
        list << QString::number(v3.x());
        list << QString::number(v3.y());
        list << QString::number(v3.z());
        s = list.join(", ");
        break;
    }
    case Uniform::Type::Vec4:
    {
        QStringList list;
        QVector4D v4 = variant.value<QVector4D>();
        list << QString::number(v4.x());
        list << QString::number(v4.y());
        list << QString::number(v4.z());
        list << QString::number(v4.w());
        s = list.join(", ");
        break;
    }
    case Uniform::Type::Color:
    {
        QStringList list;
        QColor c = variant.value<QColor>();
        list << QString::number(c.redF(), 'g', 3);
        list << QString::number(c.greenF(), 'g', 3);
        list << QString::number(c.blueF(), 'g', 3);
        list << QString::number(c.alphaF(), 'g', 3);
        s = list.join(", ");
        break;
    }
    case Uniform::Type::Sampler:
    case Uniform::Type::Define:
    {
        s = variant.toString();
        break;
    }
    }
    return s;
}

void UniformModel::appendUniform(Uniform uniform)
{
    if (!validateUniformName(uniform.name)) {
        qWarning() << "Invalid uniform name, can't add it";
        return;
    }

    int rowIndex = m_uniformTable->size();
    beginInsertRows(QModelIndex(), rowIndex, rowIndex);
    m_uniformTable->insert(rowIndex, uniform);
    endInsertRows();
    updateCanMoveStatus();

    emit dataChanged(QAbstractItemModel::createIndex(0, 0),
                     QAbstractItemModel::createIndex(rowIndex, 0));

    Q_EMIT uniformsChanged();

}

void UniformModel::setUniformValueData(Uniform *uniform, const QString &value, const QString &defaultValue, const QString &minValue, const QString &maxValue)
{
    if (!uniform)
        return;

    uniform->value = value.isEmpty() ? getInitializedVariant(uniform->type, false) : valueStringToVariant(uniform->type, value);
    uniform->defaultValue = defaultValue.isEmpty() ? getInitializedVariant(uniform->type, false) : valueStringToVariant(uniform->type, defaultValue);
    uniform->minValue = minValue.isEmpty() ? getInitializedVariant(uniform->type, false) : valueStringToVariant(uniform->type, minValue);
    uniform->maxValue = maxValue.isEmpty() ? getInitializedVariant(uniform->type, true) : valueStringToVariant(uniform->type, maxValue);

    // Update the value data in bindings
    g_propertyData.insert(uniform->name, uniform->value);
}

UniformModel::Uniform::Type UniformModel::typeFromString(const QString &typeString) const
{
    if (typeString == "bool")
        return UniformModel::Uniform::Type::Bool;
    else if (typeString == "int")
        return UniformModel::Uniform::Type::Int;
    else if (typeString == "float")
        return UniformModel::Uniform::Type::Float;
    else if (typeString == "vec2")
        return UniformModel::Uniform::Type::Vec2;
    else if (typeString == "vec3")
        return UniformModel::Uniform::Type::Vec3;
    else if (typeString == "vec4")
        return UniformModel::Uniform::Type::Vec4;
    else if (typeString == "color")
        return UniformModel::Uniform::Type::Color;
    else if (typeString == "image")
        return UniformModel::Uniform::Type::Sampler;
    else if (typeString == "define")
        return UniformModel::Uniform::Type::Define;

    qWarning() << QString("Unknown type: %1").arg(typeString).toLatin1();
    return UniformModel::Uniform::Type::Float;
}

QString UniformModel::stringFromType(Uniform::Type type)
{
    QString property;
    switch (type) {
    case Uniform::Type::Bool:
        property = "bool";
        break;
    case Uniform::Type::Int:
        property = "int";
        break;
    case Uniform::Type::Float:
        property = "float";
        break;
    case Uniform::Type::Vec2:
        property = "vec2";
        break;
    case Uniform::Type::Vec3:
        property = "vec3";
        break;
    case Uniform::Type::Vec4:
        property = "vec4";
        break;
    case Uniform::Type::Color:
        property = "color";
        break;
    case Uniform::Type::Sampler:
        property = "image";
        break;
    case Uniform::Type::Define:
        property = "define";
        break;
    }
    return property;
}

QVariant UniformModel::valueStringToVariant(const Uniform::Type type, const QString &value)
{
    QVariant variant;
    switch (type) {
    case UniformModel::Uniform::Type::Bool:
        variant = (value == "true") ? true : false;
        break;
    case UniformModel::Uniform::Type::Int:
    case UniformModel::Uniform::Type::Float:
        variant = value;
        break;
    case UniformModel::Uniform::Type::Vec2:
    {
        QStringList list = value.split(QLatin1Char(','));
        if (list.size() >= 2)
            variant = QVector2D(list.at(0).toDouble(), list.at(1).toDouble());
    }
        break;
    case UniformModel::Uniform::Type::Vec3:
    {
        QStringList list = value.split(QLatin1Char(','));
        if (list.size() >= 3)
            variant = QVector3D(list.at(0).toDouble(), list.at(1).toDouble(), list.at(2).toDouble());
    }
        break;
    case UniformModel::Uniform::Type::Vec4:
    {
        QStringList list = value.split(QLatin1Char(','));
        if (list.size() >= 4)
            variant = QVector4D(list.at(0).toDouble(), list.at(1).toDouble(), list.at(2).toDouble(), list.at(3).toDouble());
    }
        break;
    case UniformModel::Uniform::Type::Color:
    {
        QStringList list = value.split(QLatin1Char(','));
        if (list.size() >= 4)
            variant = QColor::fromRgbF(list.at(0).toDouble(), list.at(1).toDouble(), list.at(2).toDouble(), list.at(3).toDouble());
    }
        break;
    case UniformModel::Uniform::Type::Sampler:
        variant = value;
        break;
    case UniformModel::Uniform::Type::Define:
        variant = value;
        break;
    }

    return variant;
}

// Update canMoveUp and canMoveDown.
// When uniform is first/last of its node it can't move up/down.
void UniformModel::updateCanMoveStatus() {
    if (m_uniformTable->isEmpty())
            return;
    QList<int> usedIds;
    for (int i = 0; i < m_uniformTable->size() ; i++) {
        auto &uniform = (*m_uniformTable)[i];
        if (usedIds.contains(uniform.nodeId)) {
            uniform.canMoveUp = true;
        } else {
            uniform.canMoveUp = false;
            usedIds << uniform.nodeId;
        }
    }
    usedIds.clear();
    for (int i = m_uniformTable->size() - 1; i >= 0; i--) {
        auto &uniform = (*m_uniformTable)[i];
        if (usedIds.contains(uniform.nodeId)) {
            uniform.canMoveDown = true;
        } else {
            uniform.canMoveDown = false;
            usedIds << uniform.nodeId;
        }
    }
    int lastRowIndex = m_uniformTable->size();
    emit dataChanged(QAbstractItemModel::createIndex(0, 0),
                     QAbstractItemModel::createIndex(lastRowIndex, 0));
}

