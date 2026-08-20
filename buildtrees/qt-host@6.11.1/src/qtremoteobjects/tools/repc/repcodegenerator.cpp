// Copyright (C) 2017-2020 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "repcodegenerator.h"

#include <QFileInfo>
#include <QMetaType>
#include <QCryptographicHash>
#include <QRegularExpression>

using namespace Qt;

QT_BEGIN_NAMESPACE

template <typename C>
static int accumulatedSizeOfNames(const C &c)
{
    int result = 0;
    for (const auto &e : c)
        result += e.name.size();
    return result;
}

template <typename C>
static int accumulatedSizeOfTypes(const C &c)
{
    int result = 0;
    for (const auto &e : c)
        result += e.type.size();
    return result;
}

static QString cap(QString name)
{
    if (!name.isEmpty())
        name[0] = name[0].toUpper();
    return name;
}

static bool isClassEnum(const ASTClass &classContext, const QString &typeName)
{
    for (const ASTEnum &astEnum : classContext.enums) {
        if (astEnum.name == typeName) {
            return true;
        }
    }

    return false;
}

static bool hasScopedEnum(const ASTClass &classContext)
{
    for (const ASTEnum &astEnum : classContext.enums) {
        if (astEnum.isScoped) {
            return true;
        }
    }

    return false;
}

static bool hasScopedEnum(const POD &pod)
{
    for (const ASTEnum &astEnum : pod.enums) {
        if (astEnum.isScoped) {
            return true;
        }
    }

    return false;
}

static QString fullyQualifiedName(const ASTClass& classContext, const QString &className,
                                  const QString &typeName)
{
    static const QRegularExpression re = QRegularExpression(QLatin1String("([^<>,\\s]+)"));
    QString copy = typeName;
    qsizetype offset = 0;
    for (const QRegularExpressionMatch &match : re.globalMatch(typeName)) {
        if (isClassEnum(classContext, match.captured(1))) {
            copy.insert(match.capturedStart(1) + offset, className + QStringLiteral("::"));
            offset += className.size() + 2;
        }
    }
    return copy;
}

// for enums we need to transform signal/slot arguments to include the class scope
static QList<ASTFunction> transformEnumParams(const ASTClass& classContext,
                                              const QList<ASTFunction> &methodList,
                                              const QString &typeName) {
    QList<ASTFunction> localList = methodList;
    for (ASTFunction &astFunction : localList) {
        for (ASTDeclaration &astParam : astFunction.params) {
            for (const ASTEnum &astEnum : classContext.enums) {
                if (astEnum.name == astParam.type) {
                    astParam.type = typeName + QStringLiteral("::") + astParam.type;
                }
            }
        }
    }
    return localList;
}

/*
  Returns \c true if the type is a built-in type.
*/
static bool isBuiltinType(const QString &type)
 {
#define CHECK_FOR_BUILT_IN_TYPE(TypeName, Id, Name) if (type == QStringLiteral(#Name)) return true;
    QT_FOR_EACH_STATIC_TYPE(CHECK_FOR_BUILT_IN_TYPE)
#undef CHECK_FOR_BUILT_IN_TYPE
    return false;
}

RepCodeGenerator::RepCodeGenerator(QIODevice *outputDevice, const AST &ast)
    : m_stream(outputDevice), m_ast(ast)
{
}

QByteArray RepCodeGenerator::classSignature(const ASTClass &ac)
{
    return m_ast.typeSignatures[ac.name];
}

void RepCodeGenerator::generate(Mode mode, QString fileName)
{
    if (fileName.isEmpty())
        m_stream << "#pragma once" << Qt::endl << Qt::endl;
    else {
        fileName = QFileInfo(fileName).fileName();
        fileName = fileName.toUpper();
        fileName.replace(QLatin1Char('.'), QLatin1Char('_'));
        m_stream << "#ifndef " << fileName << Qt::endl;
        m_stream << "#define " << fileName << Qt::endl << Qt::endl;
    }

    generateHeader(mode);

    m_stream << Qt::endl;
    for (const QString &line : m_ast.headerLines)
        m_stream << line;
    if (!m_ast.headerLines.isEmpty())
        m_stream << Qt::endl;

    for (const ASTEnum &en : m_ast.enums)
        generateEnumGadget(en, QStringLiteral("%1Enum").arg(en.name));
    for (const POD &pod : m_ast.pods)
        generatePOD(pod);

    QSet<QString> metaTypes;
    QSet<QString> enumTypes;
    for (const POD &pod : m_ast.pods) {
        metaTypes << pod.name;
        // We register from within the code generated for classes, not PODs
        // Thus, for enums/flags in PODs, we need the to prefix with the POD
        // name.  The enumTypes set is used to make sure we don't try to
        // register the non-prefixed name if it is used as a member variable
        // type.
        for (const ASTEnum &en : pod.enums) {
            metaTypes << QLatin1String("%1::%2").arg(pod.name, en.name);
            enumTypes << en.name;
        }
        for (const ASTFlag &flag : pod.flags) {
            metaTypes << QLatin1String("%1::%2").arg(pod.name, flag.name);
            enumTypes << flag.name;
        }
        for (const PODAttribute &attribute : pod.attributes) {
            if (!enumTypes.contains(attribute.type))
                metaTypes << attribute.type;
        }
    }
    const QString metaTypeRegistrationCode = generateMetaTypeRegistration(metaTypes);

    for (const ASTClass &astClass : m_ast.classes) {
        QSet<QString> classMetaTypes;
        QSet<QString> pendingMetaTypes;
        for (const ASTEnum &en : astClass.enums)
            classMetaTypes << en.name;
        for (const ASTProperty &property : astClass.properties) {
            if (property.isPointer)
                continue;
            classMetaTypes << property.type;
        }
        const auto extractClassMetaTypes = [&](const ASTFunction &function) {
            classMetaTypes << function.returnType;
            pendingMetaTypes << function.returnType;
            for (const ASTDeclaration &decl : function.params) {
                classMetaTypes << decl.type;

                // Collect types packaged by Qt containers, to register their metatypes if needed
                QRegularExpression re(
                        QStringLiteral("(QList|QMap|QHash)<\\s*([\\w]+)\\s*(,\\s*([\\w]+))?\\s*>"));
                QRegularExpressionMatch m = re.match(decl.type);
                if (m.hasMatch()) {
                    if (auto captured = m.captured(2);
                        !captured.isNull() && !metaTypes.contains(captured)) {
                        classMetaTypes << captured;
                    }
                    if (auto captured = m.captured(4);
                        !captured.isNull() && !metaTypes.contains(captured)) {
                        classMetaTypes << captured;
                    }
                }
            }
        };
        for (const ASTFunction &function : astClass.signalsList)
            extractClassMetaTypes(function);
        for (const ASTFunction &function : astClass.slotsList)
            extractClassMetaTypes(function);

        const QString classMetaTypeRegistrationCode = metaTypeRegistrationCode
                + generateMetaTypeRegistration(classMetaTypes);
        const QString replicaMetaTypeRegistrationCode = classMetaTypeRegistrationCode
                + generateMetaTypeRegistrationForPending(pendingMetaTypes);

        if (mode == MERGED) {
            generateClass(REPLICA, astClass, replicaMetaTypeRegistrationCode);
            generateClass(SOURCE, astClass, classMetaTypeRegistrationCode);
            generateClass(SIMPLE_SOURCE, astClass, classMetaTypeRegistrationCode);
            generateSourceAPI(astClass);
        } else {
            generateClass(mode, astClass, mode == REPLICA ? replicaMetaTypeRegistrationCode
                                                          : classMetaTypeRegistrationCode);
            if (mode == SOURCE) {
                generateClass(SIMPLE_SOURCE, astClass, classMetaTypeRegistrationCode);
                generateSourceAPI(astClass);
            }
        }
    }

    m_stream << Qt::endl;
    for (const QString &line : m_ast.footerLines)
        m_stream << line;
    if (!m_ast.footerLines.isEmpty())
        m_stream << Qt::endl;

    if (!fileName.isEmpty())
        m_stream << "#endif // " << fileName << Qt::endl;
}

void RepCodeGenerator::generateHeader(Mode mode)
{
    m_stream <<
        "// This is an autogenerated file.\n"
        "// Do not edit this file, any changes made will be lost the next time it is generated.\n"
        "\n"
        "#include <QtCore/qobject.h>\n"
        "#include <QtCore/qdatastream.h>\n"
        "#include <QtCore/qvariant.h>\n"
        "#include <QtCore/qmap.h>\n"
        "#include <QtCore/qmetatype.h>\n";
    bool hasModel = false;
    for (auto c : m_ast.classes)
    {
        if (c.modelMetadata.size() > 0)
        {
            hasModel = true;
            break;
        }
    }
    if (hasModel)
        m_stream << "#include <QtCore/qabstractitemmodel.h>\n";
    m_stream << "\n"
                "#include <QtRemoteObjects/qremoteobjectnode.h>\n";

    if (mode == MERGED) {
        m_stream << "#include <QtRemoteObjects/qremoteobjectpendingcall.h>\n";
        m_stream << "#include <QtRemoteObjects/qremoteobjectreplica.h>\n";
        m_stream << "#include <QtRemoteObjects/qremoteobjectsource.h>\n";
        if (hasModel)
            m_stream << "#include <QtRemoteObjects/qremoteobjectabstractitemmodelreplica.h>\n";
    } else if (mode == REPLICA) {
        m_stream << "#include <QtRemoteObjects/qremoteobjectpendingcall.h>\n";
        m_stream << "#include <QtRemoteObjects/qremoteobjectreplica.h>\n";
        if (hasModel)
            m_stream << "#include <QtRemoteObjects/qremoteobjectabstractitemmodelreplica.h>\n";
    } else
        m_stream << "#include <QtRemoteObjects/qremoteobjectsource.h>\n";
    m_stream << "\n";

    m_stream << m_ast.preprocessorDirectives.join(QLatin1Char('\n'));
    m_stream << "\n";

    m_stream << "using namespace Qt::Literals::StringLiterals;";
    m_stream << "\n";
}

static QString formatTemplateStringArgTypeNameCapitalizedName(int numberOfTypeOccurrences,
                                                              int numberOfNameOccurrences,
                                                              QString templateString,
                                                              const POD &pod)
{
    QString out;
    const int LengthOfPlaceholderText = 2;
    Q_ASSERT(templateString.count(QRegularExpression(QStringLiteral("%\\d")))
                                  == numberOfNameOccurrences + numberOfTypeOccurrences);
    const auto expectedOutSize
            = numberOfNameOccurrences * accumulatedSizeOfNames(pod.attributes)
            + numberOfTypeOccurrences * accumulatedSizeOfTypes(pod.attributes)
            + pod.attributes.size() * (templateString.size()
                                       - (numberOfNameOccurrences + numberOfTypeOccurrences)
                                       * LengthOfPlaceholderText);
    out.reserve(expectedOutSize);
    for (const PODAttribute &a : pod.attributes)
        out += templateString.arg(a.type, a.name, cap(a.name));
    return out;
}

QString RepCodeGenerator::formatQPropertyDeclarations(const POD &pod)
{
    QString prop = QStringLiteral("    Q_PROPERTY(%1 %2 READ %2 WRITE set%3)\n");
    return formatTemplateStringArgTypeNameCapitalizedName(1, 3, prop, pod);
}

QString RepCodeGenerator::formatConstructors(const POD &pod)
{
    QString initializerString = QStringLiteral(": ");
    QString defaultInitializerString = initializerString;
    QString argString;
    for (const PODAttribute &a : pod.attributes) {
        initializerString += QString::fromLatin1("m_%1(%1), ").arg(a.name);
        defaultInitializerString += QString::fromLatin1("m_%1(), ").arg(a.name);
        argString += QString::fromLatin1("%1 %2, ").arg(a.type, a.name);
    }
    argString.chop(2);
    initializerString.chop(2);
    defaultInitializerString.chop(2);

    return QString::fromLatin1("    %1() %2 {}\n"
                   "    explicit %1(%3) %4 {}\n")
            .arg(pod.name, defaultInitializerString, argString, initializerString);
}

QString RepCodeGenerator::formatPropertyGettersAndSetters(const POD &pod)
{
    QString templateString
            = QString::fromLatin1("    %1 %2() const { return m_%2; }\n"
                                  "    void set%3(%1 %2) { if (%2 != m_%2) { m_%2 = %2; } }\n");
    return formatTemplateStringArgTypeNameCapitalizedName(2, 8, qMove(templateString), pod);
}

QString RepCodeGenerator::formatDataMembers(const POD &pod)
{
    QString out;
    const QString prefix = QStringLiteral("    ");
    const QString infix  = QStringLiteral(" m_");
    const QString suffix = QStringLiteral(";\n");
    const auto expectedOutSize
            = accumulatedSizeOfNames(pod.attributes)
            + accumulatedSizeOfTypes(pod.attributes)
            + pod.attributes.size() * (prefix.size() + infix.size() + suffix.size());
    out.reserve(expectedOutSize);
    for (const PODAttribute &a : pod.attributes) {
        out += prefix;
        out += a.type;
        out += infix;
        out += a.name;
        out += suffix;
    }
    Q_ASSERT(out.size() == expectedOutSize);
    return out;
}

QString RepCodeGenerator::formatDebugOperator(const POD &pod)
{
    QString props;
    int count = 0;
    for (const PODAttribute &attribute : pod.attributes) {
        if (count++ > 0)
            props.append(QLatin1String(" << \", \""));
        props.append(QLatin1String(" << \"%1: \" << obj.%1()").arg(attribute.name));
    }

    return QLatin1String("inline QDebug operator<<(QDebug dbg, const %1 &obj) {\n" \
                         "    dbg.nospace() << \"%1(\" %2 << \")\";\n" \
                         "    return dbg.maybeSpace();\n}\n\n").arg(pod.name, props);
}

QString RepCodeGenerator::formatMarshallingOperators(const POD &pod)
{
    return QLatin1String("inline QDataStream &operator<<(QDataStream &ds, const ") + pod.name
           + QLatin1String(" &obj) {\n"
           "    QtRemoteObjects::copyStoredProperties(&obj, ds);\n"
           "    return ds;\n"
           "}\n"
           "\n"
           "inline QDataStream &operator>>(QDataStream &ds, ") + pod.name
           + QLatin1String(" &obj) {\n"
           "    QtRemoteObjects::copyStoredProperties(ds, &obj);\n"
           "    return ds;\n"
           "}\n")
           ;
}

QString RepCodeGenerator::typeForMode(const ASTProperty &property, RepCodeGenerator::Mode mode)
{
    if (!property.isPointer)
        return property.type;

    if (property.type.startsWith(QStringLiteral("QAbstractItemModel")))
        return mode == REPLICA ? property.type + QStringLiteral("Replica*")
                               : property.type + QStringLiteral("*");

    switch (mode) {
    case REPLICA: return property.type + QStringLiteral("Replica*");
    case SIMPLE_SOURCE:
        Q_FALLTHROUGH();
    case SOURCE: return property.type + QStringLiteral("Source*");
    default: qCritical("Invalid mode");
    }

    return QStringLiteral("InvalidPropertyName");
}

void RepCodeGenerator::generateSimpleSetter(const ASTProperty &property, bool generateOverride)
{
    if (!generateOverride)
        m_stream << "    virtual ";
    else
        m_stream << "    ";
    m_stream << "void set" << cap(property.name) << "(" << typeForMode(property, SIMPLE_SOURCE)
             << " " << property.name << ")";
    if (generateOverride)
        m_stream << " override";
    m_stream << Qt::endl;
    m_stream << "    {" << Qt::endl;
    m_stream << "        if (" << property.name << " != m_" << property.name << ") {" << Qt::endl;
    m_stream << "            m_" << property.name << " = " << property.name << ";" << Qt::endl;
    m_stream << "            Q_EMIT " << property.name << "Changed(m_" << property.name << ");"
             << Qt::endl;
    m_stream << "        }" << Qt::endl;
    m_stream << "    }" << Qt::endl;
}

void RepCodeGenerator::generatePOD(const POD &pod)
{
    QStringList equalityCheck;
    const QString compilerAttr = (!pod.compilerAttribute.isEmpty() ?
                                   pod.compilerAttribute + QStringLiteral(" ") : QString());
    for (const PODAttribute &attr : pod.attributes)
        equalityCheck << QStringLiteral("left.%1() == right.%1()").arg(attr.name);
    m_stream << "class " << compilerAttr << pod.name << "\n"
                "{\n"
                "    Q_GADGET\n"
             << "\n"
             <<      formatQPropertyDeclarations(pod);
    if (hasScopedEnum(pod)) // See https://bugreports.qt.io/browse/QTBUG-73360
        m_stream << "    Q_CLASSINFO(\"RegisterEnumClassesUnscoped\", \"false\")\n";
    m_stream << "public:\n";
    generateDeclarationsForEnums(pod.enums);
    for (auto &flag : pod.flags) {
        m_stream << "    Q_DECLARE_FLAGS(" << flag.name << ", " << flag._enum << ")\n";
        m_stream << "    Q_FLAG(" << flag.name << ")\n";
    }
    m_stream <<      formatConstructors(pod)
             <<      formatPropertyGettersAndSetters(pod)
             << "private:\n"
             <<      formatDataMembers(pod)
             << "};\n"
             << "\n"
             << "inline bool operator==(const " << pod.name << " &left, const " << pod.name <<
                " &right) Q_DECL_NOTHROW {\n"
             << "    return " << equalityCheck.join(QStringLiteral(" && ")) << ";\n"
             << "}\n"
             << "inline bool operator!=(const " << pod.name << " &left, const " << pod.name <<
                " &right) Q_DECL_NOTHROW {\n"
             << "    return !(left == right);\n"
             << "}\n"
             << "\n"
             << formatDebugOperator(pod)
             << formatMarshallingOperators(pod);
    for (auto &flag : pod.flags)
        m_stream << "Q_DECLARE_OPERATORS_FOR_FLAGS(" << pod.name << "::" << flag.name << ")\n";
    m_stream << "\n";
}

QString getEnumType(const ASTEnum &en)
{
    if (!en.type.isEmpty())
        return en.type;
    if (en.isSigned) {
        if (en.max < 0x7F)
            return QStringLiteral("qint8");
        if (en.max < 0x7FFF)
            return QStringLiteral("qint16");
        return QStringLiteral("qint32");
    } else {
        if (en.max < 0xFF)
            return QStringLiteral("quint8");
        if (en.max < 0xFFFF)
            return QStringLiteral("quint16");
        return QStringLiteral("quint32");
    }
}

void RepCodeGenerator::generateDeclarationsForEnums(const QList<ASTEnum> &enums,
                                                    bool generateQENUM)
{
    if (!generateQENUM) {
        m_stream << "    // You need to add this enum as well as Q_ENUM to your" << Qt::endl;
        m_stream << "    // QObject class in order to use .rep enums over QtRO for" << Qt::endl;
        m_stream << "    // non-repc generated QObjects." << Qt::endl;
    }

    for (const ASTEnum &en : enums) {
        m_stream << "    enum " << (en.isScoped ? "class " : "") << en.name
                 << (en.type.isEmpty() ? "" : " : ") << en.type << " {\n";
        for (const ASTEnumParam &p : en.params)
            m_stream << "        " << p.name << " = " << p.value << ",\n";

        m_stream << "    };\n";

        if (generateQENUM)
            m_stream << "    Q_ENUM(" << en.name << ")\n";
    }
}

void RepCodeGenerator::generateEnumGadget(const ASTEnum &en, const QString &className)
{
    m_stream << "class " << className << "\n"
                "{\n"
                "    Q_GADGET\n";
    if (en.isScoped)
        m_stream << "    Q_CLASSINFO(\"RegisterEnumClassesUnscoped\", \"false\")\n";
    m_stream << "    " << className << "();\n"
                "\n"
                "public:\n";

    auto enums = QList<ASTEnum>() << en;
    generateDeclarationsForEnums(enums);
    if (en.flagIndex >= 0) {
        auto flag = m_ast.flags.at(en.flagIndex);
        m_stream << "    Q_DECLARE_FLAGS(" << flag.name << ", " << flag._enum << ")\n";
        m_stream << "    Q_FLAG(" << flag.name << ")\n";
        m_stream << "};\n\n";
        m_stream << "Q_DECLARE_OPERATORS_FOR_FLAGS(" << className << "::" << flag.name << ")\n\n";
    } else {
        m_stream << "};\n\n";
    }
}

QString RepCodeGenerator::generateMetaTypeRegistration(const QSet<QString> &metaTypes)
{
    QString out;
    const QString qRegisterMetaType = QStringLiteral("        qRegisterMetaType<");
    const QString lineEnding = QStringLiteral(">();\n");
    for (const QString &metaType : metaTypes) {
        if (isBuiltinType(metaType))
            continue;

        out += qRegisterMetaType;
        out += metaType;
        out += lineEnding;
    }
    return out;
}

QString RepCodeGenerator::generateMetaTypeRegistrationForPending(const QSet<QString> &metaTypes)
{
    QString out;
    if (!metaTypes.isEmpty())
        out += QLatin1String("        qRegisterMetaType<QRemoteObjectPendingCall>();\n");
    const QString qRegisterMetaType =
        QStringLiteral("        qRegisterMetaType<QRemoteObjectPendingReply<%1>>();\n");
    const QString qRegisterConverterConditional =
        QStringLiteral("        if (!QMetaType::hasRegisteredConverterFunction<"
                       "QRemoteObjectPendingReply<%1>, QRemoteObjectPendingCall>())\n");
    const QString qRegisterConverter =
        QStringLiteral("            QMetaType::registerConverter<QRemoteObjectPendingReply<%1>"
                       ", QRemoteObjectPendingCall>();\n");
    for (const QString &metaType : metaTypes) {
        out += qRegisterMetaType.arg(metaType);
        out += qRegisterConverterConditional.arg(metaType);
        out += qRegisterConverter.arg(metaType);
    }
    return out;
}

void RepCodeGenerator::generateClass(Mode mode, const ASTClass &astClass,
                                     const QString &metaTypeRegistrationCode)
{
    const QString className = (astClass.name + (mode == REPLICA ?
                               QStringLiteral("Replica") : mode == SOURCE ?
                               QStringLiteral("Source") : QStringLiteral("SimpleSource")));
    const QString compilerAttr = (!astClass.compilerAttribute.isEmpty() ?
                                   astClass.compilerAttribute + QStringLiteral(" ") : QString());
    if (mode == REPLICA)
        m_stream << "class " << compilerAttr << className << " : public QRemoteObjectReplica" << Qt::endl;
    else if (mode == SIMPLE_SOURCE)
        m_stream << "class " << compilerAttr << className << " : public " << astClass.name << "Source"
                 << Qt::endl;
    else
        m_stream << "class " << compilerAttr << className << " : public QObject" << Qt::endl;

    m_stream << "{\n";
    m_stream << "    Q_OBJECT\n";
    if (hasScopedEnum(astClass)) // See https://bugreports.qt.io/browse/QTBUG-73360
        m_stream << "    Q_CLASSINFO(\"RegisterEnumClassesUnscoped\", \"false\")\n";
    if (mode != SIMPLE_SOURCE) {
        m_stream << "    Q_CLASSINFO(QCLASSINFO_REMOTEOBJECT_TYPE, \"" << astClass.name
                 << "\")" << Qt::endl;
        m_stream << "    Q_CLASSINFO(QCLASSINFO_REMOTEOBJECT_SIGNATURE, \""
                 << QLatin1String(classSignature(astClass)) << "\")" << Qt::endl;
        for (int i = 0; i < astClass.modelMetadata.size(); i++) {
            const auto model = astClass.modelMetadata.at(i);
            const auto modelName = astClass.properties.at(model.propertyIndex).name;
            if (!model.roles.isEmpty()) {
                QStringList list;
                for (auto role : model.roles)
                    list << role.name;
                m_stream << QString::fromLatin1("    Q_CLASSINFO(\"%1_ROLES\", \"%2\")")
                            .arg(modelName.toUpper(), list.join(QChar::fromLatin1('|')))
                         << Qt::endl;
            }
        }


        //First output properties
        for (const ASTProperty &property : astClass.properties) {
            m_stream << "    Q_PROPERTY(" << typeForMode(property, mode) << " " << property.name
                     << " READ " << property.name;
            if (property.modifier == ASTProperty::Constant) {
                if (mode == REPLICA) // We still need to notify when we get the initial value
                    m_stream << " NOTIFY " << property.name << "Changed";
                else
                    m_stream << " CONSTANT";
            } else if (property.modifier == ASTProperty::ReadOnly)
                m_stream << " NOTIFY " << property.name << "Changed";
            else if (property.modifier == ASTProperty::ReadWrite)
                m_stream << " WRITE set" << cap(property.name) << " NOTIFY " << property.name
                         << "Changed";
            else if (property.modifier == ASTProperty::ReadPush ||
                     property.modifier == ASTProperty::SourceOnlySetter) {
                if (mode == REPLICA) // The setter slot isn't known to the PROP
                    m_stream << " NOTIFY " << property.name << "Changed";
                else // The Source can use the setter, since non-asynchronous
                    m_stream << " WRITE set" << cap(property.name) << " NOTIFY "
                             << property.name << "Changed";
            }
            m_stream << ")" << Qt::endl;
        }

        if (!astClass.enums.isEmpty()) {
            m_stream << "" << Qt::endl;
            m_stream << "public:" << Qt::endl;
            generateDeclarationsForEnums(astClass.enums);
            for (const auto &flag : astClass.flags) {
                m_stream << "    Q_DECLARE_FLAGS(" << flag.name << ", " << flag._enum << ")\n";
                m_stream << "    Q_FLAG(" << flag.name << ")\n";
            }
        }
    }

    m_stream << "" << Qt::endl;
    m_stream << "public:" << Qt::endl;

    if (mode == REPLICA) {
        m_stream << "    " << className << "() : QRemoteObjectReplica() { initialize(); }"
                 << Qt::endl;
        m_stream << "    static void registerMetatypes()" << Qt::endl;
        m_stream << "    {" << Qt::endl;
        m_stream << "        static bool initialized = false;" << Qt::endl;
        m_stream << "        if (initialized)" << Qt::endl;
        m_stream << "            return;" << Qt::endl;
        m_stream << "        initialized = true;" << Qt::endl;

        if (!metaTypeRegistrationCode.isEmpty())
            m_stream << metaTypeRegistrationCode << Qt::endl;

        m_stream << "    }" << Qt::endl;

        if (astClass.hasPointerObjects())
        {
            m_stream << "    void setNode(QRemoteObjectNode *node) override" << Qt::endl;
            m_stream << "    {" << Qt::endl;
            m_stream << "        QRemoteObjectReplica::setNode(node);" << Qt::endl;
            for (int index = 0; index < astClass.properties.size(); ++index) {
                const ASTProperty &property = astClass.properties.at(index);
                if (!property.isPointer)
                    continue;
                const QString acquireName = astClass.name + QLatin1String("::") + property.name;
                if (astClass.subClassPropertyIndices.contains(index))
                    m_stream << QString::fromLatin1("        setChild(%1, QVariant::fromValue("
                                "node->acquire<%2Replica>(QRemoteObjectStringLiterals::CLASS()"
                                ".arg(u\"%3\"_s))));")
                                .arg(QString::number(index), property.type, acquireName)
                             << Qt::endl;
                else
                    m_stream << QString::fromLatin1("        setChild(%1, QVariant::fromValue("
                                "node->acquireModel(QRemoteObjectStringLiterals::MODEL()"
                                ".arg(u\"%2\"_s))));")
                                .arg(QString::number(index), acquireName) << Qt::endl;
                m_stream << "        Q_EMIT " << property.name << "Changed(" << property.name
                         << "()" << ");" << Qt::endl;

            }
            m_stream << "    }" << Qt::endl;
        }
        m_stream << "" << Qt::endl;
        m_stream << "private:" << Qt::endl;
        m_stream << "    " << className
                 << "(QRemoteObjectNode *node, const QString &name = QString())" << Qt::endl;
        m_stream << "        : QRemoteObjectReplica(ConstructWithNode)" << Qt::endl;
        m_stream << "    {" << Qt::endl;
        m_stream << "        initializeNode(node, name);" << Qt::endl;
        for (int index = 0; index < astClass.properties.size(); ++index) {
            const ASTProperty &property = astClass.properties.at(index);
            if (!property.isPointer)
                continue;
            const QString acquireName = astClass.name + QLatin1String("::") + property.name;
            if (astClass.subClassPropertyIndices.contains(index))
                m_stream << QString::fromLatin1("        setChild(%1, QVariant::fromValue("
                            "node->acquire<%2Replica>(QRemoteObjectStringLiterals::CLASS()"
                            ".arg(u\"%3\"_s))));")
                            .arg(QString::number(index), property.type, acquireName) << Qt::endl;
            else
                m_stream << QString::fromLatin1("        setChild(%1, QVariant::fromValue("
                            "node->acquireModel(QRemoteObjectStringLiterals::MODEL()"
                            ".arg(u\"%2\"_s))));")
                            .arg(QString::number(index), acquireName) << Qt::endl;
        }
        m_stream << "    }" << Qt::endl;

        m_stream << "" << Qt::endl;

        m_stream << "    void initialize() override" << Qt::endl;
        m_stream << "    {" << Qt::endl;
        m_stream << "        " << className << "::registerMetatypes();" << Qt::endl;
        m_stream << "        QVariantList properties;" << Qt::endl;
        m_stream << "        properties.reserve(" << astClass.properties.size() << ");"
                 << Qt::endl;
        for (const ASTProperty &property : astClass.properties) {
            if (property.isPointer)
                m_stream << "        properties << QVariant::fromValue(("
                         << typeForMode(property, mode) << ")" << property.defaultValue
                         << ");" << Qt::endl;
            else
                m_stream << "        properties << QVariant::fromValue("
                         << typeForMode(property, mode) << "(" << property.defaultValue
                         << "));" << Qt::endl;
        }
        int nPersisted = 0;
        if (astClass.hasPersisted) {
            m_stream << "        QVariantList stored = retrieveProperties(QStringLiteral(\""
                     << astClass.name << "\"), \"" << classSignature(astClass) << "\");"
                     << Qt::endl;
            m_stream << "        if (!stored.isEmpty()) {" << Qt::endl;
            for (int i = 0; i < astClass.properties.size(); i++) {
                if (astClass.properties.at(i).persisted) {
                    m_stream << "            properties[" << i << "] = stored.at(" << nPersisted
                             << ");" << Qt::endl;
                    nPersisted++;
                }
            }
            m_stream << "        }" << Qt::endl;
        }
        m_stream << "        setProperties(std::move(properties));" << Qt::endl;
        m_stream << "    }" << Qt::endl;
    } else if (mode == SOURCE) {
        m_stream << "    explicit " << className
                 << "(QObject *parent = nullptr) : QObject(parent)" << Qt::endl;
        m_stream << "    {" << Qt::endl;
        if (!metaTypeRegistrationCode.isEmpty())
            m_stream << metaTypeRegistrationCode << Qt::endl;
        m_stream << "    }" << Qt::endl;
    } else {
        QList<int> constIndices;
        for (int index = 0; index < astClass.properties.size(); ++index) {
            const ASTProperty &property = astClass.properties.at(index);
            if (property.modifier == ASTProperty::Constant)
                constIndices.append(index);
        }
        if (constIndices.isEmpty()) {
            m_stream << "    explicit " << className << "(QObject *parent = nullptr) : "
                     << astClass.name << "Source(parent)" << Qt::endl;
        } else {
            QStringList parameters;
            for (int index : constIndices) {
                const ASTProperty &property = astClass.properties.at(index);
                parameters.append(QString::fromLatin1("%1 %2 = %3")
                                  .arg(typeForMode(property, SOURCE), property.name,
                                       property.defaultValue));
            }
            parameters.append(QStringLiteral("QObject *parent = nullptr"));
            m_stream << "    explicit " << className << "("
                     << parameters.join(QStringLiteral(", ")) << ") : " << astClass.name
                     << "Source(parent)" << Qt::endl;
        }
        for (const ASTProperty &property : astClass.properties) {
            if (property.modifier == ASTProperty::Constant)
                m_stream << "    , m_" << property.name << "(" << property.name << ")"
                         << Qt::endl;
            else
                m_stream << "    , m_" << property.name << "(" << property.defaultValue << ")"
                         << Qt::endl;
        }
        m_stream << "    {" << Qt::endl;
        m_stream << "    }" << Qt::endl;
    }

    m_stream << "" << Qt::endl;
    m_stream << "public:" << Qt::endl;

    if (mode == REPLICA && astClass.hasPersisted) {
        m_stream << "    ~" << className << "() override {" << Qt::endl;
        m_stream << "        QVariantList persisted;" << Qt::endl;
        for (int i = 0; i < astClass.properties.size(); i++) {
            if (astClass.properties.at(i).persisted) {
                m_stream << "        persisted << propAsVariant(" << i << ");" << Qt::endl;
            }
        }
        m_stream << "        persistProperties(QStringLiteral(\"" << astClass.name << "\"), \""
                 << classSignature(astClass) << "\", persisted);" << Qt::endl;
        m_stream << "    }" << Qt::endl;
    } else {
        m_stream << "    ~" << className << "() override = default;" << Qt::endl;
    }
    m_stream << "" << Qt::endl;

    //Next output getter/setter
    if (mode == REPLICA) {
        int i = 0;
        for (const ASTProperty &property : astClass.properties) {
            auto type = typeForMode(property, mode);
            if (type == QLatin1String("QVariant")) {
                m_stream << "    " << type << " " << property.name << "() const" << Qt::endl;
                m_stream << "    {" << Qt::endl;
                m_stream << "        return propAsVariant(" << i << ");" << Qt::endl;
                m_stream << "    }" << Qt::endl;
            } else {
                m_stream << "    " << type << " " << property.name << "() const" << Qt::endl;
                m_stream << "    {" << Qt::endl;
                m_stream << "        const QVariant variant = propAsVariant(" << i << ");"
                         << Qt::endl;
                m_stream << "        if (!variant.canConvert<" << type << ">()) {" << Qt::endl;
                m_stream << "            qWarning() << \"QtRO cannot convert the property "
                         << property.name << " to type " << type << "\";" << Qt::endl;
                m_stream << "        }" << Qt::endl;
                m_stream << "        return variant.value<" << type << " >();" << Qt::endl;
                m_stream << "    }" << Qt::endl;
            }
            i++;
            if (property.modifier == ASTProperty::ReadWrite) {
                m_stream << "" << Qt::endl;
                m_stream << "    void set" << cap(property.name) << "(" << property.type << " "
                         << property.name << ")" << Qt::endl;
                m_stream << "    {" << Qt::endl;
                m_stream << "        static int __repc_index = " << className
                         << "::staticMetaObject.indexOfProperty(\"" << property.name << "\");"
                         << Qt::endl;
                m_stream << "        QVariantList __repc_args;" << Qt::endl;
                m_stream << "        __repc_args << QVariant::fromValue(" << property.name << ");"
                         << Qt::endl;
                m_stream << "        send(QMetaObject::WriteProperty, __repc_index, __repc_args);"
                         << Qt::endl;
                m_stream << "    }" << Qt::endl;
            }
            m_stream << "" << Qt::endl;
        }
    } else if (mode == SOURCE) {
        for (const ASTProperty &property : astClass.properties)
            m_stream << "    virtual " << typeForMode(property, mode) << " " << property.name
                     << "() const = 0;" << Qt::endl;
        for (const ASTProperty &property : astClass.properties) {
            if (property.modifier == ASTProperty::ReadWrite ||
                    property.modifier == ASTProperty::ReadPush ||
                    property.modifier == ASTProperty::SourceOnlySetter)
                m_stream << "    virtual void set" << cap(property.name) << "("
                         << typeForMode(property, mode) << " " << property.name << ") = 0;"
                         << Qt::endl;
        }
    } else {
        for (const ASTProperty &property : astClass.properties)
            m_stream << "    " << typeForMode(property, mode) << " " << property.name
                     << "() const override { return m_"
                << property.name << "; }" << Qt::endl;
        for (const ASTProperty &property : astClass.properties) {
            if (property.modifier == ASTProperty::ReadWrite ||
                    property.modifier == ASTProperty::ReadPush ||
                    property.modifier == ASTProperty::SourceOnlySetter) {
                generateSimpleSetter(property);
            }
        }
    }

    if (mode != SIMPLE_SOURCE) {
        //Next output property signals
        if (!astClass.properties.isEmpty() || !astClass.signalsList.isEmpty()) {
            m_stream << "" << Qt::endl;
            m_stream << "Q_SIGNALS:" << Qt::endl;
            for (const ASTProperty &property : astClass.properties) {
                if (property.modifier != ASTProperty::Constant)
                    m_stream
                        << "    void " << property.name << "Changed("
                        << fullyQualifiedName(astClass, className, typeForMode(property, mode))
                        << " " << property.name << ");" << Qt::endl;
            }

            const auto signalsList = transformEnumParams(astClass, astClass.signalsList,
                                                         className);
            for (const ASTFunction &signal : signalsList)
                m_stream << "    void " << signal.name << "(" << signal.paramsAsString() << ");"
                         << Qt::endl;

            // CONSTANT source properties still need an onChanged signal on the Replica side to
            // update (once) when the value is initialized.  Put these last, so they don't mess
            // up the signal index order
            for (const ASTProperty &property : astClass.properties) {
                if (mode == REPLICA && property.modifier == ASTProperty::Constant)
                    m_stream
                        << "    void " << property.name << "Changed("
                        << fullyQualifiedName(astClass, className, typeForMode(property, mode))
                        << " " << property.name << ");" << Qt::endl;
            }
        }
        bool hasWriteSlots = false;
        for (const ASTProperty &property : astClass.properties) {
            if (property.modifier == ASTProperty::ReadPush) {
                hasWriteSlots = true;
                break;
            }
        }
        if (hasWriteSlots || !astClass.slotsList.isEmpty()) {
            m_stream << "" << Qt::endl;
            m_stream << "public Q_SLOTS:" << Qt::endl;
            for (const ASTProperty &property : astClass.properties) {
                if (property.modifier == ASTProperty::ReadPush) {
                    const auto type = fullyQualifiedName(astClass, className, property.type);
                    if (mode != REPLICA) {
                        m_stream << "    virtual void push" << cap(property.name) << "(" << type
                                 << " " << property.name << ")" << Qt::endl;
                        m_stream << "    {" << Qt::endl;
                        m_stream << "        set" << cap(property.name) << "(" << property.name
                                 << ");" << Qt::endl;
                        m_stream << "    }" << Qt::endl;
                    } else {
                        m_stream << "    void push" << cap(property.name) << "(" << type << " "
                                 << property.name << ")" << Qt::endl;
                        m_stream << "    {" << Qt::endl;
                        m_stream << "        static int __repc_index = " << className
                                 << "::staticMetaObject.indexOfSlot(\"push" << cap(property.name)
                                 << "(" << type << ")\");" << Qt::endl;
                        m_stream << "        QVariantList __repc_args;" << Qt::endl;
                        m_stream << "        __repc_args << QVariant::fromValue(" << property.name
                                 << ");" << Qt::endl;
                        m_stream << "        send(QMetaObject::InvokeMetaMethod, __repc_index,"
                                 << " __repc_args);" << Qt::endl;
                        m_stream << "    }" << Qt::endl;
                    }
                }
            }
            const auto slotsList = transformEnumParams(astClass, astClass.slotsList, className);
            for (const ASTFunction &slot : slotsList) {
                const auto returnType = fullyQualifiedName(astClass, className, slot.returnType);
                if (mode != REPLICA) {
                    m_stream << "    virtual " << returnType << " " << slot.name << "("
                             << slot.paramsAsString() << ") = 0;" << Qt::endl;
                } else {
                    // TODO: Discuss whether it is a good idea to special-case for void here,
                    const bool isVoid = slot.returnType == QStringLiteral("void");

                    if (isVoid)
                        m_stream << "    void " << slot.name << "(" << slot.paramsAsString()
                                 << ")" << Qt::endl;
                    else
                        m_stream << "    QRemoteObjectPendingReply<" << returnType << "> "
                                 << slot.name << "(" << slot.paramsAsString()<< ")" << Qt::endl;
                    m_stream << "    {" << Qt::endl;
                    m_stream << "        static int __repc_index = " << className
                             << "::staticMetaObject.indexOfSlot(\"" << slot.name << "("
                             << slot.paramsAsString(ASTFunction::Normalized) << ")\");"
                             << Qt::endl;
                    m_stream << "        QVariantList __repc_args;" << Qt::endl;
                    const auto &paramNames = slot.paramNames();
                    if (!paramNames.isEmpty()) {
                        m_stream << "        __repc_args" << Qt::endl;
                        for (const QString &name : paramNames)
                            m_stream << "            << " << "QVariant::fromValue(" << name << ")"
                                     << Qt::endl;
                        m_stream << "        ;" << Qt::endl;
                    }
                    if (isVoid)
                        m_stream << "        send(QMetaObject::InvokeMetaMethod, __repc_index,"
                                 << " __repc_args);" << Qt::endl;
                    else
                        m_stream << "        return QRemoteObjectPendingReply<" << returnType
                                 << ">(sendWithReply(QMetaObject::InvokeMetaMethod, __repc_index,"
                                 << " __repc_args));" << Qt::endl;
                    m_stream << "    }" << Qt::endl;
                }
            }
        }
    } else {
        if (!astClass.properties.isEmpty()) {
            bool addProtected = true;
            for (const ASTProperty &property : astClass.properties) {
                if (property.modifier == ASTProperty::ReadOnly) {
                    if (addProtected) {
                        m_stream << "" << Qt::endl;
                        m_stream << "protected:" << Qt::endl;
                        addProtected = false;
                    }
                    generateSimpleSetter(property, false);
                }
            }
        }
    }

    m_stream << "" << Qt::endl;
    m_stream << "private:" << Qt::endl;

    //Next output data members
    if (mode == SIMPLE_SOURCE) {
        for (const ASTProperty &property : astClass.properties)
            m_stream << "    " << typeForMode(property, SOURCE) << " " << "m_" << property.name
                     << ";" << Qt::endl;
    }

    if (mode != SIMPLE_SOURCE)
        m_stream << "    friend class QT_PREPEND_NAMESPACE(QRemoteObjectNode);" << Qt::endl;

    m_stream << "};\n\n";
    if (mode != SIMPLE_SOURCE) {
        for (const ASTFlag &flag : astClass.flags)
            m_stream << "Q_DECLARE_OPERATORS_FOR_FLAGS(" << className << "::" << flag.name
                     << ")\n\n";
    }
}

void RepCodeGenerator::generateSourceAPI(const ASTClass &astClass)
{
    const QString className = astClass.name + QStringLiteral("SourceAPI");
    m_stream << QStringLiteral("template <class ObjectType>") << Qt::endl;
    m_stream << QString::fromLatin1("struct %1 : public SourceApiMap").arg(className) << Qt::endl;
    m_stream << QStringLiteral("{") << Qt::endl;
    if (!astClass.enums.isEmpty()) {
        // Include enum definition in SourceAPI
        generateDeclarationsForEnums(astClass.enums, false);
    }
    for (const auto &flag : astClass.flags)
        m_stream << QLatin1String("    typedef QFlags<typename ObjectType::%1> %2;")
                    .arg(flag._enum, flag.name) << Qt::endl;
    m_stream << QString::fromLatin1("    %1(ObjectType *object, const QString &name = "
                                    "QLatin1String(\"%2\"))").arg(className, astClass.name)
             << Qt::endl;
    m_stream << QStringLiteral("        : SourceApiMap(), m_name(name)") << Qt::endl;
    m_stream << QStringLiteral("    {") << Qt::endl;
    if (!astClass.hasPointerObjects())
        m_stream << QStringLiteral("        Q_UNUSED(object)") << Qt::endl;

    const auto enumCount = astClass.enums.size();
    const auto totalCount = enumCount + astClass.flags.size();
    for (int i : astClass.subClassPropertyIndices) {
        const ASTProperty &child = astClass.properties.at(i);
        m_stream << QString::fromLatin1("        using %1_type_t = typename std::remove_pointer<"
                                        "decltype(object->%1())>::type;")
                                        .arg(child.name) << Qt::endl;
    }
    m_stream << QString::fromLatin1("        m_enums[0] = %1;").arg(totalCount) << Qt::endl;
    for (qsizetype i = 0; i < enumCount; ++i) {
        const auto enumerator = astClass.enums.at(i);
        m_stream << QString::fromLatin1("        m_enums[%1] = ObjectType::staticMetaObject."
                                        "indexOfEnumerator(\"%2\");")
                                        .arg(i+1).arg(enumerator.name) << Qt::endl;
    }
    for (qsizetype i = enumCount; i < totalCount; ++i) {
        const auto flag = astClass.flags.at(i - enumCount);
        m_stream << QString::fromLatin1("        m_enums[%1] = ObjectType::staticMetaObject."
                                        "indexOfEnumerator(\"%2\");")
                                        .arg(i+1).arg(flag.name) << Qt::endl;
    }
    const auto propCount = astClass.properties.size();
    m_stream << QString::fromLatin1("        m_properties[0] = %1;").arg(propCount) << Qt::endl;
    QList<ASTProperty> onChangeProperties;
    QList<qsizetype> propertyChangeIndex;
    for (qsizetype i = 0; i < propCount; ++i) {
        const ASTProperty &prop = astClass.properties.at(i);
        const QString propTypeName =
            fullyQualifiedName(astClass, QStringLiteral("typename ObjectType"),
                               typeForMode(prop, SOURCE));
        m_stream << QString::fromLatin1("        m_properties[%1] = "
                                        "QtPrivate::qtro_property_index<ObjectType>("
                                        "&ObjectType::%2, static_cast<%3 (QObject::*)()>(nullptr)"
                                        ",\"%2\");")
                                        .arg(QString::number(i+1), prop.name, propTypeName)
                 << Qt::endl;
        if (prop.modifier == prop.ReadWrite) //Make sure we have a setter function
            m_stream << QStringLiteral("        QtPrivate::qtro_method_test<ObjectType>("
                                       "&ObjectType::set%1, static_cast<void (QObject::*)(%2)>"
                                       "(nullptr));")
                                       .arg(cap(prop.name), propTypeName) << Qt::endl;
        if (prop.modifier != prop.Constant) { //Make sure we have an onChange signal
            m_stream << QStringLiteral("        QtPrivate::qtro_method_test<ObjectType>("
                                       "&ObjectType::%1Changed, static_cast<void (QObject::*)()>("
                                       "nullptr));")
                                       .arg(prop.name) << Qt::endl;
            onChangeProperties << prop;
            propertyChangeIndex << i + 1; //m_properties[0] is the count, so index is one higher
        }
    }
    const auto signalCount = astClass.signalsList.size();
    const auto changedCount = onChangeProperties.size();
    m_stream << QString::fromLatin1("        m_signals[0] = %1;")
                                    .arg(signalCount+onChangeProperties.size()) << Qt::endl;
    for (qsizetype i = 0; i < changedCount; ++i)
        m_stream
            << QString::fromLatin1("        m_signals[%1] = QtPrivate::qtro_signal_index"
                                   "<ObjectType>(&ObjectType::%2Changed, static_cast<void"
                                   "(QObject::*)(%3)>(nullptr),m_signalArgCount+%4,"
                                   "&m_signalArgTypes[%4]);")
                                   .arg(QString::number(i+1), onChangeProperties.at(i).name,
                                        fullyQualifiedName(astClass,
                                                           QStringLiteral("typename ObjectType"),
                                                           typeForMode(onChangeProperties.at(i),
                                                                       SOURCE)),
                                        QString::number(i))
            << Qt::endl;

    QList<ASTFunction> signalsList = transformEnumParams(astClass, astClass.signalsList,
                                                         QStringLiteral("typename ObjectType"));
    for (qsizetype i = 0; i < signalCount; ++i) {
        const ASTFunction &sig = signalsList.at(i);
        m_stream << QString::fromLatin1("        m_signals[%1] = QtPrivate::qtro_signal_index"
                                        "<ObjectType>(&ObjectType::%2, static_cast<void "
                                        "(QObject::*)(%3)>(nullptr),m_signalArgCount+%4,"
                                        "&m_signalArgTypes[%4]);")
                                        .arg(QString::number(changedCount+i+1), sig.name,
                                             sig.paramsAsString(ASTFunction::Normalized),
                                             QString::number(changedCount+i))
                 << Qt::endl;
    }
    const auto slotCount = astClass.slotsList.size();
    QList<ASTProperty> pushProps;
    for (const ASTProperty &property : astClass.properties) {
        if (property.modifier == ASTProperty::ReadPush)
            pushProps << property;
    }
    const auto pushCount = pushProps.size();
    const auto methodCount = slotCount + pushCount;
    m_stream << QString::fromLatin1("        m_methods[0] = %1;").arg(methodCount) << Qt::endl;
    const QString objType = QStringLiteral("typename ObjectType::");
    for (qsizetype i = 0; i < pushCount; ++i) {
        const ASTProperty &prop = pushProps.at(i);
        const QString propTypeName = fullyQualifiedName(astClass,
                                                        QStringLiteral("typename ObjectType"),
                                                        prop.type);
        m_stream <<
            QString::fromLatin1("        m_methods[%1] = QtPrivate::qtro_method_index"
                                "<ObjectType>(&ObjectType::push%2, static_cast<void "
                                "(QObject::*)(%3)>(nullptr),\"push%2(%4)\","
                                "m_methodArgCount+%5,&m_methodArgTypes[%5]);")
                                .arg(QString::number(i+1), cap(prop.name), propTypeName,
                                     // we don't want "typename ObjectType::" in the signature
                                     QString(propTypeName).remove(objType),
                                     QString::number(i))
                 << Qt::endl;
    }

    QList<ASTFunction> slotsList = transformEnumParams(astClass, astClass.slotsList,
                                                       QStringLiteral("typename ObjectType"));
    for (qsizetype i = 0; i < slotCount; ++i) {
        const ASTFunction &slot = slotsList.at(i);
        const QString params = slot.paramsAsString(ASTFunction::Normalized);
        m_stream << QString::fromLatin1("        m_methods[%1] = QtPrivate::qtro_method_index"
                                        "<ObjectType>(&ObjectType::%2, static_cast<void "
                                        "(QObject::*)(%3)>(nullptr),\"%2(%4)\","
                                        "m_methodArgCount+%5,&m_methodArgTypes[%5]);")
                                        .arg(QString::number(i+pushCount+1), slot.name, params,
                                        // we don't want "typename ObjectType::" in the signature
                                        QString(params).remove(objType),
                                        QString::number(i+pushCount))
                 << Qt::endl;
    }
    for (const auto &model : astClass.modelMetadata) {
        const ASTProperty &property = astClass.properties.at(model.propertyIndex);
        m_stream << QString::fromLatin1("        m_models << ModelInfo({object->%1(),")
                                        .arg(property.name) << Qt::endl;
        m_stream << QString::fromLatin1("                               QStringLiteral(\"%1\"),")
                                        .arg(property.name) << Qt::endl;
        QStringList list;
        if (!model.roles.isEmpty()) {
            for (auto role : model.roles)
                list << role.name;
        }
        m_stream <<
            QString::fromLatin1("                               QByteArrayLiteral(\"%1\")});")
                                .arg(list.join(QChar::fromLatin1('|'))) << Qt::endl;
    }
    for (int i : astClass.subClassPropertyIndices) {
        const ASTProperty &child = astClass.properties.at(i);
        m_stream <<
            QString::fromLatin1("        m_subclasses << new %2SourceAPI<%1_type_t>(object->%1(),"
                                " QStringLiteral(\"%1\"));")
                                .arg(child.name, child.type) << Qt::endl;
    }
    m_stream << QStringLiteral("    }") << Qt::endl;
    m_stream << QStringLiteral("") << Qt::endl;
    m_stream << QString::fromLatin1("    QString name() const override { return m_name; }")
             << Qt::endl;
    m_stream << QString::fromLatin1("    QString typeName() const override { "
                                    "return QStringLiteral(\"%1\"); }")
                                    .arg(astClass.name) << Qt::endl;
    m_stream << QStringLiteral("    int enumCount() const override { return m_enums[0]; }")
             << Qt::endl;
    m_stream <<
        QStringLiteral("    int propertyCount() const override { return m_properties[0]; }")
        << Qt::endl;
    m_stream << QStringLiteral("    int signalCount() const override { return m_signals[0]; }")
             << Qt::endl;
    m_stream << QStringLiteral("    int methodCount() const override { return m_methods[0]; }")
             << Qt::endl;
    m_stream << QStringLiteral("    int sourceEnumIndex(int index) const override") << Qt::endl;
    m_stream << QStringLiteral("    {") << Qt::endl;
    m_stream << QStringLiteral("        if (index < 0 || index >= m_enums[0]"
                               " || index + 1 >= int(std::size(m_enums)))")
             << Qt::endl;
    m_stream << QStringLiteral("            return -1;") << Qt::endl;
    m_stream << QStringLiteral("        return m_enums[index+1];") << Qt::endl;
    m_stream << QStringLiteral("    }") << Qt::endl;
    m_stream << QStringLiteral("    int sourcePropertyIndex(int index) const override")
             << Qt::endl;
    m_stream << QStringLiteral("    {") << Qt::endl;
    m_stream << QStringLiteral("        if (index < 0 || index >= m_properties[0]"
                               " || index + 1 >= int(std::size(m_properties)))")
             << Qt::endl;
    m_stream << QStringLiteral("            return -1;") << Qt::endl;
    m_stream << QStringLiteral("        return m_properties[index+1];") << Qt::endl;
    m_stream << QStringLiteral("    }") << Qt::endl;
    m_stream << QStringLiteral("    int sourceSignalIndex(int index) const override") << Qt::endl;
    m_stream << QStringLiteral("    {") << Qt::endl;
    m_stream << QStringLiteral("        if (index < 0 || index >= m_signals[0]"
                               " || index + 1 >= int(std::size(m_signals)))")
             << Qt::endl;
    m_stream << QStringLiteral("            return -1;") << Qt::endl;
    m_stream << QStringLiteral("        return m_signals[index+1];") << Qt::endl;
    m_stream << QStringLiteral("    }") << Qt::endl;
    m_stream << QStringLiteral("    int sourceMethodIndex(int index) const override") << Qt::endl;
    m_stream << QStringLiteral("    {") << Qt::endl;
    m_stream << QStringLiteral("        if (index < 0 || index >= m_methods[0]"
                               " || index + 1 >= int(std::size(m_methods)))")
             << Qt::endl;
    m_stream << QStringLiteral("            return -1;") << Qt::endl;
    m_stream << QStringLiteral("        return m_methods[index+1];") << Qt::endl;
    m_stream << QStringLiteral("    }") << Qt::endl;
    if (signalCount+changedCount > 0) {
        m_stream << QStringLiteral("    int signalParameterCount(int index) const override")
                 << Qt::endl;
        m_stream << QStringLiteral("    {") << Qt::endl;
        m_stream << QStringLiteral("        if (index < 0 || index >= m_signals[0])") << Qt::endl;
        m_stream << QStringLiteral("            return -1;") << Qt::endl;
        m_stream << QStringLiteral("        return m_signalArgCount[index];") << Qt::endl;
        m_stream << QStringLiteral("    }") << Qt::endl;
        m_stream << QStringLiteral("    int signalParameterType(int sigIndex, int paramIndex) "
                                   "const override") << Qt::endl;
        m_stream << QStringLiteral("    {") << Qt::endl;
        m_stream << QStringLiteral("        if (sigIndex < 0 || sigIndex >= m_signals[0] || "
                                   "paramIndex < 0 || paramIndex >= m_signalArgCount[sigIndex])")
                 << Qt::endl;
        m_stream << QStringLiteral("            return -1;") << Qt::endl;
        m_stream << QStringLiteral("        return m_signalArgTypes[sigIndex][paramIndex];")
                 << Qt::endl;
        m_stream << QStringLiteral("    }") << Qt::endl;
    } else {
        m_stream << QStringLiteral("    int signalParameterCount(int index) const override "
                                   "{ Q_UNUSED(index) return -1; }") << Qt::endl;
        m_stream << QStringLiteral("    int signalParameterType(int sigIndex, int paramIndex) "
                                   "const override") << Qt::endl;
        m_stream << QStringLiteral("    { Q_UNUSED(sigIndex) Q_UNUSED(paramIndex) return -1; }")
                 << Qt::endl;
    }
    if (methodCount > 0) {
        m_stream << QStringLiteral("    int methodParameterCount(int index) const override")
                 << Qt::endl;
        m_stream << QStringLiteral("    {") << Qt::endl;
        m_stream << QStringLiteral("        if (index < 0 || index >= m_methods[0])") << Qt::endl;
        m_stream << QStringLiteral("            return -1;") << Qt::endl;
        m_stream << QStringLiteral("        return m_methodArgCount[index];") << Qt::endl;
        m_stream << QStringLiteral("    }") << Qt::endl;
        m_stream << QStringLiteral("    int methodParameterType(int methodIndex, int paramIndex) "
                                   "const override") << Qt::endl;
        m_stream << QStringLiteral("    {") << Qt::endl;
        m_stream <<
            QStringLiteral("        if (methodIndex < 0 || methodIndex >= m_methods[0] || "
                           "paramIndex < 0 || paramIndex >= m_methodArgCount[methodIndex])")
            << Qt::endl;
        m_stream << QStringLiteral("            return -1;") << Qt::endl;
        m_stream << QStringLiteral("        return m_methodArgTypes[methodIndex][paramIndex];")
                 << Qt::endl;
        m_stream << QStringLiteral("    }") << Qt::endl;
    } else {
        m_stream << QStringLiteral("    int methodParameterCount(int index) const override { "
                                   "Q_UNUSED(index) return -1; }") << Qt::endl;
        m_stream << QStringLiteral("    int methodParameterType(int methodIndex, int paramIndex) "
                                   "const override") << Qt::endl;
        m_stream <<
            QStringLiteral("    { Q_UNUSED(methodIndex) Q_UNUSED(paramIndex) return -1; }")
            << Qt::endl;
    }
    //propertyIndexFromSignal method
    m_stream << QStringLiteral("    int propertyIndexFromSignal(int index) const override")
             << Qt::endl;
    m_stream << QStringLiteral("    {") << Qt::endl;
    if (!propertyChangeIndex.isEmpty()) {
        m_stream << QStringLiteral("        switch (index) {") << Qt::endl;
        for (int i = 0; i < propertyChangeIndex.size(); ++i)
            m_stream << QString::fromLatin1("        case %1: return m_properties[%2];")
                        .arg(i).arg(propertyChangeIndex.at(i)) << Qt::endl;
        m_stream << QStringLiteral("        }") << Qt::endl;
    } else
        m_stream << QStringLiteral("        Q_UNUSED(index)") << Qt::endl;
    m_stream << QStringLiteral("        return -1;") << Qt::endl;
    m_stream << QStringLiteral("    }") << Qt::endl;
    //propertyRawIndexFromSignal method
    m_stream << QStringLiteral("    int propertyRawIndexFromSignal(int index) const override")
             << Qt::endl;
    m_stream << QStringLiteral("    {") << Qt::endl;
    if (!propertyChangeIndex.isEmpty()) {
        m_stream << QStringLiteral("        switch (index) {") << Qt::endl;
        for (int i = 0; i < propertyChangeIndex.size(); ++i)
            m_stream << QString::fromLatin1("        case %1: return %2;").arg(i)
                        .arg(propertyChangeIndex.at(i)-1) << Qt::endl;
        m_stream << QStringLiteral("        }") << Qt::endl;
    } else
        m_stream << QStringLiteral("        Q_UNUSED(index)") << Qt::endl;
    m_stream << QStringLiteral("        return -1;") << Qt::endl;
    m_stream << QStringLiteral("    }") << Qt::endl;

    //signalSignature method
    m_stream << QStringLiteral("    const QByteArray signalSignature(int index) const override")
             << Qt::endl;
    m_stream << QStringLiteral("    {") << Qt::endl;
    if (signalCount+changedCount > 0) {
        m_stream << QStringLiteral("        switch (index) {") << Qt::endl;
        for (int i = 0; i < changedCount; ++i) {
            const ASTProperty &prop = onChangeProperties.at(i);
            if (isClassEnum(astClass, prop.type))
                m_stream <<
                    QString::fromLatin1("        case %1: return QByteArrayLiteral(\"%2"
                                        "Changed($1)\").replace(\"$1\", "
                                        "QtPrivate::qtro_enum_signature<ObjectType>(\"%3\"));")
                                        .arg(QString::number(i), prop.name, prop.type)
                    << Qt::endl;
            else
                m_stream <<
                    QString::fromLatin1("        case %1: return QByteArrayLiteral(\"%2"
                                        "Changed(%3)\");")
                                        .arg(QString::number(i), prop.name,
                                             typeForMode(prop, SOURCE))
                    << Qt::endl;
        }
        for (int i = 0; i < signalCount; ++i)
        {
            const ASTFunction &sig = astClass.signalsList.at(i);
            auto paramsAsString = sig.paramsAsString(ASTFunction::Normalized);
            const auto paramsAsList = paramsAsString.split(QLatin1String(","));
            int enumCount = 0;
            QString enumString;
            for (int j = 0; j < paramsAsList.size(); j++) {
                auto const p = paramsAsList.at(j);
                if (isClassEnum(astClass, p)) {
                    paramsAsString.replace(paramsAsString.indexOf(p), p.size(),
                                           QStringLiteral("$%1").arg(enumCount));
                    enumString.append(QString::fromLatin1(".replace(\"$%1\", QtPrivate::"
                                                          "qtro_enum_signature<ObjectType>"
                                                          "(\"%2\"))")
                                                          .arg(enumCount++)
                                                          .arg(paramsAsList.at(j)));
                }
            }
            m_stream <<
                QString::fromLatin1("        case %1: return QByteArrayLiteral(\"%2(%3)\")%4;")
                                    .arg(QString::number(i+changedCount), sig.name,
                                         paramsAsString, enumString) << Qt::endl;
        }
        m_stream << QStringLiteral("        }") << Qt::endl;
    } else
        m_stream << QStringLiteral("        Q_UNUSED(index)") << Qt::endl;
    m_stream << QStringLiteral("        return QByteArrayLiteral(\"\");") << Qt::endl;
    m_stream << QStringLiteral("    }") << Qt::endl;

    //signalParameterNames method
    m_stream <<
        QStringLiteral("    QByteArrayList signalParameterNames(int index) const override")
        << Qt::endl;
    m_stream << QStringLiteral("    {") << Qt::endl;
    m_stream << QStringLiteral("        if (index < 0 || index >= m_signals[0]"
                               " || index + 1 >= int(std::size(m_signals)))")
             << Qt::endl;
    m_stream << QStringLiteral("            return QByteArrayList();") << Qt::endl;
    m_stream << QStringLiteral("        return ObjectType::staticMetaObject.method(m_signals["
                               "index + 1]).parameterNames();") << Qt::endl;
    m_stream << QStringLiteral("    }") << Qt::endl;

    //methodSignature method
    m_stream << QStringLiteral("    const QByteArray methodSignature(int index) const override")
             << Qt::endl;
    m_stream << QStringLiteral("    {") << Qt::endl;
    if (methodCount > 0) {
        m_stream << QStringLiteral("        switch (index) {") << Qt::endl;
        for (int i = 0; i < pushCount; ++i)
        {
            const ASTProperty &prop = pushProps.at(i);
            if (isClassEnum(astClass, prop.type))
                m_stream << QString::fromLatin1("        case %1: return QByteArrayLiteral(\"push"
                                                "%2($1)\").replace(\"$1\", QtPrivate::"
                                                "qtro_enum_signature<ObjectType>(\"%3\"));")
                                                .arg(QString::number(i), prop.name, prop.type)
                         << Qt::endl;
            else
                m_stream <<
                    QString::fromLatin1("        case %1: return QByteArrayLiteral(\"push"
                                        "%2(%3)\");")
                                        .arg(QString::number(i), cap(prop.name), prop.type)
                    << Qt::endl;
        }
        for (int i = 0; i < slotCount; ++i)
        {
            const ASTFunction &slot = astClass.slotsList.at(i);
            auto paramsAsString = slot.paramsAsString(ASTFunction::Normalized);
            const auto paramsAsList = paramsAsString.split(QLatin1String(","));
            int enumCount = 0;
            QString enumString;
            for (int j = 0; j < paramsAsList.size(); j++) {
                auto const p = paramsAsList.at(j);
                if (isClassEnum(astClass, p)) {
                    paramsAsString.replace(paramsAsString.indexOf(p), p.size(),
                                           QStringLiteral("$%1").arg(enumCount));
                    enumString.append(QString::fromLatin1(".replace(\"$%1\", QtPrivate::"
                                                          "qtro_enum_signature<ObjectType>"
                                                          "(\"%2\"))")
                                                          .arg(enumCount++)
                                                          .arg(paramsAsList.at(j)));
                }
            }
            m_stream << QString::fromLatin1("        case %1: return QByteArrayLiteral(\"%2(%3)"
                                            "\")%4;")
                                            .arg(QString::number(i+pushCount), slot.name,
                                                 paramsAsString, enumString) << Qt::endl;
        }
        m_stream << QStringLiteral("        }") << Qt::endl;
    } else
        m_stream << QStringLiteral("        Q_UNUSED(index)") << Qt::endl;
    m_stream << QStringLiteral("        return QByteArrayLiteral(\"\");") << Qt::endl;
    m_stream << QStringLiteral("    }") << Qt::endl;

    //methodType method
    m_stream << QStringLiteral("    QMetaMethod::MethodType methodType(int) const override")
             << Qt::endl;
    m_stream << QStringLiteral("    {") << Qt::endl;
    m_stream << QStringLiteral("        return QMetaMethod::Slot;") << Qt::endl;
    m_stream << QStringLiteral("    }") << Qt::endl;

    //methodParameterNames method
    m_stream <<
        QStringLiteral("    QByteArrayList methodParameterNames(int index) const override")
        << Qt::endl;
    m_stream << QStringLiteral("    {") << Qt::endl;
    m_stream << QStringLiteral("        if (index < 0 || index >= m_methods[0]"
                               " || index + 1 >= int(std::size(m_methods)))")
             << Qt::endl;
    m_stream << QStringLiteral("            return QByteArrayList();") << Qt::endl;
    m_stream << QStringLiteral("        return ObjectType::staticMetaObject.method(m_methods["
                               "index + 1]).parameterNames();") << Qt::endl;
    m_stream << QStringLiteral("    }") << Qt::endl;

    //typeName method
    m_stream << QStringLiteral("    const QByteArray typeName(int index) const override")
             << Qt::endl;
    m_stream << QStringLiteral("    {") << Qt::endl;
    if (methodCount > 0) {
        m_stream << QStringLiteral("        switch (index) {") << Qt::endl;
        for (int i = 0; i < pushCount; ++i)
        {
            m_stream <<
                QString::fromLatin1("        case %1: return QByteArrayLiteral(\"void\");")
                .arg(QString::number(i)) << Qt::endl;
        }
        for (int i = 0; i < slotCount; ++i)
        {
            const ASTFunction &slot = astClass.slotsList.at(i);
            if (isClassEnum(astClass, slot.returnType))
                m_stream <<
                    QString::fromLatin1("        case %1: return QByteArrayLiteral(\"$1\")"
                                        ".replace(\"$1\", QtPrivate::qtro_enum_signature"
                                        "<ObjectType>(\"%2\"));")
                                        .arg(QString::number(i+pushCount), slot.returnType)
                    << Qt::endl;
            else
                m_stream <<
                    QString::fromLatin1("        case %1: return QByteArrayLiteral(\"%2\");")
                    .arg(QString::number(i+pushCount), slot.returnType) << Qt::endl;
        }
        m_stream << QStringLiteral("        }") << Qt::endl;
    } else
        m_stream << QStringLiteral("        Q_UNUSED(index)") << Qt::endl;
    m_stream << QStringLiteral("        return QByteArrayLiteral(\"\");") << Qt::endl;
    m_stream << QStringLiteral("    }") << Qt::endl;

    //objectSignature method
    m_stream <<
        QStringLiteral("    QByteArray objectSignature() const override { return QByteArray{\"")
        << QLatin1String(classSignature(astClass))
        << QStringLiteral("\"}; }") << Qt::endl;

    m_stream << QStringLiteral("") << Qt::endl;
    m_stream << QString::fromLatin1("    int m_enums[%1];").arg(totalCount + 1) << Qt::endl;
    m_stream << QString::fromLatin1("    int m_properties[%1];").arg(propCount+1) << Qt::endl;
    m_stream << QString::fromLatin1("    int m_signals[%1];").arg(signalCount+changedCount+1)
             << Qt::endl;
    m_stream << QString::fromLatin1("    int m_methods[%1];").arg(methodCount+1) << Qt::endl;
    m_stream << QString::fromLatin1("    const QString m_name;") << Qt::endl;
    if (signalCount+changedCount > 0) {
        m_stream << QString::fromLatin1("    int m_signalArgCount[%1];")
                                        .arg(signalCount+changedCount) << Qt::endl;
        m_stream << QString::fromLatin1("    const int* m_signalArgTypes[%1];")
                                        .arg(signalCount+changedCount) << Qt::endl;
    }
    if (methodCount > 0) {
        m_stream << QString::fromLatin1("    int m_methodArgCount[%1];").arg(methodCount)
                 << Qt::endl;
        m_stream << QString::fromLatin1("    const int* m_methodArgTypes[%1];").arg(methodCount)
                 << Qt::endl;
    }
    m_stream << QStringLiteral("};") << Qt::endl;
    m_stream << "" << Qt::endl;
}

QT_END_NAMESPACE
