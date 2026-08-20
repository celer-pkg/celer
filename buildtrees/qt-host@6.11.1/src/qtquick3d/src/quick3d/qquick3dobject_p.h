// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGOBJECT_P_H
#define QSSGOBJECT_P_H

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

#include "qquick3dobject.h"

#include "qtquick3dglobal_p.h"

#include "qquick3dobjectchangelistener_p.h"

#include "qquick3dscenemanager_p.h"

#include <private/qobject_p.h>
#include <private/qquickstate_p.h>
#include <private/qqmlnotifier_p.h>
#include <private/qlazilyallocated_p.h>
#include <private/qssgrendergraphobject_p.h>
#include <QtQuick3DUtils/private/qquick3dprofiler_p.h>

#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QQuick3DItem2D;

class Q_QUICK3D_EXPORT QQuick3DObjectPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QQuick3DObject)
public:
    using Type = QSSGRenderGraphObject::Type;

    enum class Flags : quint32
    {
        None = 0x0,
        RequiresSecondaryUpdate = 0x1, // Only relevant for resources
    };

    /*!
      Flags::RequiresSecondaryUpdate indicates that the object needs a secondary update, meaning an
      additional call to updateSpatialNode() will be made after the first one. This is useful for objects
      that depend on other objects being fully updated before they can update themselves properly, as
      when the second call happens all objects have had their first updateSpatialNode() called.
      In addition to setting this flag the object must also call requestSecondaryUpdate() when it knows
      that it needs the secondary update!

      NOTE: This flag is only relevant for resource objects, spatial nodes will be ignored.
     */

    using FlagsT = std::underlying_type_t<Flags>;

    struct ConnectionKey
    {
        using Handle = void (QQuick3DObject::*)(QObject *);
        QObject *context = nullptr;
        Handle unusable = nullptr;
        friend bool operator==(const ConnectionKey &a, const ConnectionKey &b) noexcept { return (a.context == b.context) && (a.unusable == b.unusable); }
    };
    using ConnectionMap = QHash<ConnectionKey, QMetaObject::Connection>;

    template<typename SceneContext, typename CallContext, typename Setter>
    static void attachWatcherPriv(SceneContext *sceneContext, CallContext *callContext, Setter setter, QQuick3DObject *newO, QObject *oldO)
    {
        static_assert(std::is_base_of_v<QQuick3DObject, SceneContext>, "The scene context must be a QQuick3DObject");
        static_assert(std::is_member_function_pointer_v<Setter>, "The assumption is that the setter is a member function!");
        static_assert(sizeof(ConnectionKey::Handle) >= sizeof(Setter), "The handle needs to be able to store the value of the setter");
        // Sanity check: Make sure we have a valid context. If the sceneContext != callContext and we're here because the
        // watched object just got destroyed by the parent (QObject dtor) and that parent also is used as the sceneContext
        // then there's nothing more to do; all involved parties are being destroyed, so just bail out.
        const bool validContext = static_cast<QObject *>(sceneContext) != static_cast<QObject *>(callContext) ? qobject_cast<QQuick3DObject *>(sceneContext) != nullptr : true;
        if (validContext) {
            auto sceneManager = QQuick3DObjectPrivate::get(sceneContext)->sceneManager;
            auto &connectionMap = QQuick3DObjectPrivate::get(sceneContext)->connectionMap;
            union
            {
                Setter s;
                ConnectionKey::Handle h;
            }; s = setter;
            ConnectionKey key{static_cast<QObject *>(callContext), h};
            // disconnect previous destruction listener
            if (oldO) {
                // NOTE: If the old object is inside the QObject's dtor (e.g., QObject::destroyed) we can't
                // call deref (and there's no point anymore either).
                if (auto old3dO = qobject_cast<QQuick3DObject *>(oldO))
                    QQuick3DObjectPrivate::derefSceneManager(old3dO);

                auto it = connectionMap.constFind(key);
                if (it != connectionMap.cend()) {
                    QObject::disconnect(*it);
                    connectionMap.erase(it);
                }
            }

            // Watch new object
            if (newO) {
                if (sceneManager)
                    QQuick3DObjectPrivate::refSceneManager(newO, *sceneManager);
                auto connection = QObject::connect(newO, &QObject::destroyed, callContext, [callContext, setter](){ (callContext->*setter)(nullptr); });
                connectionMap.insert(key, connection);
            }
        }
    }

    /*!
      Attach a object-destroyed-watcher to an object that's not owned.
      There are few checks here just to keep it simple
      (The compiler should still fail with a varying degree of helpful messages when used incorrectly).

      \a sceneContext - ususally the same as the callContext and only different if the calledContext is a non-QQuick3DObject class
      (as is the case for QQuick3DShaderUtilsTextureInput)!
      \a callContext - The object watching another object
      \a setter - The function/slot that is called for the object (context).
      \a newO - The new object being watched
      \b oldO - The previous object that should no longer be watched.

      Note: The \a setter is a function that takes one argument with a discardable return value.
    */
    template<typename Context, typename Setter, typename Object3D>
    static void attachWatcher(Context *context, Setter setter, Object3D *newO, Object3D *oldO)
    {
        attachWatcherPriv(context, context, setter, newO, oldO);
    }

    static QQuick3DObjectPrivate *get(QQuick3DObject *item) { return item->d_func(); }
    static const QQuick3DObjectPrivate *get(const QQuick3DObject *item) { return item->d_func(); }
    static QSSGRenderGraphObject *updateSpatialNode(QQuick3DObject *o, QSSGRenderGraphObject *n) { return o->updateSpatialNode(n); }

    explicit QQuick3DObjectPrivate(Type t, FlagsT f);
    explicit QQuick3DObjectPrivate(Type t, Flags f);
    explicit QQuick3DObjectPrivate(Type t);
    ~QQuick3DObjectPrivate() override;
    void init(QQuick3DObject *parent);

    QQmlListProperty<QObject> data();
    QQmlListProperty<QObject> resources();
    QQmlListProperty<QQuick3DObject> children();

    QQmlListProperty<QQuickState> states();
    QQmlListProperty<QQuickTransition> transitions();

    QString state() const;
    void setState(const QString &);

    // data property
    static void data_append(QQmlListProperty<QObject> *, QObject *);
    static qsizetype data_count(QQmlListProperty<QObject> *);
    static QObject *data_at(QQmlListProperty<QObject> *, qsizetype);
    static void data_clear(QQmlListProperty<QObject> *);

    // resources property
    static QObject *resources_at(QQmlListProperty<QObject> *, qsizetype);
    static void resources_append(QQmlListProperty<QObject> *, QObject *);
    static qsizetype resources_count(QQmlListProperty<QObject> *);
    static void resources_clear(QQmlListProperty<QObject> *);

    // children property
    static void children_append(QQmlListProperty<QQuick3DObject> *, QQuick3DObject *);
    static qsizetype children_count(QQmlListProperty<QQuick3DObject> *);
    static QQuick3DObject *children_at(QQmlListProperty<QQuick3DObject> *, qsizetype);
    static void children_clear(QQmlListProperty<QQuick3DObject> *);

    void _q_resourceObjectDeleted(QObject *);
    void _q_cleanupContentItem2D();
    quint64 _q_createJSWrapper(QQmlV4ExecutionEnginePtr engine);

    enum ChangeType {
        Geometry = 0x01,
        SiblingOrder = 0x02,
        Visibility = 0x04,
        Opacity = 0x08,
        Destroyed = 0x10,
        Parent = 0x20,
        Children = 0x40,
        Rotation = 0x80,
        ImplicitWidth = 0x100,
        ImplicitHeight = 0x200,
        Enabled = 0x400,
    };

    Q_DECLARE_FLAGS(ChangeTypes, ChangeType)

    struct ChangeListener
    {
        using ChangeTypes = QQuick3DObjectPrivate::ChangeTypes;

        ChangeListener(QQuick3DObjectChangeListener *l = nullptr, ChangeTypes t = {}) : listener(l), types(t) {}

        ChangeListener(QQuick3DObjectChangeListener *l) : listener(l), types(Geometry) {}

        bool operator==(const ChangeListener &other) const
        {
            return listener == other.listener && types == other.types;
        }

        QQuick3DObjectChangeListener *listener;
        ChangeTypes types;
    };

    struct ExtraData
    {
        ExtraData();

        int hideRefCount;
        QObjectList resourcesList;

    };
    QLazilyAllocated<ExtraData> extra;

    QVector<QQuick3DObjectPrivate::ChangeListener> changeListeners;

    void addItemChangeListener(QQuick3DObjectChangeListener *listener, ChangeTypes types);
    void updateOrAddItemChangeListener(QQuick3DObjectChangeListener *listener, ChangeTypes types);
    void removeItemChangeListener(QQuick3DObjectChangeListener *, ChangeTypes types);

    QQuickStateGroup *_states();
    QQuickStateGroup *_stateGroup;

    enum DirtyType {
        TransformOrigin = 0x00000001,
        Transform = 0x00000002,
        BasicTransform = 0x00000004,
        Position = 0x00000008,
        Size = 0x00000010,

        ZValue = 0x00000020,
        Content = 0x00000040,
        Smooth = 0x00000080,
        OpacityValue = 0x00000100,
        ChildrenChanged = 0x00000200,
        ChildrenStackingChanged = 0x00000400,
        ParentChanged = 0x00000800,

        Clip = 0x00001000,
        Window = 0x00002000,

        EffectReference = 0x00008000,
        Visible = 0x00010000,
        HideReference = 0x00020000,
        Antialiasing = 0x00040000,

        InstanceRootChanged = 0x00080000,
        // When you add an attribute here, don't forget to update
        // dirtyToString()

        TransformUpdateMask = TransformOrigin | Transform | BasicTransform | Position | Window,
        ComplexTransformUpdateMask = Transform | Window,
        ContentUpdateMask = Size | Content | Smooth | Window | Antialiasing,
        ChildrenUpdateMask = ChildrenChanged | ChildrenStackingChanged | EffectReference | Window
    };

    quint32 dirtyAttributes;
    QString dirtyToString() const;
    void dirty(DirtyType);
    void addToDirtyList();
    void removeFromDirtyList();
    QQuick3DObject *nextDirtyItem;
    QQuick3DObject **prevDirtyItem;

    bool hasFlag(Flags flag) { return (flags & FlagsT(flag)) != 0; }

    void setCulled(bool);

    QPointer<QQuick3DSceneManager> sceneManager;
    int sceneRefCount;

    QQuick3DObject *parentItem;

    QList<QQuick3DObject *> childItems;
    void addChild(QQuick3DObject *);
    void removeChild(QQuick3DObject *);
    void siblingOrderChanged();

    void refSceneManager(QQuick3DSceneManager &);
    void derefSceneManager();

    static void refSceneManager(QQuick3DObject *obj, QQuick3DSceneManager &mgr)
    {
        if (obj)
            QQuick3DObjectPrivate::get(obj)->refSceneManager(mgr);
    }
    static void derefSceneManager(QQuick3DObject *obj)
    {
        if (obj)
            QQuick3DObjectPrivate::get(obj)->derefSceneManager();
    }

    QQuick3DObject *subFocusItem;
    void updateSubFocusItem(QQuick3DObject *scope, bool focus);

    void itemChange(QQuick3DObject::ItemChange, const QQuick3DObject::ItemChangeData &);

    virtual void updatePolish() {}

    void requestSecondaryUpdate() { secondaryUpdateRequested = true; }

    QSSGRenderGraphObject *spatialNode = nullptr;

    Type type = Type::Unknown;
    FlagsT flags = FlagsT(Flags::None);
    bool secondaryUpdateRequested = false;
    bool componentComplete = true;
    bool preSyncNeeded = false;
    bool culled;
    bool sharedResource = false;
    QQuick3DItem2D *contentItem2d = nullptr;
    ConnectionMap connectionMap;
    Q_QUICK3D_PROFILE_ID
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QQuick3DObjectPrivate::ChangeTypes)
Q_DECLARE_TYPEINFO(QQuick3DObjectPrivate::ChangeListener, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(QQuick3DObjectPrivate::ConnectionKey, Q_PRIMITIVE_TYPE);

inline size_t qHash(const QQuick3DObjectPrivate::ConnectionKey &con, size_t seed = 0) noexcept
{
    return qHashBits(&con, sizeof(QQuick3DObjectPrivate::ConnectionKey), seed);
}

QT_END_NAMESPACE

#endif // QSSGOBJECT_P_H
