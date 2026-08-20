// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "moc.h"

#include <QAxObject>
#include <QAxBaseWidget>
#include <QFile>
#include <QMetaObject>
#include <QMetaEnum>
#include <QDebug>
#include <QTextStream>
#include <QSettings>
#include <QStringList>
#include <QTemporaryFile>
#include <QUuid>
#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFileInfo>
#include <qt_windows.h>
#include <ocidl.h>
#include <private/qmetaobject_p.h>
QT_BEGIN_NAMESPACE

static ITypeInfo *currentTypeInfo = nullptr;

enum ProgramMode {
    GenerateMode,
    TypeLibID
};

enum ObjectCategory
{
    DefaultObject    = 0x00,
    SubObject        = 0x001,
    ActiveX          = 0x002,
    NoMetaObject     = 0x004,
    NoImplementation = 0x008,
    NoDeclaration    = 0x010,
    NoInlines        = 0x020,
    OnlyInlines      = 0x040,
    Licensed         = 0x100,
};

Q_DECLARE_FLAGS(ObjectCategories, ObjectCategory)
Q_DECLARE_OPERATORS_FOR_FLAGS(ObjectCategories)

extern QMetaObject *qax_readEnumInfo(ITypeLib *typeLib, const QMetaObject *parentObject);
extern QMetaObject *qax_readClassInfo(ITypeLib *typeLib, ITypeInfo *typeInfo, const QMetaObject *parentObject);
extern QMetaObject *qax_readInterfaceInfo(ITypeLib *typeLib, ITypeInfo *typeInfo, const QMetaObject *parentObject);
extern QByteArrayList qax_qualified_usertypes;
extern QHash<QByteArray, QByteArray> qax_enum_values;
extern QString qax_docuFromName(ITypeInfo *typeInfo, const QString &name);
extern bool qax_dispatchEqualsIDispatch;
extern void qax_deleteMetaObject(QMetaObject *mo);

static QMap<QByteArray, QByteArray> namespaceForType;
static QList<QByteArray> strings;
static QHash<QByteArray, int> stringIndex; // Optimization, speeds up generation
static QByteArrayList vTableOnlyStubs;

void writeEnums(QTextStream &out, const QMetaObject *mo)
{
    // enums
    for (int ienum = mo->enumeratorOffset(); ienum < mo->enumeratorCount(); ++ienum) {
        formatCppEnum(out, mo->enumerator(ienum));
        out << '\n';
    }
}

void writeHeader(QTextStream &out, const QString &nameSpace, const QString &outFileName)
{
    out << "#ifndef QAX_DUMPCPP_" << outFileName.toUpper() << "_H" << Qt::endl;
    out << "#define QAX_DUMPCPP_" << outFileName.toUpper() << "_H" << Qt::endl;
    out << Qt::endl;
    out << "// Define this symbol to __declspec(dllexport) or __declspec(dllimport)" << Qt::endl;
    out << "#ifndef " << nameSpace.toUpper() << "_EXPORT" << Qt::endl;
    out << "#define " << nameSpace.toUpper() << "_EXPORT" << Qt::endl;
    out << "#endif" << Qt::endl;
    out << Qt::endl;
    out << "#include <qaxobject.h>" << Qt::endl;
    out << "#include <qaxwidget.h>" << Qt::endl;
    out << "#include <qdatetime.h>" << Qt::endl;
    out << "#include <qpixmap.h>" << Qt::endl;
    out << "#include <qpoint.h>" << Qt::endl;
    out << "#include <qrect.h>" << Qt::endl;
    out << "#include <qsize.h>" << Qt::endl;
    out << Qt::endl;
    out << "struct IDispatch;" << Qt::endl;
    out << Qt::endl;
}

void generateNameSpace(QTextStream &out, const QMetaObject *mo, const QByteArray &nameSpace)
{
    out << "namespace " << nameSpace << " {" << Qt::endl;
    out << Qt::endl;
    writeEnums(out, mo);

    // don't close on purpose
}

static QByteArray joinParameterNames(const QByteArrayList &parameterNames)
{
    QByteArray slotParameters;
    for (qsizetype p = 0; p < parameterNames.size(); ++p) {
        slotParameters += parameterNames.at(p);
        if (p < parameterNames.size() - 1)
            slotParameters += ',';
    }

    return slotParameters;
}

QByteArray constRefify(const QByteArray &type)
{
    QByteArray ctype(type);
    if (type == "QString" || type == "QPixmap"
        || type == "QVariant" || type == "QDateTime"
        || type == "QColor" || type == "QFont"
        || type == "QByteArray" || type == "QValueList<QVariant>"
        || type == "QStringList")
        ctype = "const " + ctype + '&';

    return ctype;
}

static void formatConstructorSignature(QTextStream &out, ObjectCategories category,
                                       bool declaration)
{
    out << '(';
    if (category & Licensed) {
        out << "const QString &licenseKey, ";
        if (declaration)
            out << " = QString()";
        out << ", ";
    }
    if (category & ActiveX) {
        out << "QWidget *parent";
        if (declaration)
            out << " = nullptr";
        out << ", Qt::WindowFlags f";
        if (declaration)
            out << " = {}";
    } else if (category & SubObject) {
        out << "IDispatch *subobject";
        if (declaration)
            out << " = nullptr";
        out << ", QAxObject *parent";
        if (declaration)
            out << " = nullptr";
    } else {
        out << "QObject *parent";
        if (declaration)
            out << " = nullptr";
    }
    out << ')';
}

static void formatMetaFunctions(QTextStream &out, const QByteArray &nameSpace,
                             const QByteArray &className, ObjectCategories category)
{
    // add a replacement qt_metacall()
    QStringView parentClass = (category & ActiveX) ? u"QAxWidget" : u"QAxObject";
    out << "int ";
    if (!nameSpace.isEmpty())
        out << nameSpace << "::";
    out << className << "::qt_metacall(QMetaObject::Call _c, int _id, void **_a)\n"
           "{\n"
           "    return " << parentClass << "::qt_metacall(_c, _id, _a);\n"
           "}\n\n";
}

static void formatConstructorBody(QTextStream &out, const QByteArray &nameSpace,
                                  const QByteArray &className,
                                  const QString &controlID, ObjectCategories category, bool useControlName)
{
    QString controlName;
    if (useControlName) {
        if (!nameSpace.isEmpty())
            controlName = QString::fromUtf8(nameSpace) + QStringLiteral(".");
        controlName += QString::fromUtf8(className);
    } else {
        controlName = controlID;
    }
    if (!nameSpace.isEmpty())
        out << nameSpace << "::";
    out << className << "::" << className;
    formatConstructorSignature(out, category, false);
    out << " :" << Qt::endl << "    ";
    if (category & ActiveX)
        out << "QAxWidget(parent, f";
    else if (category & SubObject)
        out << "QAxObject(subobject, parent";
    else
        out << "QAxObject(parent";
    out << ')' << Qt::endl << '{' << Qt::endl;
    if (category & SubObject) {
        out << "    internalRelease();" << Qt::endl;
    } else if (category & Licensed) {
        out << "    if (licenseKey.isEmpty())" << Qt::endl;
        out << "        setControl(QStringLiteral(\"" << controlName << "\"));" << Qt::endl;
        out << "    else" << Qt::endl;
        out << "        setControl(QStringLiteral(\"" << controlName << ":\") + licenseKey);" << Qt::endl;
    } else {
        out << "    setControl(QStringLiteral(\"" << controlName << "\"));" << Qt::endl;
    }
    out << '}' << Qt::endl << Qt::endl;
}

// Hash of C# only types.
static const QSet<QByteArray> cSharpTypes = {
    "ICloneable", "ICollection", "IDisposable", "IEnumerable",
    "IList", "ISerializable", "_Attribute"
};

// Return true if the passed string is one of the Qt type names that we re-defined in IDL files.
static bool isQtTypeName(QByteArrayView s)
{
    return s == "struct QPoint"
        || s == "struct QRect"
        || s == "struct QSize";
}

void generateClassDecl(QTextStream &out, const QMetaObject *mo,
                       const QByteArray &className, const QByteArray &nameSpace,
                       ObjectCategories category)
{
    QByteArrayList functions;

    QByteArray indent;
    if (!(category & OnlyInlines))
        indent = "    ";

    if (!(category & OnlyInlines)) {
        // constructors
        out << "class " << nameSpace.toUpper() << "_EXPORT " << className << " : public ";
        if (category & ActiveX)
            out << "QAxWidget";
        else
            out << "QAxObject";
        out << Qt::endl;

        out << '{' << Qt::endl;
        out << "public:" << Qt::endl << "    explicit " << className;
        formatConstructorSignature(out, category, true);
        out << ';' << Qt::endl;
    }

    functions << className;

    // enums
    if (nameSpace.isEmpty() && !(category & OnlyInlines))
        writeEnums(out, mo);
    // QAxBase public virtual functions.
    QByteArrayList axBase_vfuncs;
    axBase_vfuncs.append("metaObject");
    axBase_vfuncs.append("qObject");
    axBase_vfuncs.append("className");
    axBase_vfuncs.append("propertyWritable");
    axBase_vfuncs.append("setPropertyWritable");

    // properties
    for (int iprop = mo->propertyOffset(); iprop < mo->propertyCount(); ++iprop) {
        QMetaProperty property = mo->property(iprop);
        if (!property.isReadable())
            continue;

        QByteArray propertyName(property.name());
        if (propertyName == className)
            continue;

        if (!(category & OnlyInlines)) {
            out << indent << "/*" << Qt::endl << indent << "Property " << propertyName << Qt::endl;
            QString documentation = qax_docuFromName(currentTypeInfo, QString::fromLatin1(propertyName.constData()));
            if (!documentation.isEmpty()) {
                out << Qt::endl;
                out << indent << documentation << Qt::endl;
            }
            out << indent << "*/" << Qt::endl;
        }

        // Check whether the new function conflicts with any of QAxBase public virtual functions.
        // If so, prepend the function name with '<classname>_'. Since all internal metaobject magic
        // remains the same, we have to use the original name when used with QObject::connect or QMetaObject
        QByteArray propertyFunctionName(propertyName);
        if (axBase_vfuncs.contains(propertyFunctionName)) {
            propertyFunctionName = className + '_' + propertyName;
            qWarning("property conflits with QAXBase: %s changed to %s", propertyName.constData(), propertyFunctionName.constData());
        }

        QByteArray propertyType(property.typeName());

        QByteArray simplePropType = propertyType;
        simplePropType.replace('*', "");

        out << indent << "inline ";
        bool foreignNamespace = true;
        if (!propertyType.contains("::") &&
            (qax_qualified_usertypes.contains(simplePropType) || qax_qualified_usertypes.contains("enum "+ simplePropType))
           ) {
            propertyType.prepend(nameSpace + "::");
            foreignNamespace = false;
        }

        out << propertyType << ' ';

        if (category & OnlyInlines)
            out << className << "::";
        out << propertyFunctionName << "() const";

        if (!(category & NoInlines)) {
            out << Qt::endl << indent << '{' << Qt::endl;
            if (qax_qualified_usertypes.contains(simplePropType)) {
                if (foreignNamespace)
                    out << "#ifdef QAX_DUMPCPP_" << propertyType.left(propertyType.indexOf("::")).toUpper() << "_H" << Qt::endl;
                QByteArray simplePropTypeWithNamespace = propertyType;
                simplePropTypeWithNamespace.replace('*', "");
                out << indent << "    qRegisterMetaType<" << propertyType << ">(\"" << property.typeName() << "\");" << Qt::endl;
                out << indent << "    qRegisterMetaType<" << simplePropTypeWithNamespace << ">(\"" << simplePropType << "\");" << Qt::endl;
            }
            out << indent << "    QVariant qax_result = property(\"" << propertyName << "\");" << Qt::endl;
            if (propertyType.length() && propertyType.at(propertyType.length()-1) == '*')
                out << indent << "    if (qax_result.constData() == nullptr)\n"
                    << indent << "        return nullptr;\n"
                    << indent << "    Q_ASSERT(qax_result.isValid());" << Qt::endl;
            if (qax_qualified_usertypes.contains(simplePropType)) {
                simplePropType = propertyType;
                simplePropType.replace('*', "");
                out << indent << "    return *reinterpret_cast<" << propertyType << "*>(qax_result.data());\n";
                if (foreignNamespace) {
                    out << "#else" << Qt::endl;
                    out << indent << "    return nullptr; // foreign namespace not included" << Qt::endl;
                    out << "#endif" << Qt::endl;
                }

            } else {
                out << indent << "    return *reinterpret_cast<" << propertyType << "*>(qax_result.data());\n";
            }
            out << indent << '}' << Qt::endl;
        } else {
            out << "; //Returns the value of " << propertyName << Qt::endl;
        }

        functions << propertyName;

        if (property.isWritable()) {
            const QByteArray setter = setterName(propertyName);

            out << indent << "inline " << "void ";
            if (category & OnlyInlines)
                out << className << "::";
            out << setter << '(' << constRefify(propertyType) << " value)";

            if (!(category & NoInlines)) {
                if (propertyType.endsWith('*')) {
                    out << '{' << Qt::endl;
                    out << "    int typeId = qRegisterMetaType<" << propertyType << ">(\"" << propertyType << "\");" << Qt::endl;
                    out << "    setProperty(\"" << propertyName << "\", QVariant(QMetaType(typeId), &value));" << Qt::endl;
                    out << '}' << Qt::endl;
                } else {
                    out << "{ setProperty(\"" << propertyName << "\", QVariant(value)); }" << Qt::endl;
                }
            } else {
                out << "; //Sets the value of the " << propertyName << " property" << Qt::endl;
            }

            functions << setter;
        }

        out << Qt::endl;
    }

    // slots - but not property setters
    int defaultArguments = 0;
    for (int islot = mo->methodOffset(); islot < mo->methodCount(); ++islot) {
        const QMetaMethod slot(mo->method(islot));
        if (slot.methodType() != QMetaMethod::Slot)
            continue;

#if 0
        // makes not sense really to respect default arguments...
        if (slot.attributes() & Cloned) {
            ++defaultArguments;
            continue;
        }
#endif

        QByteArray slotSignature(slot.methodSignature());
        QByteArray slotName = slotSignature.left(slotSignature.indexOf('('));
        if (functions.contains(slotName))
            continue;

        if (!(category & OnlyInlines)) {
            out << indent << "/*" << Qt::endl << indent << "Method " << slotName << Qt::endl;
            QString documentation = qax_docuFromName(currentTypeInfo, QString::fromLatin1(slotName.constData()));
            if (!documentation.isEmpty()) {
                out << Qt::endl;
                out << indent << documentation << Qt::endl;
            }
            out << indent << "*/" << Qt::endl;
        }

        QByteArray slotParameters(joinParameterNames(slot.parameterNames()));
        QByteArray slotTag(slot.tag());
        QByteArray slotType(slot.typeName());

        QByteArray simpleSlotType = slotType;
        simpleSlotType.replace('*', "");
        if (!slotType.contains("::") && qax_qualified_usertypes.contains(simpleSlotType))
            slotType.prepend(nameSpace + "::");


        QByteArray slotNamedSignature;
        if (slotSignature.endsWith("()")) { // no parameters - no names
            slotNamedSignature = slotSignature;
        } else {
            slotNamedSignature = slotSignature.left(slotSignature.indexOf('(') + 1);
            QByteArray slotSignatureTruncated(slotSignature.mid(slotNamedSignature.length()));
            slotSignatureTruncated.truncate(slotSignatureTruncated.length() - 1);

            const auto signatureSplit = slotSignatureTruncated.split(',');
            QByteArrayList parameterSplit;
            if (slotParameters.isEmpty()) { // generate parameter names
                for (qsizetype i = 0; i < signatureSplit.size(); ++i)
                    parameterSplit << QByteArray("p") + QByteArray::number(i);
            } else {
                parameterSplit = slotParameters.split(',');
            }

            for (qsizetype i = 0; i < signatureSplit.count(); ++i) {
                QByteArray parameterType = signatureSplit.at(i);
                if (!parameterType.contains("::") && namespaceForType.contains(parameterType))
                    parameterType.prepend(namespaceForType.value(parameterType) + "::");

                QByteArray arraySpec; // transform array method signature "foo(int[4])" ->"foo(int p[4])"
                const qsizetype arrayPos = parameterType.lastIndexOf('[');
                if (arrayPos != -1) {
                    arraySpec = parameterType.right(parameterType.size() - arrayPos);
                    parameterType.truncate(arrayPos);
                }
                slotNamedSignature += constRefify(parameterType);
                slotNamedSignature += ' ';
                slotNamedSignature += parameterSplit.at(i);
                slotNamedSignature += arraySpec;
                if (defaultArguments >= signatureSplit.size() - i) {
                    slotNamedSignature += " = ";
                    slotNamedSignature += parameterType + "()";
                }
                if (i + 1 < signatureSplit.size())
                    slotNamedSignature += ", ";
            }
            slotNamedSignature += ')';
        }

        out << indent << "inline ";

        if (!slotTag.isEmpty())
            out << slotTag << ' ';
        else
            out << slotType << ' ';
        if (category & OnlyInlines)
            out << className << "::";

        // Update function name in case of conflicts with QAxBase public virtual functions.
        int parnIdx = slotNamedSignature.indexOf('(');
        QByteArray slotOriginalName =  slotNamedSignature.left(parnIdx);
        if (axBase_vfuncs.contains(slotOriginalName)) {
            QByteArray newSignature = className + '_' + slotOriginalName;
            newSignature += slotNamedSignature.mid(parnIdx);
            qWarning("function name conflits with QAXBase %s changed to %s", slotNamedSignature.constData(), newSignature.constData());
            slotNamedSignature = newSignature;
        }

        out << slotNamedSignature;

        if (category & NoInlines) {
            out << ';' << Qt::endl;
        } else {
            out << Qt::endl;
            out << indent << '{' << Qt::endl;

            if (slotType != QByteArrayLiteral("void")) {
                out << indent << "    " << slotType << " qax_result";
                if (slotType.endsWith('*'))
                    out << " = 0";
                out << ';' << Qt::endl;
                if (qax_qualified_usertypes.contains(simpleSlotType)) {
                    bool foreignNamespace = simpleSlotType.contains("::");
                    if (foreignNamespace)
                        out << "#ifdef QAX_DUMPCPP_" << simpleSlotType.left(simpleSlotType.indexOf(':')).toUpper() << "_H" << Qt::endl;
                    QByteArray simpleSlotTypeWithNamespace = slotType;
                    simpleSlotTypeWithNamespace.replace('*', "");
                    out << indent << "    qRegisterMetaType<" << simpleSlotTypeWithNamespace << "*>(\"" << simpleSlotType << "*\");" << Qt::endl;
                    if (!vTableOnlyStubs.contains(simpleSlotTypeWithNamespace))
                        out << indent << "    qRegisterMetaType<" << simpleSlotTypeWithNamespace << ">(\"" << simpleSlotType << "\");" << Qt::endl;
                    if (foreignNamespace)
                        out << "#endif" << Qt::endl;
                }
            }
            out << indent << "    void *_a[] = {";
            if (slotType != QByteArrayLiteral("void"))
                out << "(void*)&qax_result";
            else
                out << '0';
            if (!slotParameters.isEmpty()) {
                out << ", (void*)&";
                out << slotParameters.replace(",", ", (void*)&");
            }
            out << "};" << Qt::endl;

            out << indent << "    qt_metacall(QMetaObject::InvokeMetaMethod, " << islot << ", _a);" << Qt::endl;
            if (slotType != QByteArrayLiteral("void"))
                out << indent << "    return qax_result;" << Qt::endl;
            out << indent << '}' << Qt::endl;
        }

        out << Qt::endl;
        defaultArguments = 0;
    }

    if (!(category & OnlyInlines)) {
        if (!(category & NoMetaObject)) {
            out << "// meta object functions" << Qt::endl;
            out << "    Q_OBJECT_FAKE" << Qt::endl;
        }

        out << "};" << Qt::endl;
    }
}

bool generateClassImpl(QTextStream &out, const QMetaObject *mo, const QByteArray &className,
                       const QString &controlID,
                       const QByteArray &nameSpace, ObjectCategories category,
                       bool useControlName,
                       QString *errorString)
{
    Q_STATIC_ASSERT_X(QMetaObjectPrivate::OutputRevision == 13, "dumpcpp should generate the same version as moc");

    QByteArray qualifiedClassName;
    if (!nameSpace.isEmpty())
        qualifiedClassName = nameSpace + "::";
    qualifiedClassName += className;
    const QByteArray nestedQualifier = className + "::";

    QString moCode = mocCode(mo, QLatin1String(qualifiedClassName), errorString);
    if (moCode.isEmpty()) {
        out << "#error moc error\n";
        return false;
    }
    out << moCode << "\n\n";

    formatConstructorBody(out, nameSpace, className, controlID, category, useControlName);
    formatMetaFunctions(out, nameSpace, className, category);

    return true;
}

static void formatCommentBlockFooter(const QString &typeLibFile, QTextStream &str)
{
    str << " generated by dumpcpp v" << QT_VERSION_STR << " using\n**";
    const QStringList arguments = QCoreApplication::arguments();
    for (const QString &arg : arguments)
        str << ' ' << arg;
    str << "\n** from the type library " << typeLibFile << "\n**\n"
        << "****************************************************************************/\n\n";
}

static QByteArray classNameFromTypeInfo(ITypeInfo *typeinfo)
{
    BSTR bstr;
    QByteArray result;
    if (SUCCEEDED(typeinfo->GetDocumentation(-1, &bstr, nullptr, nullptr, nullptr))) {
        result = QString::fromWCharArray(bstr).toLatin1();
        SysFreeString(bstr);
    }
    return result;
}

// Extract a list of VTable only stubs from a ITypeLib.
static QByteArrayList vTableOnlyStubsFromTypeLib(ITypeLib *typelib, const QString &nameSpace)
{
    QByteArrayList result;
    const QByteArray nameSpacePrefix = nameSpace.toLatin1() + "::";
    for (UINT i = 0, typeCount = typelib->GetTypeInfoCount(); i < typeCount; ++i) {
        TYPEKIND typekind;
        if (SUCCEEDED(typelib->GetTypeInfoType(i, &typekind)) && typekind == TKIND_INTERFACE) {
            ITypeInfo *typeinfo = nullptr;
            if (SUCCEEDED(typelib->GetTypeInfo(i, &typeinfo) && typeinfo)) {
                result.append(nameSpacePrefix + classNameFromTypeInfo(typeinfo));
                typeinfo->Release();
            }
        }
    }
    return result;
}

static const QMetaObject *baseMetaObject(ObjectCategories c)
{
    return c.testFlag(ActiveX)
        ? &QAxBaseWidget::staticMetaObject
        : &QAxBaseObject::staticMetaObject;
}

bool generateTypeLibrary(QString typeLibFile, QString outname,
                         const QString &nameSpace, ObjectCategories category, bool useControlName)
{
    typeLibFile.replace(QLatin1Char('/'), QLatin1Char('\\'));

    ITypeLib *typelib;
    LoadTypeLibEx(reinterpret_cast<const wchar_t *>(typeLibFile.utf16()), REGKIND_NONE, &typelib);
    if (!typelib) {
        qWarning("dumpcpp: loading '%s' as a type library failed", qPrintable(typeLibFile));
        return false;
    }

    QString libName = nameSpace;
    if (libName.isEmpty()) {
        BSTR nameString = nullptr;
        if (SUCCEEDED(typelib->GetDocumentation(-1, &nameString, nullptr, nullptr, nullptr))) {
            libName = QString::fromWCharArray(nameString);
            SysFreeString(nameString);
        }
    }
    const QByteArray libNameBa = libName.toLatin1();
    vTableOnlyStubs = vTableOnlyStubsFromTypeLib(typelib, libName);

    QString libVersion(QLatin1String("1.0"));

    TLIBATTR *tlibattr = nullptr;
    typelib->GetLibAttr(&tlibattr);
    if (tlibattr) {
        libVersion = QString::fromLatin1("%1.%2").arg(tlibattr->wMajorVerNum).arg(tlibattr->wMinorVerNum);
        typelib->ReleaseTLibAttr(tlibattr);
    }

    if (outname.isEmpty())
        outname = libName.toLower();

    if (outname.isEmpty()) {
        qWarning("dumpcpp: no output filename provided, and cannot deduce output filename");
        return false;
    }

    QMetaObject *namespaceObject = qax_readEnumInfo(typelib, nullptr);

    QString classImpl;
    QTextStream classImplOut(&classImpl);
    QFile implFile(outname + QLatin1String(".cpp"));
    QTextStream implOut(&implFile);
    if (!(category & (NoMetaObject|NoImplementation))) {
        if (!implFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning("dumpcpp: Could not open output file '%s'", qPrintable(implFile.fileName()));
            return false;
        }

        implOut << "/****************************************************************************\n"
                   "**\n** Metadata for " << libName;
        formatCommentBlockFooter(typeLibFile, implOut);

        implOut << "#define QAX_DUMPCPP_" << libName.toUpper() << "_NOINLINES" << Qt::endl;

        implOut << "#include \"" << outname << ".h\"" << Qt::endl;
        implOut << "#include <OAIdl.h>" << Qt::endl; // For IDispatch
        implOut << Qt::endl;
    }

    QFile declFile(outname + QLatin1String(".h"));
    QTextStream declOut(&declFile);
    QByteArray classes;
    QTextStream classesOut(&classes, QIODevice::WriteOnly);
    QByteArray inlines;
    QTextStream inlinesOut(&inlines, QIODevice::WriteOnly);

    QMap<QByteArray, QByteArrayList> namespaces;

    if(!(category & NoDeclaration)) {
        if (!declFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning("dumpcpp: Could not open output file '%s'", qPrintable(declFile.fileName()));
            return false;
        }

        declOut << "/****************************************************************************\n"
                   "**\n** Namespace " << libName;
        formatCommentBlockFooter(typeLibFile, declOut);

        QFileInfo cppFileInfo(outname);
        writeHeader(declOut, libName, cppFileInfo.fileName());

        UINT typeCount = typelib->GetTypeInfoCount();
        if (declFile.isOpen()) {
            QByteArrayList opaquePointerTypes;
            declOut << Qt::endl;
            declOut << "// Referenced namespace" << Qt::endl;
            for (UINT index = 0; index < typeCount; ++index) {
                ITypeInfo *typeinfo = nullptr;
                typelib->GetTypeInfo(index, &typeinfo);
                if (!typeinfo)
                    continue;

                TYPEATTR *typeattr;
                typeinfo->GetTypeAttr(&typeattr);
                if (!typeattr) {
                    typeinfo->Release();
                    continue;
                }

                TYPEKIND typekind;
                typelib->GetTypeInfoType(index, &typekind);

                QMetaObject *metaObject = nullptr;

                // trigger meta object to collect references to other type libraries
                switch (typekind) {
                case TKIND_COCLASS:
                    metaObject = qax_readClassInfo(typelib, typeinfo, baseMetaObject(category));
                    break;
                case TKIND_DISPATCH:
                    metaObject = qax_readInterfaceInfo(typelib, typeinfo, baseMetaObject(category));
                    break;
                case TKIND_RECORD:
                case TKIND_ENUM:
                case TKIND_INTERFACE: // only for forward declarations
                    {
                        QByteArray className = classNameFromTypeInfo(typeinfo);
                        switch (typekind) {
                        case TKIND_RECORD:
                            className.prepend("struct ");
                            break;
                        case TKIND_ENUM:
                            className.prepend("enum ");
                            break;
                        default:
                            break;
                        }
                        namespaces[libNameBa].append(className);
                        if (!qax_qualified_usertypes.contains(className))
                            qax_qualified_usertypes << className;
                    }
                    break;
                default:
                    break;
                }
                qax_deleteMetaObject(metaObject);
                typeinfo->ReleaseTypeAttr(typeattr);
                typeinfo->Release();
            }

            for (qsizetype i = 0; i < qax_qualified_usertypes.size(); ++i) {
                QByteArray refType = qax_qualified_usertypes.at(i);
                QByteArray refTypeLib;
                if (refType.contains("::")) {
                    refTypeLib = refType;
                    refType.remove(0, refType.lastIndexOf("::") + 2);
                    if (refTypeLib.contains(' ')) {
                        refType = refTypeLib.left(refTypeLib.indexOf(' ')) + ' ' + refType;
                    }
                    refTypeLib.truncate(refTypeLib.indexOf("::"));
                    refTypeLib.remove(0, refTypeLib.lastIndexOf(' ') + 1);
                    namespaces[refTypeLib].append(refType);
                } else {
                    namespaces[libNameBa].append(refType);
                }
            }

            for (auto it = namespaces.cbegin(), end  = namespaces.cend(); it != end; ++it) {
                const QByteArray &nspace = it.key();
                if (libName != QLatin1String(nspace)) {
                    declOut << "namespace " << nspace << " {" << Qt::endl;
                    for (const auto &className : it.value()) {
                        const auto spacePos = className.indexOf(' ');
                        if (spacePos != -1) {
                            const QByteArray name = className.mid(spacePos + 1);
                            if (className.startsWith("enum ")) {
                                declOut << "    " << className << " {\n"
                                    << qax_enum_values.value(nspace + "::" + name) << "    };\n";
                            } else {
                                declOut << "    " << className << ";\n";
                            }
                            namespaceForType.insert(name, nspace);
                        } else {
                            declOut << "    class " << className << ';' << Qt::endl;
                            opaquePointerTypes.append(nspace + "::" + className);
                            namespaceForType.insert(className, nspace);
                            namespaceForType.insert(className + '*', nspace);
                            namespaceForType.insert(className + "**", nspace);
                        }
                    }
                    declOut << '}' << Qt::endl << Qt::endl;
                }
            }
            for (const QByteArray &opaquePointerType : std::as_const(opaquePointerTypes))
                declOut << "Q_DECLARE_OPAQUE_POINTER(" << opaquePointerType << "*)" << Qt::endl;
            declOut << Qt::endl;
        }
        generateNameSpace(declOut, namespaceObject, libNameBa);

        auto nspIt = namespaces.constFind(libNameBa);
        if (nspIt != namespaces.constEnd() && !nspIt.value().isEmpty()) {
            declOut << "// forward declarations" << Qt::endl;
            for (const auto &className : nspIt.value()) {
                if (isQtTypeName(className))
                    continue;
                if (className.contains(' ')) {
                    declOut << "    " << className << ';' << Qt::endl;
                    namespaceForType.insert(className.mid(className.indexOf(' ') + 1), libNameBa);
                } else {
                    declOut << "    class " << className << ';' << Qt::endl;
                    namespaceForType.insert(className, libNameBa);
                    namespaceForType.insert(className + '*', libNameBa);
                    namespaceForType.insert(className + "**", libNameBa);
                }
            }
        }

        declOut << Qt::endl;
    }

    QByteArrayList subtypes;

    UINT typeCount = typelib->GetTypeInfoCount();
    for (UINT index = 0; index < typeCount; ++index) {
        ITypeInfo *typeinfo = nullptr;
        typelib->GetTypeInfo(index, &typeinfo);
        if (!typeinfo)
            continue;

        TYPEATTR *typeattr;
        typeinfo->GetTypeAttr(&typeattr);
        if (!typeattr) {
            typeinfo->Release();
            continue;
        }

        TYPEKIND typekind;
        typelib->GetTypeInfoType(index, &typekind);

        ObjectCategories object_category = category;
        if (!(typeattr->wTypeFlags & TYPEFLAG_FCANCREATE))
            object_category |= SubObject;
        else if (typeattr->wTypeFlags & TYPEFLAG_FCONTROL)
            object_category |= ActiveX;

        QMetaObject *metaObject = nullptr;
        QUuid guid(typeattr->guid);

        if (!(object_category & ActiveX)) {
            QSettings settings(QLatin1String("HKEY_LOCAL_MACHINE\\Software\\Classes\\CLSID\\") + guid.toString(), QSettings::NativeFormat);
            if (settings.childGroups().contains(QLatin1String("Control"))) {
                object_category |= ActiveX;
                object_category &= ~SubObject;
            }
        }

        switch (typekind) {
        case TKIND_COCLASS:
            metaObject = qax_readClassInfo(typelib, typeinfo, baseMetaObject(object_category));
            break;
        case TKIND_DISPATCH:
            metaObject = qax_readInterfaceInfo(typelib, typeinfo, baseMetaObject(object_category));
            break;
        case TKIND_INTERFACE: { // only stub: QTBUG-27792, explicitly disable copy in inherited
                                // class to make related error messages clearer
                const QByteArray className = classNameFromTypeInfo(typeinfo);
                declOut << "// stub for vtable-only interface\n"
                    << "class " << className << " : public QAxObject { Q_DISABLE_COPY_MOVE("
                    << className << ") };\n\n";
            }
            break;
        default:
            break;
        }

        if (metaObject) {
            currentTypeInfo = typeinfo;
            QByteArray className(metaObject->className());
            if (!(typeattr->wTypeFlags & TYPEFLAG_FDUAL)
                && (metaObject->propertyCount() - metaObject->propertyOffset()) == 1
                && className.contains("Events")) {
                declOut << "// skipping event interface " << className << Qt::endl << Qt::endl;
            } else {
                if (declFile.isOpen()) {
                    if (typeattr->wTypeFlags & TYPEFLAG_FLICENSED)
                        object_category |= Licensed;
                    if (typekind == TKIND_COCLASS) { // write those later...
                        generateClassDecl(classesOut, metaObject, className, libNameBa,
                                          object_category | NoInlines);
                        classesOut << Qt::endl;
                    } else {
                        generateClassDecl(declOut, metaObject, className, libNameBa,
                                          object_category | NoInlines);
                        declOut << Qt::endl;
                    }
                    subtypes << className;
                    generateClassDecl(inlinesOut, metaObject, className, libNameBa,
                                      object_category | OnlyInlines);
                    inlinesOut << Qt::endl;
                }
                if (implFile.isOpen()) {
                    QString errorString;
                    if (!generateClassImpl(classImplOut, metaObject, className, guid.toString(), libNameBa,
                                           object_category, useControlName, &errorString)) {
                        qWarning("%s", qPrintable(errorString));
                        return false;
                    }
                }
            }
            currentTypeInfo = nullptr;
        }

        qax_deleteMetaObject(metaObject);

        typeinfo->ReleaseTypeAttr(typeattr);
        typeinfo->Release();
    }

    // String table generation logic was ported from moc generator, with some modifications
    // required to split large stringdata arrays.
    if (implFile.isOpen()) {
        classImplOut.flush();
        implOut << classImpl <<  Qt::endl;
    }

    qax_deleteMetaObject(namespaceObject);

    classesOut.flush();
    inlinesOut.flush();

    if (declFile.isOpen()) {
        if (classes.size()) {
            declOut << "// Actual coclasses" << Qt::endl;
            declOut << classes;
        }
        if (inlines.size()) {
            declOut << "// member function implementation" << Qt::endl;
            declOut << "#ifndef QAX_DUMPCPP_" << libName.toUpper() << "_NOINLINES" << Qt::endl;
            declOut << inlines << Qt::endl;
            declOut << "#endif" << Qt::endl << Qt::endl;
        }
        // close namespace
        declOut << '}' << Qt::endl;
        declOut << Qt::endl;

        declOut << "#endif" << Qt::endl;
        declOut << Qt::endl;
    }

    typelib->Release();
    return true;
}

QT_END_NAMESPACE

QT_USE_NAMESPACE

struct Options
{
    Options() = default;

    ProgramMode mode = GenerateMode;
    ObjectCategories category = DefaultObject;
    bool dispatchEqualsIDispatch = false;
    bool useControlName = false;

    QString outname;
    QString typeLib;
    QString nameSpace;
};

static void parseOptions(Options *options)
{
    const char helpText[] =
        "\nGenerate a C++ namespace from a type library.\n\n"
        "Examples:\n"
        "   dumpcpp -o ieframe %WINDIR%\\system32\\ieframe.dll\n"
        "   dumpcpp -o outlook Outlook.Application\n"
        "   dumpcpp {3B756301-0075-4E40-8BE8-5A81DE2426B7}\n"
        "   dumpcpp -getfile {21D6D480-A88B-11D0-83DD-00AA003CCABD}\n";

    const char outputOptionC[] = "-o";
    const char nameSpaceOptionC[] = "-n";
    const char getfileOptionC[] = "-getfile";
    const char useControlNameOptionC[] = "-controlname";

    QStringList args = QCoreApplication::arguments();
    // Convert Windows-style '/option' into '-option'.
    for (int i = 1; i < args.size(); ) {
        QString &arg = args[i];
        if (arg.startsWith(QLatin1Char('/')))
            arg[0] = QLatin1Char('-');
        const bool takesOptionValue = arg == QLatin1String(outputOptionC)
            || arg == QLatin1String(nameSpaceOptionC)
            || arg == QLatin1String(getfileOptionC);
        i += takesOptionValue ? 2 : 1;
    }

    QCommandLineParser parser;
    QCoreApplication::setApplicationVersion(QLatin1String(QT_VERSION_STR));
    parser.setApplicationDescription(QLatin1String(helpText));


    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption outputOption(QLatin1String(outputOptionC + 1),
                                    QStringLiteral("Write output to file."),
                                    QStringLiteral("file"));
    parser.addOption(outputOption);
    QCommandLineOption nameSpaceOption(QLatin1String(nameSpaceOptionC + 1),
                                       QStringLiteral("The name of the generated C++ namespace."),
                                       QStringLiteral("namespace"));
    parser.addOption(nameSpaceOption);
    QCommandLineOption noMetaObjectOption(QStringLiteral("nometaobject"),
                                          QStringLiteral("Don't generate meta object information (no .cpp file). The meta object is then generated in runtime."));
    parser.addOption(noMetaObjectOption);
    QCommandLineOption noDeclarationOption(QStringLiteral("impl"),
                                           QStringLiteral("Only generate the .cpp file."));
    parser.addOption(noDeclarationOption);
    QCommandLineOption noImplementationOption(QStringLiteral("decl"),
                                              QStringLiteral("Only generate the .h file."));
    parser.addOption(noImplementationOption);
    QCommandLineOption compatOption(QStringLiteral("compat"),
                                    QStringLiteral("Treat all coclass parameters as IDispatch."));
    parser.addOption(compatOption);
    QCommandLineOption getFileOption(QLatin1String(getfileOptionC + 1),
                                     QStringLiteral("Print the filename for the type library it to standard output."),
                                     QStringLiteral("id"));
    parser.addOption(getFileOption);
    parser.addPositionalArgument(QStringLiteral("input"),
                                 QStringLiteral("A type library file, type library ID, ProgID or CLSID."));
    QCommandLineOption useControlNameOption(QLatin1String(useControlNameOptionC + 1),
                                            QStringLiteral("Use the control class name instead of the UUID for setControl()."));
    parser.addOption(useControlNameOption);
    parser.process(args);

    if (parser.isSet(outputOption))
        options->outname = parser.value(outputOption);
    if (parser.isSet(nameSpaceOption))
        options->nameSpace = parser.value(nameSpaceOption);
    if (parser.isSet(noMetaObjectOption))
         options->category |= NoMetaObject;
    if (parser.isSet(noDeclarationOption))
        options->category |= NoDeclaration;
    if (parser.isSet(noImplementationOption))
        options->category |= NoImplementation;
    options->dispatchEqualsIDispatch = parser.isSet(compatOption);
    if (parser.isSet(getFileOption)) {
        options->typeLib = parser.value(getFileOption);
        options->mode = TypeLibID;
    }
    if (parser.isSet(useControlNameOption))
        options->useControlName = true;
    if (!parser.positionalArguments().isEmpty())
        options->typeLib = parser.positionalArguments().first();

    if (options->mode == GenerateMode && options->typeLib.isEmpty()) {
        qWarning("dumpcpp: No object class or type library name provided.\n");
        parser.showHelp(1);
    }
}

int main(int argc, char **argv)
{
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
        qErrnoWarning("CoInitializeEx() failed.");
        return -1;
    }
    QCoreApplication app(argc, argv);

    Options options;
    parseOptions(&options);
    qax_dispatchEqualsIDispatch = options.dispatchEqualsIDispatch;
    QString typeLib = options.typeLib;

    if (options.mode == TypeLibID) {
        QSettings settings(QLatin1String("HKEY_LOCAL_MACHINE\\Software\\Classes\\TypeLib\\") +
                           typeLib, QSettings::NativeFormat);
        typeLib.clear();
        QStringList codes = settings.childGroups();
        for (qsizetype c = 0; c < codes.size(); ++c) {
            const QString keyPrefix = QLatin1Char('/') + codes.at(c) + QLatin1String("/0/");
            if (QT_POINTER_SIZE == 8) {
                typeLib = settings.value(keyPrefix + QLatin1String("win64/.")).toString();
                if (QFile::exists(typeLib))
                    break;
            }
            typeLib = settings.value(keyPrefix + QLatin1String("win32/.")).toString();
            if (QFile::exists(typeLib))
                break;
        }

        if (!typeLib.isEmpty())
            fprintf(stdout, "\"%s\"\n", qPrintable(typeLib));
        return 0;
    }

    if (typeLib.isEmpty()) {
        qWarning("dumpcpp: No object class or type library name provided.\n"
            "         Use -h for help.");
        return -1;
    }

    // not a file - search registry
    if (!QFile::exists(typeLib)) {
        bool isObject = false;
        QSettings settings(QLatin1String("HKEY_LOCAL_MACHINE\\Software\\Classes"), QSettings::NativeFormat);

        // regular string and not a file - must be ProgID
        if (typeLib.at(0) != QLatin1Char('{')) {
            CLSID clsid;
            if (CLSIDFromProgID(reinterpret_cast<const wchar_t *>(typeLib.utf16()), &clsid) != S_OK) {
                qWarning("dumpcpp: '%s' is not a type library and not a registered ProgID",
                         qPrintable(typeLib));
                return -2;
            }
            typeLib = QUuid(clsid).toString();
            isObject = true;
        }

        // check if CLSID
        if (!isObject) {
            QVariant test = settings.value(QLatin1String("/CLSID/") + typeLib + QLatin1String("/."));
            isObject = test.isValid();
        }

        // search typelib ID for CLSID
        if (isObject)
            typeLib = settings.value(QLatin1String("/CLSID/") + typeLib
                                     + QLatin1String("/Typelib/.")).toString();

        // interpret input as type library ID
        QString key = QLatin1String("/TypeLib/") + typeLib;
        settings.beginGroup(key);
        QStringList versions = settings.childGroups();
        QStringList codes;
        if (!versions.isEmpty()) {
            settings.beginGroup(QLatin1Char('/') + versions.last());
            codes = settings.childGroups();
            key += QLatin1Char('/') + versions.last();
            settings.endGroup();
        }
        settings.endGroup();

        for (qsizetype c = 0; c < codes.size(); ++c) {
            const QString keyPrefix = key + QLatin1Char('/') + codes.at(c) + QLatin1Char('/');
            if (QT_POINTER_SIZE == 8) {
                typeLib = settings.value(keyPrefix + QLatin1String("win64/.")).toString();
                if (QFile::exists(typeLib))
                    break;
            }
            typeLib = settings.value(keyPrefix + QLatin1String("win32/.")).toString();
            if (QFile::exists(typeLib))
                break;
        }
    }

    if (!QFile::exists(typeLib)) {
        qWarning("dumpcpp: type library '%s' not found", qPrintable(typeLib));
        return -2;
    }

    if (!generateTypeLibrary(typeLib, options.outname, options.nameSpace, options.category, options.useControlName)) {
        qWarning("dumpcpp: error processing type library '%s'", qPrintable(typeLib));
        return -1;
    }

    return 0;
}
