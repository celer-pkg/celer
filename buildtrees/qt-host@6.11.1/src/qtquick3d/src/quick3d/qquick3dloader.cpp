// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dloader_p.h"

#include "qquick3dobject_p.h"

#include <QtQml/qqmlinfo.h>

#include <private/qqmlengine_p.h>
#include <private/qqmlglobal_p.h>

#include <private/qqmlcomponent_p.h>
#include <private/qqmlincubator_p.h>

QT_BEGIN_NAMESPACE

void QQuick3DLoaderIncubator::statusChanged(QQmlIncubator::Status status)
{
    m_loader->incubatorStateChanged(status);
}

void QQuick3DLoaderIncubator::setInitialState(QObject *o)
{
    m_loader->setInitialState(o);
}

/*!
    \qmltype Loader3D
    \inqmlmodule QtQuick3D
    \inherits Node

    \brief Allows dynamic loading of a 3D subtree from a URL or Component.

    Loader3D is used to dynamically load QML components for Qt Quick 3D.

    Loader3D can load a
    QML file (using the \l source property) or a \l Component object (using
    the \l sourceComponent property). It is useful for delaying the creation
    of a component until it is required: for example, when a component should
    be created on demand, or when a component should not be created
    unnecessarily for performance reasons.

    \note Loader3D works the same way as \l Loader. The difference between the
    two is that \l Loader provides a way to dynamically load objects that inherit
    \l Item, whereas Loader3D provides a way to load objects that inherit \l Object3D
    and is part of a 3D scene.
*/

QQuick3DLoader::QQuick3DLoader(QQuick3DNode *parent)
    : QQuick3DNode(parent)
    , m_item(nullptr)
    , m_object(nullptr)
    , m_itemContext(nullptr)
    , m_incubator(nullptr)
    , m_active(true)
    , m_loadingFromSource(false)
    , m_asynchronous(false)
{
}

QQuick3DLoader::~QQuick3DLoader()
{
    clear();
    delete m_incubator;
    m_incubator = nullptr;
}

/*!
    \qmlproperty bool QtQuick3D::Loader3D::active
    This property is \c true if the Loader3D is currently active.
    The default value for this property is \c true.

    If the Loader3D is inactive, changing the \l source or \l sourceComponent
    will not cause the item to be instantiated until the Loader3D is made active.

    Setting the value to inactive will cause any \l item loaded by the loader
    to be released, but will not affect the \l source or \l sourceComponent.

    The \l status of an inactive loader is always \c Null.

    \sa source, sourceComponent
 */

bool QQuick3DLoader::active() const
{
    return m_active;
}

void QQuick3DLoader::setActive(bool newVal)
{
    if (m_active == newVal)
        return;

    m_active = newVal;
    if (newVal) {
        if (m_loadingFromSource) {
            loadFromSource();
        } else {
            loadFromSourceComponent();
        }
    } else {
        // cancel any current incubation
        if (m_incubator) {
            m_incubator->clear();
            delete m_itemContext;
            m_itemContext = nullptr;
        }

        // Prevent any bindings from running while waiting for deletion. Without
        // this we may get transient errors from use of 'parent', for example.
        QQmlContext *context = qmlContext(m_object);
        if (context)
            QQmlContextData::get(context)->clearContextRecursively();

        if (m_item) {
            // We can't delete immediately because our item may have triggered
            // the Loader to load a different item.
            m_item->setParentItem(nullptr);
            m_item->setVisible(false);
            m_item = nullptr;
        }
        if (m_object) {
            m_object->deleteLater();
            m_object = nullptr;
            emit itemChanged();
        }
        emit statusChanged();
    }
    emit activeChanged();
}

void QQuick3DLoader::setSource(QQmlV4FunctionPtr args)
{
    bool ipvError = false;
    args->setReturnValue(QV4::Encode::undefined());
    QV4::Scope scope(args->v4engine());
    QV4::ScopedValue ipv(scope, extractInitialPropertyValues(args, &ipvError));
    if (ipvError)
        return;

    clear();
    QUrl sourceUrl = resolveSourceUrl(args);
    if (!ipv->isUndefined()) {
        disposeInitialPropertyValues();
        m_initialPropertyValues.set(args->v4engine(), ipv);
    }
    m_qmlCallingContext.set(scope.engine, scope.engine->qmlContext());

    setSource(sourceUrl, false); // already cleared and set ipv above.
}

/*!
    \qmlproperty url QtQuick3D::Loader3D::source
    This property holds the URL of the QML component to instantiate.

    To unload the currently loaded object, set this property to an empty string,
    or set \l sourceComponent to \c undefined. Setting \c source to a
    new URL will also cause the item created by the previous URL to be unloaded.

    \sa sourceComponent, status, progress
*/

QUrl QQuick3DLoader::source() const
{
    return m_source;
}

void QQuick3DLoader::setSource(const QUrl &url)
{
    setSource(url, true);
}

/*!
    \qmlproperty Component QtQuick3D::Loader3D::sourceComponent
    This property holds the \l{Component} to instantiate.

    \qml
    Item {
        Component {
            id: redCube
            Model {
                source: "#Cube"
                materials: DefaultMaterial {
                    diffuseColor: "red"
                }
            }
        }

        Loader3D { sourceComponent: redCube }
        Loader3D { sourceComponent: redCube; x: 10 }
    }
    \endqml

    To unload the currently loaded object, set this property to \c undefined.

    \sa source, progress
*/

/*!
    \qmlmethod object QtQuick3D::Loader3D::setSource(url source, object properties)

    Creates an object instance of the given \a source component that will have
    the given \a properties. The \a properties argument is optional.  The instance
    will be accessible via the \l item property once loading and instantiation
    is complete.

    If the \l active property is \c false at the time when this function is called,
    the given \a source component will not be loaded but the \a source and initial
    \a properties will be cached.  When the loader is made \l active, an instance of
    the \a source component will be created with the initial \a properties set.

    Setting the initial property values of an instance of a component in this manner
    will \b{not} trigger any associated \l{Behavior}s.

    Note that the cached \a properties will be cleared if the \l source or \l sourceComponent
    is changed after calling this function but prior to setting the loader \l active.

    \sa source, active
*/

QQmlComponent *QQuick3DLoader::sourceComponent() const
{
    return m_component;
}

void QQuick3DLoader::setSourceComponent(QQmlComponent *comp)
{
    if (comp == m_component)
        return;

    clear();

    m_component.setObject(comp, this);
    m_loadingFromSource = false;

    if (m_active)
        loadFromSourceComponent();
    else
        emit sourceComponentChanged();
}

void QQuick3DLoader::resetSourceComponent()
{
    setSourceComponent(nullptr);
}

/*!
    \qmlproperty enumeration QtQuick3D::Loader3D::status
    \readonly

    This property holds the status of QML loading.  It can be one of:

    \value Loader3D.Null The loader is inactive or no QML source has been set.
    \value Loader3D.Ready The QML source has been loaded.
    \value Loader3D.Loading The QML source is currently being loaded.
    \value Loader3D.Error An error occurred while loading the QML source.

    Use this status to provide an update or respond to the status change in some way.
    For example, you could:

    \list
    \li Trigger a state change:
    \qml
        State { name: 'loaded'; when: loader.status == Loader3D.Ready }
    \endqml

    \li Implement an \c onStatusChanged signal handler:
    \qml
        Loader3D {
            id: loader
            onStatusChanged: if (loader.status == Loader3D.Ready) console.log('Loaded')
        }
    \endqml

    \li Bind to the status value:
    \qml
        Text { text: loader.status == Loader3D.Ready ? 'Loaded' : 'Not loaded' }
    \endqml
    \endlist

    Note that if the source is a local file, the status will initially be Ready (or Error). While
    there will be no onStatusChanged signal in that case, the onLoaded will still be invoked.

    \sa progress
*/

QQuick3DLoader::Status QQuick3DLoader::status() const
{
    if (!m_active)
        return Null;

    if (m_component) {
        switch (m_component->status()) {
        case QQmlComponent::Loading:
            return Loading;
        case QQmlComponent::Error:
            return Error;
        case QQmlComponent::Null:
            return Null;
        default:
            break;
        }
    }

    if (m_incubator) {
        switch (m_incubator->status()) {
        case QQmlIncubator::Loading:
            return Loading;
        case QQmlIncubator::Error:
            return Error;
        default:
            break;
        }
    }

    if (m_object)
        return Ready;

    return m_source.isEmpty() ? Null : Error;
}

/*!
    \qmlsignal QtQuick3D::Loader3D::loaded()

    This signal is emitted when the \l status becomes \c Loader3D.Ready, or on successful
    initial load.

    The corresponding handler is \c onLoaded.
*/


/*!
    \qmlproperty real QtQuick3D::Loader3D::progress
    \readonly

    This property holds the progress of loading QML data from the network, from
    0.0 (nothing loaded) to 1.0 (finished).  Most QML files are quite small, so
    this value will rapidly change from 0 to 1.

    \sa status
*/

qreal QQuick3DLoader::progress() const
{

    if (m_object)
        return 1.0;

    if (m_component)
        return m_component->progress();

    return 0.0;
}

/*!
\qmlproperty bool QtQuick3D::Loader3D::asynchronous

This property holds whether the component will be instantiated asynchronously.
By default it is \c false.

When used in conjunction with the \l source property, loading and compilation
will also be performed in a background thread.

Loading asynchronously creates the objects declared by the component
across multiple frames, and reduces the
likelihood of glitches in animation.  When loading asynchronously the status
will change to Loader3D.Loading.  Once the entire component has been created, the
\l item will be available and the status will change to Loader.Ready.

Changing the value of this property to \c false while an asynchronous load is in
progress will force immediate, synchronous completion.  This allows beginning an
asynchronous load and then forcing completion if the Loader3D content must be
accessed before the asynchronous load has completed.

To avoid seeing the items loading progressively set \c visible appropriately, e.g.

\code
Loader3D {
    source: "mycomponent.qml"
    asynchronous: true
    visible: status == Loader3D.Ready
}
\endcode

Note that this property affects object instantiation only; it is unrelated to
loading a component asynchronously via a network.
*/

bool QQuick3DLoader::asynchronous() const
{
    return m_asynchronous;
}

void QQuick3DLoader::setAsynchronous(bool a)
{
    if (m_asynchronous == a)
        return;

    m_asynchronous = a;

    if (!m_asynchronous && isComponentComplete() && m_active) {
        if (m_loadingFromSource && m_component && m_component->isLoading()) {
            // Force a synchronous component load
            QUrl currentSource = m_source;
            clear();
            m_source = currentSource;
            loadFromSource();
        } else if (m_incubator && m_incubator->isLoading()) {
            m_incubator->forceCompletion();
        }
    }

    emit asynchronousChanged();
}

/*!
    \qmlproperty object QtQuick3D::Loader3D::item
    \readonly
    This property holds the top-level object that is currently loaded.
*/
QObject *QQuick3DLoader::item() const
{
    return m_object;
}

void QQuick3DLoader::componentComplete()
{
    QQuick3DNode::componentComplete();
    if (active()) {
        if (m_loadingFromSource)
            createComponent();
        load();
    }
}

void QQuick3DLoader::sourceLoaded()
{
    if (!m_component || !m_component->errors().isEmpty()) {
        if (m_component)
            QQmlEnginePrivate::warning(qmlEngine(this), m_component->errors());
        if (m_loadingFromSource)
            emit sourceChanged();
        else
            emit sourceComponentChanged();
        emit statusChanged();
        emit progressChanged();
        emit itemChanged(); //Like clearing source, emit itemChanged even if previous item was also null
        disposeInitialPropertyValues(); // cleanup
        return;
    }

    QQmlContext *creationContext = m_component->creationContext();
    if (!creationContext)
        creationContext = qmlContext(this);

    QQmlComponentPrivate *cp = QQmlComponentPrivate::get(m_component);
    QQmlContext *context = [&](){
        if (cp->isBound())
            return creationContext;
        m_itemContext = new QQmlContext(creationContext);
        m_itemContext->setContextObject(this);
        return m_itemContext;
    }();

    delete m_incubator;
    m_incubator = new QQuick3DLoaderIncubator(this, m_asynchronous ? QQmlIncubator::Asynchronous : QQmlIncubator::AsynchronousIfNested);

    m_component->create(*m_incubator, context);

    if (m_incubator && m_incubator->status() == QQmlIncubator::Loading)
        emit statusChanged();
}

void QQuick3DLoader::setSource(const QUrl &sourceUrl, bool needsClear)
{
    if (m_source == sourceUrl)
        return;

    if (needsClear)
        clear();

    m_source = sourceUrl;
    m_loadingFromSource = true;

    if (m_active)
        loadFromSource();
    else
        emit sourceChanged();
}

void QQuick3DLoader::loadFromSource()
{
    if (m_source.isEmpty()) {
        emit sourceChanged();
        emit statusChanged();
        emit progressChanged();
        emit itemChanged();
        return;
    }

    if (isComponentComplete()) {
        if (!m_component)
            createComponent();
        load();
    }
}

void QQuick3DLoader::loadFromSourceComponent()
{
    if (!m_component) {
        emit sourceComponentChanged();
        emit statusChanged();
        emit progressChanged();
        emit itemChanged();
        return;
    }

    if (isComponentComplete())
        load();
}

void QQuick3DLoader::clear()
{
    disposeInitialPropertyValues();

    if (m_incubator)
        m_incubator->clear();

    delete m_itemContext;
    m_itemContext = nullptr;

    // Prevent any bindings from running while waiting for deletion. Without
    // this we may get transient errors from use of 'parent', for example.
    QQmlContext *context = qmlContext(m_object);
    if (context)
        QQmlContextData::get(context)->clearContextRecursively();

    if (m_loadingFromSource && m_component) {
        // disconnect since we deleteLater
        QObject::disconnect(m_component, SIGNAL(statusChanged(QQmlComponent::Status)),
                this, SLOT(sourceLoaded()));
        QObject::disconnect(m_component, SIGNAL(progressChanged(qreal)),
                this, SIGNAL(progressChanged()));
        m_component->deleteLater();
        m_component.setObject(nullptr, this);
    } else if (m_component) {
        m_component.setObject(nullptr, this);
    }
    m_source = QUrl();

    if (m_item) {
        // We can't delete immediately because our item may have triggered
        // the Loader to load a different item.
        m_item->setParentItem(nullptr);
        m_item->setVisible(false);
        m_item = nullptr;
    }
    if (m_object) {
        m_object->deleteLater();
        m_object = nullptr;
    }
}

void QQuick3DLoader::load()
{

    if (!isComponentComplete() || !m_component)
        return;

    if (!m_component->isLoading()) {
        sourceLoaded();
    } else {
        QObject::connect(m_component, SIGNAL(statusChanged(QQmlComponent::Status)),
                this, SLOT(sourceLoaded()));
        QObject::connect(m_component, SIGNAL(progressChanged(qreal)),
                this, SIGNAL(progressChanged()));
        emit statusChanged();
        emit progressChanged();
        if (m_loadingFromSource)
            emit sourceChanged();
        else
            emit sourceComponentChanged();
        emit itemChanged();
    }
}

void QQuick3DLoader::incubatorStateChanged(QQmlIncubator::Status status)
{
    if (status == QQmlIncubator::Loading || status == QQmlIncubator::Null)
        return;

    if (status == QQmlIncubator::Ready) {
        m_object = m_incubator->object();
        m_item = qmlobject_cast<QQuick3DNode*>(m_object);
        emit itemChanged();
        m_incubator->clear();
    } else if (status == QQmlIncubator::Error) {
        if (!m_incubator->errors().isEmpty())
            QQmlEnginePrivate::warning(qmlEngine(this), m_incubator->errors());
        delete m_itemContext;
        m_itemContext = nullptr;
        delete m_incubator->object();
        m_source = QUrl();
        emit itemChanged();
    }
    if (m_loadingFromSource)
        emit sourceChanged();
    else
        emit sourceComponentChanged();
    emit statusChanged();
    emit progressChanged();
    if (status == QQmlIncubator::Ready)
        emit loaded();
    disposeInitialPropertyValues(); // cleanup
}

void QQuick3DLoader::setInitialState(QObject *obj)
{
    QQuick3DObject *item = qmlobject_cast<QQuick3DObject*>(obj);
    if (item) {
        item->setParentItem(this);
    }
    if (obj) {
        if (m_itemContext)
            QQml_setParent_noEvent(m_itemContext, obj);
        QQml_setParent_noEvent(obj, this);
        m_itemContext = nullptr;
    }

    if (m_initialPropertyValues.isUndefined())
        return;

    QQmlComponentPrivate *d = QQmlComponentPrivate::get(m_component);
    Q_ASSERT(d && d->engine());
    QV4::ExecutionEngine *v4 = d->engine()->handle();
    Q_ASSERT(v4);
    QV4::Scope scope(v4);
    QV4::ScopedValue ipv(scope, m_initialPropertyValues.value());
    QV4::Scoped<QV4::QmlContext> qmlContext(scope, m_qmlCallingContext.value());
    d->initializeObjectWithInitialProperties(qmlContext, ipv, obj, QQmlIncubatorPrivate::get(m_incubator)->requiredProperties());
}

void QQuick3DLoader::disposeInitialPropertyValues()
{

}

QUrl QQuick3DLoader::resolveSourceUrl(QQmlV4FunctionPtr args)
{
    QV4::Scope scope(args->v4engine());
    QV4::ScopedValue v(scope, (*args)[0]);
    QString arg = v->toQString();
    if (arg.isEmpty())
        return QUrl();

    auto context = scope.engine->callingQmlContext();
    Q_ASSERT(!context.isNull());
    return context->resolvedUrl(QUrl(arg));
}

QV4::ReturnedValue QQuick3DLoader::extractInitialPropertyValues(QQmlV4FunctionPtr args, bool *error)
{
    QV4::Scope scope(args->v4engine());
    QV4::ScopedValue valuemap(scope, QV4::Encode::undefined());
    if (args->length() >= 2) {
        QV4::ScopedValue v(scope, (*args)[1]);
        if (!v->isObject() || v->as<QV4::ArrayObject>()) {
            *error = true;
            qmlWarning(this) << QQuick3DLoader::tr("setSource: value is not an object");
        } else {
            *error = false;
            valuemap = v;
        }
    }

    return valuemap->asReturnedValue();
}

void QQuick3DLoader::createComponent()
{
    const QQmlComponent::CompilationMode mode = m_asynchronous
            ? QQmlComponent::Asynchronous
            : QQmlComponent::PreferSynchronous;
    QQmlContext *context = qmlContext(this);
    m_component.setObject(new QQmlComponent(context->engine(),
                                            context->resolvedUrl(m_source),
                                            mode,
                                            this),
                          this);
}

QT_END_NAMESPACE

#include "moc_qquick3dloader_p.cpp"
