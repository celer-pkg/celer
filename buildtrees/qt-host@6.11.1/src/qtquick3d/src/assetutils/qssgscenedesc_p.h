// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGSCENEDESCRIPTION_P_H
#define QSSGSCENEDESCRIPTION_P_H

#include <QtQuick3DAssetUtils/private/qtquick3dassetutilsglobal_p.h>
#include <QtQuick3DRuntimeRender/private/qssgrendergraphobject_p.h>
#include <QtQuick3DRuntimeRender/private/qssgperframeallocator_p.h>
#include <QtQuick3DUtils/private/qssgmesh_p.h>

#include <QtCore/qlist.h>
#include <QtCore/qhash.h>
#include <QtCore/qvariant.h>
#include <QtCore/qflags.h>
#include <QtQml/qqmllist.h>

// QtQuick3D
#include <QtQuick3D/private/qquick3dobject_p.h>
// Materials
#include <QtQuick3D/private/qquick3dcustommaterial_p.h>
#include <QtQuick3D/private/qquick3ddefaultmaterial_p.h>
#include <QtQuick3D/private/qquick3dprincipledmaterial_p.h>
#include <QtQuick3D/private/qquick3dspecularglossymaterial_p.h>
#include <QtQuick3D/private/qquick3dmodel_p.h>
// cameras
#include <QtQuick3D/private/qquick3dorthographiccamera_p.h>
#include <QtQuick3D/private/qquick3dperspectivecamera_p.h>
#include <QtQuick3D/private/qquick3dcustomcamera_p.h>
// Lights
#include <QtQuick3D/private/qquick3ddirectionallight_p.h>
#include <QtQuick3D/private/qquick3dpointlight_p.h>
#include <QtQuick3D/private/qquick3dspotlight_p.h>
// Texture
#include <QtQuick3D/private/qquick3dtexture_p.h>
#include <QtQuick3D/private/qquick3dcubemaptexture_p.h>
#include <QtQuick3D/private/qquick3dtexturedata_p.h>
//
#include <QtQuick3D/private/qquick3dskeleton_p.h>
#include <QtQuick3D/private/qquick3djoint_p.h>

#include <qmetatype.h>
#include <QtQuick3DUtils/private/qssginvasivelinkedlist_p.h>

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

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

namespace QSSGSceneDesc
{

struct Node;
struct Animation;

template<typename T>
using rm_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

struct Q_QUICK3DASSETUTILS_EXPORT Scene
{
    using ResourceNodes = QVarLengthArray<Node *>;
    using MeshStorage = QVector<QSSGMesh::Mesh>;
    using Animations = QVector<Animation *>;

    // Root node, usually an empty 'transform' node.
    Node *root = nullptr;
    QString id; // Don't make any assumption about the content of this id...
    ResourceNodes resources;
    MeshStorage meshStorage;
    Animations animations;
    QString sourceDir;
    mutable quint16 nodeId = 0;

    void reset();
    void cleanup();
};

struct Q_QUICK3DASSETUTILS_EXPORT PropertyCall
{
    virtual ~PropertyCall() = default;
    virtual bool set(QQuick3DObject &, const char *, const void *) = 0;
    virtual bool set(QQuick3DObject &, const char *, const QVariant &) = 0;
    virtual bool get(const QQuick3DObject &, const void *[]) const = 0;
};

Q_QUICK3DASSETUTILS_EXPORT void destructValue(QVariant &value);

struct Flag
{
    QMetaEnum me;
    int value;
};

struct Property
{
    ~Property();
    enum class Type { Static, Dynamic };
    QVariant value;
    QByteArray name;
    QSSGSceneDesc::PropertyCall *call = nullptr;
    Type type = Type::Static;
};

inline Property::~Property()
{
    delete call;
    destructValue(value);
}

Q_QUICK3DASSETUTILS_EXPORT void destructNode(QSSGSceneDesc::Node &node);

struct NodeList
{
    NodeList(void * const *data, qsizetype n)
    {
        const auto size = sizeof(Node *) * n;
        head = reinterpret_cast<Node **>(malloc(size));
        memcpy(head, data, size);
        count = n;
    }
    ~NodeList() { if (head) free(head); }
    Node **head = nullptr;
    qsizetype count = -1;
};

struct Q_QUICK3DASSETUTILS_EXPORT Node
{
    // Node type
    enum class Type : quint8
    {
        Transform,
        Camera,
        Model,
        Texture,
        Material,
        Light,
        Mesh,
        Skin,
        Skeleton,
        Joint,
        MorphTarget
    };

    using type = QQuick3DNode;
    // Runtime type type mapping between this type and the QtQuick3D type
    using RuntimeType = QSSGRenderGraphObject::Type;

    explicit Node(QByteArray name, Node::Type type, Node::RuntimeType rt)
        : name(name)
        , runtimeType(rt)
        , nodeType(type) {}
    explicit Node(Node::Type type, Node::RuntimeType rt)
        : Node(nullptr, type, rt) {}

    virtual ~Node();
    void cleanupChildren();

    QByteArray name;
    Scene *scene = nullptr;
    QObject *obj = nullptr;
    using ChildList = QList<Node *>;
    using PropertyList = QList<Property *>;
    ChildList children;
    PropertyList properties;
    quint16 id = 0;
    RuntimeType runtimeType;
    Type nodeType;
};

template<typename T>
static constexpr bool is_node_v = std::is_base_of_v<Node, T>;

// Set up type mapping from a QQuick3D type to a SceneDesc type
template <typename T> struct TypeMap {};
#define QSSG_DECLARE_NODE(NODE) \
static_assert(is_node_v<NODE>, #NODE " - does not inherit from Node!"); \
template <> struct TypeMap<NODE::type> { using type = QSSGSceneDesc::NODE; };

template<typename T>
using as_scene_type_t = typename T::type;
template<typename T>
using as_node_type_t = typename TypeMap<T>::type;

QSSG_DECLARE_NODE(Node)

struct Q_QUICK3DASSETUTILS_EXPORT Texture : Node
{
    using type = QQuick3DTexture;
    explicit Texture(Node::RuntimeType rt, const QByteArray &name = {});
};
QSSG_DECLARE_NODE(Texture)

struct Q_QUICK3DASSETUTILS_EXPORT TextureData : Node
{
    using type = QQuick3DTextureData;
    enum class Flags : quint8
    {
        Compressed = 0x1
    };

    explicit TextureData(const QByteArray &textureData, QSize size, const QByteArray &format, quint8 flags = 0, QByteArray name = {});
    QByteArray data;
    QSize sz;
    QByteArray fmt;
    quint8 flgs;
};
QSSG_DECLARE_NODE(TextureData)

struct Q_QUICK3DASSETUTILS_EXPORT Material : Node
{
    using type = QQuick3DMaterial;
    explicit Material(Node::RuntimeType rt);
};
QSSG_DECLARE_NODE(Material)

// The mesh is a special node, as it's not really a node type but
// a handle to a mesh that will be turned into a source URL...
struct Q_QUICK3DASSETUTILS_EXPORT Mesh : Node
{
    explicit Mesh(QByteArray name, qsizetype index);
    qsizetype idx; // idx to the mesh data in the mesh data storage (see Scene).
};

struct Q_QUICK3DASSETUTILS_EXPORT Model : Node
{
    using type = QQuick3DModel;
    Model();
};
QSSG_DECLARE_NODE(Model)

struct Q_QUICK3DASSETUTILS_EXPORT Camera : Node
{
    using type = QQuick3DCamera;
    explicit Camera(RuntimeType rt);
};
QSSG_DECLARE_NODE(Camera)

struct Q_QUICK3DASSETUTILS_EXPORT Light : Node
{
    using type = QQuick3DAbstractLight;
    explicit Light(RuntimeType rt);
};
QSSG_DECLARE_NODE(Light)

struct Q_QUICK3DASSETUTILS_EXPORT Skin : Node
{
    using type = QQuick3DSkin;
    Skin();
};
QSSG_DECLARE_NODE(Skin)

struct Q_QUICK3DASSETUTILS_EXPORT Skeleton : Node
{
    using type = QQuick3DSkeleton;
    Skeleton();
    // Skeleton is a virtual node, which is added for the start of the joint heirarchy.
    // parent - joint 1     ->      parent - skeleton - joint 1
    //        - joint 2                               - joint 2
    //        - model 1                    - model 1
    //        - camera 1                   - camera 1
    size_t maxIndex = 0;
};
QSSG_DECLARE_NODE(Skeleton)

struct Q_QUICK3DASSETUTILS_EXPORT Joint : Node
{
    using type = QQuick3DJoint;
    Joint();
};
QSSG_DECLARE_NODE(Joint)

struct Q_QUICK3DASSETUTILS_EXPORT MorphTarget : Node
{
    using type = QQuick3DMorphTarget;
    MorphTarget();
};
QSSG_DECLARE_NODE(MorphTarget)

// We keep our own list data structure, since Qt does not have a variant list where all the
// elements have the same type, and using a list of QVariant is very inefficient
struct ListView
{
    ~ListView() { if (data) free(data); }
    QMetaType mt;
    void *data = nullptr;
    qsizetype count = -1;
};

Q_QUICK3DASSETUTILS_EXPORT QMetaType listViewMetaType();

struct Animation
{
    struct KeyPosition
    {
        enum class KeyType : quint16
        {
            Frame = 0x100,
            Time = 0x200
        };

        enum class ValueType : quint8
        {
            Number,
            Vec2,
            Vec3,
            Vec4,
            Quaternion
        };

        ValueType getValueType() const { return ValueType(0xf & flag); }
        KeyType getKeyType() const { return KeyType(0xf00 & flag); }
        QVariant getValue() const {
            switch (getValueType()) {
            case ValueType::Number:
                return value.x();
            case ValueType::Vec2:
                return value.toVector2D();
            case ValueType::Vec3:
                return value.toVector3D();
            case ValueType::Vec4:
                return value;
            case ValueType::Quaternion:
                return QQuaternion(value);
            }
            return value;
        }
        QMetaType::Type getValueQMetaType() const {
            switch (getValueType()) {
            case ValueType::Number:
                return QMetaType::Float;
            case ValueType::Vec2:
                return QMetaType::QVector2D;
            case ValueType::Vec3:
                return QMetaType::QVector3D;
            case ValueType::Vec4:
                return QMetaType::QVector4D;
            case ValueType::Quaternion:
                return QMetaType::QQuaternion;
            }
            return QMetaType::QVector4D;
        }
        QVector4D value;
        float time = 0.0f;
        quint16 flag = 0;
    };
    using Keys = QList<Animation::KeyPosition *>;

    struct Channel
    {
        enum class TargetType : quint8
        {
            Property
        };

        // This is a bit simplistic, but is all we support so let's keep simple.
        enum class TargetProperty : quint8
        {
            Unknown,
            Position,
            Rotation,
            Scale,
            Weight // for MorphMesh
        };

        Node *target = nullptr;
        Keys keys;
        TargetType targetType = TargetType::Property;
        TargetProperty targetProperty = TargetProperty::Unknown;
    };
    using Channels = QList<Animation::Channel *>;

    Channels channels;
    // It stores the length of this Animation, every keys in every channels in
    // an animation will have the same KeyType and it will be a type of
    // the length
    float length = 0.0f;

    float framesPerSecond = 0.0f; // for translation back to frames

    QByteArray name;
};

// Add a child node to parent node.
Q_QUICK3DASSETUTILS_EXPORT void addNode(Node &parent, Node &node);
// Add node to the scene, if a node is already set the new node will
// become a child of the root node.
Q_QUICK3DASSETUTILS_EXPORT void addNode(Scene &scene, Node &node);

template<typename> struct ListParam { enum { value = 0 }; };
template<typename T> struct ListParam<QList<T>>
{
    using type = T;
    enum { value = 1 };
};

template <typename T>
using listParam_t = typename ListParam<rm_cvref_t<T>>::type;

template <typename T>
struct FuncType
{
    enum { value = 0 };
};

template <typename R, typename... A>
struct FuncType<R (*)(A...)>
{
    enum { value = sizeof...(A) == 3 };
    using Ret = R;
    using Arg0 = std::tuple_element_t<0, std::tuple<A...>>;
    using Arg1 = std::tuple_element_t<1, std::tuple<A...>>;
    using Arg2 = std::tuple_element_t<2, std::tuple<A...>>;
    using Arg2Base = rm_cvref_t<Arg2>;
};

template <typename R, typename C, typename... A>
struct FuncType<R (C::*)(A...)>
{
    enum { value = sizeof... (A) == 1 };
    using Ret = R;
    using Class = C;
    // For now we only care about single argument functions
    using Arg0 = std::tuple_element_t<0, std::tuple<A...>>;
    using Arg0Base = rm_cvref_t<Arg0>;
};

template <typename T, typename C>
struct FuncType<QQmlListProperty<T> (C::*)()>
{
    enum { value = 2 };
    using Ret = QQmlListProperty<T>;
    using Class = C;
    using Arg0 = void;
    using Arg0Base = Arg0;
};

template <typename Ret, typename Arg>
struct PropertyProxySetter : PropertyCall
{
    using Setter = Ret (*)(QQuick3DObject &, const char *, Arg);
    constexpr explicit PropertyProxySetter(Setter fn) : call(fn) {}
    Setter call = nullptr;
    bool get(const QQuick3DObject &, const void *[]) const override { return false; }
    bool set(QQuick3DObject &that, const char *name, const void *value) override
    {
        if constexpr (std::is_pointer_v<typename FuncType<Setter>::Arg2>)
            call(that, name, reinterpret_cast<typename FuncType<Setter>::Arg2>(const_cast<void *>(value)));
        else
            call(that, name, *reinterpret_cast<typename FuncType<Setter>::Arg2Base *>(const_cast<void *>(value)));
        return true;
    }
    bool set(QQuick3DObject &that, const char *name, const QVariant &var) override
    {
        call(that, name, qvariant_cast<typename FuncType<Setter>::Arg2Base>(var));
        return true;
    }
};

template <typename Ret, typename Class, typename Arg>
struct PropertySetter : PropertyCall
{
    using Setter = Ret (Class::*)(Arg);
    constexpr explicit PropertySetter(Setter fn) : call(fn) {}
    Setter call = nullptr;
    bool get(const QQuick3DObject &, const void *[]) const override { return false; }
    bool set(QQuick3DObject &that, const char *, const void *value) override
    {
        if constexpr (std::is_pointer_v<typename FuncType<Setter>::Arg0>)
            (qobject_cast<Class *>(&that)->*call)(reinterpret_cast<typename FuncType<Setter>::Arg0>(const_cast<void *>(value)));
        else {
            (qobject_cast<Class *>(&that)->*call)(*reinterpret_cast<typename FuncType<Setter>::Arg0Base *>(const_cast<void *>(value)));
        }
        return true;
    }
    bool set(QQuick3DObject &that, const char *, const QVariant &var) override
    {
        (qobject_cast<Class *>(&that)->*call)(qvariant_cast<typename FuncType<Setter>::Arg0Base>(var));
        return true;
    }
};

template <typename Ret, typename Class, typename Arg>
struct PropertyListSetter : PropertyCall
{
    using Setter = Ret (Class::*)(Arg);
    using ListT = typename FuncType<Setter>::Arg0Base;
    using It = listParam_t<ListT>;
    constexpr explicit PropertyListSetter(Setter fn) : call(fn) {}
    Setter call = nullptr;
    bool get(const QQuick3DObject &, const void *[]) const override { return false; }
    bool set(QQuick3DObject &that, const char *, const void *value) override
    {
        if (const auto listView = reinterpret_cast<const ListView *>(value)) {
            if (listView->count > 0) {
                const auto begin = reinterpret_cast<It *>(listView->data);
                const auto end = reinterpret_cast<It *>(listView->data) + listView->count;
                (qobject_cast<Class *>(&that)->*call)(ListT{begin, end});
            } else {
                (qobject_cast<Class *>(&that)->*call)(ListT{});
            }
            return true;
        }

        return false;
    }
    bool set(QQuick3DObject &that, const char *, const QVariant &var) override
    {
        if (const auto listView = qvariant_cast<const ListView *>(var)) {
            if (listView->count > 0) {
                const auto begin = reinterpret_cast<It *>(listView->data);
                const auto end = reinterpret_cast<It *>(listView->data) + listView->count;
                (qobject_cast<Class *>(&that)->*call)(ListT{begin, end});
            } else {
                (qobject_cast<Class *>(&that)->*call)(ListT{});
            }
        }
        return false;
    }
};

template <typename Class, typename T, template <typename> typename List>
struct PropertyList : PropertyCall
{
    using ListType = List<T>;
    using ListFunc = ListType (Class::*)();
    constexpr explicit PropertyList(ListFunc fn) : listfn(fn) {}
    ListFunc listfn = nullptr;

    static_assert(std::is_same_v<ListType, QQmlListProperty<T>>, "Expected QQmlListProperty!");

    bool get(const QQuick3DObject &, const void *[]) const override { return false; }


    void doSet(QQuick3DObject &that, const QSSGSceneDesc::NodeList &nodeList)
    {
        ListType list = (qobject_cast<Class *>(&that)->*listfn)();
        auto head = reinterpret_cast<as_node_type_t<T> **>(nodeList.head);
        for (int i = 0, end = nodeList.count; i != end; ++i)
            list.append(&list, qobject_cast<T *>((*(head + i))->obj));
    }

    bool set(QQuick3DObject &that, const char *, const void *value) override
    {
        if (value) {
            const auto &nodeList = *reinterpret_cast<const QSSGSceneDesc::NodeList *>(value);
            doSet(that, nodeList);
            return true;
        }
        return false;
    }

    bool set(QQuick3DObject &that, const char *, const QVariant &var) override
    {
        const auto *nodeList = qvariant_cast<const QSSGSceneDesc::NodeList *>(var);
        if (nodeList) {
            doSet(that, *nodeList);
            return true;
        }
        return false;
    }
};

template <typename NodeT>
using if_node = typename std::enable_if_t<is_node_v<NodeT>, bool>;
template <typename Setter, typename Value>
using if_compatible_t = typename std::enable_if_t<std::is_same_v<typename FuncType<Setter>::Arg0Base, rm_cvref_t<Value>>, bool>;
template <typename Setter, typename T>
using if_compatible_node_list_t = typename std::enable_if_t<std::is_same_v<typename FuncType<Setter>::Ret, QQmlListProperty<as_scene_type_t<T>>>, bool>;
template <typename Setter, typename Value>
using if_compatible_proxy_t = typename std::enable_if_t<std::is_same_v<typename FuncType<Setter>::Arg2Base, rm_cvref_t<Value>>, bool>;

// Sets a property on a node, the property is a name map to struct containing a pointer to the value and a function
// to set the value on an runtime object (QtQuick3DObject). The type is verified at compile-time, so we can assume
// the value is of the right type when setting it at run-time.
template<typename Setter, typename T, if_compatible_t<Setter, T> = false>
static void setProperty(QSSGSceneDesc::Node &node, const char *name, Setter setter, T &&value)
{
     Q_ASSERT(node.scene);
    auto prop = new Property;
    prop->name = name;
    prop->call = new PropertySetter(setter);
    prop->value = QVariant::fromValue(std::forward<T>(value));
    node.properties.push_back(prop);
}

template<typename Setter, typename T, if_compatible_t<Setter, QFlags<T>> = false>
static void setProperty(QSSGSceneDesc::Node &node, const char *name, Setter setter, QFlags<T> value)
{
    Q_ASSERT(node.scene);
    auto prop = new Property;
    prop->name = name;
    prop->call = new PropertySetter(setter);
    prop->value = QVariant::fromValue(Flag{ QMetaEnum::fromType<rm_cvref_t<T>>(), value.toInt() });
    node.properties.push_back(prop);
}


template<typename Setter, typename T, if_compatible_t<Setter, QList<T>> = false>
static void setProperty(QSSGSceneDesc::Node &node, const char *name, Setter setter, QList<T> value)
{
    Q_ASSERT(node.scene);
    static_assert(!std::is_pointer_v<T>, "Type cannot be a pointer!");
    static_assert(std::is_trivially_destructible_v<T> && std::is_trivially_copy_constructible_v<T>,
            "List parameter type needs to be trivially constructable and trivially destructible!");

    const auto count = value.size();
    void *data = nullptr;
    if (count) {
        const auto asize = count * sizeof(T);
        data = malloc(asize); // is freed in ~ListView
        memcpy(data, value.constData(), asize);
    }
    auto prop = new Property;
    prop->name = name;
    prop->call = new PropertyListSetter(setter);
    prop->value = QVariant::fromValue(new ListView{ QMetaType::fromType<rm_cvref_t<T>>(), data, count });
    node.properties.push_back(prop);
}


Q_QUICK3DASSETUTILS_EXPORT QSSGSceneDesc::Property *setProperty(QSSGSceneDesc::Node &node, const char *name, QVariant &&value);

// Calling this will omit any type checking, so make sure the type is handled correctly
// when it gets used later!
template<typename Setter>
static void setProperty(QSSGSceneDesc::Node &node, const char *name, Setter setter, QVariant &&value)
{
    Q_ASSERT(node.scene);
    Property *prop = new Property;
    prop->name = name;
    prop->call = new PropertySetter(setter);
    prop->value = value;
    node.properties.push_back(prop);
}

template<typename Setter, typename Value, if_compatible_proxy_t<Setter, Value> = true>
static void setProperty(QSSGSceneDesc::Node &node, const char *name, Setter setter, Value &&value, QSSGSceneDesc::Property::Type type = QSSGSceneDesc::Property::Type::Static)
{
    Q_ASSERT(node.scene);
    static_assert(std::is_trivially_destructible_v<rm_cvref_t<Value>>, "Value needs to be trivially destructible!");
    Property *prop = new Property;
    prop->name = name;
    prop->call = new PropertyProxySetter(setter);
    prop->value = QVariant::fromValue(value);
    prop->type = type;
    node.properties.push_back(prop);
}

template<typename Setter, typename ViewValue, if_compatible_t<Setter, typename ViewValue::type> = false>
static void setProperty(QSSGSceneDesc::Node &node, const char *name, Setter setter, ViewValue view)
{
    Q_ASSERT(node.scene);
    static_assert(std::is_same_v<typename ViewValue::type, typename FuncType<Setter>::Arg0Base>, "Type cannot be mapped to slot argument");
    Property *prop = new Property;
    prop->name = name;
    prop->call = new PropertySetter(setter);
    prop->value = QVariant::fromValue(view);
    node.properties.push_back(prop);
}

template<typename Setter, typename Value, if_compatible_t<Setter, as_scene_type_t<Value> *> = true>
static void setProperty(QSSGSceneDesc::Node &node, const char *name, Setter setter, Value *value)
{
    Q_ASSERT(node.scene);
    Property *prop = new Property;
    prop->name = name;
    prop->call = new PropertySetter(setter);
    // Always 'Node', the Node class itself contains more fine grain type information.
    prop->value = QVariant::fromValue(static_cast<Node *>(value));
    node.properties.push_back(prop);
}

// Overloaded function for setting a type to a property that's a QQmlListProperty.
template<typename Setter, typename NodeT, qsizetype Prealloc, if_compatible_node_list_t<Setter, NodeT> = true>
static void setProperty(QSSGSceneDesc::Node &node, const char *name, Setter setter, const QVarLengthArray<NodeT *, Prealloc> &list)
{
    Q_ASSERT(node.scene);
    if (!list.isEmpty()) {
        NodeList *l = new NodeList(reinterpret_cast<void * const*>(list.constData()), list.count());

        Property *prop = new Property;
        prop->name = name;
        prop->call = new PropertyList(setter);
        prop->value = QVariant::fromValue(l);
        node.properties.push_back(prop);
    }
}

} // QSSGSceneDesc

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QSSGSceneDesc::Node)
Q_DECLARE_METATYPE(QSSGSceneDesc::Texture)
Q_DECLARE_METATYPE(QSSGSceneDesc::Material)
Q_DECLARE_METATYPE(QSSGSceneDesc::Mesh)
Q_DECLARE_METATYPE(QSSGSceneDesc::Model)
Q_DECLARE_METATYPE(QSSGSceneDesc::Camera)
Q_DECLARE_METATYPE(QSSGSceneDesc::Light)
Q_DECLARE_METATYPE(QSSGSceneDesc::Skin)
Q_DECLARE_METATYPE(QSSGSceneDesc::Skeleton)
Q_DECLARE_METATYPE(QSSGSceneDesc::Joint)
Q_DECLARE_METATYPE(QSSGSceneDesc::MorphTarget)
Q_DECLARE_METATYPE(QSSGSceneDesc::NodeList)
Q_DECLARE_METATYPE(QSSGSceneDesc::Animation)

Q_DECLARE_METATYPE(QSSGSceneDesc::ListView)

Q_DECLARE_METATYPE(QSSGSceneDesc::Flag)

#endif // QSSGSCENEDESCRIPTION_P_H
