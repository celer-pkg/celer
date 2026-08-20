// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dobject.h"
#include "qquick3dobject_p.h"
#include "qquick3dscenemanager_p.h"
#include "qquick3ditem2d_p.h"
#include "qquick3dmodel_p.h"

#include <QtQuick3DRuntimeRender/private/qssgrendergraphobject_p.h>
#include <ssg/qssgrenderextensions.h>

#include <QtQml/private/qqmlglobal_p.h>
#include <QtQuick/private/qquickstategroup_p.h>
#include <QtQuick/private/qquickstate_p.h>
#include <QtQuick/private/qquickitem_p.h>

#include <private/qv4object_p.h>
#include <private/qv4qobjectwrapper_p.h>

QT_BEGIN_NAMESPACE


/*!
    \qmltype Object3D
    \inqmlmodule QtQuick3D
    \nativetype QQuick3DObject
    \inherits QtObject
    \brief Abstract base type of all 3D nodes and resources.

    Object3D is the base class for all Qt Quick 3D types. This includes:

    \list

    \li Spatial types that represent objects in the 3D scene, these will normally have a position and/or a direction.
    For example, \l Model, \l Camera, or \l Light. Such types inherit from \l Node, which in turn inherits from Object3D.

    \li Resource types that do not themselves represent an object in the 3D world, but rather serve
    as components to \l Node subclasses, providing data of some kind. This includes, among
    others, \l Material, \l Geometry, and \l Texture.

    \endlist

    In addition to the above types, Object3D can also serve as the parent for \l{Item}{Qt
    Quick items}, as well as arbitrary QObject instances. For more information on adding
    2D items to the 3D scene, refer to \l{Qt Quick 3D Scenes with 2D Content}.

    \sa Node
*/

/*!
    \class QQuick3DObject
    \inmodule QtQuick3D
    \since 5.15
    \brief Base class of all 3D nodes and resources.

    Object3D is the base class for all Qt Quick 3D scene objects. Currently the
    types available in C++ are:

    \list
    \li QQuick3DGeometry
    \li QQuick3DTextureData
    \endlist

    Both of these types are resource objects which directly inherit QQuick3DObject.

    It should not be necessary to use QQuick3DObject directly anywhere currently
    because it is just an interface for supporting spatial items and resources in
    a 3D scene, as well as exposing similar functionality as QQuickItem for 3D
    scene content.
*/

QQuick3DObject::QQuick3DObject(QQuick3DObject *parent)
    : QObject(*(new QQuick3DObjectPrivate(QQuick3DObjectPrivate::Type::Unknown)), parent)
{
    Q_D(QQuick3DObject);
    d->init(parent);
}

QQuick3DObject::~QQuick3DObject()
{
    Q_D(QQuick3DObject);
    // XXX todo - optimize
    while (!d->childItems.isEmpty())
        d->childItems.constFirst()->setParentItem(nullptr);

    delete d->_stateGroup;
    d->_stateGroup = nullptr;
    delete d->contentItem2d;
    d->contentItem2d = nullptr;

    if (d->parentItem)
        setParentItem(nullptr);

    if (d->sceneRefCount > 1)
        d->sceneRefCount = 1; // Make sure the scene is set to null in next call to derefSceneManager().

    if (!d->parentItem && d->sceneManager)
        QQuick3DObjectPrivate::derefSceneManager(this);
}

void QQuick3DObject::update()
{
    Q_D(QQuick3DObject);
    d->dirty(QQuick3DObjectPrivate::Content);
}

/*!
    \qmlproperty Object3D QtQuick3D::Object3D::parent
    This property holds the parent of the Object3D in a 3D scene.

    \note An Object3D's parent may not necessarily be the same as its object
    parent. This is necessary because the object parent may be an item that is
    not of type Object3D, for example the root object in a scene.
*/
/*!
    \property QQuick3DObject::parent
    This property holds the parent of the Object3D in a 3D scene.

    \note An Object3D's parent may not necessarily be the same as its object
    parent. This is necessary because the object parent may be an item that is
    not of type Object3D, for example the root object in a scene.

    \note Currently for 3D items to be correctly handled by the scene manager
    when parenting 3D objects from C++ it is necessary to call
    QQuick3DObject::setParentItem before the QObject::setParent. This requirement is
    likely to change in a future release though.
    \code
    QQuick3DObject *newItem = new QQuick3DObject();
    newItem->setParentItem(parentItem);
    newItem->setParent(parentItem);
    \endcode
*/

void QQuick3DObject::setParentItem(QQuick3DObject *parentItem)
{
    Q_D(QQuick3DObject);
    if (parentItem == d->parentItem)
        return;

    if (parentItem) {
        QQuick3DObject *itemAncestor = parentItem;
        while (itemAncestor != nullptr) {
            if (Q_UNLIKELY(itemAncestor == this)) {
                qWarning() << "QSSGObject::setParentItem: Parent" << parentItem << "is already part of the subtree of" << this;
                return;
            }
            itemAncestor = itemAncestor->parentItem();
        }
    }

    d->removeFromDirtyList();

    QQuick3DObject *oldParentItem = d->parentItem;

    if (oldParentItem) {
        QQuick3DObjectPrivate *op = QQuick3DObjectPrivate::get(oldParentItem);

        op->removeChild(this);
    } else if (d->sceneManager) {
        d->sceneManager->parentlessItems.remove(this);
    }

    const auto parentSceneManager = parentItem ? QQuick3DObjectPrivate::get(parentItem)->sceneManager : nullptr;
    if (d->sceneManager == parentSceneManager) {
        // Avoid freeing and reallocating resources if the window stays the same.
        d->parentItem = parentItem;
    } else {
        if (d->sceneManager)
            QQuick3DObjectPrivate::derefSceneManager(this);
        d->parentItem = parentItem;
        if (parentSceneManager)
            QQuick3DObjectPrivate::refSceneManager(this, *parentSceneManager);
    }

    d->dirty(QQuick3DObjectPrivate::ParentChanged);

    if (d->parentItem)
        QQuick3DObjectPrivate::get(d->parentItem)->addChild(this);
    else if (d->sceneManager)
        d->sceneManager->parentlessItems.insert(this);

    d->itemChange(ItemParentHasChanged, d->parentItem);

    emit parentChanged();
}

QString QQuick3DObject::state() const
{
    Q_D(const QQuick3DObject);
    return d->state();
}

void QQuick3DObject::setState(const QString &state)
{
    Q_D(QQuick3DObject);
    d->setState(state);
}

QList<QQuick3DObject *> QQuick3DObject::childItems() const
{
    Q_D(const QQuick3DObject);
    return d->childItems;
}

QQuick3DObject *QQuick3DObject::parentItem() const
{
    Q_D(const QQuick3DObject);
    return d->parentItem;
}

QSSGRenderGraphObject *QQuick3DObject::updateSpatialNode(QSSGRenderGraphObject *node)
{
    Q_QUICK3D_PROFILE_ASSIGN_ID_SG(this, node);
    return node;
}

void QQuick3DObject::markAllDirty()
{
}

void QQuick3DObject::itemChange(QQuick3DObject::ItemChange, const QQuick3DObject::ItemChangeData &)
{
}

QQuick3DObject::QQuick3DObject(QQuick3DObjectPrivate &dd, QQuick3DObject *parent)
    : QObject(dd, parent)
{
    Q_D(QQuick3DObject);
    d->init(parent);
}

void QQuick3DObject::classBegin()
{
    Q_D(QQuick3DObject);
    d->componentComplete = false;
    if (d->_stateGroup)
        d->_stateGroup->classBegin();
}

void QQuick3DObject::componentComplete()
{
    Q_D(QQuick3DObject);
    d->componentComplete = true;
    if (d->_stateGroup)
        d->_stateGroup->componentComplete();

    if (d->sceneManager && d->dirtyAttributes) {
        d->addToDirtyList();
    }
    Q_QUICK3D_PROFILE_REGISTER_D(this);
}

bool QQuick3DObject::isComponentComplete() const
{
    Q_D(const QQuick3DObject);
    return d->componentComplete;
}

void QQuick3DObject::preSync()
{

}

QQuick3DObjectPrivate::QQuick3DObjectPrivate(Type t, FlagsT f)
    : _stateGroup(nullptr)
    , dirtyAttributes(0)
    , nextDirtyItem(nullptr)
    , prevDirtyItem(nullptr)
    , sceneManager(nullptr)
    , sceneRefCount(0)
    , parentItem(nullptr)
    , subFocusItem(nullptr)
    , type(t)
    , flags(f)
{
}

QQuick3DObjectPrivate::QQuick3DObjectPrivate(Type t, Flags f)
    : QQuick3DObjectPrivate(t, FlagsT(f))
{
}

QQuick3DObjectPrivate::QQuick3DObjectPrivate(QQuick3DObjectPrivate::Type t)
    : QQuick3DObjectPrivate(t, Flags::None)
{
}

QQuick3DObjectPrivate::~QQuick3DObjectPrivate()
{
}

void QQuick3DObjectPrivate::init(QQuick3DObject *parent)
{
    Q_Q(QQuick3DObject);

    if (parent)
        q->setParentItem(parent);
}

/*!
    \qmlproperty list<Object> QtQuick3D::Object3D::data
    \qmldefault

    The data property allows you to freely mix Object3D children and resources
    in an object. If you assign a Object3D to the data list it becomes a child
    and if you assign any other object type, it is added as a resource.

    So you can write:
    \qml
    Object3D {
        Node {}
        DirectionalLight {}
        Timer {}
    }
    \endqml

    instead of:
    \qml
    Item {
        children: [
            Node {},
            DirectionalLight {}
        ]
        resources: [
            Timer {}
        ]
    }
    \endqml

    It should not generally be necessary to refer to the \c data property,
    as it is the default property for Object3D and thus all child objects are
    automatically assigned to this property.
 */

/*!
  \property QQuick3DObject::data
  \internal
*/
QQmlListProperty<QObject> QQuick3DObjectPrivate::data()
{
    return QQmlListProperty<QObject>(q_func(),
                                     nullptr,
                                     QQuick3DObjectPrivate::data_append,
                                     QQuick3DObjectPrivate::data_count,
                                     QQuick3DObjectPrivate::data_at,
                                     QQuick3DObjectPrivate::data_clear);
}

/*!
    \property QQuick3DObject::resources
    \internal
*/
QQmlListProperty<QObject> QQuick3DObjectPrivate::resources()
{
    return QQmlListProperty<QObject>(q_func(),
                                     nullptr,
                                     QQuick3DObjectPrivate::resources_append,
                                     QQuick3DObjectPrivate::resources_count,
                                     QQuick3DObjectPrivate::resources_at,
                                     QQuick3DObjectPrivate::resources_clear);
}

/*!
    \qmlproperty list<Object3D> QtQuick3D::Object3D::children
    \qmlproperty list<Object> QtQuick3D::Object3D::resources

    The children property contains the list of visual children of this object.
    The resources property contains non-visual resources that you want to
    reference by name.

    It is not generally necessary to refer to these properties when adding
    child objects or resources, as the default \l data property will
    automatically assign child objects to the \c children and \c resources
    properties as appropriate. See the \l QtQuick3D::Object3D::data
    documentation for details.

    \note QtQuick3D::Object3D::resources does not return a list of 3D resources
    despite the name. The name comes from the semantics of QQuickItem.  3D
    resources are subclasses of QQuickObjec3D and thus will be returned in the
    list of QtQuick3D::Objec3D::children.
*/
/*!
    \property QQuick3DObject::children
    \internal
*/
QQmlListProperty<QQuick3DObject> QQuick3DObjectPrivate::children()
{
    return QQmlListProperty<QQuick3DObject>(q_func(),
                                          nullptr,
                                          QQuick3DObjectPrivate::children_append,
                                          QQuick3DObjectPrivate::children_count,
                                          QQuick3DObjectPrivate::children_at,
                                          QQuick3DObjectPrivate::children_clear);
}

/*!
    \qmlproperty list<State> QtQuick3D::Object3D::states

    This property holds the list of possible states for this object. To change
    the state of this object, set the \l state property to one of these states,
    or set the \l state property to an empty string to revert the object to its
    default state.

    This property is specified as a list of \l State objects. For example,
    below is an QtQuick3D::Node with "above_state" and "below_state" states:

    \qml
    import QtQuick
    import QtQuick3D

    Node {
        id: root
        y: 0

        states: [
            State {
                name: "above_state"
                PropertyChanges { target: root; y: 100 }
            },
            State {
                name: "below_state"
                PropertyChanges { target: root; y: -100 }
            }
        ]
    }
    \endqml

    See \l{Qt Quick States} and \l{Animation and Transitions in Qt Quick} for
    more details on using states and transitions.

    \note This property works the same as QtQuick::Item::states but is
    necessary because QtQuick3D::Object3D is not a QtQuick::Item subclass.

    \sa QtQuick3D::Object3D::transitions
*/

/*!
    \property QQuick3DObject::states
    \internal
*/
QQmlListProperty<QQuickState> QQuick3DObjectPrivate::states()
{
    return _states()->statesProperty();
}

/*!
    \qmlproperty list<Transition> QtQuick3D::Object3D::transitions

    This property holds the list of transitions for this object. These define the
    transitions to be applied to the object whenever it changes its \l state.

    This property is specified as a list of \l Transition objects. For example:

    \qml
    import QtQuick
    import QtQuick3D

    Node {
        transitions: [
            Transition {
                //...
            },
            Transition {
                //...
            }
        ]
    }
    \endqml

    See \l{Qt Quick States} and \l{Animation and Transitions in Qt Quick} for
    more details on using states and transitions.

    \note This property works the same as QtQuick::Item::transitions but is
    necessary because QtQuick3D::Object3D is not a QtQuick::Item subclass.

    \sa QtQuick3D::Object3D::states
*/
/*!
    \property QQuick3DObject::transitions
    \internal
  */

QQmlListProperty<QQuickTransition> QQuick3DObjectPrivate::transitions()
{
    return _states()->transitionsProperty();
}

/*!
    \qmlproperty string QtQuick3D::Object3D::state

    This property holds the name of the current state of the object.

    If the item is in its default state, that is, no explicit state has been
    set, then this property holds an empty string. Likewise, you can return
    an item to its default state by setting this property to an empty string.

    \sa {Qt Quick States}
*/
/*!
    \property QQuick3DObject::state

    This property holds the name of the current state of the object.

    If the item is in its default state, that is, no explicit state has been
    set, then this property holds an empty string. Likewise, you can return
    an item to its default state by setting this property to an empty string.

    \sa {Qt Quick States}
*/

QString QQuick3DObjectPrivate::state() const
{
    return _stateGroup ? _stateGroup->state() : QString();
}

void QQuick3DObjectPrivate::setState(const QString &state)
{
    _states()->setState(state);
}

void QQuick3DObjectPrivate::data_append(QQmlListProperty<QObject> *prop, QObject *o)
{
    if (!o)
        return;

    QQuick3DObject *that = static_cast<QQuick3DObject *>(prop->object);
    QQuick3DObjectPrivate *privateItem = QQuick3DObjectPrivate::get(that);

    if (QQuick3DObject *item = qmlobject_cast<QQuick3DObject *>(o)) {
        item->setParentItem(that);

    } else {
        QQuickItem *quickItem = qobject_cast<QQuickItem *>(o);
        if (quickItem) {
            if (!privateItem->contentItem2d) {
                privateItem->contentItem2d = new QQuick3DItem2D(quickItem);
                privateItem->contentItem2d->setParent(that);
                privateItem->contentItem2d->setParentItem(that);
            } else {
                privateItem->contentItem2d->addChildItem(quickItem);
            }
            qmlobject_connect(privateItem->contentItem2d, QQuick3DItem2D, SIGNAL(allChildrenRemoved()),
                              that, QQuick3DObject, SLOT(_q_cleanupContentItem2D()));
        } else {
            o->setParent(that);
        }
    }
    resources_append(prop, o);
}

qsizetype QQuick3DObjectPrivate::data_count(QQmlListProperty<QObject> *property)
{
    QQuick3DObject *item = static_cast<QQuick3DObject *>(property->object);
    QQuick3DObjectPrivate *privateItem = QQuick3DObjectPrivate::get(item);
    QQmlListProperty<QObject> resourcesProperty = privateItem->resources();
    QQmlListProperty<QQuick3DObject> childrenProperty = privateItem->children();

    return resources_count(&resourcesProperty) + children_count(&childrenProperty);
}

QObject *QQuick3DObjectPrivate::data_at(QQmlListProperty<QObject> *property, qsizetype i)
{
    QQuick3DObject *item = static_cast<QQuick3DObject *>(property->object);
    QQuick3DObjectPrivate *privateItem = QQuick3DObjectPrivate::get(item);
    QQmlListProperty<QObject> resourcesProperty = privateItem->resources();
    QQmlListProperty<QQuick3DObject> childrenProperty = privateItem->children();

    int resourcesCount = resources_count(&resourcesProperty);
    if (i < resourcesCount)
        return resources_at(&resourcesProperty, i);
    const int j = i - resourcesCount;
    if (j < children_count(&childrenProperty))
        return children_at(&childrenProperty, j);
    return nullptr;
}

void QQuick3DObjectPrivate::data_clear(QQmlListProperty<QObject> *property)
{
    QQuick3DObject *item = static_cast<QQuick3DObject *>(property->object);
    QQuick3DObjectPrivate *privateItem = QQuick3DObjectPrivate::get(item);
    QQmlListProperty<QObject> resourcesProperty = privateItem->resources();
    QQmlListProperty<QQuick3DObject> childrenProperty = privateItem->children();

    resources_clear(&resourcesProperty);
    children_clear(&childrenProperty);
}

QObject *QQuick3DObjectPrivate::resources_at(QQmlListProperty<QObject> *prop, qsizetype index)
{
    QQuick3DObjectPrivate *quickItemPrivate = QQuick3DObjectPrivate::get(static_cast<QQuick3DObject *>(prop->object));
    return quickItemPrivate->extra.isAllocated() ? quickItemPrivate->extra->resourcesList.value(index) : 0;
}

void QQuick3DObjectPrivate::resources_append(QQmlListProperty<QObject> *prop, QObject *object)
{
    QQuick3DObject *quickItem = static_cast<QQuick3DObject *>(prop->object);
    QQuick3DObjectPrivate *quickItemPrivate = QQuick3DObjectPrivate::get(quickItem);
    if (!quickItemPrivate->extra.value().resourcesList.contains(object)) {
        quickItemPrivate->extra.value().resourcesList.append(object);
        // clang-format off
        qmlobject_connect(object, QObject, SIGNAL(destroyed(QObject*)),
                          quickItem, QQuick3DObject, SLOT(_q_resourceObjectDeleted(QObject*)));
        // clang-format on
    }
}

qsizetype QQuick3DObjectPrivate::resources_count(QQmlListProperty<QObject> *prop)
{
    QQuick3DObjectPrivate *quickItemPrivate = QQuick3DObjectPrivate::get(static_cast<QQuick3DObject *>(prop->object));
    return quickItemPrivate->extra.isAllocated() ? quickItemPrivate->extra->resourcesList.size() : 0;
}

void QQuick3DObjectPrivate::resources_clear(QQmlListProperty<QObject> *prop)
{
    QQuick3DObject *quickItem = static_cast<QQuick3DObject *>(prop->object);
    QQuick3DObjectPrivate *quickItemPrivate = QQuick3DObjectPrivate::get(quickItem);
    if (quickItemPrivate->extra.isAllocated()) { // If extra is not allocated resources is empty.
        for (QObject *object : std::as_const(quickItemPrivate->extra->resourcesList)) {
            // clang-format off
            qmlobject_disconnect(object, QObject, SIGNAL(destroyed(QObject*)),
                                 quickItem, QQuick3DObject, SLOT(_q_resourceObjectDeleted(QObject*)));
            // clang-format on
        }
        quickItemPrivate->extra->resourcesList.clear();
    }
}

void QQuick3DObjectPrivate::children_append(QQmlListProperty<QQuick3DObject> *prop, QQuick3DObject *o)
{
    if (!o)
        return;

    QQuick3DObject *that = static_cast<QQuick3DObject *>(prop->object);
    if (o->parentItem() == that)
        o->setParentItem(nullptr);

    o->setParentItem(that);
}

qsizetype QQuick3DObjectPrivate::children_count(QQmlListProperty<QQuick3DObject> *prop)
{
    QQuick3DObjectPrivate *p = QQuick3DObjectPrivate::get(static_cast<QQuick3DObject *>(prop->object));
    return p->childItems.size();
}

QQuick3DObject *QQuick3DObjectPrivate::children_at(QQmlListProperty<QQuick3DObject> *prop, qsizetype index)
{
    QQuick3DObjectPrivate *p = QQuick3DObjectPrivate::get(static_cast<QQuick3DObject *>(prop->object));
    if (index >= p->childItems.size() || index < 0)
        return nullptr;

    return p->childItems.at(index);
}

void QQuick3DObjectPrivate::children_clear(QQmlListProperty<QQuick3DObject> *prop)
{
    QQuick3DObject *that = static_cast<QQuick3DObject *>(prop->object);
    QQuick3DObjectPrivate *p = QQuick3DObjectPrivate::get(that);
    while (!p->childItems.isEmpty())
        p->childItems.at(0)->setParentItem(nullptr);
}

void QQuick3DObjectPrivate::_q_resourceObjectDeleted(QObject *object)
{
    if (extra.isAllocated() && extra->resourcesList.contains(object))
        extra->resourcesList.removeAll(object);
}

void QQuick3DObjectPrivate::_q_cleanupContentItem2D()
{
    // Only Schedule the item for deletion, as this may get called
    // as a result of teardown as well leading to a double delete
    if (contentItem2d)
        contentItem2d->deleteLater();
    contentItem2d = nullptr;
}

void QQuick3DObjectPrivate::addItemChangeListener(QQuick3DObjectChangeListener *listener, ChangeTypes types)
{
    changeListeners.append(ChangeListener(listener, types));
}

void QQuick3DObjectPrivate::updateOrAddItemChangeListener(QQuick3DObjectChangeListener *listener, ChangeTypes types)
{
    const ChangeListener changeListener(listener, types);
    const int index = changeListeners.indexOf(changeListener);
    if (index > -1)
        changeListeners[index].types = changeListener.types;
    else
        changeListeners.append(changeListener);
}

void QQuick3DObjectPrivate::removeItemChangeListener(QQuick3DObjectChangeListener *listener, ChangeTypes types)
{
    ChangeListener change(listener, types);
    changeListeners.removeOne(change);
}

QQuickStateGroup *QQuick3DObjectPrivate::_states()
{
    Q_Q(QQuick3DObject);
    if (!_stateGroup) {
        _stateGroup = new QQuickStateGroup;
        if (!componentComplete)
            _stateGroup->classBegin();
        // clang-format off
        qmlobject_connect(_stateGroup, QQuickStateGroup, SIGNAL(stateChanged(QString)),
                          q, QQuick3DObject, SIGNAL(stateChanged()));
        // clang-format on
    }

    return _stateGroup;
}

QString QQuick3DObjectPrivate::dirtyToString() const
{
#define DIRTY_TO_STRING(value)                                                                                         \
    if (dirtyAttributes & value) {                                                                                     \
        if (!rv.isEmpty())                                                                                             \
            rv.append(QLatin1Char('|'));                                                                               \
        rv.append(QLatin1String(#value));                                                                              \
    }

    //    QString rv = QLatin1String("0x") + QString::number(dirtyAttributes, 16);
    QString rv;

    DIRTY_TO_STRING(TransformOrigin);
    DIRTY_TO_STRING(Transform);
    DIRTY_TO_STRING(BasicTransform);
    DIRTY_TO_STRING(Position);
    DIRTY_TO_STRING(Size);
    DIRTY_TO_STRING(ZValue);
    DIRTY_TO_STRING(Content);
    DIRTY_TO_STRING(Smooth);
    DIRTY_TO_STRING(OpacityValue);
    DIRTY_TO_STRING(ChildrenChanged);
    DIRTY_TO_STRING(ChildrenStackingChanged);
    DIRTY_TO_STRING(ParentChanged);
    DIRTY_TO_STRING(Clip);
    DIRTY_TO_STRING(Window);
    DIRTY_TO_STRING(EffectReference);
    DIRTY_TO_STRING(Visible);
    DIRTY_TO_STRING(HideReference);
    DIRTY_TO_STRING(Antialiasing);
    DIRTY_TO_STRING(InstanceRootChanged);

    return rv;
}

void QQuick3DObjectPrivate::dirty(QQuick3DObjectPrivate::DirtyType type)
{
    // NOTE: Models that get an instance root has an "external" node that affects its transform
    // we therefore give models with an instance root a lower priority in the update list (See: addToDirtyList()).
    // For this to work we need to re-evaluate models priority when the instance root changes.
    if ((type & InstanceRootChanged) != 0)
        removeFromDirtyList();

    if (!(dirtyAttributes & type) || (sceneManager && !prevDirtyItem)) {
        dirtyAttributes |= type;
        if (sceneManager && componentComplete) {
            addToDirtyList();
        }
    }
}

void QQuick3DObjectPrivate::addToDirtyList()
{
    Q_Q(QQuick3DObject);

    Q_ASSERT(sceneManager);
    if (!prevDirtyItem) {
        Q_ASSERT(!nextDirtyItem);

        if (QSSGRenderGraphObject::isNodeType(type)) {
            // NOTE: We do special handling of models with an instance root (that is not itself...)
            // to ensure those models are processed after instance root nodes.
            const bool hasInstanceRoot = (type == Type::Model && static_cast<QQuick3DModel *>(q)->instanceRoot() && static_cast<QQuick3DModel *>(q)->instanceRoot() != q);
            const auto dirtyListIdx = !hasInstanceRoot ? QQuick3DSceneManager::nodeListIndex(type)
                                                       : size_t(QQuick3DSceneManager::NodePriority::ModelWithInstanceRoot);
            nextDirtyItem = sceneManager->dirtyNodes[dirtyListIdx];
            if (nextDirtyItem)
                QQuick3DObjectPrivate::get(nextDirtyItem)->prevDirtyItem = &nextDirtyItem;
            prevDirtyItem = &sceneManager->dirtyNodes[dirtyListIdx];
            sceneManager->dirtyNodes[dirtyListIdx] = q;
        } else if (QSSGRenderGraphObject::isExtension(type)) {
            const auto dirtyListIdx = QQuick3DSceneManager::extensionListIndex(type);
            nextDirtyItem = sceneManager->dirtyExtensions[dirtyListIdx];
            if (nextDirtyItem)
                QQuick3DObjectPrivate::get(nextDirtyItem)->prevDirtyItem = &nextDirtyItem;
            prevDirtyItem = &sceneManager->dirtyExtensions[dirtyListIdx];
            sceneManager->dirtyExtensions[dirtyListIdx] = q;
        } else {
            const auto dirtyListIdx = QQuick3DSceneManager::resourceListIndex(type);
            nextDirtyItem = sceneManager->dirtyResources[dirtyListIdx];
            if (nextDirtyItem)
                QQuick3DObjectPrivate::get(nextDirtyItem)->prevDirtyItem = &nextDirtyItem;
            prevDirtyItem = &sceneManager->dirtyResources[dirtyListIdx];
            sceneManager->dirtyResources[dirtyListIdx] = q;
        }
    }
    sceneManager->dirtyItem(q);
    Q_ASSERT(prevDirtyItem);
}

void QQuick3DObjectPrivate::removeFromDirtyList()
{
    if (prevDirtyItem) {
        if (nextDirtyItem)
            QQuick3DObjectPrivate::get(nextDirtyItem)->prevDirtyItem = prevDirtyItem;
        *prevDirtyItem = nextDirtyItem;
        prevDirtyItem = nullptr;
        nextDirtyItem = nullptr;
    }
    Q_ASSERT(!prevDirtyItem);
    Q_ASSERT(!nextDirtyItem);
}

void QQuick3DObjectPrivate::setCulled(bool cull)
{
    if (cull == culled)
        return;

    culled = cull;
    if ((cull && ++extra.value().hideRefCount == 1) || (!cull && --extra.value().hideRefCount == 0))
        dirty(HideReference);
}

void QQuick3DObjectPrivate::addChild(QQuick3DObject *child)
{
    Q_Q(QQuick3DObject);

    Q_ASSERT(!childItems.contains(child));

    childItems.append(child);

    dirty(QQuick3DObjectPrivate::ChildrenChanged);

    itemChange(QQuick3DObject::ItemChildAddedChange, child);

    emit q->childrenChanged();
}

void QQuick3DObjectPrivate::removeChild(QQuick3DObject *child)
{
    Q_Q(QQuick3DObject);

    Q_ASSERT(child);
    Q_ASSERT(childItems.contains(child));
    childItems.removeOne(child);
    Q_ASSERT(!childItems.contains(child));

    dirty(QQuick3DObjectPrivate::ChildrenChanged);

    itemChange(QQuick3DObject::ItemChildRemovedChange, child);

    emit q->childrenChanged();
}

void QQuick3DObjectPrivate::siblingOrderChanged()
{
    Q_Q(QQuick3DObject);
    if (!changeListeners.isEmpty()) {
        const auto listeners = changeListeners; // NOTE: intentional copy (QTBUG-54732)
        for (const QQuick3DObjectPrivate::ChangeListener &change : listeners) {
            if (change.types & QQuick3DObjectPrivate::SiblingOrder) {
                change.listener->itemSiblingOrderChanged(q);
            }
        }
    }
}

void QQuick3DObjectPrivate::refSceneManager(QQuick3DSceneManager &c)
{
    // An item needs a scene manager if it is referenced by another item which has a scene manager.
    // Typically the item is referenced by a parent, but can also be referenced by a
    // ShaderEffect or ShaderEffectSource. 'sceneRefCount' counts how many items with
    // a scene manager is referencing this item. When the reference count goes from zero to one,
    // or one to zero, the scene manager of this item is updated and propagated to the children.
    // As long as the reference count stays above zero, the scene manager is unchanged.
    // refSceneManager() increments the reference count.
    // derefSceneManager() decrements the reference count.

    Q_Q(QQuick3DObject);

    // Handle the case where the view has been deleted while the object lives on.
    if (sceneManager.isNull() && sceneRefCount == 1)
        sceneRefCount = 0;

    Q_ASSERT((sceneManager != nullptr) == (sceneRefCount > 0));
    if (++sceneRefCount > 1) {
        // Sanity check. Even if there's a different scene manager the window should be the same.
        if (c.window() != sceneManager->window()) {
            qWarning("QSSGObject: Cannot use same item on different windows at the same time.");
            return;
        }

        // NOTE: Simple tracking for resources that are shared between scenes.
        if (&c != sceneManager) {
            sharedResource = true;
            for (int ii = 0; ii < childItems.size(); ++ii) {
                QQuick3DObject *child = childItems.at(ii);
                QQuick3DObjectPrivate::get(child)->sharedResource = sharedResource;
                QQuick3DObjectPrivate::refSceneManager(child, c);
            }
        }

        return; // Scene manager already set.
    }

    Q_ASSERT(sceneManager == nullptr);
    sceneManager = &c;

    //    if (polishScheduled)
    //        QSSGWindowPrivate::get(window)->itemsToPolish.append(q);

    if (!parentItem)
        sceneManager->parentlessItems.insert(q);
    else
        sharedResource = QQuick3DObjectPrivate::get(parentItem)->sharedResource;

    for (int ii = 0; ii < childItems.size(); ++ii) {
        QQuick3DObject *child = childItems.at(ii);
        QQuick3DObjectPrivate::get(child)->sharedResource = sharedResource;
        QQuick3DObjectPrivate::refSceneManager(child, c);
    }

    dirty(Window);

    itemChange(QQuick3DObject::ItemSceneChange, &c);
}

void QQuick3DObjectPrivate::derefSceneManager()
{
    Q_Q(QQuick3DObject);

    if (!sceneManager)
        return; // This can happen when destroying recursive shader effect sources.

    if (--sceneRefCount > 0)
        return; // There are still other references, so don't set the scene manager to null yet.

    removeFromDirtyList();
    if (sceneManager)
        sceneManager->dirtyBoundingBoxList.removeAll(q);

    for (int ii = 0; ii < childItems.size(); ++ii) {
        QQuick3DObject *child = childItems.at(ii);
        QQuick3DObjectPrivate::derefSceneManager(child);
    }

    if (!parentItem)
        sceneManager->parentlessItems.remove(q);

    if (spatialNode) {
        sceneManager->cleanup(spatialNode);
        spatialNode = nullptr;
    }

    sceneManager = nullptr;

    dirty(Window);

    itemChange(QQuick3DObject::ItemSceneChange, sceneManager.data());
}

void QQuick3DObjectPrivate::updateSubFocusItem(QQuick3DObject *scope, bool focus)
{
    Q_Q(QQuick3DObject);
    Q_ASSERT(scope);

    QQuick3DObjectPrivate *scopePrivate = QQuick3DObjectPrivate::get(scope);

    QQuick3DObject *oldSubFocusItem = scopePrivate->subFocusItem;
    // Correct focus chain in scope
    if (oldSubFocusItem) {
        QQuick3DObject *sfi = scopePrivate->subFocusItem->parentItem();
        while (sfi && sfi != scope) {
            QQuick3DObjectPrivate::get(sfi)->subFocusItem = nullptr;
            sfi = sfi->parentItem();
        }
    }

    if (focus) {
        scopePrivate->subFocusItem = q;
        QQuick3DObject *sfi = scopePrivate->subFocusItem->parentItem();
        while (sfi && sfi != scope) {
            QQuick3DObjectPrivate::get(sfi)->subFocusItem = q;
            sfi = sfi->parentItem();
        }
    } else {
        scopePrivate->subFocusItem = nullptr;
    }
}

void QQuick3DObjectPrivate::itemChange(QQuick3DObject::ItemChange change, const QQuick3DObject::ItemChangeData &data)
{
    Q_Q(QQuick3DObject);
    switch (change) {
    case QQuick3DObject::ItemRotationHasChanged:
        // TODO:
        qWarning("ItemRoationHasChange is unhandled!!!!");
        break;
    case QQuick3DObject::ItemChildAddedChange: {
        q->itemChange(change, data);
        if (!changeListeners.isEmpty()) {
            const auto listeners = changeListeners; // NOTE: intentional copy (QTBUG-54732)
            for (const QQuick3DObjectPrivate::ChangeListener &change : listeners) {
                if (change.types & QQuick3DObjectPrivate::Children) {
                    change.listener->itemChildAdded(q, data.item);
                }
            }
        }
        break;
    }
    case QQuick3DObject::ItemChildRemovedChange: {
        q->itemChange(change, data);
        if (!changeListeners.isEmpty()) {
            const auto listeners = changeListeners; // NOTE: intentional copy (QTBUG-54732)
            for (const QQuick3DObjectPrivate::ChangeListener &change : listeners) {
                if (change.types & QQuick3DObjectPrivate::Children) {
                    change.listener->itemChildRemoved(q, data.item);
                }
            }
        }
        break;
    }
    case QQuick3DObject::ItemSceneChange:
        q->itemChange(change, data);
        break;
    case QQuick3DObject::ItemVisibleHasChanged: {
        q->itemChange(change, data);
        if (!changeListeners.isEmpty()) {
            const auto listeners = changeListeners; // NOTE: intentional copy (QTBUG-54732)
            for (const QQuick3DObjectPrivate::ChangeListener &change : listeners) {
                if (change.types & QQuick3DObjectPrivate::Visibility) {
                    change.listener->itemVisibilityChanged(q);
                }
            }
        }
        break;
    }
    case QQuick3DObject::ItemEnabledHasChanged: {
        q->itemChange(change, data);
        if (!changeListeners.isEmpty()) {
            const auto listeners = changeListeners; // NOTE: intentional copy (QTBUG-54732)
            for (const QQuick3DObjectPrivate::ChangeListener &change : listeners) {
                if (change.types & QQuick3DObjectPrivate::Enabled) {
                    change.listener->itemEnabledChanged(q);
                }
            }
        }
        break;
    }
    case QQuick3DObject::ItemParentHasChanged: {
        q->itemChange(change, data);
        if (!changeListeners.isEmpty()) {
            const auto listeners = changeListeners; // NOTE: intentional copy (QTBUG-54732)
            for (const QQuick3DObjectPrivate::ChangeListener &change : listeners) {
                if (change.types & QQuick3DObjectPrivate::Parent) {
                    change.listener->itemParentChanged(q, data.item);
                }
            }
        }
        break;
    }
    case QQuick3DObject::ItemOpacityHasChanged: {
        q->itemChange(change, data);
        if (!changeListeners.isEmpty()) {
            const auto listeners = changeListeners; // NOTE: intentional copy (QTBUG-54732)
            for (const QQuick3DObjectPrivate::ChangeListener &change : listeners) {
                if (change.types & QQuick3DObjectPrivate::Opacity) {
                    change.listener->itemOpacityChanged(q);
                }
            }
        }
        break;
    }
    case QQuick3DObject::ItemActiveFocusHasChanged:
        q->itemChange(change, data);
        break;
    case QQuick3DObject::ItemAntialiasingHasChanged:
        // fall through
    case QQuick3DObject::ItemDevicePixelRatioHasChanged:
        q->itemChange(change, data);
        break;
    }
}

namespace QV4 {
namespace Heap {
struct QSSGItemWrapper : public QObjectWrapper
{
    static void markObjects(QV4::Heap::Base *that, QV4::MarkStack *markStack);
};
}
}

struct QSSGItemWrapper : public QV4::QObjectWrapper
{
    V4_OBJECT2(QSSGItemWrapper, QV4::QObjectWrapper)
};

DEFINE_OBJECT_VTABLE(QSSGItemWrapper);

void QV4::Heap::QSSGItemWrapper::markObjects(QV4::Heap::Base *that, QV4::MarkStack *markStack)
{
    QObjectWrapper *This = static_cast<QObjectWrapper *>(that);
    if (QQuick3DObject *item = static_cast<QQuick3DObject *>(This->object())) {
        for (QQuick3DObject *child : std::as_const(QQuick3DObjectPrivate::get(item)->childItems))
            QV4::QObjectWrapper::markWrapper(child, markStack);
    }
    QObjectWrapper::markObjects(that, markStack);
}

quint64 QQuick3DObjectPrivate::_q_createJSWrapper(QQmlV4ExecutionEnginePtr engine)
{
    return (engine->memoryManager->allocate<QSSGItemWrapper>(q_func()))->asReturnedValue();
}

QQuick3DObjectPrivate::ExtraData::ExtraData() : hideRefCount(0) {}


QT_END_NAMESPACE

#include "moc_qquick3dobject.cpp"
