// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qssgrtutilities_p.h"

#include "qssgqmlutilities_p.h"
#include "qssgscenedesc_p.h"

#include <QtCore/qurl.h>
#include <QtCore/qbuffer.h>

#include <QtGui/qimage.h>
#include <QtGui/qimagereader.h>
#include <QtGui/qimagewriter.h>
#include <QtGui/qquaternion.h>

#include <QtQuick3DRuntimeRender/private/qssgrenderbuffermanager_p.h>

QT_BEGIN_NAMESPACE


// Actually set the property on node->obj, using QMetaProperty::write()
void QSSGRuntimeUtils::applyPropertyValue(const QSSGSceneDesc::Node *node, QObject *o, QSSGSceneDesc::Property *property)
{
    auto *obj = qobject_cast<QQuick3DObject *>(o ? o : node->obj);
    if (!obj)
        return;

    auto *metaObj = obj->metaObject();
    int propertyIndex = metaObj->indexOfProperty(property->name);
    if (propertyIndex < 0) {
        qWarning() << "QSSGSceneDesc: could not find property" << property->name << "in" << obj;
        return;
    }
    auto metaProp = metaObj->property(propertyIndex);
    QVariant value;

    auto metaId = property->value.metaType().id();
    auto *scene = node->scene;

    if (metaId == qMetaTypeId<QSSGSceneDesc::Node *>()) {
        const auto *valueNode = qvariant_cast<QSSGSceneDesc::Node *>(property->value);
        QObject *obj = valueNode ? valueNode->obj : nullptr;
        value = QVariant::fromValue(obj);
    } else if (metaId == qMetaTypeId<QSSGSceneDesc::Mesh *>()) { // Special handling for mesh nodes.
        // Mesh nodes does not have an equivalent in the QtQuick3D scene, but is registered
        // as a source property in the intermediate scene we therefore need to convert it to
        // be a usable source url now.
        const auto meshNode = qvariant_cast<const QSSGSceneDesc::Mesh *>(property->value);
        const auto url = meshNode ? QUrl(QSSGBufferManager::runtimeMeshSourceName(node->scene->id, meshNode->idx)) : QUrl{};
        value = QVariant::fromValue(url);
    } else if (metaId == qMetaTypeId<QUrl>()) {
        const auto url = qvariant_cast<QUrl>(property->value);
        // TODO: Use QUrl::resolved() instead??
        QString workingDir = scene->sourceDir;
        const QUrl qurl = url.isValid() ? QUrl::fromUserInput(url.path(), workingDir) : QUrl{};
        value = QVariant::fromValue(qurl);
    } else if (metaId == qMetaTypeId<QSSGSceneDesc::Flag>() && property->call) {
        // If we have a QSSGSceneDesc::Flag variant, then it came from setProperty(), and the setter function is defined.
        const auto flag = qvariant_cast<QSSGSceneDesc::Flag>(property->value);
        property->call->set(*obj, property->name, flag.value);
        qDebug() << "Flag special case, probably shouldn't happen" << node->name << property->name << property->value << flag.value;
        return;
    } else {
        value = property->value;
    }

    if (value.metaType().id() == qMetaTypeId<QString>()) {
        auto str = value.toString();
        auto propType = metaProp.metaType();
        if (propType.id() == qMetaTypeId<QVector3D>()) {
            QStringList l = str.split(u',');
            if (l.length() != 3) {
                qWarning() << "Wrong format for QVector3D:" << str;
            } else {
                QVector3D vec3(l.at(0).toFloat(), l.at(1).toFloat(), l.at(2).toFloat());
                value = QVariant::fromValue(vec3);
            }
        } else if (propType.id() == qMetaTypeId<QVector2D>()) {
            QStringList l = str.split(u',');
            if (l.length() != 2) {
                qWarning() << "Wrong format for QVector2D:" << str;
            } else {
                QVector2D vec(l.at(0).toFloat(), l.at(1).toFloat());
                value = QVariant::fromValue(vec);
            }
        } else if (propType.id() == qMetaTypeId<QVector4D>()) {
            QStringList l = str.split(u',');
            if (l.length() != 2) {
                qWarning() << "Wrong format for QVector4D:" << str;
            } else {
                QVector4D vec(l.at(0).toFloat(), l.at(1).toFloat(), l.at(2).toFloat(), l.at(3).toFloat());
                value = QVariant::fromValue(vec);
            }
        } else if (propType.id() == qMetaTypeId<QQuaternion>()) {
            QStringList l = str.split(u',');
            if (l.length() != 4) {
                qWarning() << "Wrong format for QQuaternion:" << str;
            } else {
                QQuaternion quat(l.at(0).toFloat(), l.at(1).toFloat(), l.at(2).toFloat(), l.at(3).toFloat());
                value = QVariant::fromValue(quat);
            }
        } else {
            // All other strings are supposed to be in QML-compatible format, so they can be written out directly
        }
    } else if (value.metaType().id() == qMetaTypeId<QSSGSceneDesc::NodeList*>()) {
        auto qmlListVar = metaProp.read(obj);
        // We have to write explicit code for each list property type, since metatype can't
        // tell us if we have a QQmlListProperty (if we had known, we could have made a naughty
        // hack and just static_cast to QQmlListProperty<QObject>
        if (qmlListVar.metaType().id() == qMetaTypeId<QQmlListProperty<QQuick3DMaterial>>()) {
            auto qmlList = qvariant_cast<QQmlListProperty<QQuick3DMaterial>>(qmlListVar);
            auto nodeList = qvariant_cast<QSSGSceneDesc::NodeList*>(value);
            auto head = reinterpret_cast<QSSGSceneDesc::Node **>(nodeList->head);

            for (int i = 0, end = nodeList->count; i != end; ++i)
                qmlList.append(&qmlList, qobject_cast<QQuick3DMaterial *>((*(head + i))->obj));

        } else {
            qWarning() << "Can't handle list property type" << qmlListVar.metaType();
        }
        return; //In any case, we can't send NodeList to QMetaProperty::write()
    }

    // qobject_cast doesn't work on nullptr, so we must convert the pointer manually
    if ((metaProp.metaType().flags() & QMetaType::PointerToQObject) && value.isNull())
        value.convert(metaProp.metaType()); //This will return false, but convert to the type anyway

    // Q_ENUMS with '|' is explicitly handled, otherwise uses QVariant::convert. This means
    // we get implicit qobject_cast and string to:
    //    QMetaType::Bool, QMetaType::QByteArray, QMetaType::QChar, QMetaType::QColor, QMetaType::QDate, QMetaType::QDateTime,
    //    QMetaType::Double, QMetaType::QFont, QMetaType::Int, QMetaType::QKeySequence, QMetaType::LongLong,
    //    QMetaType::QStringList, QMetaType::QTime, QMetaType::UInt, QMetaType::ULongLong, QMetaType::QUuid

    bool success = metaProp.write(obj, value);

    if (!success) {
        qWarning() << "Failure when setting property" << property->name << "to" << property->value << "maps to" << value
                   << "property metatype:" << metaProp.typeName();
    }
}


static void setProperties(QQuick3DObject &obj, const QSSGSceneDesc::Node &node, const QString &workingDir = {})
{
    using namespace QSSGSceneDesc;
    const auto &properties = node.properties;
    auto it = properties.begin();
    const auto end = properties.end();
    for (; it != end; ++it) {
        const auto &v = *it;
        if (!v->call) {
            QSSGRuntimeUtils::applyPropertyValue(&node, &obj, v);
            continue;
        }
        const auto &var = v->value;
        if (var.metaType().id() == qMetaTypeId<Node *>()) {
            const auto *node = qvariant_cast<Node *>(var);
            v->call->set(obj, v->name, node ? node->obj : nullptr);
        } else if (var.metaType() == QMetaType::fromType<Mesh *>()) { // Special handling for mesh nodes.
            // Mesh nodes does not have an equivalent in the QtQuick3D scene, but is registered
            // as a source property in the intermediate scene we therefore need to convert it to
            // be a usable source url now.
            const auto meshNode = qvariant_cast<const Mesh *>(var);
            const auto url = meshNode ? QUrl(QSSGBufferManager::runtimeMeshSourceName(node.scene->id, meshNode->idx)) : QUrl{};
            v->call->set(obj, v->name, &url);
        } else if (var.metaType() == QMetaType::fromType<QUrl>()) {
            const auto url = qvariant_cast<QUrl>(var);
            // TODO: Use QUrl::resolved() instead
            const QUrl qurl = url.isValid() ? QUrl::fromUserInput(url.toString(), workingDir) : QUrl{};
            v->call->set(obj, v->name, &qurl);
        } else if (var.metaType().id() == qMetaTypeId<QSSGSceneDesc::Flag>()) {
            const auto flag = qvariant_cast<QSSGSceneDesc::Flag>(var);
            v->call->set(obj, v->name, flag.value);
        } else {
            v->call->set(obj, v->name, var);
        }
    }
}

template<typename GraphObjectType, typename NodeType>
GraphObjectType *createRuntimeObject(NodeType &node, QQuick3DObject &parent)
{
    GraphObjectType *obj = qobject_cast<GraphObjectType *>(node.obj);
    if (!obj) {
        node.obj = qobject_cast<QQuick3DObject *>(obj = new GraphObjectType);
        obj->setParent(&parent);
        obj->setParentItem(&parent);
    }
    Q_ASSERT(obj == node.obj);

    return obj;
}

template<>
QQuick3DTextureData *createRuntimeObject<QQuick3DTextureData>(QSSGSceneDesc::TextureData &node, QQuick3DObject &parent)
{
    QQuick3DTextureData *obj = qobject_cast<QQuick3DTextureData *>(node.obj);
    if (!obj) {
        node.obj = qobject_cast<QQuick3DObject *>(obj = new QQuick3DTextureData);
        obj->setParent(&parent);
        obj->setParentItem(&parent);

        const auto &texData = node.data;
        const bool isCompressed = ((node.flgs & quint8(QSSGSceneDesc::TextureData::Flags::Compressed)) != 0);

        if (!texData.isEmpty()) {
            QImage image;
            if (isCompressed) {
                QByteArray data = texData;
                QBuffer readBuffer(&data);
                QImageReader imageReader(&readBuffer, node.fmt);
                image = imageReader.read();
                if (image.isNull())
                    qWarning() << imageReader.errorString();
            } else {
                const auto &size = node.sz;
                image = QImage(reinterpret_cast<const uchar *>(texData.data()), size.width(), size.height(), QImage::Format::Format_RGBA8888);
            }

            if (!image.isNull()) {
                const QPixelFormat pixFormat = image.pixelFormat();
                QImage::Format targetFormat = QImage::Format_RGBA8888_Premultiplied;
                QQuick3DTextureData::Format textureFormat = QQuick3DTextureData::Format::RGBA8;
                if (image.colorCount()) { // a palleted image
                    targetFormat = QImage::Format_RGBA8888;
                } else if (pixFormat.channelCount() == 1) {
                    targetFormat = QImage::Format_Grayscale8;
                    textureFormat = QQuick3DTextureData::Format::R8;
                } else if (pixFormat.alphaUsage() == QPixelFormat::IgnoresAlpha) {
                    targetFormat = QImage::Format_RGBX8888;
                } else if (pixFormat.premultiplied() == QPixelFormat::NotPremultiplied) {
                    targetFormat = QImage::Format_RGBA8888;
                }

                image.convertTo(targetFormat); // convert to a format mappable to QRhiTexture::Format
                image.flip(); // Flip vertically to the conventional Y-up orientation

                const auto bytes = image.sizeInBytes();
                obj->setSize(image.size());
                obj->setFormat(textureFormat);
                obj->setTextureData(QByteArray(reinterpret_cast<const char *>(image.constBits()), bytes));
            }
        }
    }

    return obj;
}


// Resources may refer to other resources and/or nodes, so we first generate all the resources without setting properties,
// then all the nodes with properties, and finally we set properties for the resources
// TODO: split this into different functions

void QSSGRuntimeUtils::createGraphObject(QSSGSceneDesc::Node &node,
                                         QQuick3DObject &parent, bool traverseChildrenAndSetProperties)
{
    using namespace QSSGSceneDesc;

    QQuick3DObject *obj = nullptr;
    switch (node.nodeType) {
    case Node::Type::Skeleton:
        // Skeleton is a resource that also needs to be in the node tree. We don't do that anymore.
        qWarning("Skeleton runtime import not supported");

        // NOTE: The skeleton is special as it's a resource and a node, the
        // hierarchical parent is therefore important here.
        if (!node.obj) {// 1st Phase : 'create Resources'
            obj = createRuntimeObject<QQuick3DSkeleton>(static_cast<Skeleton &>(node), parent);
        } else { // 2nd Phase : setParent for the Node hierarchy.
            obj = qobject_cast<QQuick3DSkeleton *>(node.obj);
            obj->setParent(&parent);
            obj->setParentItem(&parent);
        }
        break;
    case Node::Type::Joint:
        obj = createRuntimeObject<QQuick3DJoint>(static_cast<Joint &>(node), parent);
        break;
    case Node::Type::Skin:
        obj = createRuntimeObject<QQuick3DSkin>(static_cast<Skin &>(node), parent);
        break;
    case Node::Type::MorphTarget:
        obj = createRuntimeObject<QQuick3DMorphTarget>(static_cast<MorphTarget &>(node), parent);
        break;
    case Node::Type::Light:
    {
        auto &light = static_cast<Light &>(node);
        if (light.runtimeType == Node::RuntimeType::DirectionalLight)
            obj = createRuntimeObject<QQuick3DDirectionalLight>(light, parent);
        else if (light.runtimeType == Node::RuntimeType::PointLight)
            obj = createRuntimeObject<QQuick3DPointLight>(light, parent);
        else if (light.runtimeType == Node::RuntimeType::SpotLight)
            obj = createRuntimeObject<QQuick3DSpotLight>(light, parent);
        else
            Q_UNREACHABLE();
    }
        break;
    case Node::Type::Transform:
        obj = createRuntimeObject<QQuick3DNode>(node, parent);
        break;
    case Node::Type::Camera:
    {
        auto &camera = static_cast<Camera &>(node);
        if (camera.runtimeType == Node::RuntimeType::OrthographicCamera)
            obj = createRuntimeObject<QQuick3DOrthographicCamera>(camera, parent);
        else if (camera.runtimeType == Node::RuntimeType::PerspectiveCamera)
            obj = createRuntimeObject<QQuick3DPerspectiveCamera>(camera, parent);
        else if (camera.runtimeType == Node::RuntimeType::CustomCamera)
            obj = createRuntimeObject<QQuick3DCustomCamera>(camera, parent);
        else
            Q_UNREACHABLE();
    }
        break;
    case Node::Type::Model:
        obj = createRuntimeObject<QQuick3DModel>(static_cast<Model &>(node), parent);
        break;
    case Node::Type::Texture:
        if (node.runtimeType == Node::RuntimeType::TextureData)
            obj = createRuntimeObject<QQuick3DTextureData>(static_cast<TextureData &>(node), parent);
        else if (node.runtimeType == Node::RuntimeType::Image2D)
            obj = createRuntimeObject<QQuick3DTexture>(static_cast<Texture &>(node), parent);
        else if (node.runtimeType == Node::RuntimeType::ImageCube)
            obj = createRuntimeObject<QQuick3DCubeMapTexture>(static_cast<Texture &>(node), parent);
        else
            Q_UNREACHABLE();
        break;
    case Node::Type::Material:
    {
        if (node.runtimeType == Node::RuntimeType::PrincipledMaterial)
            obj = createRuntimeObject<QQuick3DPrincipledMaterial>(static_cast<Material &>(node), parent);
        else if (node.runtimeType == Node::RuntimeType::CustomMaterial)
            obj = createRuntimeObject<QQuick3DCustomMaterial>(static_cast<Material &>(node), parent);
        else if (node.runtimeType == Node::RuntimeType::SpecularGlossyMaterial)
            obj = createRuntimeObject<QQuick3DSpecularGlossyMaterial>(static_cast<Material &>(node), parent);
        else
            Q_UNREACHABLE();
    }
        break;
    case Node::Type::Mesh:
        // There's no runtime object for this type, but we need to register the mesh with the
        // buffer manager (this will happen once the mesh property is processed on the model).
        break;
    }

    if (obj && traverseChildrenAndSetProperties) {
        setProperties(*obj, node);
        for (auto &chld : node.children)
            createGraphObject(*chld, *obj);
    }
}

QQuick3DNode *QSSGRuntimeUtils::createScene(QQuick3DNode &parent, const QSSGSceneDesc::Scene &scene)
{
    if (!scene.root) {
        qWarning("Incomplete scene description (missing plugin?)");
        return nullptr;
    }

    Q_ASSERT(QQuick3DObjectPrivate::get(&parent)->sceneManager);

    QSSGBufferManager::registerMeshData(scene.id, scene.meshStorage);

    auto root = scene.root;
    for (const auto &resource : scene.resources)
        createGraphObject(*resource, parent, false);

    createGraphObject(*root, parent);

    // Some resources such as Skin have properties related with the node
    // hierarchy. Therefore, resources are handled after nodes.
    for (const auto &resource : scene.resources) {
        if (resource->obj != nullptr) // A mesh node has no runtime object.
            setProperties(static_cast<QQuick3DObject &>(*resource->obj), *resource, scene.sourceDir);
    }

    // Usually it makes sense to only enable 1 timeline at a time
    // so for now we just enable the first one.
    bool isFirstAnimation = true;
    for (const auto &anim: scene.animations) {
        QSSGQmlUtilities::createTimelineAnimation(*anim, root->obj, isFirstAnimation);
        if (isFirstAnimation)
            isFirstAnimation = false;
    }

    return qobject_cast<QQuick3DNode *>(scene.root->obj);
}

QT_END_NAMESPACE
