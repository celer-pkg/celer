// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3drepeater_p.h"

#include <private/qqmlglobal_p.h>
#include <private/qqmllistaccessor_p.h>
#include <private/qqmlchangeset_p.h>

#include <QtQml/QQmlInfo>

QT_BEGIN_NAMESPACE


/*!
    \qmltype Repeater3D
    \inqmlmodule QtQuick3D
    \inherits Node
    \brief Instantiates a number of Node-based components using a provided model.

    The Repeater3D type is used to create a large number of
    similar items. Like other view types, a Repeater3D has a \l model and a \l delegate:
    for each entry in the model, the delegate is instantiated
    in a context seeded with data from the model.

    A Repeater's \l model can be any of the supported \l {qml-data-models}{data models}.
    Additionally, like delegates for other views, a Repeater delegate can access
    its index within the repeater, as well as the model data relevant to the
    delegate. See the \l delegate property documentation for details.

    \note A Repeater3D item owns all items it instantiates. Removing or dynamically destroying
    an item created by a Repeater3D results in unpredictable behavior.

    \note Repeater3D is \l {Node}-based, and can only repeat \l {Node}-derived objects.
 */

/*!
    \qmlsignal QtQuick3D::Repeater3D::objectAdded(int index, Object3D object)

    This signal is emitted when an object is added to the repeater. The \a index
    parameter holds the index at which object has been inserted within the
    repeater, and the \a object parameter holds the \l Object3D that has been added.

    The corresponding handler is \c onObjectAdded.
*/

/*!
    \qmlsignal QtQuick3D::Repeater3D::objectRemoved(int index, Object3D object)

    This signal is emitted when an object is removed from the repeater. The \a index
    parameter holds the index at which the item was removed from the repeater,
    and the \a object parameter holds the \l Object3D that was removed.

    Do not keep a reference to \a object if it was created by this repeater, as
    in these cases it will be deleted shortly after the signal is handled.

    The corresponding handler is \c onObjectRemoved.
*/

QQuick3DRepeater::QQuick3DRepeater(QQuick3DNode *parent)
    : QQuick3DNode(parent)
    , m_model(nullptr)
    , m_itemCount(0)
    , m_ownModel(false)
    , m_delegateValidated(false)
    , m_explicitDelegate(false)
    , m_explicitDelegateModelAccess(false)
{
}

QQuick3DRepeater::~QQuick3DRepeater()
{
    if (m_ownModel) {
        delete m_model;
    } else {
        QQmlDelegateModelPointer model(m_model);
        disconnectModel(&model);
    }
}

void QQuick3DRepeater::connectModel(QQmlDelegateModelPointer *model)
{
    QQmlInstanceModel *instanceModel = model->instanceModel();
    if (!instanceModel)
        return;

    connect(instanceModel, &QQmlInstanceModel::modelUpdated,
            this, &QQuick3DRepeater::modelUpdated);
    connect(instanceModel, &QQmlInstanceModel::createdItem,
            this, &QQuick3DRepeater::createdObject);
    connect(instanceModel, &QQmlInstanceModel::initItem,
            this, &QQuick3DRepeater::initObject);
    if (QQmlDelegateModel *dataModel = model->delegateModel()) {
        QObject::connect(
                dataModel, &QQmlDelegateModel::delegateChanged,
                this, &QQuick3DRepeater::applyDelegateChange);
        if (m_ownModel) {
            QObject::connect(
                    dataModel, &QQmlDelegateModel::modelChanged,
                    this, &QQuick3DRepeater::modelChanged);
        }
    }

    regenerate();
}

void QQuick3DRepeater::disconnectModel(QQmlDelegateModelPointer *model)
{
    QQmlInstanceModel *instanceModel = model->instanceModel();
    if (!instanceModel)
        return;

    disconnect(instanceModel, &QQmlInstanceModel::modelUpdated,
               this, &QQuick3DRepeater::modelUpdated);
    disconnect(instanceModel, &QQmlInstanceModel::createdItem,
               this, &QQuick3DRepeater::createdObject);
    disconnect(instanceModel, &QQmlInstanceModel::initItem,
               this, &QQuick3DRepeater::initObject);
    if (QQmlDelegateModel *delegateModel = model->delegateModel()) {
        QObject::disconnect(
                delegateModel, &QQmlDelegateModel::delegateChanged,
                this, &QQuick3DRepeater::applyDelegateChange);
        if (m_ownModel) {
            QObject::disconnect(
                    delegateModel, &QQmlDelegateModel::modelChanged,
                    this, &QQuick3DRepeater::modelChanged);
        }
    }
}

/*!
    \qmlproperty any QtQuick3D::Repeater3D::model

    The model providing data for the repeater.

    This property can be set to any of the supported \l {qml-data-models}{data models}:

    \list
    \li A number that indicates the number of delegates to be created by the repeater
    \li A model (e.g. a ListModel item, or a QAbstractItemModel subclass)
    \li A string list
    \li An object list
    \endlist

    The type of model affects the properties that are exposed to the \l delegate.

    \sa {qml-data-models}{Data Models}
*/

QVariant QQuick3DRepeater::model() const
{
    if (m_ownModel)
        return static_cast<QQmlDelegateModel *>(m_model.data())->model();
    if (m_model)
        return QVariant::fromValue(m_model.data());
    return QVariant();
}

void QQuick3DRepeater::applyDelegateChange()
{
    if (m_explicitDelegate) {
        qmlWarning(this) << "Explicitly set delegate is externally overridden";
        m_explicitDelegate = false;
    }

    emit delegateChanged();
}

QQmlDelegateModel *QQuick3DRepeater::createDelegateModel()
{
    Q_ASSERT(m_model.isNull());
    QQmlDelegateModel *delegateModel = new QQmlDelegateModel(qmlContext(this), this);
    m_model = delegateModel;
    m_ownModel = true;
    if (isComponentComplete())
        delegateModel->componentComplete();
    return delegateModel;
}

void QQuick3DRepeater::setModel(const QVariant &m)
{
    QVariant model = m;
    if (model.userType() == qMetaTypeId<QJSValue>())
        model = model.value<QJSValue>().toVariant();

    QQmlDelegateModelPointer oldModel(m_model);
    if (m_ownModel) {
        if (oldModel.delegateModel()->model() == model)
            return;
    } else if (QVariant::fromValue(m_model) == model) {
        return;
    }

    clear();

    disconnectModel(&oldModel);

    m_model = nullptr;

    QObject *object = qvariant_cast<QObject *>(model);

    QQmlDelegateModelPointer newModel(qobject_cast<QQmlInstanceModel *>(object));
    if (newModel) {
        if (m_explicitDelegate) {
            QQmlComponent *delegate = nullptr;
            if (QQmlDelegateModel *old = oldModel.delegateModel())
                delegate = old->delegate();
            if (QQmlDelegateModel *delegateModel = newModel.delegateModel()) {
                delegateModel->setDelegate(delegate);
            } else if (delegate) {
                qmlWarning(this) << "Cannot retain explicitly set delegate on non-DelegateModel";
                m_explicitDelegate = false;
            }
        }
        if (m_ownModel) {
            delete oldModel.instanceModel();
            m_ownModel = false;
        }
        m_model = newModel.instanceModel();
    } else if (m_ownModel) {
        // m_ownModel can only be set if the old model is a QQmlDelegateModel.
        Q_ASSERT(oldModel.delegateModel());
        newModel = oldModel;
        m_model = newModel.instanceModel();
        newModel.delegateModel()->setModel(model);
    } else {
        newModel = createDelegateModel();
        if (m_explicitDelegate) {
            QQmlComponent *delegate = nullptr;
            if (QQmlDelegateModel *old = oldModel.delegateModel())
                delegate = old->delegate();
            newModel.delegateModel()->setDelegate(delegate);
        }

        newModel.delegateModel()->setModel(model);
    }

    connectModel(&newModel);

    emit modelChanged();
    emit countChanged();
}

/*!
    \qmlproperty Component QtQuick3D::Repeater3D::delegate
    \qmldefault

    The delegate provides a template defining each object instantiated by the repeater.

    Delegates are exposed to a read-only \c index property that indicates the index
    of the delegate within the repeater.

    If the \l model is a model object (such as a \l ListModel) the delegate
    can access all model roles as named properties, in the same way that delegates
    do for view classes like ListView.

    \sa {QML Data Models}
 */

QQmlComponent *QQuick3DRepeater::delegate() const
{
    if (m_model) {
        if (QQmlDelegateModel *dataModel = qobject_cast<QQmlDelegateModel*>(m_model))
            return dataModel->delegate();
    }

    return nullptr;
}

void QQuick3DRepeater::setDelegate(QQmlComponent *delegate)
{
    const auto setExplicitDelegate = [&](QQmlDelegateModel *delegateModel) {
        if (delegateModel->delegate() == delegate) {
            m_explicitDelegate = true;
            return;
        }

        const int oldCount = delegateModel->count();
        delegateModel->setDelegate(delegate);
        regenerate();
        if (oldCount != delegateModel->count())
            emit countChanged();
        m_explicitDelegate = true;
        m_delegateValidated = false;
    };

    if (!m_model) {
        if (!delegate) {
            // Explicitly set a null delegate. We can do this without model.
            m_explicitDelegate = true;
            return;
        }

        setExplicitDelegate(createDelegateModel());
        // The new model is not connected to applyDelegateChange, yet. We only do this once
        // there is actual data, via an explicit setModel(). So we have to manually emit the
        // delegateChanged() here.
        emit delegateChanged();
        return;
    }

    if (QQmlDelegateModel *delegateModel = qobject_cast<QQmlDelegateModel *>(m_model)) {
        // Disable the warning in applyDelegateChange since the new delegate is also explicit.
        m_explicitDelegate = false;
        setExplicitDelegate(delegateModel);
        return;
    }

    if (delegate)
        qmlWarning(this) << "Cannot set a delegate on an explicitly provided non-DelegateModel";
    else
        m_explicitDelegate = true; // Explicitly set null delegate always works
}

/*!
    \qmlproperty int QtQuick3D::Repeater3D::count
    \readonly

    This property holds the number of items in the model.

    \note The number of items in the model as reported by count may differ from
    the number of created delegates if the Repeater3D is in the process of
    instantiating delegates or is incorrectly set up.
*/

int QQuick3DRepeater::count() const
{
    if (m_model)
        return m_model->count();
    return 0;
}

/*!
    \qmlmethod Object3D QtQuick3D::Repeater3D::objectAt(index)

    Returns the \l Object3D that has been created at the given \a index, or \c null
    if no item exists at \a index.
*/

QQuick3DObject *QQuick3DRepeater::objectAt(int index) const
{
    if (index >= 0 && index < m_deletables.size())
        return m_deletables[index];
    return nullptr;
}

/*!
    \qmlproperty enumeration QtQuick3D::Repeater3D::delegateModelAccess
    \since 6.10

    This property determines how delegates can access the model.

    \value DelegateModel.ReadOnly
        Prohibit delegates from writing the model via either context properties,
        the \c model object, or required properties.

    \value DelegateModel.ReadWrite
        Allow delegates to write the model via either context properties,
        the \c model object, or required properties.

    \value DelegateModel.Qt5ReadWrite
        Allow delegates to write the model via the \c model object and context
        properties but \e not via required properties.

    The default is \c DelegateModel.Qt5ReadWrite.

    \sa {Models and Views in Qt Quick#Changing Model Data}
*/
QQmlDelegateModel::DelegateModelAccess QQuick3DRepeater::delegateModelAccess() const
{
    if (QQmlDelegateModel *dataModel = qobject_cast<QQmlDelegateModel *>(m_model))
        return dataModel->delegateModelAccess();
    return QQmlDelegateModel::Qt5ReadWrite;
}

void QQuick3DRepeater::setDelegateModelAccess(QQmlDelegateModel::DelegateModelAccess delegateModelAccess)
{
    const auto setExplicitDelegateModelAccess = [&](QQmlDelegateModel *delegateModel) {
        delegateModel->setDelegateModelAccess(delegateModelAccess);
        m_explicitDelegateModelAccess = true;
    };

    if (!m_model) {
        if (delegateModelAccess == QQmlDelegateModel::Qt5ReadWrite) {
            // Explicitly set delegateModelAccess to Legacy. We can do this without model.
            m_explicitDelegateModelAccess = true;
            return;
        }

        QQmlDelegateModel *delegateModel = new QQmlDelegateModel(qmlContext(this), this);
        m_model = delegateModel;
        m_ownModel = true;
        if (isComponentComplete())
            delegateModel->componentComplete();

        setExplicitDelegateModelAccess(delegateModel);

        // The new model is not connected to applyDelegateModelAccessChange, yet. We only do this
        // once there is actual data, via an explicit setModel(). So we have to manually emit the
        // delegateModelAccessChanged() here.
        emit delegateModelAccessChanged();
        return;
    }

    if (QQmlDelegateModel *delegateModel = qobject_cast<QQmlDelegateModel *>(m_model)) {
        // Disable the warning in applyDelegateModelAccessChange since the new delegate model
        // access is also explicit.
        m_explicitDelegateModelAccess = false;
        setExplicitDelegateModelAccess(delegateModel);
        return;
    }

    if (delegateModelAccess == QQmlDelegateModel::Qt5ReadWrite) {
        m_explicitDelegateModelAccess = true; // Explicitly set null delegate always works
    } else {
        qmlWarning(this) << "Cannot set a delegateModelAccess on an explicitly provided "
                            "non-DelegateModel";
    }
}

void QQuick3DRepeater::clear()
{
    bool complete = isComponentComplete();

    if (m_model) {
        // We remove in reverse order deliberately; so that signals are emitted
        // with sensible indices.
        for (int i = m_deletables.size() - 1; i >= 0; --i) {
            if (QQuick3DObject *item = m_deletables.at(i)) {
                if (complete)
                    emit objectRemoved(i, item);
                m_model->release(item);
            }
        }
        for (QQuick3DObject *item : std::as_const(m_deletables)) {
            if (item)
                item->setParentItem(nullptr);
        }
    }
    m_deletables.clear();
    m_itemCount = 0;
}

void QQuick3DRepeater::regenerate()
{
    if (!isComponentComplete())
        return;

    clear();

    if (!m_model || !m_model->count() || !m_model->isValid() || !parentItem() || !isComponentComplete())
        return;

    m_itemCount = count();
    m_deletables.resize(m_itemCount);
    requestItems();
}

void QQuick3DRepeater::componentComplete()
{
    if (m_model && m_ownModel)
        static_cast<QQmlDelegateModel *>(m_model.data())->componentComplete();
    QQuick3DNode::componentComplete();
    regenerate();
    if (m_model && m_model->count())
        emit countChanged();
}

void QQuick3DRepeater::itemChange(QQuick3DObject::ItemChange change, const QQuick3DObject::ItemChangeData &value)
{
    QQuick3DObject::itemChange(change, value);
    if (change == ItemParentHasChanged) {
        regenerate();
    }
}

void QQuick3DRepeater::createdObject(int index, QObject *)
{
    QObject *object = m_model->object(index, QQmlIncubator::AsynchronousIfNested);
    QQuick3DObject *item = qmlobject_cast<QQuick3DObject*>(object);
    emit objectAdded(index, item);
}

void QQuick3DRepeater::initObject(int index, QObject *object)
{
    QQuick3DNode *item = qmlobject_cast<QQuick3DNode*>(object);

    if (!m_deletables.at(index)) {
        if (!item) {
            if (object) {
                m_model->release(object);
                if (!m_delegateValidated) {
                    m_delegateValidated = true;
                    QObject* delegate = this->delegate();
                    qmlWarning(delegate ? delegate : this) << QQuick3DRepeater::tr("Delegate must be of Node type");
                }
            }
            return;
        }
        m_deletables[index] = item;
        item->setParent(this);
        item->setParentItem(static_cast<QQuick3DNode*>(this));
        initDelegate(index, item);
    }
}

void QQuick3DRepeater::modelUpdated(const QQmlChangeSet &changeSet, bool reset)
{
    if (!isComponentComplete())
        return;

    if (reset) {
        regenerate();
        if (changeSet.difference() != 0)
            emit countChanged();
        return;
    }

    int difference = 0;
    QHash<int, QVector<QPointer<QQuick3DNode> > > moved;
    for (const QQmlChangeSet::Change &remove : changeSet.removes()) {
        int index = qMin(remove.index, m_deletables.size());
        int count = qMin(remove.index + remove.count, m_deletables.size()) - index;
        if (remove.isMove()) {
            moved.insert(remove.moveId, m_deletables.mid(index, count));
            m_deletables.erase(
                    m_deletables.begin() + index,
                    m_deletables.begin() + index + count);
        } else while (count--) {
            QQuick3DNode *item = m_deletables.at(index);
            m_deletables.remove(index);
            emit objectRemoved(index, item);
            if (item) {
                m_model->release(item);
                item->setParentItem(nullptr);
            }
            --m_itemCount;
        }

        difference -= remove.count;
    }

    for (const QQmlChangeSet::Change &insert : changeSet.inserts()) {
        int index = qMin(insert.index, m_deletables.size());
        if (insert.isMove()) {
            QVector<QPointer<QQuick3DNode> > items = moved.value(insert.moveId);
            m_deletables = m_deletables.mid(0, index) + items + m_deletables.mid(index);
        } else for (int i = 0; i < insert.count; ++i) {
            int modelIndex = index + i;
            ++m_itemCount;
            m_deletables.insert(modelIndex, nullptr);
            QObject *object = m_model->object(modelIndex, QQmlIncubator::AsynchronousIfNested);
            if (object)
                m_model->release(object);
        }
        difference += insert.count;
    }

    if (difference != 0)
        emit countChanged();
}

void QQuick3DRepeater::requestItems()
{
    for (int i = 0; i < m_itemCount; i++) {
        QObject *object = m_model->object(i, QQmlIncubator::AsynchronousIfNested);
        if (object)
            m_model->release(object);
    }
}

QT_END_NAMESPACE

#include "moc_qquick3drepeater_p.cpp"
