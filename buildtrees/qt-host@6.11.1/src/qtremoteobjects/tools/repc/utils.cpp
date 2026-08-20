// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <qjsonvalue.h>
#include <qjsonarray.h>
#include <qjsonobject.h>

#include "utils.h"
#include "repparser.h"


#define _(X) QLatin1String(X)

QT_BEGIN_NAMESPACE

namespace JSON
{
    enum Types {
        Any,
        Array,
        Object,
        String,
        Bool
    };

    static QJsonValue getItem(const QJsonValue &json, const char *key, JSON::Types type = JSON::Any)
    {
        if (json.isUndefined())
            qCritical() << "Invalid metadata json file. Unexpected Undefined value when looking for key:" << key;
        if (!json.isObject())
            qCritical() << "Invalid metadata json file. Input (" << json << ") is not an object when looking for key:" << key;
        QJsonValue value = json.toObject()[_(key)];
        switch (type) {
            case JSON::Any: break;
            case JSON::Array:
                if (!value.isArray())
                    qCritical() << "Invalid metadata json file. Value (" << value << ") is not an array when looking for key:" << key;
                break;
            case JSON::Object:
                if (!value.isObject())
                    qCritical() << "Invalid metadata json file. Value (" << value << ") is not an object when looking for key:" << key;
                break;
            case JSON::String:
                if (!value.isString())
                    qCritical() << "Invalid metadata json file. Value (" << value << ") is not a string when looking for key:" << key;
                break;
            case JSON::Bool:
                if (!value.isBool())
                    qCritical() << "Invalid metadata json file. Value (" << value << ") is not a bool when looking for key:" << key;
                break;
        }
        return value;
    }

    static bool containsKey(const QJsonValue &json, const char *key)
    {
        if (json.isUndefined())
            qCritical() << "Invalid metadata json file. Unexpected Undefined value when looking for key:" << key;
        if (!json.isObject())
            qCritical() << "Invalid metadata json file. Input (" << json << ") is not an object when looking for key:" << key;
        return json.toObject().contains(_(key));
    }

    static bool isEmptyArray(const QJsonValue &json, const char *key)
    {
        if (!containsKey(json, key))
            return true;
        const auto value = getItem(json, key);
        if (!value.isArray())
            qCritical() << "Invalid metadata json file." << key << "is not an array.";
        return value.toArray().count() == 0;
    }

    static QJsonArray getArray(const QJsonValue &json, const char *key)
    {
        return getItem(json, key, JSON::Array).toArray();
    }
    static QString getString(const QJsonValue &json, const char *key)
    {
        return getItem(json, key, JSON::String).toString();
    }
    static QByteArray getBytes(const QJsonValue &json, const char *key)
    {
        return getItem(json, key, JSON::String).toString().toLatin1();
    }
    static bool getBool(const QJsonValue &json, const char *key)
    {
        return getItem(json, key, JSON::Bool).toBool();
    }
    static bool getBool(const QJsonValue &json, const char *key, bool missingValue)
    {
        if (!containsKey(json, key))
            return missingValue;
        bool res = getBool(json, key);
        return res;
    }
}

using namespace JSON;

static QByteArray join(const QByteArrayList &array, const QByteArray &separator)
{
    QByteArray res;
    const auto sz = array.size();
    if (!sz)
        return res;
    for (qsizetype i = 0; i < sz - 1; i++)
        res += array.at(i) + separator;
    res += array.at(sz - 1);
    return res;
}

static QByteArrayList generateProperties(const QJsonArray &properties, bool isPod=false)
{
    QByteArrayList ret;
    for (const QJsonValue prop : properties) {
        if (!isPod && !containsKey(prop, "notify") && !getBool(prop, "constant")) {
            qWarning() << "Skipping property" << getString(prop, "name")
                       << "because it is non-notifiable & non-constant";
            continue; // skip non-notifiable properties
        }
        QByteArray output = getBytes(prop, "type") + " " + getBytes(prop, "name");
        if (getBool(prop, "constant"))
            output += " CONSTANT";
        if (!containsKey(prop, "write") && containsKey(prop, "read."))
            output += " READONLY";
        ret << output;
    }
    return ret;
}

static QByteArray generateFunctions(const QByteArray &type, const QJsonArray &functions)
{
    QByteArray ret;
    for (const QJsonValue func : functions) {
        ret += type + "(" + getBytes(func, "returnType") + " " + getBytes(func, "name") + "(";
        const auto arguments = getArray(func, "arguments");
        for (const QJsonValue arg : arguments)
            ret += getBytes(arg, "type") + " " + getBytes(arg, "name") + ", ";
        if (arguments.count())
            ret.chop(2);
        ret += "));\n";
    }
    return ret;
}

const auto filterNotPublic = [](const QJsonValue &value) {
    return getString(value, "access") != QStringLiteral("public");
};

static QJsonArray cleanedSignalList(const QJsonValue &cls)
{
    if (isEmptyArray(cls, "signals"))
        return QJsonArray();

    auto signalList = getArray(cls, "signals");
    if (isEmptyArray(cls, "properties"))
        return signalList;

    const auto props = getArray(cls, "properties");
    const auto filterNotify = [&props](const QJsonValue &value) {
        const auto filter = [&value](const QJsonValue &prop) {
            return getItem(value, "name") == getItem(prop, "notify");
        };
        return std::find_if(props.begin(), props.end(), filter) != props.end();
    };
    for (auto it = signalList.begin(); it != signalList.end(); /* blank */ ) {
        if (filterNotify(*it))
            it = signalList.erase(it);
        else if (filterNotPublic(*it))
            it = signalList.erase(it);
        else
            it++;
    }
    return signalList;
}

static QJsonArray cleanedSlotList(const QJsonValue &cls)
{
    if (isEmptyArray(cls, "slots"))
        return QJsonArray();

    auto slotList = getArray(cls, "slots");
    if (!isEmptyArray(cls, "properties"))
        return slotList;

    const auto props = getArray(cls, "properties");
    const auto filterWrite = [&props](const QJsonValue &value) {
        const auto filter = [&value](const QJsonValue &prop) {
            const auto args = getArray(prop, "arguments");
            return getItem(value, "name") == getItem(prop, "write") && args.count() == 1
                    && getItem(args.at(0), "type") == getItem(prop, "type");
        };
        return std::find_if(props.begin(), props.end(), filter) != props.end();
    };
    for (auto it = slotList.begin(); it != slotList.end(); /* blank */ ) {
        if (filterWrite(*it))
            it = slotList.erase(it);
        else if (filterNotPublic(*it))
            it = slotList.erase(it);
        else
            it++;
    }
    return slotList;
}

QByteArray generateClass(const QJsonValue &cls, bool alwaysGenerateClass)
{
    if (getBool(cls, "gadget", false) || alwaysGenerateClass
        || (isEmptyArray(cls, "signals") && isEmptyArray(cls, "slots")))
        return "POD " + getBytes(cls, "className") + "("
                + join(generateProperties(getArray(cls, "properties"), true), ", ") + ")\n";

    QByteArray ret("class " + getBytes(cls, "className") + "\n{\n");
    if (!isEmptyArray(cls, "properties"))
        ret += "    PROP(" + join(generateProperties(getArray(cls, "properties")), ");\n    PROP(")
                + ");\n";
    ret += generateFunctions("    SLOT", cleanedSlotList(cls));
    ret += generateFunctions("    SIGNAL", cleanedSignalList(cls));
    ret += "}\n";
    return ret;
}

static QList<PODAttribute> propertyList2PODAttributes(const QJsonArray &list)
{
    QList<PODAttribute> ret;
    for (const QJsonValue prop : list)
        ret.push_back(PODAttribute(getString(prop, "type"), getString(prop, "name")));
    return ret;
}

QList<ASTProperty> propertyList2AstProperties(const QJsonArray &list)
{
    QList<ASTProperty> ret;
    for (const QJsonValue property : list) {
        if (!containsKey(property, "notify") && !getBool(property, "constant")) {
            qWarning() << "Skipping property" << getString(property, "name")
                       << "because it is non-notifiable & non-constant";
            continue; // skip non-notifiable properties
        }
        ASTProperty prop;
        prop.name = getString(property, "name");
        prop.type = getString(property, "type");
        prop.modifier = getBool(property, "constant") ? ASTProperty::Constant
                : !containsKey(property, "write") && containsKey(property, "read")
                ? ASTProperty::ReadOnly
                : ASTProperty::ReadWrite;
        ret.push_back(prop);
    }
    return ret;
}

QList<ASTFunction> functionList2AstFunctionList(const QJsonArray &list)
{
    QList<ASTFunction> ret;
    for (const QJsonValue function : list) {
        ASTFunction func;
        func.name = getString(function, "name");
        func.returnType = getString(function, "returnType");
        const auto arguments = getArray(function, "arguments");
        for (const QJsonValue arg : arguments)
            func.params.push_back(ASTDeclaration(getString(arg, "type"), getString(arg, "name")));
        ret.push_back(func);
    }
    return ret;
}

AST classList2AST(const QJsonArray &classes)
{
    AST ret;
    for (const QJsonValue cls : classes) {
        if (isEmptyArray(cls, "signals") && isEmptyArray(cls, "slots")) {
            POD pod;
            pod.name = getString(cls, "className");
            pod.attributes = propertyList2PODAttributes(getArray(cls, "properties"));
            ret.pods.push_back(pod);
        } else {
            ASTClass cl(getString(cls, "className"));
            cl.properties = propertyList2AstProperties(getArray(cls, "properties"));
            cl.signalsList = functionList2AstFunctionList(cleanedSignalList(cls));
            cl.slotsList = functionList2AstFunctionList(cleanedSlotList(cls));
            ret.classes.push_back(cl);
        }
    }
    return ret;
}

QT_END_NAMESPACE
