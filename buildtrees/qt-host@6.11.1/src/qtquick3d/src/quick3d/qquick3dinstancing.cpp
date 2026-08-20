// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qquick3dinstancing_p.h"
#include "qquick3dscenemanager_p.h"
#include <QtQuick3DRuntimeRender/private/qssgrenderinstancetable_p.h>
#include <QtQuick3DUtils/private/qssgutils_p.h>
#include <QXmlStreamReader>
#include <QtQml/QQmlFile>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Instancing
    \inherits Object3D
    \inqmlmodule QtQuick3D
    \nativetype QQuick3DInstancing
    \since 6.2
    \brief Base type for instance tables.

    \l {Instanced Rendering}{Instanced rendering} allows duplicating a model with variations.

    The Instancing type defines a table that specifies how each instance is modified relative to the
    base model. The table has an entry for each index, containing a transform matrix, a color, and
    generic data for use by custom materials. To use instancing, set a model's
    \l{Model::instancing}{instancing} property to reference an Instancing object.

    An application can define an Instancing object in C++ by subclassing QQuick3DInstancing,
    or it can use one of the pre-defined QML types: InstanceList FileInstancing, or RandomInstancing.
    In addition, it is possible to use a \l {ParticleSystem3D}{particle system} to define an
    instancing table by using the \l{ModelParticle3D::instanceTable}{ModelParticle3D.instanceTable}
    property.
*/

/*!
    \qmlproperty int Instancing::instanceCountOverride

    Set this property to limit the number of instances without regenerating or re-uploading the instance table.
    This allows very inexpensive animation of the number of instances rendered.
*/

/*!
    \property QQuick3DInstancing::instanceCountOverride

    Set this property to limit the number of instances without regenerating or re-uploading the instance table.
    This allows very inexpensive animation of the number of instances rendered.
*/

/*!
    \qmlproperty bool Instancing::hasTransparency

    Set this property to true if the instancing table contains alpha values that should be used when
    rendering the model. This property only makes a difference if the model is opaque: If the model has a
    transparent \l{Model::materials}{material}, or an \l{Node::opacity}{opacity} less than one, the
    alpha value from the table will be used regardless.

    \note Enabling alpha blending may cause rendering issues when instances overlap. See the
    \l{Alpha-blending and instancing}{alpha blending and instancing} documentation for details.
*/

/*!
    \property QQuick3DInstancing::hasTransparency

    Set this property to true if the instancing table contains alpha values that should be used when
    rendering the model. This property only makes a difference if the model is opaque: If the model has a
    transparent \l{Model::materials}{material}, or an \l{Node::opacity}{opacity} less than one, the
    alpha value from the table will be used regardless.

    \note Enabling alpha blending may cause rendering issues when instances overlap. See the
    \l{Alpha-blending and instancing}{alpha blending and instancing} documentation for details.
*/

/*!
    \qmlproperty bool Instancing::depthSortingEnabled

    Holds the depth sorting enabled value for the instance table. When enabled, instances are sorted
    and rendered from the furthest instance from the camera to the nearest i.e. back-to-front.
    If disabled, which is the default, instances are rendered in the order they are specified in
    the instance table.

    \note The instances are only sorted against each other. Instances are not sorted against other
    objects in the scene.
    \note The sorting increases the frame preparation time especially with large instance counts.
*/

/*!
    \property QQuick3DInstancing::depthSortingEnabled

    Holds the depth sorting enabled value for the instance table. When enabled, instances are sorted
    and rendered from the furthest instance from the camera to the nearest i.e. back-to-front.
    If disabled, which is the default, instances are rendered in the order they are specified in
    the instance table.

    \note The instances are only sorted against each other. Instances are not sorted against other
    objects in the scene.
    \note The sorting increases the frame preparation time especially with large instance counts.
*/

/*!
    \property QQuick3DInstancing::shadowBoundsMinimum
    \since 6.9

    Sets the minimum bounds used when calculating the shadow map bounds of the
    models in the instance table.

    Default value: \c{(1, 1, 1)}

    \note This property is only enabled when the respective components of
          shadowBoundsMinimum are smaller than those in shadowBoundsMaximum.
          Otherwise the bounds are calculated automatically.

    \sa shadowBoundsMaximum
*/
/*!
    \qmlproperty vector3d Instancing::shadowBoundsMinimum

    Sets the minimum bounds used when calculating the shadow map bounds of the models in the
    instance table.

    Default value: \c{(1, 1, 1)}

    \note This property is only enabled when the respective components of
    Instancing::shadowBoundsMinimum are smaller than those in
    Instancing::shadowBoundsMaximum. Otherwise the bounds are calculated automatically.
    \sa shadowBoundsMaximum
*/

/*!
    \property QQuick3DInstancing::shadowBoundsMaximum
    \since 6.9

    Sets the maximum bounds used when calculating the shadow map bounds of the
    models in the instance table.

    Default value: \c{(-1, -1, -1)}

    \note This property is only enabled when the respective components of
          shadowBoundsMinimum are smaller than those in shadowBoundsMaximum.
          Otherwise the bounds are calculated automatically.

    \sa shadowBoundsMinimum
*/
/*!
    \qmlproperty vector3d Instancing::shadowBoundsMaximum

    Sets the maximum bounds used when calculating the shadow map bounds of the models in the
    instance table.

    Default value: \c{(-1, -1, -1)}

    \note This property is only enabled when the respective components of
    Instancing::shadowBoundsMinimum are smaller than those in
    Instancing::shadowBoundsMaximum. Otherwise the bounds are calculated automatically.
    \sa shadowBoundsMinimum
*/

/*!
    \class QQuick3DInstancing
    \inmodule QtQuick3D
    \inherits QQuick3DObject
    \since 6.2
    \brief Base class for defining instance tables.

    The QQuick3DInstancing class can be inherited to specify a custom instance table
    for a Model in the Qt Quick 3D scene.

    This class is abstract: To use it, create a subclass and implement \l getInstanceBuffer().
*/

/*!
    \fn QByteArray QQuick3DInstancing::getInstanceBuffer(int *instanceCount)

    Implement this function to return the contents of the instance table. The number of instances should be
    returned in \a instanceCount. The subclass is responsible for caching the result if necessary. If the
    instance table changes, the subclass should call markDirty().
 */

QQuick3DInstancingPrivate::QQuick3DInstancingPrivate()
    : QQuick3DObjectPrivate(QQuick3DObjectPrivate::Type::ModelInstance)
{
}

QQuick3DInstancing::QQuick3DInstancing(QQuick3DObject *parent)
    : QQuick3DObject(*new QQuick3DInstancingPrivate, parent)
{
}

QQuick3DInstancing::~QQuick3DInstancing()
{
}

/*!
  \internal
  Returns the content of the instancing table for testing purposes.
*/
QByteArray QQuick3DInstancing::instanceBuffer(int *instanceCount)
{
    Q_D(QQuick3DInstancing);
    QByteArray retval = getInstanceBuffer(instanceCount);
    if (instanceCount && d->m_instanceCountOverride >= 0)
        *instanceCount = qMin(d->m_instanceCountOverride, *instanceCount);
    return retval;
}

int QQuick3DInstancing::instanceCountOverride() const
{
    Q_D(const QQuick3DInstancing);
    return d->m_instanceCountOverride;
}

bool QQuick3DInstancing::hasTransparency() const
{
    Q_D(const QQuick3DInstancing);
    return d->m_hasTransparency;
}

bool QQuick3DInstancing::depthSortingEnabled() const
{
    Q_D(const QQuick3DInstancing);
    return d->m_depthSortingEnabled;
}

QVector3D QQuick3DInstancing::shadowBoundsMinimum() const
{
    Q_D(const QQuick3DInstancing);
    return d->m_shadowBoundsMinimum;
}

QVector3D QQuick3DInstancing::shadowBoundsMaximum() const
{
    Q_D(const QQuick3DInstancing);
    return d->m_shadowBoundsMaximum;
}

const QQuick3DInstancing::InstanceTableEntry *QQuick3DInstancing::getInstanceEntry(int index)
{
    const QByteArray data = getInstanceBuffer(nullptr);
    if (index >= int(data.size() / sizeof(InstanceTableEntry)))
        return nullptr;
    return reinterpret_cast<const QQuick3DInstancing::InstanceTableEntry*>(data.constData()) + index;
}

QVector3D QQuick3DInstancing::InstanceTableEntry::getPosition() const
{
    return QVector3D{ row0[3], row1[3], row2[3] };
}

QVector3D QQuick3DInstancing::InstanceTableEntry::getScale() const
{
    const QVector3D col0{row0[0], row1[0], row2[0]};
    const QVector3D col1{row0[1], row1[1], row2[1]};
    const QVector3D col2{row0[2], row1[2], row2[2]};
    const float scaleX = col0.length();
    const float scaleY = col1.length();
    const float scaleZ = col2.length();
    return QVector3D(scaleX, scaleY, scaleZ);
}

QQuaternion QQuick3DInstancing::InstanceTableEntry::getRotation() const
{
    const QVector3D col0 = QVector3D(row0[0], row1[0], row2[0]).normalized();
    const QVector3D col1 = QVector3D(row0[1], row1[1], row2[1]).normalized();
    const QVector3D col2 = QVector3D(row0[2], row1[2], row2[2]).normalized();

    const float data3x3[3*3] { // row-major order
        col0[0], col1[0], col2[0],
        col0[1], col1[1], col2[1],
        col0[2], col1[2], col2[2],
    };
    QMatrix3x3 rot(data3x3);
    return QQuaternion::fromRotationMatrix(rot).normalized();
}

QColor QQuick3DInstancing::InstanceTableEntry::getColor() const
{
    return QColor::fromRgbF(color[0], color[1], color[2], color[3]);
}

/*!
    \qmlmethod vector3d QtQuick3D::Instancing::instancePosition(int index)
    \since 6.3

    Returns the position of the instance at \a index

    \sa instanceScale, instanceRotation, instanceColor, instanceCustomData
*/

QVector3D QQuick3DInstancing::instancePosition(int index)
{
    auto *entry = getInstanceEntry(index);
    if (!entry)
        return {};

    return QVector3D{ entry->row0[3], entry->row1[3], entry->row2[3] };
}

/*!
    \qmlmethod vector3d QtQuick3D::Instancing::instanceScale(int index)
    \since 6.3

    Returns the scale of the instance at \a index

    \sa instancePosition, instanceScale, instanceRotation, instanceColor, instanceCustomData
*/

QVector3D QQuick3DInstancing::instanceScale(int index)
{
    auto *entry = getInstanceEntry(index);
    if (!entry)
        return {};
    return entry->getScale();
}

/*!
    \qmlmethod quaternion QtQuick3D::Instancing::instanceRotation(int index)
    \since 6.3

    Returns a quaternion representing the rotation of the instance at \a index

    \sa instancePosition, instanceScale, instanceRotation, instanceColor, instanceCustomData
*/

QQuaternion QQuick3DInstancing::instanceRotation(int index)
{
    const auto *entry = getInstanceEntry(index);
    if (!entry)
        return {};
    return entry->getRotation();
}

/*!
    \qmlmethod color QtQuick3D::Instancing::instanceColor(int index)
    \since 6.3

    Returns the color of the instance at \a index

    \sa instancePosition, instanceScale, instanceRotation, instanceColor, instanceCustomData
*/

QColor QQuick3DInstancing::instanceColor(int index)
{
    const auto *entry = getInstanceEntry(index);
    if (!entry)
        return {};
    return entry->getColor();
}

/*!
    \qmlmethod vector3d QtQuick3D::Instancing::instanceCustomData(int index)
    \since 6.3

    Returns the custom data for the instance at \a index

    \sa instancePosition, instanceScale, instanceRotation, instanceColor, instanceCustomData
*/

QVector4D QQuick3DInstancing::instanceCustomData(int index)
{
    const auto *entry = getInstanceEntry(index);
    if (!entry)
        return {};
    return entry->instanceData;
}

void QQuick3DInstancing::setInstanceCountOverride(int instanceCountOverride)
{
    Q_D(QQuick3DInstancing);
    if (d->m_instanceCountOverride == instanceCountOverride)
        return;

    d->m_instanceCountOverride = instanceCountOverride;
    d->m_instanceCountOverrideChanged = true;
    d->dirty(QQuick3DObjectPrivate::DirtyType::Content);
    emit instanceCountOverrideChanged();
}

void QQuick3DInstancing::setHasTransparency(bool hasTransparency)
{
    Q_D(QQuick3DInstancing);
    if (d->m_hasTransparency == hasTransparency)
        return;

    d->m_hasTransparency = hasTransparency;
    d->dirty(QQuick3DObjectPrivate::DirtyType::Content);
    emit hasTransparencyChanged();
}

void QQuick3DInstancing::setDepthSortingEnabled(bool enabled)
{
    Q_D(QQuick3DInstancing);
    if (d->m_depthSortingEnabled == enabled)
        return;

    d->m_depthSortingEnabled = enabled;
    d->dirty(QQuick3DObjectPrivate::DirtyType::Content);
    emit depthSortingEnabledChanged();
}

void QQuick3DInstancing::setShadowBoundsMinimum(const QVector3D &newShadowBoundsMinimum)
{
    Q_D(QQuick3DInstancing);
    if (d->m_shadowBoundsMinimum == newShadowBoundsMinimum)
        return;

    d->m_shadowBoundsMinimum = newShadowBoundsMinimum;
    d->dirty(QQuick3DObjectPrivate::DirtyType::Content);
    emit shadowBoundsMinimumChanged();
}

void QQuick3DInstancing::setShadowBoundsMaximum(const QVector3D &newShadowBoundsMaximum)
{
    Q_D(QQuick3DInstancing);
    if (d->m_shadowBoundsMinimum == newShadowBoundsMaximum)
        return;

    d->m_shadowBoundsMaximum = newShadowBoundsMaximum;
    d->dirty(QQuick3DObjectPrivate::DirtyType::Content);
    emit shadowBoundsMaximumChanged();
}

/*!
  Mark that the instance data has changed and must be uploaded again.

  \sa getInstanceBuffer, instanceCountOverride
  */

void QQuick3DInstancing::markDirty()
{
    Q_D(QQuick3DInstancing);
    d->dirty(QQuick3DObjectPrivate::DirtyType::Content);
    d->m_instanceDataChanged = true;
    emit instanceTableChanged();
}

QSSGRenderGraphObject *QQuick3DInstancing::updateSpatialNode(QSSGRenderGraphObject *node)
{
    Q_D(QQuick3DInstancing);
    if (!node) {
        markAllDirty();
        node = new QSSGRenderInstanceTable();
        emit instanceNodeDirty();
        d->m_instanceDataChanged = true;
    }
    QQuick3DObject::updateSpatialNode(node);
    auto effectiveInstanceCount = [d]() {
        if (d->m_instanceCountOverride >= 0)
            return qMin(d->m_instanceCount, d->m_instanceCountOverride);
        return d->m_instanceCount;
    };
    auto *instanceTable = static_cast<QSSGRenderInstanceTable *>(node);
    if (d->m_instanceDataChanged) {
        QByteArray buffer = getInstanceBuffer(&d->m_instanceCount);
        instanceTable->setData(buffer, effectiveInstanceCount(), sizeof(InstanceTableEntry));
        d->m_instanceDataChanged = false;
    } else if (d->m_instanceCountOverrideChanged) {
        instanceTable->setInstanceCountOverride(effectiveInstanceCount());
    }
    d->m_instanceCountOverrideChanged = false;
    instanceTable->setHasTransparency(d->m_hasTransparency);
    instanceTable->setDepthSorting(d->m_depthSortingEnabled);
    instanceTable->setShadowBoundsMinimum(d->m_shadowBoundsMinimum);
    instanceTable->setShadowBoundsMaximum(d->m_shadowBoundsMaximum);
    return node;
}

static inline QQuick3DInstancing::InstanceTableEntry calculate(const QVector3D &position, const QVector3D &scale,
                                                                         const QVector3D &eulerRotation, const QColor &color,
                                                                         const QVector4D &customData)
{
    QMatrix4x4 xform;

    xform(0, 0) = scale[0];
    xform(1, 1) = scale[1];
    xform(2, 2) = scale[2];

    QQuaternion quaternion = QQuaternion::fromEulerAngles(eulerRotation);
    xform = QMatrix4x4(quaternion.toRotationMatrix()) * xform;

    xform(0, 3) += position[0];
    xform(1, 3) += position[1];
    xform(2, 3) += position[2];

    auto linearColor = QSSGUtils::color::sRGBToLinear(color);

    return {
        xform.row(0),
        xform.row(1),
        xform.row(2),
        linearColor,
        customData
    };
}

/*!
   Converts the
   \a position
   \a scale
   \a eulerRotation
   \a color
   and
   \a customData
   to the instance table format expected by the standard vertex shaders. Typical pattern:

   \code
    QByteArray MyInstanceTable::getInstanceBuffer(int *instanceCount)
    {
        QByteArray instanceData;

        ...

        auto entry = calculateTableEntry({xPos, yPos, zPos}, {xScale, yScale, zScale}, {xRot, yRot, zRot}, color, {});
        instanceData.append(reinterpret_cast<const char *>(&entry), sizeof(entry));
   \endcode

   \sa calculateTableEntryFromQuaternion
 */
QQuick3DInstancing::InstanceTableEntry QQuick3DInstancing::calculateTableEntry(const QVector3D &position, const QVector3D &scale,
                                   const QVector3D &eulerRotation, const QColor &color, const QVector4D &customData)
{
    return calculate(position, scale, eulerRotation, color, customData);
}

/*!
   Converts the
   \a position
   \a scale
   \a rotation
   \a color
   and
   \a customData
   to the instance table format expected by the standard vertex shaders.

    This is the same as calculateTableEntry(), except for using a quaternion to specify the rotation.
 */
QQuick3DInstancing::InstanceTableEntry QQuick3DInstancing::calculateTableEntryFromQuaternion(const QVector3D &position, const QVector3D &scale, const QQuaternion &rotation, const QColor &color, const QVector4D &customData)
{
    QMatrix4x4 xform;

    xform(0, 0) = scale[0];
    xform(1, 1) = scale[1];
    xform(2, 2) = scale[2];

    xform = QMatrix4x4(rotation.toRotationMatrix()) * xform;

    xform(0, 3) += position[0];
    xform(1, 3) += position[1];
    xform(2, 3) += position[2];

    auto linearColor = QSSGUtils::color::sRGBToLinear(color);

    return {
        xform.row(0),
        xform.row(1),
        xform.row(2),
        linearColor,
        customData
    };
}

/*!
    \qmltype InstanceList
    \inherits Instancing
    \inqmlmodule QtQuick3D
    \brief Allows manually specifying instancing in QML.

    The InstanceList type makes it possible to define an instance table manually in QML.

    The following example creates an instance table with two items:
    \qml
    InstanceList {
        id: manualInstancing
        instances: [
            InstanceListEntry {
                position: Qt.vector3d(0, 0, -60)
                eulerRotation: Qt.vector3d(-10, 0, 30)
                color: "red"
            },
            InstanceListEntry {
                position: Qt.vector3d(50, 10, 100)
                eulerRotation: Qt.vector3d(0, 180, 0)
                color: "green"
            }
        ]
    }
    \endqml

    It is also possible to populate the instances property by just adding children to the
    InstanceList. The following example is equivalent to the previous one:
    \qml
    InstanceList {
        id: manualInstancing
        InstanceListEntry {
            position: Qt.vector3d(0, 0, -60)
            eulerRotation: Qt.vector3d(-10, 0, 30)
            color: "red"
        }
        InstanceListEntry {
            position: Qt.vector3d(50, 10, 100)
            eulerRotation: Qt.vector3d(0, 180, 0)
            color: "green"
        }
    }
    \endqml

    Each InstanceListEntry is an object that can have property bindings and animations. This gives
    great flexibility, but also causes memory overhead. Therefore, it is not recommended to use
    InstanceList for procedurally generated tables containing thousands (or millions) of
    instances. Also, any property change to an entry will cause the entire instance table to be
    recalculated and uploaded to the GPU.

    \sa RandomInstancing, QQuick3DInstancing
*/

/*!
    \qmlproperty List<QtQuick3D::InstanceListEntry> InstanceList::instances
    \qmldefault

    This property contains the list of instance definitions. Modifying this list, or any of its elements, will cause the instance table to be updated.
*/

QQuick3DInstanceList::QQuick3DInstanceList(QQuick3DObject *parent) : QQuick3DInstancing(parent) {}

QQuick3DInstanceList::~QQuick3DInstanceList() {}

QByteArray QQuick3DInstanceList::getInstanceBuffer(int *instanceCount)
{
    if (m_dirty)
        generateInstanceData();
    if (instanceCount)
        *instanceCount = m_instances.size();
    return m_instanceData;
}

QQmlListProperty<QQuick3DInstanceListEntry> QQuick3DInstanceList::instances()
{

    return QQmlListProperty<QQuick3DInstanceListEntry>(this,
                                                          nullptr,
                                                          qmlAppendInstanceListEntry,
                                                          qmlInstanceListEntriesCount,
                                                          qmlInstanceListEntryAt,
                                                          qmlClearInstanceListEntries);
}

/*!
    \qmlproperty int QtQuick3D::InstanceList::instanceCount
    \since 6.3

    This read-only property contains the number of instances in the list.
*/

int QQuick3DInstanceList::instanceCount() const
{
    return m_instances.size();
}

void QQuick3DInstanceList::onInstanceDestroyed(QObject *object)
{
    if (m_instances.removeAll(object))
        handleInstanceChange();
}

void QQuick3DInstanceList::qmlAppendInstanceListEntry(QQmlListProperty<QQuick3DInstanceListEntry> *list, QQuick3DInstanceListEntry *instance)
{
    if (instance == nullptr)
        return;
    auto *self = static_cast<QQuick3DInstanceList *>(list->object);
    self->m_instances.push_back(instance);

    if (instance->parentItem() == nullptr)
        instance->setParentItem(self);
    connect(instance, &QQuick3DInstanceListEntry::changed, self, &QQuick3DInstanceList::handleInstanceChange);
    connect(instance, &QObject::destroyed, self, &QQuick3DInstanceList::onInstanceDestroyed);
    self->handleInstanceChange();
}

QQuick3DInstanceListEntry *QQuick3DInstanceList::qmlInstanceListEntryAt(QQmlListProperty<QQuick3DInstanceListEntry> *list, qsizetype index)
{
    auto *self = static_cast<QQuick3DInstanceList *>(list->object);
    return self->m_instances.at(index);
}

qsizetype QQuick3DInstanceList::qmlInstanceListEntriesCount(QQmlListProperty<QQuick3DInstanceListEntry> *list)
{
    auto *self = static_cast<QQuick3DInstanceList *>(list->object);
    return self->m_instances.size();
}

void QQuick3DInstanceList::qmlClearInstanceListEntries(QQmlListProperty<QQuick3DInstanceListEntry> *list)
{
    auto *self = static_cast<QQuick3DInstanceList *>(list->object);
    for (auto *instance : self->m_instances) {
        disconnect(instance, &QObject::destroyed, self, &QQuick3DInstanceList::onInstanceDestroyed);
        disconnect(instance, &QQuick3DInstanceListEntry::changed, self, &QQuick3DInstanceList::handleInstanceChange);
    }
    self->m_instances.clear();
    self->handleInstanceChange();
}

void QQuick3DInstanceList::handleInstanceChange()
{
    m_dirty = true;
    markDirty();
    emit instanceCountChanged();
}

void QQuick3DInstanceList::generateInstanceData()
{
    m_dirty = false;
    const int count = m_instances.size();

    qsizetype tableSize = count * sizeof(InstanceTableEntry);
    m_instanceData.resize(tableSize);
    auto *array = reinterpret_cast<InstanceTableEntry*>(m_instanceData.data());
    for (int i = 0; i < count; ++i) {
        const auto *inst = m_instances.at(i);
        if (inst->m_useEulerRotation)
            array[i] = calculateTableEntry(inst->position(), inst->scale(), inst->eulerRotation(), inst->color(), inst->customData());
        else
            array[i] = calculateTableEntryFromQuaternion(inst->position(), inst->scale(), inst->rotation(), inst->color(), inst->customData());
    }
}

/*!
    \qmltype InstanceListEntry
    \inherits Object3D
    \inqmlmodule QtQuick3D
    \since 6.2
    \brief Specifies an instance in an InstanceList.

    The InstanceListEntry QML type is used to specify one instance in an instance list.

    All the properties can have bindings and animation. Changing a property will cause the entire
    instance table to be recalculated and uploaded to the GPU, so this can be expensive for instance
    lists with many members.
*/

QQuick3DInstanceListEntry::QQuick3DInstanceListEntry(QQuick3DObject *parent)
    : QQuick3DObject(parent)
{
}

/*!
    \qmlproperty vector3d QtQuick3D::InstanceListEntry::position

    This property specifies the position for the instance.
*/
void QQuick3DInstanceListEntry::setPosition(QVector3D position)
{
    if (m_position == position)
        return;

    m_position = position;
    emit positionChanged();
    emit changed();
}

/*!
    \qmlproperty vector3d QtQuick3D::InstanceListEntry::scale

    This property specifies the scale for the instance as a vector containing the scale factor along the x, y and z axes.
*/
void QQuick3DInstanceListEntry::setScale(QVector3D scale)
{
    if (m_scale == scale)
        return;

    m_scale = scale;
    emit scaleChanged();
    emit changed();
}

/*!
    \qmlproperty vector3d QtQuick3D::InstanceListEntry::eulerRotation

    This property specifies the rotation for the instance as an Euler vector, that
    is a vector containing the rotation in degrees around the x, y and z axes.
*/
void QQuick3DInstanceListEntry::setEulerRotation(QVector3D eulerRotation)
{
    if (m_useEulerRotation && m_eulerRotation == eulerRotation)
        return;
    m_eulerRotation = eulerRotation;
    m_useEulerRotation = true;
    emit eulerRotationChanged();
    emit changed();
}

/*!
    \qmlproperty quaternion QtQuick3D::InstanceListEntry::rotation

    This property specifies the rotation for the instance as a quaternion.
*/
void QQuick3DInstanceListEntry::setRotation(QQuaternion rotation)
{
    if (!m_useEulerRotation && m_rotation == rotation)
        return;

    m_rotation = rotation;
    m_useEulerRotation = false;
    emit rotationChanged();
    emit changed();
}

/*!
    \qmlproperty color QtQuick3D::InstanceListEntry::color

    This property specifies the color for the instance.
*/
void QQuick3DInstanceListEntry::setColor(QColor color)
{
    if (m_color == color)
        return;

    m_color = color;
    emit colorChanged();
    emit changed();
}

/*!
    \qmlproperty vector4d QtQuick3D::InstanceListEntry::customData

    This property specifies the custom data for the instance. This is not used by default,
    but is made available to the vertex shader of custom materials as \c INSTANCE_DATA.
*/
void QQuick3DInstanceListEntry::setCustomData(QVector4D customData)
{
    if (m_customData == customData)
        return;

    m_customData = customData;
    emit customDataChanged();
    emit changed();
}

/*!
    \qmltype FileInstancing
    \inherits Instancing
    \inqmlmodule QtQuick3D
    \since 6.2
    \brief Allows reading instance tables from file.

    The FileInstancing type makes it possible to read instance tables from files.

    There are two supported file formats: XML, and a Qt-specific binary format. The
    binary file format uses the same layout as the table that is uploaded to the GPU,
    so it can be directly mapped to memory. The \l{Instancer Tool}{instancer} tool converts
    from XML to the binary format.

    This is an example of the XML file format:
    \badcode
    <?xml version="1.0" encoding="UTF-8" ?>
    <InstanceTable>
      <Instance position="0 200 0"  scale="0.75 0.75 0.75" custom="20 20" color="#ffcf7f"/>
      <Instance position="0 -100 0" scale="0.5 0.5 0.5" color="red"/>
      <Instance position="0 -200 0" eulerRotation="0 0 60" color="darkred" custom="10 40 0 0"/>
    </InstanceTable>
    \endcode

    In order to be valid, the XML file must have a top-level \c{InstanceTable} element. Each
    instance is represented by an \c{Instance} element inside the \c{InstanceTable}. Unknown
    elements are silently ignored.

    An \c{Instance} element can have a number of attributes. \c{color} attributes are specified by the normal Qt
    SVG color names, or by hexadecimal notation. \c{vector3d} and {vector4d} attributes are specified by
    a string of space-separated numbers, where missing trailing numbers indicate zeroes. The following
    attributes are supported:
    \table
    \header
    \li name
    \li type
    \row
    \li \c position
    \li \c vector3d
    \row
    \li \c scale
    \li \c vector3d
    \row
    \li \c eulerRotation
    \li \c vector3d
    \row
    \li \c quaternion
    \li \c vector4d
    \row
    \li \c custom
    \li \c vector4d
    \row
    \li \c color
    \li \c color
    \endtable
    Unknown attributes are silently ignored.
*/

/*!
    \qmlproperty url QtQuick3D::FileInstancing::source

    This property holds the location of an XML or binary file containing the instance data.

    If the file name has a ".bin" extension, it is assumed to refer to a binary file.
    Otherwise it is assumed to refer to an XML file. If an XML file \e{foo.xml} is specified, and
    the file \e{foo.xml.bin} exists, the binary file \e{foo.xml.bin} will be loaded instead.
*/

/*!
    \qmlproperty int QtQuick3D::FileInstancing::instanceCount
    \since 6.3

    This read-only property contains the number of instances in the instance table.
*/

static constexpr quint16 currentMajorVersion = 1;

struct QQuick3DInstancingBinaryFileHeader
{
    char magic[4] = { 'Q', 't', 'I', 'R' };
    const quint16 majorVersion = currentMajorVersion;
    const quint16 minorVersion = 0;
    const quint32 stride = sizeof(QQuick3DInstancing::InstanceTableEntry);
    quint32 offset;
    quint32 count;
};

static bool writeInstanceTable(QIODevice *out, const QByteArray &instanceData, int instanceCount)
{
    QQuick3DInstancingBinaryFileHeader header;

    header.offset = sizeof(header);
    header.count = instanceCount;

    if (instanceData.size() != qsizetype(header.stride) * instanceCount) {
        qWarning() << "inconsistent data";
        return false;
    }

    // Ignoring endianness: Assume we always create on little-endian, and then special-case reading if we need to.

    out->write(reinterpret_cast<const char *>(&header), sizeof(header));
    out->write(instanceData.constData(), instanceData.size());
    return true;
}


bool QQuick3DFileInstancing::loadFromBinaryFile(const QString &filename)
{
    auto binaryFile = std::make_unique<QFile>(filename);
    if (!binaryFile->open(QFile::ReadOnly))
        return false;

    constexpr auto headerSize = sizeof(QQuick3DInstancingBinaryFileHeader);
    const quint64 fileSize = binaryFile->size();
    if (fileSize < headerSize) {
        qWarning() << "data file too small";
        return false;
    }
    const char *data = reinterpret_cast<const char *>(binaryFile->map(0, fileSize));
    const auto *header = reinterpret_cast<const QQuick3DInstancingBinaryFileHeader *>(data);

    if (header->majorVersion > currentMajorVersion) {
        qWarning() << "Version" << header->majorVersion << "is too new";
        return false;
    }

    if (fileSize != headerSize + header->count * header->stride) {
        qWarning() << "wrong data size";
        return false;
    }

    delete m_dataFile;

    // In order to use fromRawData safely, the file has to stay open so that the mmap stays valid
    m_dataFile = binaryFile.release();

    m_instanceData = QByteArray::fromRawData(data + header->offset, header->count * header->stride);
    m_instanceCount = header->count;

    return true;
}

bool QQuick3DFileInstancing::loadFromXmlFile(const QString &filename)
{
    QFile f(filename);
    if (!f.open(QFile::ReadOnly))
        return false;

    bool valid = false;
    QXmlStreamReader reader(&f);
    int instances = 0;

    //### Why is there not a QTextStream constructor that takes a QStringView
    const auto toVector3D = [](const QStringView &str) {
        float x, y, z;
        QTextStream(str.toLocal8Bit()) >> x >> y >> z;
        return QVector3D { x, y, z };
    };
    const auto toVector4D = [](const QStringView &str) {
        float x, y, z, w;
        QTextStream(str.toLocal8Bit()) >> x >> y >> z >> w;
        return QVector4D { x, y, z, w };
    };

    QByteArray instanceData;

    while (reader.readNextStartElement()) {

        if (reader.name() == QLatin1String("InstanceTable")) {
            valid = true;
            while (reader.readNextStartElement()) {
                if (reader.name() == QLatin1String("Instance")) {
                    QColor color = Qt::white;
                    QVector3D position;
                    QVector3D eulerRotation;
                    QQuaternion quaternion;
                    bool useQuaternion = false;
                    QVector4D custom;
                    QVector3D scale { 1, 1, 1 };
                    for (auto &attr : reader.attributes()) {
                        if (attr.name() == QLatin1String("color")) {
                            color = QColor::fromString(attr.value());
                        } else if (attr.name() == QLatin1String("position")) {
                            position = toVector3D(attr.value());
                        } else if (attr.name() == QLatin1String("eulerRotation")) {
                            eulerRotation = toVector3D(attr.value());
                        } else if (attr.name() == QLatin1String("scale")) {
                            scale = toVector3D(attr.value());
                        } else if (attr.name() == QLatin1String("quaternion")) {
                            quaternion = QQuaternion(toVector4D(attr.value()));
                            useQuaternion = true;
                        } else if (attr.name() == QLatin1String("custom")) {
                            custom = toVector4D(attr.value());
                        }
                    }
                    auto entry = useQuaternion ? calculateTableEntryFromQuaternion(position, scale, quaternion, color, custom)
                                               : calculateTableEntry(position, scale, eulerRotation, color, custom);
                    instanceData.append(reinterpret_cast<const char *>(&entry), sizeof(entry));
                    instances++;
                }
                reader.skipCurrentElement();
            }
        } else {
            reader.skipCurrentElement();
        }
    }

    if (valid) {
        m_instanceCount = instances;
        m_instanceData = instanceData;
    }

    f.close();
    return valid;
}

int QQuick3DFileInstancing::writeToBinaryFile(QIODevice *out)
{
    bool success = writeInstanceTable(out, m_instanceData, m_instanceCount);
    return success ? m_instanceCount : -1;
}

int QQuick3DFileInstancing::instanceCount() const
{
    return m_instanceCount;
}

bool QQuick3DFileInstancing::loadFromFile(const QUrl &source)
{
    const QQmlContext *context = qmlContext(this);

    const QString filePath = QQmlFile::urlToLocalFileOrQrc(context ? context->resolvedUrl(source) : source);

    if (filePath.endsWith(QStringLiteral(".bin")))
        return loadFromBinaryFile(filePath);

    const QString binaryFilePath = filePath + QStringLiteral(".bin");

    int oldCount = m_instanceCount;
    bool success = loadFromBinaryFile(binaryFilePath) || loadFromXmlFile(filePath);
    if (m_instanceCount != oldCount)
        emit instanceCountChanged();

    return success;
}

QQuick3DFileInstancing::QQuick3DFileInstancing(QQuick3DObject *parent) : QQuick3DInstancing(parent) { }

QQuick3DFileInstancing::~QQuick3DFileInstancing()
{
    delete m_dataFile;
}

const QUrl &QQuick3DFileInstancing::source() const
{
    return m_source;
}

void QQuick3DFileInstancing::setSource(const QUrl &newSource)
{
    if (m_source == newSource)
        return;
    m_source = newSource;
    m_dirty = true;
    markDirty();

    emit sourceChanged();
}

QByteArray QQuick3DFileInstancing::getInstanceBuffer(int *instanceCount)
{
    if (m_dirty) {
        if (!loadFromFile(m_source))  {
            qWarning() << Q_FUNC_INFO << "could not load" << m_source;
            m_instanceData = {};
            m_instanceCount = 0;
        }
        m_dirty = false;
    }

    if (instanceCount)
        *instanceCount = m_instanceCount;
    return m_instanceData;
}

static_assert(sizeof(QQuick3DInstancing::InstanceTableEntry) == sizeof(QSSGRenderInstanceTableEntry)
                && alignof(QQuick3DInstancing::InstanceTableEntry) == alignof(QSSGRenderInstanceTableEntry),
              "QSSGRenderInstanceTableEntry and QQuick3DInstancing::InstanceTableEntry do not match");

QT_END_NAMESPACE
