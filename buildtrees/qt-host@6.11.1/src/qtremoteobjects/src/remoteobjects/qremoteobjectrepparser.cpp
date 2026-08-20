// Copyright (C) 2024 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qremoteobjectnode_p.h"
#include "qremoteobjectrepparser_p.h"
#include "qremoteobjectstructs_p.h"

QT_BEGIN_NAMESPACE

using namespace QRemoteObjectInternalTypes;

// Make a central function to convert QStrings to QByteArrays in case we need
// to change how this is done.
auto toQBA = [](const QString &string) { return string.toUtf8(); };

static QByteArray capName(const QString &name)
{
    /*
     * Helper method to capitalize the first letter of a QString and return
     * the result as a QByteArray
    */
    return toQBA(name.at(0).toUpper() + name.mid(1));
}

static QByteArrayList toByteArrayList(const QStringList &stringList)
{
    /*
     * Helper method to convert a list of strings into a list of QByteArrays
    */
    QByteArrayList byteArrayList;
    byteArrayList.reserve(stringList.size());
    for (const QString &str : stringList)
        byteArrayList.append(toQBA(str));

    return byteArrayList;
};

static EnumData enumToDefinition(const ASTEnum &astEnum)
{
    EnumData enumEntry;
    enumEntry.name = toQBA(astEnum.name);
    enumEntry.isFlag = false;
    enumEntry.isScoped = astEnum.isScoped;
    enumEntry.keyCount = astEnum.params.size();
    enumEntry.size = sizeof(int);
    if (!astEnum.type.isEmpty()) {
        auto metaType = QMetaType::fromName(toQBA(astEnum.type));
        if (metaType.isValid())
            enumEntry.size = metaType.sizeOf();
    }

    for (const auto &param : astEnum.params) {
        EnumPair pair;
        pair.name = toQBA(param.name);
        pair.value = param.value;
        enumEntry.values.append(pair);
    }

    return enumEntry;
}

static GadgetData podToGadget(const POD &pod)
{
    // This class converts the AST for a POD into a GadgetData structure
    GadgetData gadgetData;

    for (const auto &attribute : pod.attributes) {
        GadgetProperty property;
        property.name = toQBA(attribute.name);
        property.type = toQBA(attribute.type);
        gadgetData.properties.append(property);
    }

    for (const auto &_enum : pod.enums)
        gadgetData.enums.append(enumToDefinition(_enum));

    // TODO: Fix support for Flags - don't think this currently works for dynamic gadgets
    // Unclear what the TypeInfo would be for a Flag - it relies on a template type
    // Should this error out with a warning?
    /*
    for (const auto &flag : pod.flags) {
        EnumData enumEntry;
        enumEntry.name = toQBA(flag.name);
        enumEntry.isFlag = true;
        auto it = std::find_if(gadgetData.enums.begin(), gadgetData.enums.end(),
                            [&flag](const EnumData &enumData) { return enumData.name == toQBA(flag._enum); });
        if (it != gadgetData.enums.end()) {
            it->isFlag = true;
        }
        gadgetData.enums.append(enumEntry);
    }
    */

    return gadgetData;
}

QMetaObject *createAndRegisterMetaTypeFromPOD(const POD &pod, QObject *reference)
{
    auto gadget = podToGadget(pod);
    return registerGadget(reference, gadget, toQBA(pod.name));
}

static std::tuple<bool, bool, bool> sourceModifiers(ASTProperty::Modifier modifier)
{
    /*
     * Helper method to set several variables appropriately for a Source object type,
     * based on the modifier set on the property in the .rep file.
    */
    bool isWritable = modifier == ASTProperty::ReadWrite || modifier == ASTProperty::ReadPush || modifier == ASTProperty::SourceOnlySetter;
    bool hasNotify = modifier != ASTProperty::Constant;
    bool hasPush = modifier == ASTProperty::ReadPush;
    return std::make_tuple(isWritable, hasNotify, hasPush);
};

static std::tuple<bool, bool, bool> replicaModifiers(ASTProperty::Modifier modifier)
{
    /*
     * Helper method to set several variables appropriately for a Replica object type,
     * based on the modifier set on the property in the .rep file.
    */
    bool isWritable = modifier == ASTProperty::ReadWrite;
    bool hasNotify = true;
    bool hasPush = modifier == ASTProperty::ReadPush;
    return std::make_tuple(isWritable, hasNotify, hasPush);
};

static ClassData classToDefinition(const ASTClass &astClass, bool isSource=false)
{
    std::function<std::tuple<bool, bool, bool>(ASTProperty::Modifier)> modifiers = isSource ? sourceModifiers : replicaModifiers;

    // This class converts the AST for a Class into a ClassData structure
    ClassData classData(isSource);
    classData.type = toQBA(astClass.name);
    for (const auto &prop : astClass.properties) {
        auto [isWritable, hasNotify, hasPush] = modifiers(prop.modifier);
        ClassProperty property;
        property.name = toQBA(prop.name);
        property.typeName = toQBA(prop.type);
        property.isWritable = isWritable;
        if (hasNotify) {
            QByteArray signature = property.name + "Changed(" + property.typeName + ")";
            classData._signals.append({signature, QByteArrayList() << property.name});
            property.signalName = signature;
        }
        if (hasPush) {
            ClassSlot classSlot;
            classSlot.signature = QByteArray("push") + capName(prop.name) + '(' + toQBA(prop.type) + ')';
            classSlot.parameterNames = QByteArrayList() << toQBA(prop.name);
            classData._slots.append(classSlot);
        }
        classData.properties.append(property);
    }
    for (const auto &signal : astClass.signalsList) {
        ClassSignal classSignal;
        classSignal.signature = toQBA(signal.name) + '(' + toQBA(signal.paramsAsString(ASTFunction::Normalized)) + ')';
        classSignal.parameterNames = toByteArrayList(signal.paramNames());
        classData._signals.append(classSignal);
    }
    for (const auto &slot : astClass.slotsList) {
        ClassSlot classSlot;
        classSlot.signature = toQBA(slot.name) + '(' + toQBA(slot.paramsAsString(ASTFunction::Normalized)) + ')';
        const bool isVoid = slot.returnType.isEmpty() || slot.returnType == QStringLiteral("void");
        if (!isVoid) {
            if (isSource)
                classSlot.returnType = toQBA(slot.returnType);
            else
                classSlot.returnType = QByteArrayLiteral("QRemoteObjectPendingCall");
        }
        classSlot.parameterNames = toByteArrayList(slot.paramNames());
        classData._slots.append(classSlot);
    }
    for (const auto &_enum : astClass.enums)
        classData.enums.append(enumToDefinition(_enum));

    // TODO: Fix support for Flags - don't think this currently works for dynamic gadgets
    // Unclear what the TypeInfo would be for a Flag - it relies on a template type
    // Should this error out with a warning?
    /*
    for (const auto &flag : pod.flags) {
        EnumData enumEntry;
        enumEntry.name = toQBA(flag.name);
        enumEntry.isFlag = true;
        auto it = std::find_if(gadgetData.enums.begin(), gadgetData.enums.end(),
                            [&flag](const EnumData &enumData) { return enumData.name == toQBA(flag._enum); });
        if (it != gadgetData.enums.end()) {
            it->isFlag = true;
        }
        gadgetData.enums.append(enumEntry);
    }
    */

    return classData;
}

QMetaObject *createAndRegisterReplicaFromASTClass(const ASTClass &astClass, QObject *reference)
{
    auto classDef = classToDefinition(astClass);
    return registerAndTrackDefinition(classDef, reference);
}

QMetaObject *createAndRegisterSourceFromASTClass(const ASTClass &astClass, QObject *reference)
{
    auto classDef = classToDefinition(astClass, true);
    return registerAndTrackDefinition(classDef, reference);
}

bool addTracker(const QByteArray &typeName, QObject *reference)
{
    /*
     * Helper method to add additional QObject pointers as a reference to a type.
     * Memory will be freed when the last reference is destroyed.
    */
    return trackAdditionalReference(reference, typeName);
}

QT_END_NAMESPACE
