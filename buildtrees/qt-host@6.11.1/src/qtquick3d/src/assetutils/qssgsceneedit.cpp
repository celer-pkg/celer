// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgsceneedit_p.h"

#include <QtGui/QGuiApplication>

#include <QtCore/QVariant>
#include <QtCore/QHash>
#include <QtCore/QMetaProperty>
#include <QtCore/QUrl>

#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>

#include <QtQuick3DAssetImport/private/qssgassetimportmanager_p.h>
#include <QtQuick3DAssetUtils/private/qssgscenedesc_p.h>
#include <QtQuick3DAssetUtils/private/qssgqmlutilities_p.h>
#include <QtQuick3DAssetUtils/private/qssgrtutilities_p.h>

QT_BEGIN_NAMESPACE
using namespace Qt::Literals::StringLiterals;
namespace QSSGQmlUtilities {

static const char* typeNames[] =
{
    "Transform",
    "Camera",
    "Model",
    "Texture",
    "Material",
    "Light",
    "Mesh",
    "Skin",
    "Skeleton",
    "Joint",
    "MorphTarget",
    "ERROR"
};

static constexpr qsizetype nNodeTypes = std::size(typeNames) - 1;

static QSSGSceneDesc::Node::Type nodeTypeFromName(const QByteArrayView &typeName)
{
    int i = 0;
    while (i < nNodeTypes) {
        if (typeName == typeNames[i])
            break;
        ++i;
    }
    return QSSGSceneDesc::Node::Type(i);
}

static void replaceReferencesToResource(QSSGSceneDesc::Node *node, QSSGSceneDesc::Node *resource, QSSGSceneDesc::Node *replacement)
{
    for (auto *prop : node->properties) {
        auto &val = prop->value;
        if (qvariant_cast<QSSGSceneDesc::Node *>(val) == resource) {
            if (replacement)
                val = QVariant::fromValue(replacement);
        }
        if (val.metaType().id() == qMetaTypeId<QSSGSceneDesc::NodeList *>()) {
            const auto &list = *qvariant_cast<QSSGSceneDesc::NodeList *>(val);
            for (int i = 0, end = list.count; i != end; ++i) {
                if (list.head[i] == resource) {
                    list.head[i] = replacement;
                }
            }
        }
    }
    for (auto *child : node->children)
        replaceReferencesToResource(child, resource, replacement);
}

// TODO: optimize this by using a hashmap or similar
static QSSGSceneDesc::Node *findNode(QSSGSceneDesc::Node *root, const QByteArrayView name,
                                     QSSGSceneDesc::Node::Type type, QSSGSceneDesc::Node **parent = nullptr)
{
    if (!root || name.isEmpty())
        return nullptr;

    if (root->name == name && root->nodeType == type)
        return root;

    for (auto *child : root->children) {
        if (auto *ret = findNode(child, name, type, parent)) {
            if (parent && !*parent)
                *parent = root;
            return ret;
        }
    }
    return nullptr;
}

static QSSGSceneDesc::Node *findResource(const QSSGSceneDesc::Scene *scene, const QByteArrayView &name, QSSGSceneDesc::Node::Type nodeType)
{
    if (name.isEmpty())
        return nullptr; // Empty strings by definition means nothing
    for (auto *resource : scene->resources) {
        if (resource->name == name && resource->nodeType == nodeType)
            return resource;
    }

    return nullptr;
}

using NodeSet = QSet<QSSGSceneDesc::Node *>;
typedef bool NodeFilter(QSSGSceneDesc::Node *);

static NodeSet flattenTree(QSSGSceneDesc::Node *node, NodeFilter *excludeFunction = nullptr)
{
    NodeSet ret = { node };
    for (auto *child : node->children)
        if (!excludeFunction || !excludeFunction(child))
            ret.unite(flattenTree(child));
    return ret;
}

static void unlinkChild(QSSGSceneDesc::Node *child, QSSGSceneDesc::Node *parent)
{
    parent->children.removeOne(child);
}

static void removeFromAnimation(QSSGSceneDesc::Animation *animation, const NodeSet &nodes)
{
    auto isTargeted = [nodes](QSSGSceneDesc::Animation::Channel *channel) { return nodes.contains(channel->target); };
    const auto end_it = animation->channels.end();
    auto remove_it = std::remove_if(animation->channels.begin(), end_it, isTargeted);
    for (auto it = remove_it; it != end_it; ++it)
        delete *it;
    animation->channels.erase(remove_it, end_it);
}

static void deleteTree(QSSGSceneDesc::Node *node)
{
    const auto children = flattenTree(node);
    for (auto *animation : node->scene->animations)
        removeFromAnimation(animation, children);
    for (auto *child : children)
        delete child;
}

static void removeProperty(QSSGSceneDesc::Node *node, const QByteArrayView &name)
{
    auto *propList = &node->properties;

    auto findName = [name](QSSGSceneDesc::Property *p) { return p->name == name; };
    auto it = std::find_if(propList->begin(), propList->end(), findName);
    if (it != propList->end()) {
        QSSGSceneDesc::Property *p = *it;
        propList->erase(it);
        delete p;
    }
}

static QSSGSceneDesc::Node *nodeFromJson(const QSSGSceneDesc::Scene *scene, const QJsonObject &nodeRef)
{
    auto it = nodeRef.constBegin();
    if (it == nodeRef.constEnd())
        return nullptr;
    auto nodeType = nodeTypeFromName(it.key().toUtf8());
    auto nodeName = it.value().toString().toUtf8();
    auto *node = findResource(scene, nodeName, nodeType);
    if (!node)
        node = findNode(scene->root, nodeName, nodeType);
    return node;
}

static QSSGSceneDesc::NodeList *nodeListFromJson(const QSSGSceneDesc::Scene *scene, const QJsonArray &array)
{
    QVarLengthArray<QSSGSceneDesc::Node *> nodes;

    for (auto json : array) {
        auto *node = nodeFromJson(scene, json.toObject());
        if (!node) {
            qWarning() << "Could not find node for" << json;
            continue;
        }
        nodes.append(node);
    }
    auto *nodeList = new QSSGSceneDesc::NodeList(reinterpret_cast<void **>(nodes.data()), nodes.count());
    return nodeList;
}

/*
  JSON format

  Node reference:  {"<nodeTypeName>": "<name>"}
  URL:             {"url": "<filepath>"}
  List:             [ {"<nodeTypeName>": "<name>"}, ... ]
  */

void setProperty(QSSGSceneDesc::Node *node, const QStringView propertyName, const QJsonValue &value)
{
    QVariant var;

    if (value.isArray()) {
        var = QVariant::fromValue(nodeListFromJson(node->scene, value.toArray()));
    } else if (value.isObject()) {
        auto obj = value.toObject();
        if (obj.contains(u"url")) {
            auto path = obj.value(u"url").toString();
            var = QVariant::fromValue(QUrl(path));
        } else {
            QSSGSceneDesc::Node *n = nodeFromJson(node->scene, obj);
            var = QVariant::fromValue(n);
        }
    } else {
        var = value.toVariant(); // The rest of the special handling happens in QSSGRuntimeUtils::applyPropertyValue
    }

    const auto name = propertyName.toUtf8();
    removeProperty(node, name); // TODO: change property if it exists, instead of deleting and adding
    auto *property = QSSGSceneDesc::setProperty(*node, name, std::move(var));

    if (node->obj)
        QSSGRuntimeUtils::applyPropertyValue(node, node->obj, property);
}


QSSGSceneDesc::Node *addResource(QSSGSceneDesc::Scene *scene, const QJsonObject &addition)
{
    auto name = addition.value(u"name").toString().toUtf8();
    auto typeName = addition.value(u"type").toString().toUtf8();
    if (name.isEmpty() || typeName.isEmpty()) {
        qWarning("Can't create node without name or type");
        return nullptr;
    }

    QSSGSceneDesc::Node *node = nullptr;
    QSSGSceneDesc::Node *prevResource = findResource(scene, name, nodeTypeFromName(typeName));

    if (typeName == "Material") {
        bool isSpecGlossy = addition.contains(u"albedoColor") || addition.contains(u"albedoMap")
                || addition.contains(u"glossinessMap") || addition.contains(u"glossiness");
        typeName = isSpecGlossy ? "SpecularGlossyMaterial" : "PrincipledMaterial";
    }

    if (typeName == "PrincipledMaterial") {
        node = new QSSGSceneDesc::Node(name, QSSGSceneDesc::Node::Type::Material,
                                       QSSGRenderGraphObject::Type::PrincipledMaterial);
    } else if (typeName == "SpecularGlossyMaterial") {
        node = new QSSGSceneDesc::Node(name, QSSGSceneDesc::Node::Type::Material,
                                       QSSGRenderGraphObject::Type::SpecularGlossyMaterial);
    } else if (typeName == "Texture") {
        node = new QSSGSceneDesc::Node(name, QSSGSceneDesc::Node::Type::Texture,
                                       QSSGRenderGraphObject::Type::Image2D);
    } else {
        qWarning() << "Not supported. Don't know how to create" << typeName;
        return nullptr;
    }
    Q_ASSERT(node);
    node->scene = scene;
    for (auto it = addition.constBegin(); it != addition.constEnd(); ++it) {
        const auto &propertyName = it.key();
        if (propertyName == u"name" || propertyName == u"type" || propertyName == u"comment" || propertyName == u"command")
            continue;
        setProperty(node, it.key(), it.value());
    }

    if (prevResource) {
        replaceReferencesToResource(scene->root, prevResource, node);
        scene->resources.removeOne(prevResource);
        delete prevResource;
    }

    QSSGSceneDesc::addNode(*scene, *node);
    return node;
}

void applyEdit(QSSGSceneDesc::Scene *scene, const QJsonObject &changes)
{
    auto doApply = [scene](const QJsonObject &obj) {
        QByteArray name = obj.value(u"name").toString().toUtf8();
        QByteArray typeName = obj.value(u"type").toString().toUtf8();
        auto command = obj.value(u"command").toString(u"edit"_s);
        auto nodeType = nodeTypeFromName(typeName);
        if (command == u"edit") {
            auto *node = findNode(scene->root, name, nodeType);
            if (!node)
                node = findResource(scene, name, nodeType);
            if (node) {
                for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
                    const auto &propertyName = it.key();
                    if (propertyName == u"name" || propertyName == u"type" || propertyName == u"comment" || propertyName == u"command")
                        continue;
                    setProperty(node, it.key(), it.value());
                }
            }
        } else if (command == u"add") {
            addResource(scene, obj);
        } else if (command == u"delete") {
            QSSGSceneDesc::Node *parent = nullptr;
            auto *node = findNode(scene->root, name, nodeType, &parent);
            if (node) {
                deleteTree(node);
                if (parent)
                    unlinkChild(node, parent);
                else
                    qWarning("Delete: could not find parent for node");
            }
        }
    };

    const auto editList = changes.value(u"editList").toArray();

    // Do all the adds first, since the edits may depend on them
    // If adds depend on each other, they need to be in dependency order
    for (auto edit : editList) {
        auto obj = edit.toObject();
        if (obj.value(u"command") == u"add"_s)
            doApply(obj);
    }

    for (auto edit : editList) {
        auto obj = edit.toObject();
        if (obj.value(u"command") != u"add"_s)
            doApply(obj);
    }

}

}

QT_END_NAMESPACE
