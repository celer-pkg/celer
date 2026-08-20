// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "parser.h"

#include <QtCore/qdir.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qstringview.h>

#include <QtQml/qqmllist.h>

// Parsing
#include <QtQml/private/qqmljsengine_p.h>
#include <QtQml/private/qqmljslexer_p.h>
#include <QtQml/private/qqmljsparser_p.h>

#include <QtQuick3D/private/qquick3dobject_p.h>
#include <QtQuick3D/private/qquick3dviewport_p.h>
// Scene
#include <QtQuick3D/private/qquick3dsceneenvironment_p.h>
// Material(s)
#include <QtQuick3D/private/qquick3dprincipledmaterial_p.h>
#include <QtQuick3D/private/qquick3ddefaultmaterial_p.h>
#include <QtQuick3D/private/qquick3dcustommaterial_p.h>
// Lights
#include <QtQuick3D/private/qquick3dspotlight_p.h>
#include <QtQuick3D/private/qquick3dpointlight_p.h>
#include <QtQuick3D/private/qquick3ddirectionallight_p.h>

// Instancing
#include <QtQuick3D/private/qquick3dinstancing_p.h>

#include <QtQuick3D/private/qquick3dshaderutils_p.h>

#include <QtQuick3DUtils/private/qssginvasivelinkedlist_p.h>

#include <QtGui/qquaternion.h>

QT_BEGIN_NAMESPACE

template<typename T, T *T::*N = &T::next>
struct InvasiveListView : protected QSSGInvasiveSingleLinkedList<T, N>
{
    using QSSGInvasiveSingleLinkedList<T, N>::begin;
    using QSSGInvasiveSingleLinkedList<T, N>::end;
    using QSSGInvasiveSingleLinkedList<T, N>::m_head;

    explicit InvasiveListView(const T &obj) { m_head = &const_cast<T &>(obj); }
};

template <typename T> struct TypeInfo
{
    static constexpr const char *qmlTypeName() { return "Unknown"; }
    static constexpr int typeId() { return QMetaType::UnknownType; }
};

#define DECLARE_QQ3D_TYPE(CLASS, TYPE_NAME) \
template<> struct TypeInfo<CLASS> \
{ \
    static constexpr const char *qmlTypeName() { return #TYPE_NAME; } \
    inline static constexpr int typeId() { return qMetaTypeId<CLASS>(); } \
    inline static constexpr int qmlListTypeId() { return qMetaTypeId<QQmlListProperty<CLASS>>(); } \
}

DECLARE_QQ3D_TYPE(QQuick3DViewport, View3D);
DECLARE_QQ3D_TYPE(QQuick3DSceneEnvironment, SceneEnvironment);
DECLARE_QQ3D_TYPE(QQuick3DMaterial, Material);
DECLARE_QQ3D_TYPE(QQuick3DPrincipledMaterial, PrincipledMaterial);
DECLARE_QQ3D_TYPE(QQuick3DDefaultMaterial, DefaultMaterial);
DECLARE_QQ3D_TYPE(QQuick3DCustomMaterial, CustomMaterial);
DECLARE_QQ3D_TYPE(QQuick3DDirectionalLight, DirectionalLight);
DECLARE_QQ3D_TYPE(QQuick3DPointLight, PointLight);
DECLARE_QQ3D_TYPE(QQuick3DSpotLight, SpotLight);
DECLARE_QQ3D_TYPE(QQuick3DTexture, Texture);
DECLARE_QQ3D_TYPE(QQuick3DShaderUtilsTextureInput, TextureInput);
DECLARE_QQ3D_TYPE(QQuick3DModel, Model);
DECLARE_QQ3D_TYPE(QQuick3DEffect, Effect);
DECLARE_QQ3D_TYPE(QQuick3DShaderUtilsRenderPass, Pass);
DECLARE_QQ3D_TYPE(QQuick3DShaderUtilsShader, Shader);
DECLARE_QQ3D_TYPE(QQuick3DInstanceList, InstanceList);
DECLARE_QQ3D_TYPE(QQuick3DInstanceListEntry, InstanceListEntry);

using QmlTypeNames = QHash<QString, int>;

#define QQ3D_TYPE_ENTRY(TYPE) { TypeInfo<TYPE>::qmlTypeName(), TypeInfo<TYPE>::typeId() }

QmlTypeNames baseTypeMap()
{
    return { QQ3D_TYPE_ENTRY(QQuick3DViewport),
             QQ3D_TYPE_ENTRY(QQuick3DSceneEnvironment),
             QQ3D_TYPE_ENTRY(QQuick3DPrincipledMaterial),
             QQ3D_TYPE_ENTRY(QQuick3DDefaultMaterial),
             QQ3D_TYPE_ENTRY(QQuick3DCustomMaterial),
             QQ3D_TYPE_ENTRY(QQuick3DDirectionalLight),
             QQ3D_TYPE_ENTRY(QQuick3DPointLight),
             QQ3D_TYPE_ENTRY(QQuick3DSpotLight),
             QQ3D_TYPE_ENTRY(QQuick3DTexture),
             QQ3D_TYPE_ENTRY(QQuick3DShaderUtilsTextureInput),
             QQ3D_TYPE_ENTRY(QQuick3DModel),
             QQ3D_TYPE_ENTRY(QQuick3DEffect),
             QQ3D_TYPE_ENTRY(QQuick3DShaderUtilsRenderPass),
             QQ3D_TYPE_ENTRY(QQuick3DShaderUtilsShader),
             QQ3D_TYPE_ENTRY(QQuick3DInstanceList),
             QQ3D_TYPE_ENTRY(QQuick3DInstanceListEntry)
    };
}

Q_GLOBAL_STATIC(QmlTypeNames, s_typeMap)

struct Context
{
    enum class Type
    {
        Application,
        Component
    };

    struct Property {
        enum MemberState : quint8
        {
            Uninitialized,
            Initialized
        };

        QObject *target = nullptr;
        QStringView name;
        int targetType = QMetaType::UnknownType;
        QMetaType::Type type = QMetaType::UnknownType;
        MemberState memberState = Uninitialized;
    };

    template<typename T> using Vector = QVector<T>;
    using InterceptObjDefFunc = bool (*)(const QQmlJS::AST::UiObjectDefinition &, Context &, int &);
    using InterceptObjBinding = bool (*)(const QQmlJS::AST::UiObjectBinding &, Context &, int &);
    using InterceptPublicMember = bool (*)(const QQmlJS::AST::UiPublicMember &, Context &, int &);
    using InterceptCallExpression = bool (*)(const QQmlJS::AST::CallExpression &, Context &, int &);

    QQmlJS::Engine *engine = nullptr;
    QDir workingDir; // aka source directory
    QFileInfo currentFileInfo;

    MaterialParser::SceneData sceneData;
    Property property;

    struct Component
    {
        QObject *ptr = nullptr;
        int type = QMetaType::UnknownType;
    };

    QHash<QStringView, QObject *> identifierMap;
    QHash<QString, Component> components;
    InterceptObjDefFunc interceptODFunc = nullptr;
    InterceptObjBinding interceptOBFunc = nullptr;
    InterceptPublicMember interceptPMFunc = nullptr;
    InterceptCallExpression interceptCallExpr = nullptr;
    Type type = Type::Application;
    bool dbgprint = false;
};

Q_DECLARE_TYPEINFO(Context::Component, Q_PRIMITIVE_TYPE);

namespace BuiltinHelpers {

using ArgumentListView = InvasiveListView<QQmlJS::AST::ArgumentList>;

template <typename T> Q_REQUIRED_RESULT constexpr quint8 componentCount() { Q_STATIC_ASSERT(true); return 0; }
template <> Q_REQUIRED_RESULT constexpr quint8 componentCount<QVector2D>() { return 2; }
template <> Q_REQUIRED_RESULT constexpr quint8 componentCount<QVector3D>() { return 3; }
template <> Q_REQUIRED_RESULT constexpr quint8 componentCount<QVector4D>() { return 4; }

static double expressionValue(const QQmlJS::AST::ExpressionNode &expr) {
    using namespace QQmlJS::AST;

    if (expr.kind == Node::Kind_NumericLiteral) {
        return static_cast<const NumericLiteral &>(expr).value;
    } else if (expr.kind == Node::Kind_UnaryMinusExpression) {
        const auto &minusExpr = static_cast<const UnaryMinusExpression &>(expr);
        if (minusExpr.expression && minusExpr.expression->kind == Node::Kind_NumericLiteral)
            return static_cast<const NumericLiteral &>(*minusExpr.expression).value * -1.0;
    } else if (expr.kind == Node::Kind_UnaryPlusExpression) {
        const auto &plusExpr = static_cast<const UnaryPlusExpression &>(expr);
        if (plusExpr.expression && plusExpr.expression->kind == Node::Kind_NumericLiteral)
            return static_cast<const NumericLiteral &>(*plusExpr.expression).value;
    } else {
        printf("Expression type \'%d\' unhandled!\n", expr.kind);
    }

    return 0.0;
}

template <typename T>
static inline bool setProperty(const Context::Property &property, const T &v)
{
    return property.target->setProperty(property.name.toLatin1(), QVariant::fromValue(v));
}

template<typename Vec>
static Vec toVec(const ArgumentListView &list, bool *ok = nullptr)
{
    using namespace QQmlJS::AST;
    int i = 0;
    const int e = componentCount<Vec>();
    Vec vec;
    for (const auto &listItem : list) {
        if (listItem.expression && i != e)
            vec[i] = expressionValue(*listItem.expression);
        ++i;
    }

    if (ok)
        *ok = (i == e);

    return vec;
}

static QPointF toPoint(const ArgumentListView &list, bool *ok = nullptr)
{
    using namespace QQmlJS::AST;
    int i = 0;
    const int e = 2;
    qreal args[e];
    for (const auto &listItem : list) {
        if (listItem.expression && i != e)
            args[i] = expressionValue(*listItem.expression);
        ++i;
    }

    if (ok)
        *ok = (i == e);

    return QPointF(args[0], args[1]);
}

static QSizeF toSize(const ArgumentListView &list, bool *ok = nullptr)
{
    using namespace QQmlJS::AST;
    int i = 0;
    const int e = 2;
    qreal args[e];
    for (const auto &listItem : list) {
        if (listItem.expression && listItem.expression->kind == Node::Kind_NumericLiteral && i != e)
            args[i] = expressionValue(*listItem.expression);
        ++i;
    }

    if (ok)
        *ok = (i == e);

    return QSizeF(args[0], args[1]);
}

static QRectF toRect(const ArgumentListView &list, bool *ok = nullptr)
{
    using namespace QQmlJS::AST;
    int i = 0;
    const int e = 4;
    qreal args[e];
    for (const auto &listItem : list) {
        if (listItem.expression && i != e)
            args[i] = expressionValue(*listItem.expression);
        ++i;
    }

    if (ok)
        *ok = (i == e);

    return QRectF(args[0], args[1], args[2], args[3]);
}

static QMatrix4x4 toMat44(const ArgumentListView &list, bool *ok = nullptr)
{
    using namespace QQmlJS::AST;
    int i = 0;
    const int e = 16;
    float args[e];
    for (const auto &listItem : list) {
        if (listItem.expression && i != e)
            args[i] = float(expressionValue(*listItem.expression));
        ++i;
    }

    if (ok)
        *ok = (i == e);

    return QMatrix4x4(args);
}

static QQuaternion toQuaternion(const ArgumentListView &list, bool *ok = nullptr)
{
    using namespace QQmlJS::AST;
    int i = 0;
    const int e = 4;
    float args[e];
    for (const auto &listItem : list) {
        if (listItem.expression && listItem.expression->kind == Node::Kind_NumericLiteral && i != e)
            args[i] = float(expressionValue(*listItem.expression));
        ++i;
    }

    if (ok)
        *ok = (i == e);

    return QQuaternion(args[0], args[1], args[2], args[3]);
}

// String variants
// Note: Unlike the call variants we assume arguments are correct (can be converted),
// as they should have failed earlier, during parsing, if they were not.
template <typename Vec>
static Vec toVec(const QStringView &ref)
{
    const auto args = ref.split(u',');
    Vec vec;
    bool ok = false;
    if (args.size() == componentCount<Vec>()) {
        for (int i = 0; i != componentCount<Vec>(); ++i) {
            vec[i] = args.at(i).toDouble(&ok);
            Q_ASSERT(ok);
        }
    }

    return vec;
}

static QPointF toPoint(const QStringView &ref)
{
    const auto args = ref.split(u",");
    if (args.size() == 2) {
        bool ok = false;
        const auto arg0 = args.at(0).toDouble(&ok);
        Q_ASSERT(ok);
        const auto arg1 = args.at(1).toDouble(&ok);
        Q_ASSERT(ok);
        return QPointF(arg0, arg1);
    }
    return QPointF();
}

static QSizeF toSize(const QStringView &ref)
{
    const auto args = ref.split(u'x');
    if (args.size() == 2) {
        bool ok = false;
        const auto arg0 = args.at(0).toDouble(&ok);
        Q_ASSERT(ok);
        const auto arg1 = args.at(1).toDouble(&ok);
        Q_ASSERT(ok);
        return QSizeF(arg0, arg1);
    }
    return QSizeF();
}

static QRectF toRect(const QStringView &ref)
{
    auto args = ref.split(u",");
    if (args.size() == 3) {
        bool ok = false;
        const auto arg0 = args.at(0).toDouble(&ok);
        Q_ASSERT(ok);
        const auto arg1 = args.at(1).toDouble(&ok);
        Q_ASSERT(ok);
        args = args.at(2).split(u'x');
        if (args.size() == 2) {
            const auto arg2 = args.at(0).toDouble(&ok);
            Q_ASSERT(ok);
            const auto arg3 = args.at(1).toDouble(&ok);
            Q_ASSERT(ok);
            return QRectF(arg0, arg1, arg2, arg3);
        }
    }
    return QRectF();
}

static QQuaternion toQuaternion(const QStringView &ref)
{
    const auto args = ref.split(u',');
    if (args.size() == 4) {
        bool ok = false;
        const auto arg0 = args.at(0).toDouble(&ok);
        Q_ASSERT(ok);
        const auto arg1 = args.at(1).toDouble(&ok);
        Q_ASSERT(ok);
        const auto arg2 = args.at(2).toDouble(&ok);
        Q_ASSERT(ok);
        const auto arg3 = args.at(3).toDouble(&ok);
        Q_ASSERT(ok);
        return QQuaternion(arg0, arg1, arg2, arg3);
    }
    return QQuaternion();
}

} // BuiltinHelpers

template <typename T>
static void cloneQmlList(const QObject &so, QMetaProperty &sp, QObject &to, QMetaProperty &tp) {
    Q_ASSERT(sp.typeId() == tp.typeId());
    const auto tv = tp.read(&to);
    const auto sv = sp.read(&so);
    if (sv.isValid() && tv.isValid()) {
        auto tl = tv.value<QQmlListProperty<T>>();
        auto sl = sv.value<QQmlListProperty<T>>();
        const auto count = sl.count(&sl);
        for (int i = 0; count != i; ++i)
            tl.append(&tl, sl.at(&sl, i));
    }
}

static void cloneProperties(QObject &target, const QObject &source)
{
    Q_ASSERT(target.metaObject() == source.metaObject());
    const auto smo = source.metaObject();
    auto tmo = target.metaObject();
    const int propCount = smo->propertyCount();
    for (int i = 0; i != propCount; ++i) {
        auto sp = smo->property(i);
        auto tp = tmo->property(i);
        if (sp.typeId() == tp.typeId()) {
            if (sp.typeId() == TypeInfo<QQuick3DMaterial>::qmlListTypeId())
                cloneQmlList<QQuick3DMaterial>(source, sp, target, tp);
            else if (sp.typeId() == TypeInfo<QQuick3DEffect>::qmlListTypeId())
                cloneQmlList<QQuick3DEffect>(source, sp, target, tp);
            else if (sp.typeId() == TypeInfo<QQuick3DShaderUtilsRenderPass>::qmlListTypeId())
                cloneQmlList<QQuick3DShaderUtilsRenderPass>(source, sp, target, tp);
            else if (sp.typeId() == TypeInfo<QQuick3DShaderUtilsShader>::qmlListTypeId())
                cloneQmlList<QQuick3DShaderUtilsShader>(source, sp, target, tp);
            else
                tmo->property(i).write(&target, smo->property(i).read(&source));
        }
    }

    // Clone the dynamic properties as well
    for (const auto &prop : source.dynamicPropertyNames())
        target.setProperty(prop.constData(), source.property(prop.constData()));
}

template <typename T>
inline QVariant fromStringEnumHelper(const QStringView &ref, const QMetaProperty &property)
{
    bool ok = false;
    const int v = property.enumerator().keyToValue(ref.toLatin1(), &ok);
    return ok ? QVariant::fromValue(T(v)) : QVariant();
}

static QVariant fromString(const QStringView &ref, const Context &ctx)
{
    const auto &p = ctx.property;
    if (!p.target)
        return QVariant();

    static const auto toBuiltinType = [](int type, const QStringView &ref, const QDir &workingDir) {
        using namespace BuiltinHelpers;
        bool ok = false;
        switch (type) {
        case QMetaType::Int:
        {
            const auto v = ref.toInt(&ok);
            return (ok ? QVariant::fromValue(v) : QVariant());
        }
            break;
        case QMetaType::Bool:
        {
            const auto v = ref.toInt(&ok);
            return (ok ? QVariant::fromValue(bool(v)) : QVariant());
        }
        case QMetaType::Double:
        {
            const auto v = ref.toDouble(&ok);
            return (ok ? QVariant::fromValue(qreal(v)) : QVariant());
        }
            break;
        case QMetaType::QString:
            return QVariant::fromValue(ref);
        case QMetaType::QUrl:
        {
            if (ref.startsWith(u':') || ref.startsWith(QDir::separator()))
                return QVariant::fromValue(QUrl::fromLocalFile(ref.toString()));
            else if (ref.startsWith(u'#'))
                return QVariant::fromValue(QUrl(ref.toString()));
            else
                return QVariant::fromValue(QUrl::fromUserInput(ref.toString(), workingDir.canonicalPath()));
        }
        case QMetaType::QColor:
            return QVariant::fromValue(QColor(ref));
        case QMetaType::QTime:
            return QVariant::fromValue(QTime::fromString(ref.toString()));
        case QMetaType::QDate:
            return QVariant::fromValue(QDate::fromString(ref.toString()));
        case QMetaType::QDateTime:
            return QVariant::fromValue(QDateTime::fromString(ref.toString()));
        case QMetaType::QRectF:
            return QVariant::fromValue(toRect(ref));
        case QMetaType::QPointF:
            return QVariant::fromValue(toPoint(ref));
        case QMetaType::QSizeF:
            return QVariant::fromValue(toSize(ref));
        case QMetaType::QVector2D:
            return QVariant::fromValue(toVec<QVector2D>(ref));
        case QMetaType::QVector3D:
            return QVariant::fromValue(toVec<QVector3D>(ref));
        case QMetaType::QVector4D:
            return QVariant::fromValue(toVec<QVector4D>(ref));
        case QMetaType::QQuaternion:
            return QVariant::fromValue(toQuaternion(ref));
        }

        return QVariant();
    };

    if (p.type != QMetaType::UnknownType) // Built in Qt types int, vector3d etc
        return toBuiltinType(p.type, ref, ctx.workingDir);

    // hard mode, detect the property type
    // We only care about the types that are relevant for us
    if (p.targetType != QMetaType::UnknownType) {
        Q_ASSERT(p.target);
        Q_ASSERT(!p.name.isEmpty());
        const int idx = p.target->metaObject()->indexOfProperty(p.name.toLatin1().constData());
        const auto property = p.target->metaObject()->property(idx);
        if (property.metaType().id() >= QMetaType::User) {
            const QMetaType metaType = property.metaType();
            if (p.targetType == TypeInfo<QQuick3DDefaultMaterial>::typeId() || p.targetType == TypeInfo<QQuick3DPrincipledMaterial>::typeId()) {
                // Common for both materials
                if (metaType.id() == qMetaTypeId<QQuick3DMaterial::CullMode>())
                    return fromStringEnumHelper<QQuick3DMaterial::CullMode>(ref, property);
                if (metaType.id() == qMetaTypeId<QQuick3DMaterial::TextureChannelMapping>())
                    return fromStringEnumHelper<QQuick3DMaterial::TextureChannelMapping>(ref, property);

                if (p.targetType == TypeInfo<QQuick3DPrincipledMaterial>::typeId()) {
                    if (metaType.id() == qMetaTypeId<QQuick3DPrincipledMaterial::Lighting>())
                        return fromStringEnumHelper<QQuick3DPrincipledMaterial::Lighting>(ref, property);
                    if (metaType.id() == qMetaTypeId<QQuick3DPrincipledMaterial::BlendMode>())
                        return fromStringEnumHelper<QQuick3DPrincipledMaterial::BlendMode>(ref, property);
                    if (metaType.id() == qMetaTypeId<QQuick3DPrincipledMaterial::AlphaMode>())
                        return fromStringEnumHelper<QQuick3DPrincipledMaterial::AlphaMode>(ref, property);
                } else if (p.targetType == TypeInfo<QQuick3DDefaultMaterial>::typeId()) {
                    if (metaType.id() == qMetaTypeId<QQuick3DDefaultMaterial::Lighting>())
                        return fromStringEnumHelper<QQuick3DDefaultMaterial::Lighting>(ref, property);
                    if (metaType.id() == qMetaTypeId<QQuick3DDefaultMaterial::BlendMode>())
                        return fromStringEnumHelper<QQuick3DDefaultMaterial::BlendMode>(ref, property);
                    if (metaType.id() == qMetaTypeId<QQuick3DDefaultMaterial::SpecularModel>())
                        return fromStringEnumHelper<QQuick3DDefaultMaterial::SpecularModel>(ref, property);
                }
            } else if (p.targetType == TypeInfo<QQuick3DCustomMaterial>::typeId()) {
                if (metaType.id() == qMetaTypeId<QQuick3DCustomMaterial::ShadingMode>())
                    return fromStringEnumHelper<QQuick3DCustomMaterial::ShadingMode>(ref, property);
                if (metaType.id() == qMetaTypeId<QQuick3DCustomMaterial::BlendMode>())
                    return fromStringEnumHelper<QQuick3DCustomMaterial::BlendMode>(ref, property);
            } else if (p.targetType == TypeInfo<QQuick3DSpotLight>::typeId() || p.targetType == TypeInfo<QQuick3DPointLight>::typeId()) {
                if (metaType.id() == qMetaTypeId<QQuick3DAbstractLight::QSSGShadowMapQuality>())
                    return fromStringEnumHelper<QQuick3DAbstractLight::QSSGShadowMapQuality>(ref, property);
            } else if (p.targetType == TypeInfo<QQuick3DSceneEnvironment>::typeId()) {
                if (metaType.id() == qMetaTypeId<QQuick3DSceneEnvironment::QQuick3DEnvironmentBackgroundTypes>())
                    return fromStringEnumHelper<QQuick3DSceneEnvironment::QQuick3DEnvironmentBackgroundTypes>(ref, property);
                if (metaType.id() == qMetaTypeId<QQuick3DSceneEnvironment::QQuick3DEnvironmentAAModeValues>())
                    return fromStringEnumHelper<QQuick3DSceneEnvironment::QQuick3DEnvironmentAAModeValues>(ref, property);
                if (metaType.id() == qMetaTypeId<QQuick3DSceneEnvironment::QQuick3DEnvironmentAAQualityValues>())
                    return fromStringEnumHelper<QQuick3DSceneEnvironment::QQuick3DEnvironmentAAQualityValues>(ref, property);
                if (metaType.id() == qMetaTypeId<QQuick3DSceneEnvironment::QQuick3DEnvironmentTonemapModes>())
                    return fromStringEnumHelper<QQuick3DSceneEnvironment::QQuick3DEnvironmentTonemapModes>(ref, property);
            } else if (p.targetType == TypeInfo<QQuick3DShaderUtilsShader>::typeId()) {
                if (metaType.id() == qMetaTypeId<QQuick3DShaderUtilsShader::Stage>())
                    return fromStringEnumHelper<QQuick3DShaderUtilsShader::Stage>(ref, property);
            }
        } else { // Qt type
            return toBuiltinType(property.metaType().id(), ref, ctx.workingDir);
        }
    }

    if (ctx.dbgprint)
        printf("Unhandled type for property %s\n", ref.toLatin1().constData());

    return QVariant();
}

static QString getQmlFileExtension() { return QStringLiteral("qml"); }

struct Visitors
{
    static void visit(const QQmlJS::AST::UiProgram &program, Context &ctx, int &ret)
    {
        using namespace QQmlJS::AST;
        const bool readHeaders = false;
        if (readHeaders && program.headers) { // No real need for us to look at the includes
            using HeaderItem = UiHeaderItemList;
            using Headers = InvasiveListView<HeaderItem>;

            Headers headers(*program.headers);
            for (const auto &header : headers)
                printf("Type: %d\n", header.kind);
        }
        if (program.members)
            visit(*program.members, ctx, ret);
    }
    static void visit(const QQmlJS::AST::UiObjectInitializer &objInitializer, Context &ctx, int &ret)
    {
        using namespace QQmlJS::AST;
        if (objInitializer.members)
            visit(*objInitializer.members, ctx, ret);
    }

    static void visit(const QQmlJS::AST::UiObjectDefinition &def, Context &ctx, int &ret)
    {
        using namespace QQmlJS::AST;
        if (ctx.dbgprint)
            printf("Object definition -> %s\n", def.qualifiedTypeNameId->name.toLocal8Bit().constData());
        if (!(ctx.interceptODFunc && ctx.interceptODFunc(def, ctx, ret))) {
            if (def.initializer)
                visit(*static_cast<UiObjectInitializer *>(def.initializer), ctx, ret);
        }
    }

    static void visit(const QQmlJS::AST::UiArrayMemberList &memberList, Context &ctx, int &ret)
    {
        using namespace QQmlJS::AST;
        using ArrayMemberItem = UiArrayMemberList;
        using ArrayMembers = InvasiveListView<ArrayMemberItem>;

        ArrayMembers arrayMembers(memberList);
        for (auto &object : arrayMembers) {
            if (object.member->kind == Node::Kind_UiObjectDefinition) {
                const auto &def = *static_cast<UiObjectDefinition *>(object.member);
                visit(def, ctx, ret);
            }
        }
    }

    static void visit(const QQmlJS::AST::UiScriptBinding &binding, Context &ctx, int &ret)
    {
        using namespace QQmlJS::AST;
        if (ctx.dbgprint)
            printf("Script binding -> %s ", binding.qualifiedId->name.toLocal8Bit().constData());

        const auto oldName = ctx.property.name; // reentrancy
        ctx.property.name = binding.qualifiedId->name;

        if (binding.statement) {
            if (binding.statement->kind == Node::Kind_ExpressionStatement) {
                const auto &expressionStatement = static_cast<const ExpressionStatement &>(*binding.statement);
                visit(expressionStatement, ctx, ret);
            }
        }
        ctx.property.name = oldName;
    }

    static void visit(const QQmlJS::AST::UiArrayBinding &arrayBinding, Context &ctx, int &ret)
    {
        using namespace QQmlJS::AST;
        if (ctx.dbgprint)
            printf("Array binding(s) -> %s: [\n", arrayBinding.qualifiedId->name.toLocal8Bit().constData());

        const auto oldName = ctx.property.name; // reentrancy
        ctx.property.name = arrayBinding.qualifiedId->name;

        if (arrayBinding.members)
            visit(*arrayBinding.members, ctx, ret);

        if (ctx.dbgprint)
            printf("]\n");

        ctx.property.name = oldName;
    }

    static void visit(const QQmlJS::AST::UiObjectBinding &objectBinding, Context &ctx, int &ret)
    {
        using namespace QQmlJS::AST;

        if (ctx.dbgprint)
            printf("Object binding -> %s: %s {\n", objectBinding.qualifiedId->name.toLocal8Bit().constData(), objectBinding.qualifiedTypeNameId->name.toLocal8Bit().constData());

        if (objectBinding.initializer) {
            if (!(ctx.interceptOBFunc && ctx.interceptOBFunc(objectBinding, ctx, ret)))
                visit(*objectBinding.initializer, ctx, ret);
        }

        if (ctx.dbgprint)
            printf("}\n");
    }

    static void visit(const QQmlJS::AST::UiPublicMember &member, Context &ctx, int &ret)
    {
        using namespace QQmlJS::AST;

        if (ctx.dbgprint)
            printf("%s member -> %s ", (member.type == UiPublicMember::Signal ? "Signal" : "Property"), member.name.toLocal8Bit().constData());

        auto name = ctx.property.name;
        ctx.property.name = member.name;
        const auto typeCpy = ctx.property.type;

        if (!(ctx.interceptPMFunc && ctx.interceptPMFunc(member, ctx, ret))) {
            if (member.statement) {
                const auto &statement = member.statement;
                if (statement->kind == Node::Kind_ExpressionStatement)
                    visit(static_cast<const ExpressionStatement &>(*statement), ctx, ret);
                else if (ctx.dbgprint)
                    printf("Unhandled statement (%d)\n", statement->kind);
            } else if (member.binding) {
                const auto &binding = member.binding;
                if (binding->kind == Node::Kind_UiObjectBinding)
                    visit(static_cast<const UiObjectBinding &>(*binding), ctx, ret);
                else if (ctx.dbgprint)
                    printf("Unhandled binding (%d)\n", binding->kind);
            }
        }

        qSwap(ctx.property.name, name);
        ctx.property.type = typeCpy;
    }

    static void visit(const QQmlJS::AST::ExpressionStatement &exprStatement, Context &ctx, int &ret)
    {
        using namespace QQmlJS::AST;
        if (exprStatement.expression) {
            if (exprStatement.expression->kind == Node::Kind_IdentifierExpression) {
                const auto &identExpression = static_cast<const IdentifierExpression &>(*exprStatement.expression);
                visit(identExpression, ctx, ret);
            } else if (exprStatement.expression->kind == Node::Kind_StringLiteral) {
                const auto &stringLiteral = static_cast<const StringLiteral &>(*exprStatement.expression);
                visit(stringLiteral, ctx, ret);
            } else if (exprStatement.expression->kind == Node::Kind_NumericLiteral) {
                const auto &numericLiteral = static_cast<const NumericLiteral &>(*exprStatement.expression);
                visit(numericLiteral, ctx, ret);
            } else if (exprStatement.expression->kind == Node::Kind_FieldMemberExpression) {
                const auto &fieldMemberExpression = static_cast<const FieldMemberExpression &>(*exprStatement.expression);
                visit(fieldMemberExpression, ctx, ret);
            } else if (exprStatement.expression->kind == Node::Kind_TrueLiteral || exprStatement.expression->kind == Node::Kind_FalseLiteral) {
                const bool v = (exprStatement.expression->kind == Node::Kind_TrueLiteral);
                if (ctx.dbgprint)
                    printf("-> TrueLiteral: %s\n", v ? "true" : "false");
                if (ctx.property.target) {
                    auto target = ctx.property.target;
                    const auto &name = ctx.property.name;
                    target->setProperty(name.toLatin1(), QVariant::fromValue(v));
                }
            } else if (exprStatement.expression->kind == Node::Kind_ArrayPattern) {
                const auto &arrayPattern = static_cast<const ArrayPattern &>(*exprStatement.expression);
                visit(arrayPattern, ctx, ret);
            } else if (exprStatement.expression->kind == Node::Kind_CallExpression) {
                const auto &callExpression = static_cast<const CallExpression &>(*exprStatement.expression);
                visit(callExpression, ctx, ret);
            } else if (exprStatement.expression->kind == Node::Kind_UnaryMinusExpression) {
                const auto &unaryMinusExpr = static_cast<const UnaryMinusExpression &>(*exprStatement.expression);
                if (unaryMinusExpr.expression && unaryMinusExpr.expression->kind == Node::Kind_NumericLiteral) {
                    auto &numericLiteral = static_cast<NumericLiteral &>(*unaryMinusExpr.expression);
                    const auto value = numericLiteral.value;
                    numericLiteral.value *= -1;
                    visit(numericLiteral, ctx, ret);
                    numericLiteral.value = value;
                }
            } else if (exprStatement.expression->kind == Node::Kind_UnaryPlusExpression) {
                const auto &unaryPlusExpr = static_cast<const UnaryPlusExpression &>(*exprStatement.expression);
                if (unaryPlusExpr.expression)
                    visit(static_cast<const NumericLiteral &>(*unaryPlusExpr.expression), ctx, ret);
            } else {
                if (ctx.dbgprint)
                    printf("<expression: %d>\n", exprStatement.expression->kind);
            }
        }
    }

    static void visit(const QQmlJS::AST::IdentifierExpression &idExpr, Context &ctx, int &ret)
    {
        Q_UNUSED(ret);
        if (ctx.dbgprint)
            printf("-> Identifier: %s\n", idExpr.name.toLocal8Bit().constData());

        if (ctx.property.target && ctx.type != Context::Type::Component) {
            const auto foundIt = ctx.identifierMap.constFind(idExpr.name);
            const auto end = ctx.identifierMap.constEnd();
            if (foundIt != end) { // If an item was found it means this is a reference
                if (ctx.property.targetType == TypeInfo<QQuick3DModel>::typeId()) {
                    if (QQuick3DMaterial *mat = qobject_cast<QQuick3DMaterial *>(*foundIt)) {
                        auto materials = qobject_cast<QQuick3DModel *>(ctx.property.target)->materials();
                        // Since we are initializing this for the first time, make sure we clean out any inherited data!
                        if (ctx.property.memberState == Context::Property::Uninitialized) {
                            if (ctx.dbgprint)
                                printf("Clearing inherited materials\n");
                            materials.clear(&materials);
                            ctx.property.memberState = Context::Property::Initialized;
                        }
                        materials.append(&materials, mat);
                        if (ctx.dbgprint)
                            printf("Appending material to %s\n", ctx.property.name.toLatin1().constData());
                    } else if (QQuick3DInstanceList *instancingList = qobject_cast<QQuick3DInstanceList *>(*foundIt)) {
                        qobject_cast<QQuick3DModel *>(ctx.property.target)->setInstancing(instancingList);
                        if (ctx.dbgprint)
                            printf("Setting instance list on model\n");
                    }
                } else if (ctx.property.targetType == TypeInfo<QQuick3DSceneEnvironment>::typeId()) {
                    if (QQuick3DEffect *effect = qobject_cast<QQuick3DEffect *>(*foundIt)) {
                        auto effects = qobject_cast<QQuick3DSceneEnvironment *>(ctx.property.target)->effects();
                        // Since we are initializing this for the first time, make sure we clean out any inherited data!
                        if (ctx.property.memberState == Context::Property::Uninitialized) {
                            if (ctx.dbgprint)
                                printf("Clearing inherited effects\n");
                            effects.clear(&effects);
                            ctx.property.memberState = Context::Property::Initialized;
                        }
                        effects.append(&effects, effect);
                        if (ctx.dbgprint)
                            printf("Appending effect to \'%s\'\n", ctx.property.name.toLatin1().constData());
                    }
                } else if (ctx.property.targetType == TypeInfo<QQuick3DShaderUtilsRenderPass>::typeId()) {
                    if (QQuick3DShaderUtilsShader *shader = qobject_cast<QQuick3DShaderUtilsShader *>(*foundIt)) {
                        auto shaders = qobject_cast<QQuick3DShaderUtilsRenderPass *>(ctx.property.target)->shaders();
                        // Since we are initializing this for the first time, make sure we clean out any inherited data!
                        if (ctx.property.memberState == Context::Property::Uninitialized) {
                            if (ctx.dbgprint)
                                printf("Clearing inherited shaders\n");
                            shaders.clear(&shaders);
                            ctx.property.memberState = Context::Property::Initialized;
                        }
                        shaders.append(&shaders, shader);
                        if (ctx.dbgprint)
                            printf("Appending shader to \'%s\'\n", ctx.property.name.toLatin1().constData());
                    }
                } else if (ctx.property.targetType == TypeInfo<QQuick3DInstanceList>::typeId()) {
                    if (QQuick3DInstanceListEntry *listEntry = qobject_cast<QQuick3DInstanceListEntry *>(*foundIt)) {
                        auto instances = qobject_cast<QQuick3DInstanceList *>(ctx.property.target)->instances();
                        // Since we are initializing this for the first time, make sure we clean out any inherited data!
                        if (ctx.property.memberState == Context::Property::Uninitialized) {
                            if (ctx.dbgprint)
                                printf("Clearing inherited instances\n");
                            instances.clear(&instances);
                            ctx.property.memberState = Context::Property::Initialized;
                        }
                        instances.append(&instances, listEntry);
                        if (ctx.dbgprint)
                            printf("Appending instance entry to %s\n", ctx.property.name.toLatin1().constData());
                    } else if (QQuick3DInstanceList *instancingList = qobject_cast<QQuick3DInstanceList *>(*foundIt)) {
                        qobject_cast<QQuick3DModel *>(ctx.property.target)->setInstancing(instancingList);
                        if (ctx.dbgprint)
                            printf("Setting instance list on model\n");
                    }
                } else if (ctx.dbgprint) {
                    printf("Unhandled binding: %s\n", idExpr.name.toLatin1().constData());
                }
            } else {
                // If no item with 'this' id was found, then that id is for 'this' object (if this not the case, then something is broken, e.g., a ref to an unknown item).
                // NOTE: This can be a problem in the future and we might need to add some more guards, but for now it just won't generate the correct shader(s).
                ctx.identifierMap.insert(idExpr.name, ctx.property.target);
            }
        }
    }

    static void visit(const QQmlJS::AST::StringLiteral &stringLiteral, Context &ctx, int &ret)
    {
        Q_UNUSED(ret);
        if (ctx.dbgprint)
            printf("-> StringLiteral: \"%s\"\n", stringLiteral.value.toLocal8Bit().constData());

        if (ctx.property.target) {
            const auto &name = ctx.property.name;
            const auto v = fromString(stringLiteral.value, ctx);
            if (v.isValid()) {
                const bool b = ctx.property.target->setProperty(name.toLatin1(), v);
                if (b && ctx.dbgprint)
                    printf("Property %s updated!\n", name.toLatin1().constData());
            }
        }
    }

    static void visit(const QQmlJS::AST::NumericLiteral &numericLiteral, Context &ctx, int &ret)
    {
        Q_UNUSED(ret);
        if (ctx.dbgprint)
            printf("-> NumericLiteral: %f\n", numericLiteral.value);

        if (ctx.property.target) {
            auto target = ctx.property.target;
            const auto &name = ctx.property.name;
            target->setProperty(name.toLatin1(), QVariant::fromValue(numericLiteral.value));
        }
    }

    static void visit(const QQmlJS::AST::FieldMemberExpression &fieldMemberExpression, Context &ctx, int &ret)
    {
        using namespace QQmlJS::AST;

        Q_UNUSED(ret);
        if (ctx.dbgprint)
            printf("-> FieldMemberExpression: %s\n", fieldMemberExpression.name.toLocal8Bit().constData());

        if (ctx.property.target) {
            const auto &name = ctx.property.name;
            const auto v = fromString(fieldMemberExpression.name, ctx);
            if (v.isValid())
                ctx.property.target->setProperty(name.toLatin1(), v);
        }
    }

    static void visit(const QQmlJS::AST::ArrayPattern &arrayPattern, Context &ctx, int &ret)
    {
        Q_UNUSED(ret);
        if (ctx.dbgprint)
            printf("-> [ ");

        using namespace QQmlJS::AST;
        using PatternElementItem = PatternElementList;
        using PatternElementListView = InvasiveListView<PatternElementItem>;

        PatternElementListView elements(*arrayPattern.elements);
        for (auto &element : elements) {
            auto patternElement = element.element;
            if (patternElement->type == PatternElement::Literal) {
                if (patternElement->initializer && patternElement->initializer->kind == Node::Kind_IdentifierExpression) {
                    const auto &identExpression = static_cast<const IdentifierExpression &>(*patternElement->initializer);
                    visit(identExpression, ctx, ret);
                }
            } else if (ctx.dbgprint) {
                printf("Unahandled(%d), ", patternElement->type);
            }
        }

        if (ctx.dbgprint)
            printf(" ]\n");

    }

    static void visit(const QQmlJS::AST::CallExpression &callExpression, Context &ctx, int &ret)
    {
        using namespace QQmlJS::AST;

        Q_UNUSED(ret);
        Q_UNUSED(callExpression);
        if (ctx.dbgprint)
            printf("-> Call(%d)\n", callExpression.base->kind);

        (ctx.interceptCallExpr && ctx.interceptCallExpr(callExpression, ctx, ret));
    }

    static void visit(const QQmlJS::AST::UiObjectMember &member, Context &ctx, int &ret)
    {
        using namespace QQmlJS::AST;

        if (member.kind == Node::Kind_UiObjectBinding)
            visit(static_cast<const UiObjectBinding &>(member), ctx, ret);
        else if (ctx.dbgprint)
            printf("Unhandled member (%d)\n", member.kind);
    }

    static void visit(const QQmlJS::AST::UiObjectMemberList &memberList, Context &ctx, int &ret)
    {
        using namespace QQmlJS::AST;
        using ObjectMemberItem = UiObjectMemberList;
        using ObjectMembers = InvasiveListView<ObjectMemberItem>;

        const auto oldEvalType = ctx.property.memberState;

        ObjectMembers objectMembers(memberList);
        for (const auto &member : objectMembers) {
            if (member.member) {
                if (member.member->kind == Node::Kind_UiScriptBinding) {
                    ctx.property.memberState = Context::Property::MemberState::Uninitialized;
                    const auto &scriptBinding = static_cast<const UiScriptBinding &>(*member.member);
                    visit(scriptBinding, ctx, ret);
                } else if (member.member->kind == Node::Kind_UiArrayBinding) {
                    ctx.property.memberState = Context::Property::MemberState::Uninitialized;
                    const auto &arrayBinding = static_cast<const UiArrayBinding &>(*member.member);
                    visit(arrayBinding, ctx, ret);
                } else if (member.member->kind == Node::Kind_UiObjectDefinition) {
                    const auto &objectDef = static_cast<const UiObjectDefinition &>(*member.member);
                    visit(objectDef, ctx, ret);
                } else if (member.member->kind == Node::Kind_UiObjectBinding) {
                    ctx.property.memberState = Context::Property::MemberState::Uninitialized;
                    const auto &objBinding = static_cast<const UiObjectBinding &>(*member.member);
                    visit(objBinding, ctx, ret);
                } else if (member.member->kind == Node::Kind_UiPublicMember) {
                    ctx.property.memberState = Context::Property::MemberState::Uninitialized;
                    const auto &pubMember = static_cast<const UiPublicMember &>(*member.member);
                    visit(pubMember, ctx, ret);
                } else {
                    if (ctx.dbgprint)
                        printf("<member %d>\n", member.member->kind);
                }
            }
        }

        ctx.property.memberState = oldEvalType;
    }

private:
    Visitors() = delete;
    Q_DISABLE_COPY(Visitors);
};

template <typename O, typename T>
T *buildType(const O &obj, Context &ctx, int &ret, const T *base = nullptr)
{
    // swap -> reentrancy
    Context::Property property;
    qSwap(property, ctx.property);
    Q_ASSERT(ctx.property.target == nullptr);

    T *instance = nullptr;

    if (ctx.dbgprint)
        printf("Building %s!\n", TypeInfo<T>::qmlTypeName());

    if (obj.initializer) {
        instance = new T;
        if (base)
            cloneProperties(*instance, *base);

        if (obj.initializer) {
            ctx.property.target = instance;
            ctx.property.targetType = TypeInfo<T>::typeId();
            Visitors::visit(*obj.initializer, ctx, ret);
        }
    }

    // swap back
    qSwap(property, ctx.property);

    return instance;
}

static QQuick3DAbstractLight *buildLight(const QQmlJS::AST::UiObjectDefinition &def,
                                         Context &ctx,
                                         int &ret,
                                         int lightType,
                                         const QQuick3DAbstractLight *base = nullptr)
{
    if (lightType == TypeInfo<QQuick3DDirectionalLight>::typeId())
        return buildType(def, ctx, ret, qobject_cast<const QQuick3DDirectionalLight *>(base));
    if (lightType == TypeInfo<QQuick3DPointLight>::typeId())
        return buildType(def, ctx, ret, qobject_cast<const QQuick3DPointLight *>(base));
    if (lightType == TypeInfo<QQuick3DSpotLight>::typeId())
        return buildType(def, ctx, ret, qobject_cast<const QQuick3DSpotLight *>(base));
    return nullptr;
}


template <typename T>
static void updateProperty(Context &ctx, T type, QStringView propName)
{
    if (ctx.property.target) {
        if (ctx.dbgprint)
            printf("Updating property %s\n", propName.toLatin1().constData());
        const auto &target = ctx.property.target;
        if (ctx.property.memberState == Context::Property::Uninitialized) {
            target->setProperty(propName.toLatin1().constData(), QVariant::fromValue(type));
            ctx.property.memberState = Context::Property::Initialized;
        } else {
            const int idx = target->metaObject()->indexOfProperty(propName.toLatin1().constData());
            if (idx != -1) {
                auto prop = target->metaObject()->property(idx);
                prop.write(target, QVariant::fromValue(type));
            }
        }
    }
}

static bool interceptObjectBinding(const QQmlJS::AST::UiObjectBinding &objectBinding, Context &ctx, int &ret)
{
    if (ctx.dbgprint)
        printf("Intercepted object binding!\n");

    bool handled = false;

    const auto &typeName = objectBinding.qualifiedTypeNameId->name.toString();
    const auto &propName = objectBinding.qualifiedId->name;

    int type = -1;

    // Base type?
    const auto typeIt = s_typeMap->constFind(typeName);
    if (typeIt != s_typeMap->cend())
        type = *typeIt;

    // Component?
    auto &components = ctx.components;
    const auto compIt = (type == -1) ? components.constFind(typeName) : components.cend();
    QObject *base = nullptr;
    if (compIt != components.cend()) {
        type = compIt->type;
        base = compIt->ptr;
    }

    if (type != -1) {
        if (ctx.dbgprint)
            printf("Resolving: \'%s\'\n", qPrintable(typeName));

        if (type == TypeInfo<QQuick3DSceneEnvironment>::typeId()) {
            if (ctx.property.targetType == TypeInfo<QQuick3DViewport>::typeId()) {
                if (auto environment = buildType(objectBinding, ctx, ret, qobject_cast<QQuick3DSceneEnvironment *>(base))) {
                    auto viewport = qobject_cast<QQuick3DViewport *>(ctx.property.target);
                    Q_ASSERT(viewport);
                    viewport->setEnvironment(environment);
                    handled = true;
                }
            }
        } else if (type == TypeInfo<QQuick3DTexture>::typeId()) {
            if (auto tex = buildType(objectBinding, ctx, ret, qobject_cast<QQuick3DTexture *>(base))) {
                updateProperty(ctx, tex, propName);
                ctx.sceneData.textures.append(tex);
            }
            handled = true;
        } else if (type == TypeInfo<QQuick3DShaderUtilsTextureInput>::typeId()) {
            auto texInput = buildType(objectBinding, ctx, ret, qobject_cast<QQuick3DShaderUtilsTextureInput *>(base));
            if (texInput && texInput->texture()) {
                updateProperty(ctx, texInput, propName);
                ctx.sceneData.textures.append(texInput->texture());
            }
            handled = true;
        } else if (type == TypeInfo<QQuick3DEffect>::typeId()) {
            if (ctx.property.targetType == TypeInfo<QQuick3DSceneEnvironment>::typeId()) {
                if (auto effect = buildType(objectBinding, ctx, ret, qobject_cast<QQuick3DEffect *>(base))) {
                    auto sceneEnvironment = qobject_cast<QQuick3DSceneEnvironment *>(ctx.property.target);
                    Q_ASSERT(sceneEnvironment);
                    auto effects = sceneEnvironment->effects();
                    effects.append(&effects, effect);
                    handled = true;
                }
            }
        } else if (type == TypeInfo<QQuick3DShaderUtilsRenderPass>::typeId()) {
            if (ctx.property.targetType == TypeInfo<QQuick3DEffect>::typeId()) {
                if (auto pass = buildType(objectBinding, ctx, ret, qobject_cast<QQuick3DShaderUtilsRenderPass *>(base))) {
                    auto effect = qobject_cast<QQuick3DEffect *>(ctx.property.target);
                    Q_ASSERT(effect);
                    auto passes = effect->passes();
                    passes.append(&passes, pass);
                    handled = true;
                }
            }
        } else if (type == TypeInfo<QQuick3DShaderUtilsShader>::typeId()) {
            if (ctx.property.targetType == TypeInfo<QQuick3DShaderUtilsRenderPass>::typeId()) {
                if (auto shader = buildType(objectBinding, ctx, ret, qobject_cast<QQuick3DShaderUtilsShader *>(base))) {
                    auto pass = qobject_cast<QQuick3DShaderUtilsRenderPass *>(ctx.property.target);
                    Q_ASSERT(pass);
                    auto shaders = pass->shaders();
                    shaders.append(&shaders, shader);
                    handled = true;
                }
            }
        } else if (type == TypeInfo<QQuick3DDefaultMaterial>::typeId()) {
            if (ctx.property.targetType == TypeInfo<QQuick3DModel>::typeId()) {
                if (auto mat = buildType(objectBinding, ctx, ret, qobject_cast<QQuick3DDefaultMaterial *>(base))) {
                    auto model = qobject_cast<QQuick3DModel *>(ctx.property.target);
                    Q_ASSERT(model);
                    auto materials = model->materials();
                    materials.append(&materials, mat);
                    handled = true;
                    if (ctx.dbgprint)
                        printf("Appending material to %s\n", ctx.property.name.toLatin1().constData());
                }
            }
        } else if (type == TypeInfo<QQuick3DPrincipledMaterial>::typeId()) {
            if (ctx.property.targetType == TypeInfo<QQuick3DModel>::typeId()) {
                if (auto mat = buildType(objectBinding, ctx, ret, qobject_cast<QQuick3DPrincipledMaterial *>(base))) {
                    auto model = qobject_cast<QQuick3DModel *>(ctx.property.target);
                    Q_ASSERT(model);
                    auto materials = model->materials();
                    materials.append(&materials, mat);
                    handled = true;
                }
            }
        } else if (type == TypeInfo<QQuick3DCustomMaterial>::typeId()) {
            if (ctx.property.targetType == TypeInfo<QQuick3DModel>::typeId()) {
                if (auto mat = buildType(objectBinding, ctx, ret, qobject_cast<QQuick3DCustomMaterial *>(base))) {
                    auto model = qobject_cast<QQuick3DModel *>(ctx.property.target);
                    Q_ASSERT(model);
                    auto materials = model->materials();
                    materials.append(&materials, mat);
                    handled = true;
                }
            }
        } else if (ctx.dbgprint) {
            printf("Unhandled type\n");
        }
    }

    return handled;
}

static bool interceptObjectDef(const QQmlJS::AST::UiObjectDefinition &def, Context &ctx, int &ret)
{
    const auto &typeName = def.qualifiedTypeNameId->name.toString();

    if (ctx.dbgprint)
        printf("Intercepted object definition (\'%s\')!\n", typeName.toLatin1().constData());

    QString componentName;
    int type = -1;
    bool doRegisterComponent = false;

    // Base type?
    const auto typeIt = s_typeMap->constFind(typeName);
    if (typeIt != s_typeMap->cend())
        type = *typeIt;

    // Component?
    auto &components = ctx.components;
    const auto compIt = (type == -1) ? components.constFind(typeName) : components.cend();
    if (compIt != components.cend())
        type = compIt->type;

    // If this is a new component register it
    if (ctx.type == Context::Type::Component && ctx.property.target == nullptr && type != -1) {
        const auto &fileName = ctx.currentFileInfo.fileName();
        componentName = fileName.left(fileName.size() - 4);
        doRegisterComponent = !componentName.isEmpty();
    }

    const auto registerComponent = [&ctx, &components, &componentName](Context::Component component) {
        if (ctx.dbgprint)
            printf("Registering component \'%s\'\n", qPrintable(componentName));
        components.insert(componentName, component);
    };

    if (type == TypeInfo<QQuick3DViewport>::typeId()) {
        const QQuick3DViewport *base = (compIt != components.cend()) ? qobject_cast<QQuick3DViewport *>(compIt->ptr) : nullptr;
        if (QQuick3DViewport *viewport = buildType(def, ctx, ret, base)) {
            // If this is a component we'll store it for lookups later.
            if (doRegisterComponent)
                registerComponent({ viewport, type });
            // Only one viewport supported atm (see SceneEnvironment case as well).
            if (!ctx.sceneData.viewport)
                ctx.sceneData.viewport = viewport;
        }
    } else if (type == TypeInfo<QQuick3DSceneEnvironment>::typeId()) {
        const QQuick3DSceneEnvironment *base = (compIt != components.cend()) ? qobject_cast<QQuick3DSceneEnvironment *>(compIt->ptr) : nullptr;
        if (QQuick3DSceneEnvironment *sceneEnv = buildType(def, ctx, ret, base)) {
            // If this is a component we'll store it for lookups later.
            if (doRegisterComponent)
                registerComponent({ sceneEnv, type });

            if (ctx.sceneData.viewport)
                ctx.sceneData.viewport->setEnvironment(sceneEnv);
        }
    } else if (type == TypeInfo<QQuick3DPrincipledMaterial>::typeId()) {
        const QQuick3DPrincipledMaterial *base = (compIt != components.cend()) ? qobject_cast<QQuick3DPrincipledMaterial *>(compIt->ptr) : nullptr;
        if (QQuick3DPrincipledMaterial *mat = buildType(def, ctx, ret, base)) {
            // If this is a component we'll store it for lookups later.
            if (doRegisterComponent)
                registerComponent({ mat, type });

            if (ctx.property.target) {
                if (ctx.property.targetType == TypeInfo<QQuick3DModel>::typeId()) {
                    auto materials = qobject_cast<QQuick3DModel *>(ctx.property.target)->materials();
                    if (ctx.property.memberState == Context::Property::Uninitialized) {
                        if (ctx.dbgprint)
                            printf("Clearing inherited materials\n");
                        materials.clear(&materials);
                        ctx.property.memberState = Context::Property::Initialized;
                    }
                    materials.append(&materials, mat);
                    if (ctx.dbgprint)
                        printf("Appending material to %s\n", ctx.property.name.toLatin1().constData());
                }
            }

            // At this point we don't know if this material is going to be referenced somewhere else, so keep it in the list
            ctx.sceneData.materials.push_back(mat);
        }
    } else if (type == TypeInfo<QQuick3DDefaultMaterial>::typeId()) {
        const QQuick3DDefaultMaterial *base = (compIt != components.cend()) ? qobject_cast<QQuick3DDefaultMaterial *>(compIt->ptr) : nullptr;
        if (QQuick3DDefaultMaterial *mat = buildType(def, ctx, ret, base)) {
            // If this is a component we'll store it for lookups later.
            if (doRegisterComponent)
                registerComponent({ mat, type });

            if (ctx.property.target) {
                if (ctx.property.targetType == TypeInfo<QQuick3DModel>::typeId()) {
                    auto materials = qobject_cast<QQuick3DModel *>(ctx.property.target)->materials();
                    if (ctx.property.memberState == Context::Property::Uninitialized) {
                        if (ctx.dbgprint)
                            printf("Clearing inherited materials\n");
                        materials.clear(&materials);
                        ctx.property.memberState = Context::Property::Initialized;
                    }
                    materials.append(&materials, mat);
                    if (ctx.dbgprint)
                        printf("Appending material to %s\n", ctx.property.name.toLatin1().constData());
                }
            }

            // At this point we don't know if this material is going to be referenced somewhere else, so keep it in the list
            ctx.sceneData.materials.push_back(mat);
        }
    } else if (type == TypeInfo<QQuick3DCustomMaterial>::typeId()) {
        const QQuick3DCustomMaterial *base = (compIt != components.cend()) ? qobject_cast<QQuick3DCustomMaterial *>(compIt->ptr) : nullptr;
        if (QQuick3DCustomMaterial *mat = buildType(def, ctx, ret, base)) {
            // If this is a component we'll store it for lookups later.
            if (doRegisterComponent)
                registerComponent({ mat, type });

            if (ctx.property.target) {
                if (ctx.property.targetType == TypeInfo<QQuick3DModel>::typeId()) {
                    auto materials = qobject_cast<QQuick3DModel *>(ctx.property.target)->materials();
                    if (ctx.property.memberState == Context::Property::Uninitialized) {
                        if (ctx.dbgprint)
                            printf("Clearing inherited materials\n");
                        materials.clear(&materials);
                        ctx.property.memberState = Context::Property::Initialized;
                    }
                    materials.append(&materials, mat);
                    if (ctx.dbgprint)
                        printf("Appending material to %s\n", ctx.property.name.toLatin1().constData());
                }
            }

            // At this point we don't know if this material is going to be referenced somewhere else, so keep it in the list
            ctx.sceneData.materials.push_back(mat);
        }
    } else if (type == TypeInfo<QQuick3DEffect>::typeId()) {
        const QQuick3DEffect *base = (compIt != components.cend()) ? qobject_cast<QQuick3DEffect *>(compIt->ptr) : nullptr;
        if (QQuick3DEffect *effect = buildType(def, ctx, ret, base)) {
            // If this is a component we'll store it for lookups later.
            if (doRegisterComponent)
                registerComponent({ effect, type });

            if (ctx.property.target) {
                if (ctx.property.targetType == TypeInfo<QQuick3DSceneEnvironment>::typeId()) {
                    auto effects = qobject_cast<QQuick3DSceneEnvironment *>(ctx.property.target)->effects();
                    if (ctx.property.memberState == Context::Property::Uninitialized) {
                        if (ctx.dbgprint)
                            printf("Clearing inherited effects\n");
                        effects.clear(&effects);
                        ctx.property.memberState = Context::Property::Initialized;
                    }
                    effects.append(&effects, effect);
                    if (ctx.dbgprint)
                        printf("Appending effect to %s\n", ctx.property.name.toLatin1().constData());
                }
            }

            // At this point we don't know if this effect is going to be referenced somewhere else, so keep it in the list
            ctx.sceneData.effects.push_back(effect);
        }
    } else if (type == TypeInfo<QQuick3DDirectionalLight>::typeId() || type == TypeInfo<QQuick3DPointLight>::typeId() || type == TypeInfo<QQuick3DSpotLight>::typeId())  {
        const QQuick3DAbstractLight *base = (compIt != components.cend()) ? qobject_cast<QQuick3DAbstractLight *>(compIt->ptr) : nullptr;
        if (QQuick3DAbstractLight *light = buildLight(def, ctx, ret, type, base)) {
            // If this is a component we'll store it for lookups later.
            if (doRegisterComponent)
                registerComponent({ light, type });

            ctx.sceneData.lights.push_back(light);
        }
    } else if (type == TypeInfo<QQuick3DTexture>::typeId()) {
        const QQuick3DTexture *base = (compIt != components.cend()) ? qobject_cast<QQuick3DTexture *>(compIt->ptr) : nullptr;
        if (QQuick3DTexture *tex = buildType(def, ctx, ret, base)) {
            // If this is a component we'll store it for lookups later.
            if (doRegisterComponent)
                registerComponent({ tex, type });

            ctx.sceneData.textures.push_back(tex);
        }
    } else if (type == TypeInfo<QQuick3DModel>::typeId()) {
        const auto *base = (compIt != components.cend()) ? qobject_cast<QQuick3DModel *>(compIt->ptr) : nullptr;
        if (auto *model = buildType(def, ctx, ret, base)) {
            // If this is a component we'll store it for lookups later.
            if (doRegisterComponent)
                registerComponent({ model, type });

            ctx.sceneData.models.push_back(model);
        }
    } else if (type == TypeInfo<QQuick3DShaderUtilsShader>::typeId()) {
        const auto *base = (compIt != components.cend()) ? qobject_cast<QQuick3DShaderUtilsShader *>(compIt->ptr) : nullptr;
        if (auto *shader = buildType(def, ctx, ret, base)) {
            // If this is a component we'll store it for lookups later.
            if (doRegisterComponent)
                registerComponent({ shader, type });
            if (ctx.property.target) {
                if (ctx.property.targetType == TypeInfo<QQuick3DShaderUtilsRenderPass>::typeId()) {
                    auto shaders = qobject_cast<QQuick3DShaderUtilsRenderPass *>(ctx.property.target)->shaders();
                    if (ctx.property.memberState == Context::Property::Uninitialized) {
                        if (ctx.dbgprint)
                            printf("Clearing inherited shaders\n");
                        shaders.clear(&shaders);
                        ctx.property.memberState = Context::Property::Initialized;
                    }
                    shaders.append(&shaders, shader);
                    if (ctx.dbgprint)
                        printf("Appending shader to %s\n", ctx.property.name.toLatin1().constData());
                }
            }

            ctx.sceneData.shaders.push_back(shader);
        }
    } else if (type == TypeInfo<QQuick3DShaderUtilsRenderPass>::typeId()) {
        const auto *base = (compIt != components.cend()) ? qobject_cast<QQuick3DShaderUtilsRenderPass *>(compIt->ptr) : nullptr;
        if (auto *pass = buildType(def, ctx, ret, base)) {
            // If this is a component we'll store it for lookups later.
            if (doRegisterComponent)
                registerComponent({ pass, type });
            if (ctx.property.target) {
                if (ctx.property.targetType == TypeInfo<QQuick3DEffect>::typeId()) {
                    auto passes = qobject_cast<QQuick3DEffect *>(ctx.property.target)->passes();
                    if (ctx.property.memberState == Context::Property::Uninitialized) {
                        if (ctx.dbgprint)
                            printf("Clearing inherited passes\n");
                        passes.clear(&passes);
                        ctx.property.memberState = Context::Property::Initialized;
                    }
                    passes.append(&passes, pass);
                    if (ctx.dbgprint)
                        printf("Appending pass to %s\n", ctx.property.name.toLatin1().constData());
                }
            }
        }
    } else if (type == TypeInfo<QQuick3DInstanceList>::typeId()) {
        const auto *base = (compIt != components.cend()) ? qobject_cast<QQuick3DInstanceList *>(compIt->ptr) : nullptr;
        if (auto *instanceList = buildType(def, ctx, ret, base)) {
            // If this is a component we'll store it for lookups later.
            if (doRegisterComponent)
                registerComponent({ instanceList, type });
            if (ctx.property.target) {
                if (ctx.property.targetType == TypeInfo<QQuick3DModel>::typeId()) {
                    qobject_cast<QQuick3DModel *>(ctx.property.target)->setInstancing(instanceList);
                    if (ctx.dbgprint)
                        printf("Setting instance list on %s\n", ctx.property.name.toLatin1().constData());
                }
            }
        }
    } else if (type == TypeInfo<QQuick3DInstanceListEntry>::typeId()) {
        const auto *base = (compIt != components.cend()) ? qobject_cast<QQuick3DInstanceListEntry *>(compIt->ptr) : nullptr;
        if (auto *instanceListEntry = buildType(def, ctx, ret, base)) {
            // If this is a component we'll store it for lookups later.
            if (doRegisterComponent)
                registerComponent({ instanceListEntry, type });
            if (ctx.property.target) {
                if (ctx.property.targetType == TypeInfo<QQuick3DInstanceList>::typeId()) {
                    auto instances = qobject_cast<QQuick3DInstanceList *>(ctx.property.target)->instances();
                    instances.append(&instances, instanceListEntry);
                    if (ctx.dbgprint)
                        printf("Appending instance list entry to %s\n", ctx.property.name.toLatin1().constData());
                }
            }
        }
    } else {
        if (ctx.dbgprint)
            printf("Object def for \'%s\' was not handled\n", ctx.property.name.toLatin1().constData());
        return false;
    }

    return true;
}

static bool interceptPublicMember(const QQmlJS::AST::UiPublicMember &member, Context &ctx, int &ret)
{
    Q_UNUSED(ret);
    using namespace QQmlJS::AST;

    if (ctx.dbgprint)
        printf("Intercepted public member!\n");

    if (member.statement && member.statement->kind == Node::Kind_ExpressionStatement) {
        if ((ctx.property.targetType == TypeInfo<QQuick3DCustomMaterial>::typeId() || ctx.property.targetType == TypeInfo<QQuick3DEffect>::typeId()) && member.memberType) {
            // For custom materials we have properties that are user provided, so we'll
            // need to add these to the objects properties (we add these here to be able
            // to piggyback on the existing type matching code)
            if (member.memberType->name == u"real") {
                ctx.property.type = QMetaType::Double;
            } else if (member.memberType->name == u"bool") {
                ctx.property.type = QMetaType::Bool;
            } else if (member.memberType->name == u"int") {
                ctx.property.type = QMetaType::Int;
            } else if (member.memberType->name == u"size") {
                ctx.property.type = QMetaType::QSizeF;
            } else if (member.memberType->name == u"rect") {
                ctx.property.type = QMetaType::QRectF;
            } else if (member.memberType->name == u"point") {
                ctx.property.type = QMetaType::QPointF;
            } else if (member.memberType->name == u"color") {
                ctx.property.type = QMetaType::QColor;
            } else if (member.memberType->name.startsWith(u"vector")) {
                if (member.memberType->name.endsWith(u"2d")) {
                    ctx.property.type = QMetaType::QVector2D;
                } else if (member.memberType->name.endsWith(u"3d")) {
                    ctx.property.type = QMetaType::QVector3D;
                } else if (member.memberType->name.endsWith(u"4d")) {
                    ctx.property.type = QMetaType::QVector4D;
                }
            } else if (member.memberType->name == u"matrix4x4") {
                ctx.property.type = QMetaType::QMatrix4x4;;
            } else if (member.memberType->name == u"quaternion") {
                ctx.property.type = QMetaType::QQuaternion;
            } else if (member.memberType->name == u"var") {
                ctx.property.type = QMetaType::QVariant;
            }
        }
    }

    return false;
}

static bool interceptCallExpression(const QQmlJS::AST::CallExpression &callExpression, Context &ctx, int &ret)
{
    Q_UNUSED(ret);
    using namespace QQmlJS::AST;
    using namespace BuiltinHelpers;

    if (ctx.dbgprint)
        printf("Intercepted call expression!\n");

    const bool ok = (ctx.property.target && !ctx.property.name.isEmpty());
    if (callExpression.base && ok) {
        if (callExpression.base->kind == Node::Kind_FieldMemberExpression) {
            const auto &fieldMemberExpression = static_cast<const FieldMemberExpression &>(*callExpression.base);
            if (fieldMemberExpression.base) {
                if (fieldMemberExpression.base->kind == Node::Kind_IdentifierExpression) {
                    const auto &identExpr = static_cast<const IdentifierExpression &>(*fieldMemberExpression.base);
                    if (identExpr.name == u"Qt") {
                        bool ok = false;
                        QVariant v;
                        if (fieldMemberExpression.name == u"point") {
                            const auto point = toPoint(ArgumentListView(*callExpression.arguments), &ok);
                            if (ctx.dbgprint)
                                printf("Qt.point(%f, %f)\n", point.x(), point.y());
                            setProperty(ctx.property, point);
                        } else if (fieldMemberExpression.name == u"size") {
                            const auto size = toSize(ArgumentListView(*callExpression.arguments), &ok);
                            if (ctx.dbgprint)
                                printf("Qt.size(%f, %f)\n", size.width(), size.height());
                            setProperty(ctx.property, size);
                        } else if (fieldMemberExpression.name == u"rect") {
                            const auto rect = toRect(ArgumentListView(*callExpression.arguments), &ok);
                            if (ctx.dbgprint)
                                printf("Qt.rect(%f, %f, %f, %f)\n", rect.x(), rect.y(), rect.width(), rect.height());
                            setProperty(ctx.property, rect);
                        } else if (fieldMemberExpression.name.startsWith(u"vector")) {
                            if (fieldMemberExpression.name.endsWith(u"2d")) {
                                const auto vec2 = toVec<QVector2D>(ArgumentListView(*callExpression.arguments), &ok);
                                if (ctx.dbgprint)
                                    printf("Qt.vector2d(%f, %f)\n", vec2.x(), vec2.y());
                                setProperty(ctx.property, vec2);
                            } else if (fieldMemberExpression.name.endsWith(u"3d")) {
                                const auto vec3 = toVec<QVector3D>(ArgumentListView(*callExpression.arguments), &ok);
                                if (ctx.dbgprint)
                                    printf("Qt.vector3d(%f, %f, %f)\n", vec3.x(), vec3.y(), vec3.z());
                                setProperty(ctx.property, vec3);
                            } else if (fieldMemberExpression.name.endsWith(u"4d")) {
                                const auto vec4 = toVec<QVector4D>(ArgumentListView(*callExpression.arguments), &ok);
                                if (ctx.dbgprint)
                                    printf("Qt.vector4d(%f, %f, %f, %f)\n", vec4.x(), vec4.y(), vec4.z(), vec4.w());
                                setProperty(ctx.property, vec4);
                            }
                        } else if (fieldMemberExpression.name == u"matrix4x4") {
                            const auto mat44 = toMat44(ArgumentListView(*callExpression.arguments), &ok);
                            if (ctx.dbgprint)
                                printf("Qt.matrix4x4(%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f)\n",
                                       mat44(0, 0), mat44(0, 1), mat44(0, 2), mat44(0, 3),
                                       mat44(1, 0), mat44(1, 1), mat44(1, 2), mat44(1, 3),
                                       mat44(2, 0), mat44(2, 1), mat44(2, 2), mat44(2, 3),
                                       mat44(3, 0), mat44(3, 1), mat44(3, 2), mat44(3, 3));
                            setProperty(ctx.property, mat44);
                        } else if (fieldMemberExpression.name == u"quaternion") {
                            const auto quat = toQuaternion(ArgumentListView(*callExpression.arguments), &ok);
                            if (ctx.dbgprint)
                                printf("Qt.quaternion(%f, %f, %f, %f)\n", quat.scalar(), quat.x(), quat.y(), quat.z());
                            setProperty(ctx.property, quat);
                        } else if (fieldMemberExpression.name == u"rgba") {
                            const auto vec4 = toVec<QVector4D>(ArgumentListView(*callExpression.arguments), &ok);
                            if (ok) {
                                QColor color = QColor::fromRgbF(vec4.x(), vec4.y(), vec4.z(), vec4.w());
                                if (ctx.dbgprint)
                                    printf("Qt.rgba(%f, %f, %f, %f)\n", color.redF(), color.greenF(), color.blueF(), color.alphaF());
                                setProperty(ctx.property, color);
                            }
                        }
                        if (ok && v.isValid() && ctx.property.target)
                            ctx.property.target->setProperty(ctx.property.name.toLatin1().constData(), v);
                    }
                }
            }
        }
    }

    return false;
}

static int parseQmlData(const QByteArray &code, Context &ctx)
{
    Q_ASSERT(ctx.engine && ctx.engine->lexer());
    ctx.identifierMap.clear(); // not visible outside the scope of this "code"
    if (ctx.dbgprint)
        printf("Parsing %s\n", qPrintable(ctx.currentFileInfo.filePath()));
    int ret = 0;
    ctx.engine->lexer()->setCode(QString::fromUtf8(code), 1, true);
    QQmlJS::Parser parser(ctx.engine);
    const bool ok = parser.parse();
    if (ok) {
        const auto program = parser.ast();
        if (program)
            Visitors::visit(*program, ctx, ret);
    } else {
        ret = -1;
        qWarning("Parsing failed due to %s in %s:%d%d", qPrintable(parser.errorMessage()), qPrintable(ctx.currentFileInfo.fileName()), parser.errorLineNumber(), parser.errorColumnNumber());
    }

    return ret;
}

int MaterialParser::parseQmlData(const QByteArray &code, const QString &fileName, MaterialParser::SceneData &sceneData)
{
    // set initial type map
    *s_typeMap = baseTypeMap();

    QQmlJS::Engine engine;
    QQmlJS::Lexer lexer(&engine);

    Context ctx;
    ctx.engine = &engine;
    ctx.interceptODFunc = &interceptObjectDef;
    ctx.interceptOBFunc = &interceptObjectBinding;
    ctx.interceptPMFunc = &interceptPublicMember;
    ctx.interceptCallExpr = &interceptCallExpression;
    ctx.currentFileInfo = QFileInfo(fileName);
    ctx.type = Context::Type::Component;

    const int ret = ::parseQmlData(code, ctx);
    sceneData = std::move(ctx.sceneData);

    return ret;
}

int MaterialParser::parseQmlFiles(const QVector<QString> &filePaths, const QDir &sourceDir, SceneData &sceneData, bool verboseOutput)
{
    // set initial type map
    *s_typeMap = baseTypeMap();

    int ret = 0;

    if (filePaths.isEmpty()) {
        qWarning("No input files");
        return ret;
    }

    QQmlJS::Engine engine;
    QQmlJS::Lexer lexer(&engine);

    Context ctx;
    ctx.dbgprint = verboseOutput;
    ctx.engine = &engine;
    ctx.interceptODFunc = &interceptObjectDef;
    ctx.interceptOBFunc = &interceptObjectBinding;
    ctx.interceptPMFunc = &interceptPublicMember;
    ctx.interceptCallExpr = &interceptCallExpression;
    ctx.workingDir = sourceDir;

    QVector<QString> deferredOther;
    QVector<QString> deferredComponets;

    const QString sourcePath = sourceDir.canonicalPath() + QDir::separator();

    const bool isMultifile = filePaths.size() != 1;

    // Go through and find the material components first
    for (const auto &v : filePaths) {
        QFileInfo &currentFileInfo = ctx.currentFileInfo;
        if (!QFileInfo(v).isAbsolute())
            currentFileInfo.setFile(sourcePath + v);
        else
            currentFileInfo.setFile(v);
        const bool maybeComponent = currentFileInfo.fileName().at(0).isUpper();
        if (currentFileInfo.isFile() && currentFileInfo.suffix() == getQmlFileExtension()) {
            const QString filePath = currentFileInfo.canonicalFilePath();
            if (isMultifile && maybeComponent) {
                QFile f(filePath);
                if (!f.open(QFile::ReadOnly)) {
                    qWarning("Could not open file %s for reading!", qPrintable(filePath));
                    return -1;
                }

                const QByteArray code = f.readAll();
                int idx = code.indexOf('{');
                if (idx != -1) {
                    const QByteArray section = code.mid(0, idx);
                    QVarLengthArray<const char *, 3> componentTypes { TypeInfo<QQuick3DPrincipledMaterial>::qmlTypeName(),
                                                                           TypeInfo<QQuick3DCustomMaterial>::qmlTypeName(),
                                                                           TypeInfo<QQuick3DDefaultMaterial>::qmlTypeName()};
                    for (const auto compType : std::as_const(componentTypes)) {
                        if ((idx = section.indexOf(compType)) != -1)
                            break;
                    }
                    if (idx != -1) {
                        ctx.type = Context::Type::Component;
                        ret = parseQmlData(code, ctx);
                        if (ret != 0)
                            break;
                    } else {
                        deferredComponets.push_back(filePath);
                    }
                } else {
                    qWarning("No items found in %s\n", qPrintable(filePath));
                }
            } else {
                deferredOther.push_back(filePath);
            }
        } else {
            qWarning("The file %s is either not a file or has the wrong extension!", qPrintable(v));
        }
    }

    const auto parsePaths = [&ctx, &ret](const QVector<QString> &paths, Context::Type type) {
        ctx.type = type;
        for (const auto &path : paths) {
            QFileInfo &currentFileInfo = ctx.currentFileInfo;
            currentFileInfo.setFile(path);
            if (currentFileInfo.isFile() && currentFileInfo.suffix() == getQmlFileExtension()) {
                const QString filePath = currentFileInfo.canonicalFilePath();
                QFile f(filePath);
                if (!f.open(QFile::ReadOnly)) {
                    qWarning("Could not open file %s for reading!", qPrintable(filePath));
                    ret =  -1;
                    return;
                }

                const QByteArray code = f.readAll();
                ret = parseQmlData(code, ctx);
                if (ret != 0)
                    break;
            }
        }
    };

    // Other components
    parsePaths(deferredComponets, Context::Type::Component);

    // Now parse the rest
    parsePaths(deferredOther, Context::Type::Application);

    sceneData = std::move(ctx.sceneData);

    return ret;
}

QT_END_NAMESPACE
