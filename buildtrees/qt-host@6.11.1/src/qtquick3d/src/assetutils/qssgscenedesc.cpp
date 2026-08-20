// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgscenedesc_p.h"

QT_BEGIN_NAMESPACE

bool QSSGSceneDesc::PropertyCall::set(QQuick3DObject &, const char *, const void *) { return false; }
bool QSSGSceneDesc::PropertyCall::get(const QQuick3DObject &, const void *[]) const { return false; }

static inline quint16 getNextNodeId(QSSGSceneDesc::Scene &scene)
{
    /* root node uses the default value 0 */
    return ++scene.nodeId;
}

void QSSGSceneDesc::addNode(QSSGSceneDesc::Node &parent, QSSGSceneDesc::Node &node)
{
    Q_ASSERT(parent.scene);
    node.scene = parent.scene;
    node.id = getNextNodeId(*parent.scene);

    if (QSSGRenderGraphObject::isResource(node.runtimeType) || node.nodeType == Node::Type::Mesh || node.nodeType == Node::Type::Skeleton)
        node.scene->resources.push_back(&node);
    else // Here goes nothing: kick all the resources out of the tree...
        parent.children.push_back(&node);
}

void QSSGSceneDesc::addNode(QSSGSceneDesc::Scene &scene, QSSGSceneDesc::Node &node)
{
    if (scene.root) {
        addNode(*scene.root, node);
    } else {
        Q_ASSERT(node.id == 0);
        node.scene = &scene;
        scene.root = &node;
    }
}

void QSSGSceneDesc::Scene::reset()
{
    id.clear();
    nodeId = 0;
    root = nullptr;
    resources.clear();
    meshStorage.clear();
}

void QSSGSceneDesc::Scene::cleanup()
{
    id.clear();
    nodeId = 0;

    root->cleanupChildren();
    delete root;
    root = nullptr;

    qDeleteAll(resources);
    resources.clear();

    for (auto *anim: animations) {
        for (auto *ch: anim->channels) {
            qDeleteAll(ch->keys);
            ch->keys.clear();
            delete ch;
        }
        delete anim;
    }
    animations.clear();
}

QMetaType QSSGSceneDesc::listViewMetaType()
{
    return QMetaType::fromType<QSSGSceneDesc::ListView *>();
}

void QSSGSceneDesc::destructValue(QVariant &value)
{
    if (!value.isValid())
        return;

    if (value.metaType() == QMetaType::fromType<QSSGSceneDesc::NodeList *>())
        delete value.value<NodeList *>();
    else if (value.metaType() == QMetaType::fromType<QSSGSceneDesc::ListView *>())
        delete value.value<ListView *>();
    // Non-pointer types are destructed by ~QVariant
    else if ((value.metaType().flags() & QMetaType::TypeFlag::IsPointer)
            // Mesh node will be deleted when cleaning up resources.
            && (value.metaType() != QMetaType::fromType<QSSGSceneDesc::Mesh *>())
            // Referencing nodes will not be deleted here.
            // They should be deleted in the node hierarchy or resources.
            && (value.metaType().id() != qMetaTypeId<QSSGSceneDesc::Node *>())) {
        qWarning() << value.metaType().name() << " was not destroyed correctly.";
    }
}

void QSSGSceneDesc::destructNode(Node &node)
{
    for (auto *prop : node.properties)
        delete prop;
    // Not necessary to clear the list as long as we only call this from the destructor
}

QSSGSceneDesc::Node::~Node() { destructNode(*this); }

void QSSGSceneDesc::Node::cleanupChildren()
{
    auto firstIt = children.begin();
    auto lastIt = children.end();
    for (auto it = firstIt; it != lastIt; ++it) {
        Node *node = *it;
        node->cleanupChildren();
        delete node;
    }
}

QSSGSceneDesc::Property *QSSGSceneDesc::setProperty(Node &node, const char *name, QVariant &&value)
{
    Q_ASSERT(node.scene);
    QSSGSceneDesc::Property *prop = new QSSGSceneDesc::Property;
    prop->name = name;
    prop->call = nullptr;
    prop->value = value;
    node.properties.push_back(prop);
    return prop;
}

QSSGSceneDesc::Model::Model() : Node(Node::Type::Model, Node::RuntimeType::Model) {}

QSSGSceneDesc::Camera::Camera(RuntimeType rt) : Node(Node::Type::Camera, rt) {}

QSSGSceneDesc::Light::Light(RuntimeType rt) : Node(Node::Type::Light, rt) {}

QSSGSceneDesc::Skin::Skin() : Node(Node::Type::Skin, Node::RuntimeType::Skin) {}

QSSGSceneDesc::Skeleton::Skeleton() : Node(Node::Type::Skeleton, Node::RuntimeType::Skeleton) {}

QSSGSceneDesc::Joint::Joint() : Node(Node::Type::Joint, Node::RuntimeType::Joint) {}

QSSGSceneDesc::MorphTarget::MorphTarget() : Node(Node::Type::MorphTarget, Node::RuntimeType::MorphTarget) {}

QSSGSceneDesc::Material::Material(RuntimeType rt) : Node(Node::Type::Material, rt) {}

QSSGSceneDesc::Texture::Texture(RuntimeType rt, const QByteArray &name)
    : Node(name, Node::Type::Texture, rt)
{
}

QSSGSceneDesc::TextureData::TextureData(const QByteArray &textureData, QSize size, const QByteArray &format, quint8 flags, QByteArray name)
    : Node(name, Node::Type::Texture, RuntimeType::TextureData)
    , data(textureData)
    , sz(size)
    , fmt(format)
    , flgs(flags)
{}

QSSGSceneDesc::Mesh::Mesh(QByteArray name, qsizetype index)
    : Node(name, Node::Type::Mesh, RuntimeType::Node)
    , idx(index)
{}

QT_END_NAMESPACE
