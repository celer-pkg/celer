// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "instancerepeater_p.h"
#include <math.h>
#include <QMatrix4x4>

QT_BEGIN_NAMESPACE

/*!
    \qmltype InstanceModel
    \inherits Object3D
    \inqmlmodule QtQuick3D.Helpers
    \since 6.4
    \brief Defines a data model based on an instance table.

    The InstanceModel QML type is a data model that provides access to the elements of an \l Instancing table.

    The following roles are available:
    \table
        \header
            \li Role name
            \li Description
        \row
            \li \c modelPosition
            \li The position of the instance as a \l vector3d
        \row
            \li \c modelRotation
            \li The rotation of the instance as a \l quaternion
        \row
            \li \c modelScale
            \li The scale of the instance  as a \l vector3d
        \row
            \li \c modelColor
            \li The \l color of the instance
        \row
            \li \c modelData
            \li The custom data of the instance as a \l vector4d
    \endtable

    \sa InstanceRepeater
*/

/*!
    \qmlproperty Instancing InstanceModel::instancingTable

    This property specifies the underlying instance table of the model.
*/

InstanceModel::InstanceModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QVariant InstanceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    ensureTable();

    int idx = index.row();

    if (idx >= m_count) {
        qWarning("InstanceModel: index out of range");
        return QVariant();
    }

    auto *instanceData = reinterpret_cast<const QQuick3DInstancing::InstanceTableEntry*>(m_instanceData.data()) + idx;

    switch (role) {
    case ColorRole:
        return instanceData->getColor();
    case PositionRole:
        return instanceData->getPosition();
    case RotationRole:
        return instanceData->getRotation();
    case ScaleRole:
        return instanceData->getScale();
    case CustomDataRole:
        return instanceData->instanceData;
    }
    return QVariant();
}

int InstanceModel::rowCount(const QModelIndex &) const
{
    ensureTable();
    return m_count;
}

void InstanceModel::setInstancing(QQuick3DInstancing *instancing)
{
    if (m_instancing == instancing)
        return;
    QObject::disconnect(m_tableConnection);
    m_instancing = instancing;
    m_tableConnection = QObject::connect(instancing, &QQuick3DInstancing::instanceTableChanged, this, &InstanceModel::reset);
    emit instancingChanged();
}

const QQuick3DInstancing::InstanceTableEntry *InstanceModel::instanceData(int index) const
{
    if (index >= m_count)
        return nullptr;
    return reinterpret_cast<const QQuick3DInstancing::InstanceTableEntry*>(m_instanceData.constData()) + index;
}

void InstanceModel::ensureTable() const
{
    auto *that = const_cast<InstanceModel*>(this);
    that->m_instanceData = m_instancing->instanceBuffer(&that->m_count);
}

void InstanceModel::reset()
{
    beginResetModel();
    m_instanceData.clear();
    endResetModel();
}

/*!
    \qmltype InstanceRepeater
    \inherits Repeater3D
    \inqmlmodule QtQuick3D.Helpers
    \since 6.4
    \brief Instantiates components based on an instance table.

    The InstanceRepeater type is used to create a number of objects based on an
    \l Instancing table. It is a \l Repeater3D subtype that takes an Instancing table instead
    of a data model, and automatically applies \c position, \c scale, and \c rotation.

    One use case is to implement \l {View3D::pick}{picking} by creating invisible dummy objects
    that match the rendered instances. To improve performance, the dummy objects can be created with a
    simpler geometry than the instanced models.

    For example:
    \qml
        InstanceRepeater {
            instancingTable: myInstanceTable
            Model {
                source: "#Cube"
                pickable: true
                property int instanceIndex: index // expose the index, so we can identify the instance
                opacity: 0
            }
        }
    \endqml

    \sa InstanceModel
*/

/*!
    \qmlproperty Instancing InstanceRepeater::instancingTable

    This property specifies the instance table used by the repeater.
*/

InstanceRepeater::InstanceRepeater(QQuick3DNode *parent)
    : QQuick3DRepeater(parent)
{
}

QQuick3DInstancing *InstanceRepeater::instancing() const
{
    return m_model ? m_model->instancing() : nullptr;
}

void InstanceRepeater::setInstancing(QQuick3DInstancing *instancing)
{
    if (m_model && m_model->instancing() == instancing)
        return;
    if (!m_model)
        m_model = new InstanceModel(this);
    m_model->setInstancing(instancing);
    setModel(QVariant::fromValue(m_model));
    emit instancingChanged();
}

void InstanceRepeater::initDelegate(int index, QQuick3DNode *node)
{
    Q_ASSERT(m_model);
    auto *entry = m_model->instanceData(index);
    Q_ASSERT(entry);
    node->setPosition(entry->getPosition());
    node->setScale(entry->getScale());
    node->setRotation(entry->getRotation());
}

QT_END_NAMESPACE
