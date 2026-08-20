// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// Qt-Security score:significant reason:default


#include "qaxbase.h"

#include <QtAxBase/private/qaxtypefunctions_p.h>

#include <qmetaobject.h>
#include <quuid.h>
#include <qt_windows.h>
#include <qtextstream.h>
#include <qiodevicebase.h>
#include <private/qtools_p.h>

#include <ctype.h>

#include "../shared/qaxtypes_p.h"

#include <QtCore/private/qcomptr_p.h>

QT_BEGIN_NAMESPACE

QString qax_docuFromName(ITypeInfo *typeInfo, const QString &name)
{
    QString docu;
    if (!typeInfo)
        return docu;

    MEMBERID memId;
    BSTR names = QStringToBSTR(name);
    typeInfo->GetIDsOfNames(&names, 1, &memId);
    SysFreeString(names);
    if (memId != DISPID_UNKNOWN) {
        BSTR docStringBstr, helpFileBstr;
        ulong helpContext;
        HRESULT hres = typeInfo->GetDocumentation(memId, nullptr, &docStringBstr, &helpContext, &helpFileBstr);
        QString docString = QString::fromWCharArray(docStringBstr);
        QString helpFile = QString::fromWCharArray(helpFileBstr);
        SysFreeString(docStringBstr);
        SysFreeString(helpFileBstr);
        if (hres == S_OK) {
            if (!docString.isEmpty())
                docu += docString + QLatin1String("\n");
            if (!helpFile.isEmpty())
                docu += QString::fromLatin1("For more information, see help context %1 in %2.")
                        .arg(uint(helpContext)).arg(helpFile);
        }
    }

    return docu;
}

static inline QString docuFromName(ITypeInfo *typeInfo, const QString &name)
{
    return QLatin1String("<p>") + qax_docuFromName(typeInfo, name) + QLatin1String("\n");
}

static QByteArray namedPrototype(const QByteArrayList &parameterTypes, const QByteArrayList &parameterNames, int numDefArgs = 0)
{
    QByteArray prototype("(");
    for (qsizetype p = 0; p < parameterTypes.size(); ++p) {
        prototype += parameterTypes.at(p);

        if (p < parameterNames.size())
            prototype += ' ' + parameterNames.at(p);

        if (numDefArgs >= parameterTypes.size() - p)
            prototype += " = 0";
        if (p < parameterTypes.size() - 1)
            prototype += ", ";
    }
    prototype += ')';

    return prototype;
}

static QByteArray toType(const QByteArray &t)
{
    QByteArray type = QMetaType::fromName(t).id() != QMetaType::UnknownType
        ? t : QByteArrayLiteral("int");

    if (type.at(0) == 'Q')
        type.remove(0, 1);
    type[0] = QtMiscUtils::toAsciiLower(type.at(0));
    if (type == "VariantList")
        type = "List";
    else if (type == "Map<QVariant,QVariant>")
        type = "Map";
    else if (type == "Uint")
        type = "UInt";

    return "to" + type + "()";
}

QString qax_generateDocumentation(QAxBase *that)
{
    that->axBaseMetaObject();

    if (that->isNull())
        return QString();

    ComPtr<ITypeInfo> typeInfo;
    ComPtr<IDispatch> dispatch;
    that->queryInterface(IID_IDispatch, &dispatch);
    if (dispatch)
        dispatch->GetTypeInfo(0, LOCALE_SYSTEM_DEFAULT, &typeInfo);

    QString docu;
    QTextStream stream(&docu, QIODeviceBase::WriteOnly);

    const QMetaObject *mo = that->axBaseMetaObject();
    QString coClass  = QLatin1String(mo->classInfo(mo->indexOfClassInfo("CoClass")).value());

    stream << "<h1 align=center>" << coClass << " Reference</h1>" << Qt::endl;
    stream << "<p>The " << coClass << " COM object is a " << that->qObject()->metaObject()->className();
    stream << " with the CLSID " <<  that->control() << ".</p>" << Qt::endl;

    stream << "<h3>Interfaces</h3>" << Qt::endl;
    stream << "<ul>" << Qt::endl;
    for (int interCount = 1; ; ++interCount) {
         const QByteArray name = "Interface " + QByteArray::number(interCount);
         const int index = mo->indexOfClassInfo(name.constData());
         if (index < 0)
             break;
         stream << "<li>" << mo->classInfo(index).value() << Qt::endl;
    }
    stream << "</ul>" << Qt::endl;

    stream << "<h3>Event Interfaces</h3>" << Qt::endl;
    stream << "<ul>" << Qt::endl;
    for (int interCount = 1; ; ++interCount) {
        const QByteArray name = ("Event Interface " + QByteArray::number(interCount));
        const int index = mo->indexOfClassInfo(name.constData());
        if (index < 0)
            break;
        stream << "<li>" <<  mo->classInfo(index).value() << Qt::endl;
    }
    stream << "</ul>" << Qt::endl;

    QStringList methodDetails;
    QStringList propDetails;

    const int slotCount = mo->methodCount();
    if (slotCount) {
        stream << "<h2>Public Slots:</h2>" << Qt::endl;
        stream << "<ul>" << Qt::endl;

        int defArgCount = 0;
        for (int islot = mo->methodOffset(); islot < slotCount; ++islot) {
            const QMetaMethod slot = mo->method(islot);
            if (slot.methodType() != QMetaMethod::Slot)
                continue;

            if (slot.attributes() & QMetaMethod::Cloned) {
                ++defArgCount;
                continue;
            }

            QByteArray returntype(slot.typeName());
            if (returntype.isEmpty())
                returntype = "void";
            QByteArray prototype = namedPrototype(slot.parameterTypes(), slot.parameterNames(), defArgCount);
            QByteArray signature = slot.methodSignature();
            QByteArray name = signature.left(signature.indexOf('('));
            stream << "<li>" << returntype << " <a href=\"#" << name << "\"><b>" << name << "</b></a>" << prototype << ";</li>" << Qt::endl;

            prototype = namedPrototype(slot.parameterTypes(), slot.parameterNames());
            QString detail = QString::fromLatin1("<h3><a name=") + QString::fromLatin1(name.constData()) + QLatin1String("></a>") +
                             QLatin1String(returntype.constData()) + QLatin1Char(' ') +
                             QLatin1String(name.constData()) + QLatin1Char(' ') +
                             QString::fromLatin1(prototype.constData()) + QLatin1String("<tt> [slot]</tt></h3>\n");
            prototype = namedPrototype(slot.parameterTypes(), {});
            detail += docuFromName(typeInfo.Get(), QString::fromLatin1(name.constData()));
            detail += QLatin1String("<p>Connect a signal to this slot:<pre>\n");
            detail += QString::fromLatin1("\tQObject::connect(sender, SIGNAL(someSignal") + QString::fromLatin1(prototype.constData()) +
                      QLatin1String("), object, SLOT(") + QString::fromLatin1(name.constData()) +
                      QString::fromLatin1(prototype.constData()) + QLatin1String("));");
            detail += QLatin1String("</pre>\n");

            detail += QLatin1String("<p>Or call the function directly:<pre>\n");

            const bool hasParams = !slot.parameterTypes().isEmpty();
            if (hasParams)
                detail += QLatin1String("\tQVariantList params = ...\n");
            detail += QLatin1String("\t");
            QByteArray functionToCall = "dynamicCall";
            if (returntype == "IDispatch*" || returntype == "IUnknown*") {
                functionToCall = "querySubObject";
                returntype = "QAxObject *";
            }
            if (returntype != "void")
                detail += QLatin1String(returntype.constData()) + QLatin1String(" result = ");
            detail += QLatin1String("object->") + QLatin1String(functionToCall.constData()) +
                      QLatin1String("(\"" + name + prototype + '\"');
            if (hasParams)
                detail += QLatin1String(", params");
            detail += QLatin1Char(')');
            if (returntype != "void" && returntype != "QAxObject *" && returntype != "QVariant")
                detail += QLatin1Char('.') + QLatin1String(toType(returntype));
            detail += QLatin1String(";</pre>\n");

            methodDetails << detail;
            defArgCount = 0;
        }

        stream << "</ul>" << Qt::endl;
    }
    int signalCount = mo->methodCount();
    if (signalCount) {
        ComPtr<ITypeLib> typeLib;
        if (typeInfo) {
            UINT index = 0;
            typeInfo->GetContainingTypeLib(&typeLib, &index);
        }
        typeInfo.Reset();

        stream << "<h2>Signals:</h2>" << Qt::endl;
        stream << "<ul>" << Qt::endl;

        for (int isignal = mo->methodOffset(); isignal < signalCount; ++isignal) {
            const QMetaMethod signal(mo->method(isignal));
            if (signal.methodType() != QMetaMethod::Signal)
                continue;

            QByteArray prototype = namedPrototype(signal.parameterTypes(), signal.parameterNames());
            QByteArray signature = signal.methodSignature();
            QByteArray name = signature.left(signature.indexOf('('));
            stream << "<li>void <a href=\"#" << name << "\"><b>" << name << "</b></a>" << prototype << ";</li>" << Qt::endl;

            QString detail = QLatin1String("<h3><a name=") + QLatin1String(name.constData()) + QLatin1String("></a>void ") +
                             QLatin1String(name.constData()) + QLatin1Char(' ') +
                             QLatin1String(prototype.constData()) + QLatin1String("<tt> [signal]</tt></h3>\n");
            if (typeLib) {
                UINT interCount = 0;
                do {
                    typeInfo.Reset();
                    typeLib->GetTypeInfo(++interCount, &typeInfo);
                    QString typeLibDocu = docuFromName(typeInfo.Get(), QString::fromLatin1(name.constData()));
                    if (!typeLibDocu.isEmpty()) {
                        detail += typeLibDocu;
                        break;
                    }
                } while (typeInfo);
            }
            prototype = namedPrototype(signal.parameterTypes(), {});
            detail += QLatin1String("<p>Connect a slot to this signal:<pre>\n");
            detail += QLatin1String("\tQObject::connect(object, SIGNAL(") + QString::fromLatin1(name.constData()) +
                      QString::fromLatin1(prototype.constData()) +
                      QLatin1String("), receiver, SLOT(someSlot") + QString::fromLatin1(prototype.constData()) + QLatin1String("));");
            detail += QLatin1String("</pre>\n");

            methodDetails << detail;
            typeInfo.Reset();
        }
        stream << "</ul>" << Qt::endl;
    }

    const int propCount = mo->propertyCount();
    if (propCount) {
        if (dispatch)
            dispatch->GetTypeInfo(0, LOCALE_SYSTEM_DEFAULT, &typeInfo);
        stream << "<h2>Properties:</h2>" << Qt::endl;
        stream << "<ul>" << Qt::endl;

        for (int iprop = 0; iprop < propCount; ++iprop) {
            const QMetaProperty prop = mo->property(iprop);
            QByteArray name(prop.name());
            QByteArray type(prop.typeName());

            stream << "<li>" << type << " <a href=\"#" << name << "\"><b>" << name << "</b></a>;</li>" << Qt::endl;
            QString detail = QLatin1String("<h3><a name=") + QString::fromLatin1(name.constData()) + QLatin1String("></a>") +
                             QLatin1String(type.constData()) +
                             QLatin1Char(' ') + QLatin1String(name.constData()) + QLatin1String("</h3>\n");
            detail += docuFromName(typeInfo.Get(), QString::fromLatin1(name));
            if (!prop.isReadable())
                continue;

            const int vartype = prop.isEnumType()
                ? int(QMetaType::Int)
                : QMetaType::fromName(type).id();

            if (vartype != QMetaType::UnknownType) {
                detail += QLatin1String("<p>Read this property's value using QObject::property:<pre>\n");
                if (prop.isEnumType())
                    detail += QLatin1String("\tint val = ");
                else
                    detail += QLatin1Char('\t') + QLatin1String(type.constData()) + QLatin1String(" val = ");
                detail += QLatin1String("object->property(\"") + QLatin1String(name.constData()) +
                          QLatin1String("\").") + QLatin1String(toType(type).constData()) + QLatin1String(";\n");
                detail += QLatin1String("</pre>\n");
            } else if (type == "IDispatch*" || type == "IUnknown*") {
                detail += QLatin1String("<p>Get the subobject using querySubObject:<pre>\n");
                detail += QLatin1String("\tQAxObject *") + QLatin1String(name.constData()) +
                          QLatin1String(" = object->querySubObject(\"") + QLatin1String(name.constData()) + QLatin1String("\");\n");
                detail += QLatin1String("</pre>\n");
            } else {
                detail += QLatin1String("<p>This property is of an unsupported type.\n");
            }
            if (prop.isWritable()) {
                detail += QLatin1String("Set this property' value using QObject::setProperty:<pre>\n");
                if (prop.isEnumType()) {
                    detail += QLatin1String("\tint newValue = ... // string representation of values also supported\n");
                } else {
                    detail += QLatin1String("\t") + QString::fromLatin1(type.constData()) + QLatin1String(" newValue = ...\n");
                }
                detail += QLatin1String("\tobject->setProperty(\"") + QString::fromLatin1(name) + QLatin1String("\", newValue);\n");
                detail += QLatin1String("</pre>\n");
                detail += QLatin1String("Or using the ");
                QByteArray setterSlot;
                if (isupper(name.at(0))) {
                    setterSlot = "Set" + name;
                } else {
                    QByteArray nameUp = name;
                    nameUp[0] = QtMiscUtils::toAsciiUpper(nameUp.at(0));
                    setterSlot = "set" + nameUp;
                }
                detail += QLatin1String("<a href=\"#") + QString::fromLatin1(setterSlot) + QLatin1String("\">") +
                          QString::fromLatin1(setterSlot.constData()) + QLatin1String("</a> slot.\n");
            }
            if (prop.isEnumType()) {
                detail += QLatin1String("<p>See also <a href=\"#") + QString::fromLatin1(type) +
                QLatin1String("\">") + QString::fromLatin1(type) + QLatin1String("</a>.\n");
            }

            propDetails << detail;
        }
        stream << "</ul>" << Qt::endl;
    }

    const int enumCount = mo->enumeratorCount();
    if (enumCount) {
        stream << "<hr><h2>Member Type Documentation</h2>" << Qt::endl;
        for (int i = 0; i < enumCount; ++i) {
            const QMetaEnum enumdata = mo->enumerator(i);
            stream << "<h3><a name=" << enumdata.name() << "></a>" << enumdata.name() << "</h3>" << Qt::endl;
            stream << "<ul>" << Qt::endl;
            for (int e = 0; e < enumdata.keyCount(); ++e) {
                stream << "<li>" << enumdata.key(e) << "\t=" << enumdata.value(e) << "</li>" << Qt::endl;
            }
            stream << "</ul>" << Qt::endl;
        }
    }
    if (!methodDetails.isEmpty()) {
        stream << "<hr><h2>Member Function Documentation</h2>" << Qt::endl;
        for (qsizetype i = 0; i < methodDetails.size(); ++i)
            stream << methodDetails.at(i) << Qt::endl;
    }
    if (!propDetails.isEmpty()) {
        stream << "<hr><h2>Property Documentation</h2>" << Qt::endl;
        for (qsizetype i = 0; i < propDetails.size(); ++i)
            stream << propDetails.at(i) << Qt::endl;
    }

    return docu;
}

QT_END_NAMESPACE
