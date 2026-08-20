-- Copyright (C) 2014-2020 Ford Motor Company.
-- SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
-- Qt-Security score:significant reason:parsing-build-time-only

%parser rep_grammar
%decl repparser.h
%impl repparser.cpp

%token_prefix Token_
%token semicolon "[semicolon];"
%token class "[class]class[ \\t]+(?:(?<attribute>[A-Za-z_][A-Za-z0-9_]*)[ \\t]+)?(?<name>[A-Za-z_][A-Za-z0-9_]+)[ \\t]*"
%token pod "[pod]POD[ \\t]*(?:(?<attribute>[A-Za-z_][A-Za-z0-9_]*)[ \\t]+)?(?<name>[A-Za-z_][A-Za-z0-9_]+)[ \\t]*\\((?<types>[^\\)]*)\\);?[ \\t]*"
%token pod2 "[pod2]POD[ \\t]*(?:(?<attribute>[A-Za-z_][A-Za-z0-9_]*)[ \\t]+)?(?<name>[A-Za-z_][A-Za-z0-9_]+)[ \\t]*"
%token flag "[flag][ \\t]*FLAG[ \t]*\\([ \t]*(?<name>[A-Za-z_][A-Za-z0-9_]*)[ \t]+(?<enum>[A-Za-z_][A-Za-z0-9_]*)[ \t]*\\)[ \t]*"
%token enum "[enum][ \\t]*ENUM[ \t]+(?:(?<class>class[ \t]+))?(?<name>[A-Za-z_][A-Za-z0-9_]*)[ \t]*(?::[ \t]*(?<type>[a-zA-Z0-9 _:]*[a-zA-Z0-9_])[ \t]*)?"
%token prop "[prop][ \\t]*PROP[ \\t]*\\((?<args>[^\\)]+)\\);?[ \\t]*"
%token use_enum "[use_enum]USE_ENUM[ \\t]*\\((?<name>[^\\)]*)\\);?[ \\t]*"
%token signal "[signal][ \\t]*SIGNAL[ \\t]*\\([ \\t]*(?<name>\\S+)[ \\t]*\\((?<args>[^\\)]*)\\)[ \\t]*\\);?[ \\t]*"
%token slot "[slot][ \\t]*SLOT[ \\t]*\\((?<type>[^\\(]*)\\((?<args>[^\\)]*)\\)[ \\t]*\\);?[ \\t]*"
%token model "[model][ \\t]*MODEL[ \\t]+(?<name>[A-Za-z_][A-Za-z0-9_]+)\\((?<args>[^\\)]+)\\)[ \\t]*;?[ \\t]*"
%token childrep "[childrep][ \\t]*CLASS[ \\t]+(?<name>[A-Za-z_][A-Za-z0-9_]+)\\((?<type>[^\\)]+)\\)[ \\t]*;?[ \\t]*"
%token qualifier "[qualifier][ \\t]*(?<value>const|unsigned|signed)[ \\t]*"
%token passbyqual "[passbyqual][ \\t]*(?<value>&)[ \\t]*"
%token symbol "[symbol][ \\t]*(?<symbol>[A-Za-z_][A-Za-z0-9_]*)[ \\t]*"
%token value "[value][ \\t]*(?<value>-\\d+|0[xX][0-9A-Fa-f]+|\\d+)[ \\t]*"
%token start "[start][ \\t]*\\{[ \\t]*"
%token stop "[stop][ \\t]*\\};?[ \\t]*"
%token comma "[comma],"
%token equals "[equals]="
%token comment "[comment](?<comment>[ \\t]*//[^\\n]*\\n)"
%token mcomment "[mcomment,M](?<comment>/\\*(.*?)\\*/)"
%token preprocessor_directive "[preprocessor_directive](?<preprocessor_directive>#[ \\t]*[^\\n]*\\n)"
%token newline "[newline](\\r)?\\n"
%token tstart "[tstart]<"
%token tstop "[tstop]>[ \\t]*"
-- Define associativity for newline to resolve ambiguity when there are multiple
%right newline

%start TopLevel

/:
#ifndef REPPARSER_H
#define REPPARSER_H

#include <rep_grammar_p.h>
#include <qregexparser.h>
#include <QStringList>
#include <QList>
#include <QHash>
#include <QRegularExpression>
#include <QSet>

QT_BEGIN_NAMESPACE
class QIODevice;
class QCryptographicHash;

struct AST;

struct SignedType
{
    SignedType(const QString &name = QString());
    virtual ~SignedType() {}
    void generateSignature(AST &ast);
    virtual QString typeName() const;
    virtual void signature_impl(const AST &ast, QCryptographicHash &checksum) = 0;
    QString name;
    QString compilerAttribute;
};

/// A property of a Class declaration
struct ASTProperty
{
    enum Modifier
    {
        Constant,
        ReadOnly,
        ReadPush,
        ReadWrite,
        SourceOnlySetter
    };

    ASTProperty();
    ASTProperty(const QString &type, const QString &name, const QString &defaultValue, Modifier modifier, bool persisted,
                bool isPointer=false);

    QString type;
    QString name;
    QString defaultValue;
    Modifier modifier;
    bool persisted;
    bool isPointer;
};
Q_DECLARE_TYPEINFO(ASTProperty, Q_RELOCATABLE_TYPE);

struct ASTDeclaration
{
    enum VariableType {
        None = 0,
        Constant = 1,
        Reference = 2,
    };
    Q_DECLARE_FLAGS(VariableTypes, VariableType)

    ASTDeclaration(const QString &declarationType = QString(), const QString &declarationName = QString(), VariableTypes declarationVariableType = None)
        : type(declarationType),
          name(declarationName),
          variableType(declarationVariableType)
    {
    }

    QString asString(bool withName) const;

    QString type;
    QString name;
    VariableTypes variableType;
};
Q_DECLARE_TYPEINFO(ASTDeclaration, Q_RELOCATABLE_TYPE);

struct ASTFunction
{
    enum ParamsAsStringFormat {
        Default,
        Normalized
    };

    explicit ASTFunction(const QString &name = QString(), const QString &returnType = QLatin1String("void"));

    QString paramsAsString(ParamsAsStringFormat format = Default) const;
    QStringList paramNames() const;

    QString returnType;
    QString name;
    QList<ASTDeclaration> params;
};
Q_DECLARE_TYPEINFO(ASTFunction, Q_RELOCATABLE_TYPE);

struct ASTEnumParam
{
    ASTEnumParam(const QString &paramName = QString(), int paramValue = 0)
        : name(paramName),
          value(paramValue)
    {
    }

    QString name;
    int value;
};
Q_DECLARE_TYPEINFO(ASTEnumParam, Q_RELOCATABLE_TYPE);

struct ASTEnum : public SignedType
{
    explicit ASTEnum(const QString &name = QString());
    void signature_impl(const AST &ast, QCryptographicHash &checksum) override;
    QString typeName() const override;

    QString type;
    QString scope;
    QList<ASTEnumParam> params;
    bool isSigned;
    bool isScoped;
    int max;
    int flagIndex = -1;
};
Q_DECLARE_TYPEINFO(ASTEnum, Q_RELOCATABLE_TYPE);

struct ASTFlag : public SignedType
{
    explicit ASTFlag(const QString &name = {}, const QString &_enum = {});
    void signature_impl(const AST &ast, QCryptographicHash &checksum) override;
    QString typeName() const override;

    bool isValid() const;
    QString _enum;
    QString scope;
};
Q_DECLARE_TYPEINFO(ASTFlag, Q_RELOCATABLE_TYPE);

struct ASTModelRole
{
    ASTModelRole(const QString &roleName = QString())
        : name(roleName)
    {
    }

    QString name;
};
Q_DECLARE_TYPEINFO(ASTModelRole, Q_RELOCATABLE_TYPE);

struct ASTModel : public SignedType
{
    ASTModel(const QString &name, const QString &scope, int index = -1)
        : SignedType(name), scope(scope), propertyIndex(index) {}
    void signature_impl(const AST &ast, QCryptographicHash &checksum) override;
    QString typeName() const override;

    QList<ASTModelRole> roles;
    QString scope;
    int propertyIndex;
};
Q_DECLARE_TYPEINFO(ASTModel, Q_RELOCATABLE_TYPE);

/// A Class declaration
struct ASTClass : public SignedType
{
    explicit ASTClass(const QString& name = QString());
    void signature_impl(const AST &ast, QCryptographicHash &checksum) override;

    bool isValid() const;
    bool hasPointerObjects() const;

    QList<ASTProperty> properties;
    QList<ASTFunction> signalsList;
    QList<ASTFunction> slotsList;
    QList<ASTEnum> enums;
    QList<ASTFlag> flags;
    bool hasPersisted;
    QList<ASTModel> modelMetadata;
    QList<int> subClassPropertyIndices;
};
Q_DECLARE_TYPEINFO(ASTClass, Q_RELOCATABLE_TYPE);

// The attribute of a POD
struct PODAttribute
{
    explicit PODAttribute(const QString &type_ = QString(), const QString &name_ = QString())
        : type(type_),
          name(name_)
    {}
    QString type;
    QString name;
};
Q_DECLARE_TYPEINFO(PODAttribute, Q_RELOCATABLE_TYPE);

// A POD declaration
struct POD : public SignedType
{
    void signature_impl(const AST &ast, QCryptographicHash &checksum) override;

    QList<PODAttribute> attributes;
    QList<ASTEnum> enums;
    QList<ASTFlag> flags;
};
Q_DECLARE_TYPEINFO(POD, Q_RELOCATABLE_TYPE);

// The AST representation of a .rep file
struct AST
{
    QList<ASTClass> classes;
    QList<POD> pods;
    QList<ASTEnum> enums;
    QList<ASTFlag> flags;
    QList<QString> enumUses;
    QStringList preprocessorDirectives;
    QStringList headerLines;
    QStringList footerLines;
    QHash<QString, QByteArray> typeSignatures;
    QByteArray typeData(const QString &type, const QString &className) const;
    QByteArray functionsData(const QList<ASTFunction> &functions, const QString &className) const;
};
Q_DECLARE_TYPEINFO(AST, Q_RELOCATABLE_TYPE);

class RepParser: public QRegexParser<RepParser, $table>
{
public:
    explicit RepParser(QIODevice &outputDevice);
    virtual ~RepParser() {}

    bool parse() override { return QRegexParser<RepParser, $table>::parse(); }

    void reset() override;
    int nextToken();
    bool consumeRule(int ruleno);

    AST ast() const;

private:
    struct TypeParser
    {
        void parseArguments(const QString &arguments);
        void appendParams(ASTFunction &slot);
        void appendPods(POD &pods);
        void generateFunctionParameter(QString variableName, const QString &propertyType, int &variableNameIndex, ASTDeclaration::VariableTypes variableType);
        //Type, Variable
        QList<ASTDeclaration> arguments;
    };

    bool parseProperty(ASTClass &astClass, const QString &propertyDeclaration);
    /// A helper function to parse modifier flag of property declaration
    bool parseModifierFlag(const QString &flag, ASTProperty::Modifier &modifier, bool &persisted);

    bool parseRoles(ASTModel &astModel, const QString &modelRoles);

    AST m_ast;

    ASTClass m_astClass;
    POD m_astPod;
    QString m_symbol;
    QString m_argString;
    ASTEnum m_astEnum;
    int m_astEnumValue;
};
QT_END_NAMESPACE
#endif
:/


/.
#include "repparser.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QTextStream>

// for normalizeTypeInternal
#include <private/qmetaobject_p.h>
#include <private/qmetaobject_moc_p.h>

// Code copied from moc.cpp
// We cannot depend on QMetaObject::normalizedSignature,
// since repc is linked against Qt5Bootstrap (which doesn't offer QMetaObject) when cross-compiling
// Thus, just use internal API which is exported in private headers, as moc does
static QByteArray normalizeType(const QByteArray &ba)
{
    const char *s = ba.constData();
    int len = ba.size();
    char stackbuf[64];
    char *buf = (len >= 64 ? new char[len + 1] : stackbuf);
    char *d = buf;
    char last = 0;
    while (*s && is_space(*s))
        s++;
    while (*s) {
        while (*s && !is_space(*s))
            last = *d++ = *s++;
        while (*s && is_space(*s))
            s++;
        if (*s && ((is_ident_char(*s) && is_ident_char(last))
                   || ((*s == ':') && (last == '<')))) {
            last = *d++ = ' ';
        }
    }
    *d = '\0';
    QByteArray result = normalizeTypeInternal(buf, d);
    if (buf != stackbuf)
        delete [] buf;
    return result;
}

SignedType::SignedType(const QString &name) : name(name)
{
}

void SignedType::generateSignature(AST &ast)
{
    QCryptographicHash checksum(QCryptographicHash::Sha1);
    signature_impl(ast, checksum);
    ast.typeSignatures[typeName()] = checksum.result().toHex();
}

QString SignedType::typeName() const
{
    return name;
}

ASTProperty::ASTProperty()
    : modifier(ReadPush), persisted(false), isPointer(false)
{
}

ASTProperty::ASTProperty(const QString &type, const QString &name, const QString &defaultValue, Modifier modifier, bool persisted, bool isPointer)
    : type(type), name(name), defaultValue(defaultValue), modifier(modifier), persisted(persisted), isPointer(isPointer)
{
}

QString ASTDeclaration::asString(bool withName) const
{
    QString str;
    if (variableType & ASTDeclaration::Constant)
        str += QLatin1String("const ");
    str += type;
    if (variableType & ASTDeclaration::Reference)
        str += QLatin1String(" &");
    if (withName)
        str += QString::fromLatin1(" %1").arg(name);
    return str;
}

ASTFunction::ASTFunction(const QString &name, const QString &returnType)
    : returnType(returnType), name(name)
{
}

QString ASTFunction::paramsAsString(ParamsAsStringFormat format) const
{
    QString str;
    for (const ASTDeclaration &param : params) {
        QString paramStr = param.asString(format != Normalized);
        if (format == Normalized) {
            paramStr = QString::fromLatin1(::normalizeType(paramStr.toLatin1().constData()));
            str += paramStr + QLatin1Char(',');
        } else {
            str += paramStr + QLatin1String(", ");
        }
    }

    str.chop((format == Normalized ? 1 : 2)); // chop trailing ',' or ', '

    return str;
}

QStringList ASTFunction::paramNames() const
{
    QStringList names;
    names.reserve(params.size());
    for (const ASTDeclaration &param : params)
        names << param.name;
    return names;
}

ASTEnum::ASTEnum(const QString &name)
    : SignedType(name), isSigned(false), isScoped(false), max(0)
{
}

void ASTEnum::signature_impl(const AST &ast, QCryptographicHash &checksum)
{
    Q_UNUSED(ast)
    checksum.addData(name.toLatin1());
    if (isScoped)
        checksum.addData("class");
    if (!type.isEmpty())
        checksum.addData(type.toLatin1());
    for (const ASTEnumParam &param : params) {
        checksum.addData(param.name.toLatin1());
        checksum.addData(QByteArray::number(param.value));
    }
}

QString ASTEnum::typeName() const
{
    if (scope.isEmpty())
        return name;

    return QLatin1String("%1::%2").arg(scope, name);
}

ASTFlag::ASTFlag(const QString &name, const QString &_enum)
    : SignedType(name), _enum(_enum)
{
}

void ASTFlag::signature_impl(const AST &ast, QCryptographicHash &checksum)
{
    checksum.addData(name.toLatin1());
    checksum.addData(ast.typeData(_enum, scope));
}

QString ASTFlag::typeName() const
{
    if (scope.isEmpty())
        return name;

    return QLatin1String("%1::%2").arg(scope, name);
}

bool ASTFlag::isValid() const
{
    return !name.isEmpty();
}

void ASTModel::signature_impl(const AST &ast, QCryptographicHash &checksum)
{
    Q_UNUSED(ast)
    QByteArrayList _roles;
    for (const auto &role : roles)
        _roles << role.name.toLatin1();
    std::sort(_roles.begin(), _roles.end());
    checksum.addData(_roles.join('_'));
}

QString ASTModel::typeName() const
{
    return QLatin1String("%1::%2").arg(scope, name);
}

ASTClass::ASTClass(const QString &name)
    : SignedType(name), hasPersisted(false)
{
}

void ASTClass::signature_impl(const AST &ast, QCryptographicHash &checksum)
{
    checksum.addData(name.toLatin1());

    // Checksum properties
    QSet<int> classIndices{ subClassPropertyIndices.begin(),
                            subClassPropertyIndices.end() };
    int propertyIndex = -1;
    int modelIndex = 0;
    for (const ASTProperty &p : properties) {
        propertyIndex++;
        checksum.addData(p.name.toLatin1());
        if (p.type == QLatin1String("QAbstractItemModel"))
            checksum.addData(ast.typeSignatures[modelMetadata[modelIndex++].typeName()]);
        else if (classIndices.contains(propertyIndex))
            checksum.addData(ast.typeSignatures[p.type]);
        else
            checksum.addData(ast.typeData(p.type, name));
        ASTProperty::Modifier m = p.modifier;
        // Treat ReadOnly and SourceOnlySetter the same (interface-wise they are)
        if (m == ASTProperty::SourceOnlySetter)
            m = ASTProperty::ReadOnly;
        checksum.addData({reinterpret_cast<const char *>(&m), sizeof(m)});
    }

    // Checksum signals
    checksum.addData(ast.functionsData(signalsList, name));

    // Checksum slots
    checksum.addData(ast.functionsData(slotsList, name));
}

void POD::signature_impl(const AST &ast, QCryptographicHash &checksum)
{
    checksum.addData(name.toLatin1());
    for (const PODAttribute &attr : attributes) {
        checksum.addData(attr.name.toLatin1());
        checksum.addData(ast.typeData(attr.type, name));
    }
}

bool ASTClass::isValid() const
{
    return !name.isEmpty();
}

bool ASTClass::hasPointerObjects() const
{
    int count = modelMetadata.size() + subClassPropertyIndices.size();
    return count > 0;
}

QByteArray AST::typeData(const QString &type, const QString &className) const
{
    static const QRegularExpression re = QRegularExpression(QLatin1String("([^<>,\\s]+)"));
    if (type.contains(QLatin1Char('<'))) { // templated type
        QByteArray result;
        for (const QRegularExpressionMatch &match : re.globalMatch(type))
            result += typeData(match.captured(1), className);
        return result;
    }
    // Try enum/flags within the class first
    if (!className.isEmpty()) {
        auto classType = QLatin1String("%1::%2").arg(className, type);
        auto it = typeSignatures.find(classType);
        if (it != typeSignatures.end())
            return it.value();
    }
    auto it = typeSignatures.find(type);
    if (it != typeSignatures.end())
        return it.value();
    const auto pos = type.lastIndexOf(QLatin1String("::"));
    if (pos > 0)
        return typeData(type.mid(pos + 2), className);
    return type.toLatin1();
}

QByteArray AST::functionsData(const QList<ASTFunction> &functions, const QString &className) const
{
    QByteArray ret;
    for (const ASTFunction &func : functions) {
        ret += func.name.toLatin1();
        for (const ASTDeclaration &param : func.params) {
            ret += param.name.toLatin1();
            ret += typeData(param.type, className);
            ret += QByteArray(reinterpret_cast<const char *>(&param.variableType),
                              sizeof(param.variableType));
        }
        ret += typeData(func.returnType, className);
    }
    return ret;
}

RepParser::RepParser(QIODevice &outputDevice)
    : QRegexParser(), m_astEnumValue(-1)
{
    setBufferFromDevice(&outputDevice);
}

void RepParser::reset()
{
    m_ast = AST();
    m_astClass = ASTClass();
    m_astPod = POD();
    m_argString.clear();
    m_astEnum = ASTEnum();
    //setDebug();
}

bool RepParser::parseModifierFlag(const QString &flag, ASTProperty::Modifier &modifier, bool &persisted)
{
    QRegularExpression regex(QStringLiteral("\\s*,\\s*"));
    QStringList flags = flag.split(regex);
    persisted = flags.removeAll(QStringLiteral("PERSISTED")) > 0;
    if (flags.length() == 0)
        return true;
    if (flags.length() > 1) {
        // Only valid combination is "READONLY" and "CONSTANT"
        if (flags.length() == 2 && flags.contains(QStringLiteral("READONLY")) &&
            flags.contains(QStringLiteral("CONSTANT"))) {
            // If we have READONLY and CONSTANT that means CONSTANT
            modifier = ASTProperty::Constant;
            return true;
        } else {
            setErrorString(QLatin1String("Invalid property declaration: combination not allowed (%1)").arg(flag));
            return false;
        }
    }
    const QString &f = flags.at(0);
    if (f == QLatin1String("READONLY"))
        modifier = ASTProperty::ReadOnly;
    else if (f == QLatin1String("CONSTANT"))
        modifier = ASTProperty::Constant;
    else if (f == QLatin1String("READPUSH"))
        modifier = ASTProperty::ReadPush;
    else if (f == QLatin1String("READWRITE"))
        modifier = ASTProperty::ReadWrite;
    else if (f == QLatin1String("SOURCEONLYSETTER"))
        modifier = ASTProperty::SourceOnlySetter;
    else {
        setErrorString(QLatin1String("Invalid property declaration: flag %1 is unknown").arg(flag));
        return false;
    }

    return true;
}

QString stripArgs(const QString &arguments)
{
    // This repc parser searches for the longest possible matches, which can be multiline.
    // This method "cleans" the string input, removing comments and converting to a single
    // line for subsequent parsing.
    QStringList lines = arguments.split(QRegularExpression(QStringLiteral("\r?\n")));
    for (auto & line : lines)
        line.replace(QRegularExpression(QStringLiteral("//.*")),QString());
    return lines.join(QString());
}

bool RepParser::parseProperty(ASTClass &astClass, const QString &propertyDeclaration)
{
    QString input = stripArgs(propertyDeclaration).trimmed();
    const QRegularExpression whitespace(QStringLiteral("\\s"));

    QString propertyType;
    QString propertyName;
    QString propertyDefaultValue;
    ASTProperty::Modifier propertyModifier = ASTProperty::ReadPush;
    bool persisted = false;

    // parse type declaration which could be a nested template as well
    bool inTemplate = false;
    int templateDepth = 0;
    int nameIndex = -1;

    for (int i = 0; i < input.size(); ++i) {
        const QChar inputChar(input.at(i));
        if (inputChar == QLatin1Char('<')) {
            propertyType += inputChar;
            inTemplate = true;
            ++templateDepth;
        } else if (inputChar == QLatin1Char('>')) {
            propertyType += inputChar;
            --templateDepth;
            if (templateDepth == 0)
                inTemplate = false;
        } else if (inputChar.isSpace()) {
            if (!inTemplate) {
                nameIndex = i;
                break;
            } else {
                propertyType += inputChar;
            }
        } else {
            propertyType += inputChar;
        }
    }

    if (nameIndex == -1) {
        setErrorString(QLatin1String("PROP: Invalid property declaration: %1").arg(propertyDeclaration));
        return false;
    }

    // parse the name of the property
    input = input.mid(nameIndex).trimmed();

    const int equalSignIndex = input.indexOf(QLatin1Char('='));
    if (equalSignIndex != -1) { // we have a default value
        propertyName = input.left(equalSignIndex).trimmed();

        input = input.mid(equalSignIndex + 1).trimmed();
        const int lastQuoteIndex = input.lastIndexOf(QLatin1Char('"'));
        if (lastQuoteIndex != -1) {
            propertyDefaultValue = input.left(lastQuoteIndex + 1);
            input = input.mid(lastQuoteIndex + 1);
        }
        const int whitespaceIndex = input.indexOf(whitespace);
        if (whitespaceIndex == -1) { // no flag given
            if (propertyDefaultValue.isEmpty())
                propertyDefaultValue = input;
            propertyModifier = ASTProperty::ReadPush;
        } else { // flag given
            if (propertyDefaultValue.isEmpty())
                propertyDefaultValue = input.left(whitespaceIndex).trimmed();

            const QString flag = input.mid(whitespaceIndex + 1).trimmed();
            if (!parseModifierFlag(flag, propertyModifier, persisted))
                return false;
        }
    } else { // there is no default value
        const int whitespaceIndex = input.indexOf(whitespace);
        if (whitespaceIndex == -1) { // no flag given
            propertyName = input;
            propertyModifier = ASTProperty::ReadPush;
        } else { // flag given
            propertyName = input.left(whitespaceIndex).trimmed();

            const QString flag = input.mid(whitespaceIndex + 1).trimmed();
            if (!parseModifierFlag(flag, propertyModifier, persisted))
                return false;
        }
    }

    astClass.properties << ASTProperty(propertyType, propertyName, propertyDefaultValue, propertyModifier, persisted);
    if (persisted)
        astClass.hasPersisted = true;
    return true;
}

bool RepParser::parseRoles(ASTModel &astModel, const QString &modelRoles)
{
    const QString input = modelRoles.trimmed();

    if (input.isEmpty())
        return true;

    const QStringList roleStrings = input.split(QChar(QLatin1Char(',')));
    for (auto role : roleStrings)
        astModel.roles << ASTModelRole(role.trimmed());
    return true;
}

AST RepParser::ast() const
{
    return m_ast;
}

void RepParser::TypeParser::parseArguments(const QString &arguments)
{
    const QString strippedArgs = stripArgs(arguments);
    int templateDepth = 0;
    bool inTemplate = false;
    bool inVariable = false;
    QString propertyType;
    QString variableName;
    ASTDeclaration::VariableTypes variableType = ASTDeclaration::None;
    int variableNameIndex = 0;
    for (int i = 0; i < strippedArgs.size(); ++i) {
        const QChar inputChar(strippedArgs.at(i));
        if (inputChar == QLatin1Char('<')) {
            propertyType += inputChar;
            inTemplate = true;
            ++templateDepth;
        } else if (inputChar == QLatin1Char('>')) {
            propertyType += inputChar;
            --templateDepth;
            if (templateDepth == 0)
                inTemplate = false;
        } else if (inputChar.isSpace()) {
            if (inTemplate)
                propertyType += inputChar;
            else if (!propertyType.isEmpty()) {
                if (propertyType == QLatin1String("const")) {
                    propertyType.clear();
                    variableType |= ASTDeclaration::Constant;
                } else {
                    inVariable = true;
                }
            }
        } else if (inputChar == QLatin1Char('&')) {
            variableType |= ASTDeclaration::Reference;
        } else if (inputChar == QLatin1Char(',')) {
            if (!inTemplate) {
                RepParser::TypeParser::generateFunctionParameter(variableName, propertyType, variableNameIndex, variableType);
                propertyType.clear();
                variableName.clear();
                variableType = ASTDeclaration::None;
                inVariable = false;
            } else {
                propertyType += inputChar;
            }
        } else {
            if (inVariable)
                variableName += inputChar;
            else
                propertyType += inputChar;
        }
    }
    if (!propertyType.isEmpty()) {
        RepParser::TypeParser::generateFunctionParameter(variableName, propertyType, variableNameIndex, variableType);
    }
}

void RepParser::TypeParser::generateFunctionParameter(QString variableName, const QString &propertyType, int &variableNameIndex, ASTDeclaration::VariableTypes variableType)
{
    if (!variableName.isEmpty())
        variableName = variableName.trimmed();
    else
        variableName = QString::fromLatin1("__repc_variable_%1").arg(++variableNameIndex);
    arguments.append(ASTDeclaration(propertyType, variableName, variableType));
}

void RepParser::TypeParser::appendParams(ASTFunction &slot)
{
    for (const ASTDeclaration &arg : std::as_const(arguments))
        slot.params << arg;
}

void RepParser::TypeParser::appendPods(POD &pods)
{
    for (const ASTDeclaration &arg : std::as_const(arguments)) {
        PODAttribute attr;
        attr.type = arg.type;
        attr.name = arg.name;
        pods.attributes.append(qMove(attr));
    }
}

bool RepParser::consumeRule(int ruleno)
{
    if (isDebug()) {
        qDebug() << "consumeRule:" << ruleno << spell[rule_info[rule_index[ruleno]]];
    }
    switch (ruleno) {
./

TopLevel: Types | Newlines Types;

Types: Type | Types Type;

Newlines: newline | Newlines newline;
Comments: Comment | Comments Comment;
Comment: comment | comment Newlines | mcomment | mcomment Newlines;
Type: PreprocessorDirective | PreprocessorDirective Newlines;
Type: Pod | Pod Newlines;
Type: Pod2;
Type: Class;
Type: UseEnum | UseEnum Newlines;
Type: Comment | Comment Newlines;
Type: Enum;
/.
    case $rule_number:
    {
        m_astEnum.generateSignature(m_ast);
        m_ast.enums.append(m_astEnum);
    }
    break;
./
Type: Flag | Flag Newlines;

Comma: comma | comma Newlines;

Equals: equals;

PreprocessorDirective: preprocessor_directive;
/.
    case $rule_number:
    {
        const QString line = captured().value(QStringLiteral("preprocessor_directive"));
        if (line.startsWith(QStringLiteral("#HEADER")))
            m_ast.headerLines.append(line.sliced(8));
        else if (line.startsWith(QStringLiteral("#FOOTER")))
            m_ast.footerLines.append(line.sliced(8));
        else
            m_ast.preprocessorDirectives.append(captured().value(QStringLiteral("preprocessor_directive")));
    }
    break;
./

Pod: pod;
/.
    case $rule_number:
    {
        POD pod;
        pod.name = captured().value(QStringLiteral("name")).trimmed();
        pod.compilerAttribute = captured().value(QStringLiteral("attribute")).trimmed();

        const QString argString = captured().value(QLatin1String("types")).trimmed();
        if (argString.isEmpty()) {
            qWarning() << "[repc] - Ignoring POD with no data members.  POD name: " << qPrintable(pod.name);
            return true;
        }
        for (const QStringView &token : argString.tokenize(u' ')) {
            if (token.startsWith(QLatin1String("ENUM"))) {
                if (token.size() == 4 || !(token[4].isLetterOrNumber() || token[4] == u'_')) {
                    setErrorString(QLatin1String("ENUMs are only available in PODs using bracket syntax ('{'), not parentheses"));
                    return false;
                }
            }
        }

        RepParser::TypeParser parseType;
        parseType.parseArguments(argString);
        parseType.appendPods(pod);
        pod.generateSignature(m_ast);
        m_ast.pods.append(pod);
    }
    break;
./

Class: ClassStart Start ClassContent Stop;
/.
    case $rule_number:
./
Class: ClassStart Start Stop;
/.
    case $rule_number:
    {
        m_astClass.generateSignature(m_ast);
        m_ast.classes.append(m_astClass);
    }
    break;
./

--"Decorated" types capture comments _before_ the type and newlines _after_ the type
--This leaves newlines at the start of the scope and comments at the end of the scope still to be handled
ClassContent: ClassTypes | ClassTypes Comments | Newlines ClassTypes | Newlines ClassTypes Comments;
--Allow empty/ignored content in the scope for testing purposes
ClassContent: Comments | Newlines | Newlines Comments | Comments Newlines | Newlines Comments Newlines;
ClassTypes: ClassType | ClassTypes ClassType;
ClassType: DecoratedProp | DecoratedSignal | DecoratedSlot | DecoratedModel | DecoratedClass | DecoratedClassFlag;
ClassType: DecoratedEnum;
/.
    case $rule_number:
    {
        m_astEnum.scope = m_astClass.name;
        m_astEnum.generateSignature(m_ast);
        m_astClass.enums.append(m_astEnum);
    }
    break;
./

Pod2: PodStart Start PodContent Stop;
/.
    case $rule_number:
./
Pod2: PodStart Start Stop;
/.
    case $rule_number:
    {
        RepParser::TypeParser parseType;
        parseType.parseArguments(m_argString);
        parseType.appendPods(m_astPod);
        m_astPod.generateSignature(m_ast);
        m_ast.pods.append(m_astPod);
    }
    break;
./

--"Decorated" types capture comments _before_ the type and newlines _after_ the type
--This leaves newlines at the start of the scope and comments at the end of the scope still to be handled
PodContent: PodTypes | PodTypes Comments | Newlines PodTypes | Newlines PodTypes Comments;
--Allow empty/ignored content in the scope for testing purposes
PodContent: Comments | Newlines | Newlines Comments | Comments Newlines | Newlines Comments Newlines;
PodTypes: PodType | PodTypes CaptureComma PodType;
--Enums are now legal in PODs, but don't usually have a comma afterwards so we need to accept matches without
--separating commas unless we expand the language to distinguish enums from other PodTypes.
PodTypes: PodTypes PodType;
PodType: DecoratedPODFlag;
PodType: DecoratedEnum;
/.
    case $rule_number:
    {
        m_astEnum.generateSignature(m_ast);
        m_astPod.enums.append(m_astEnum);
    }
    break;
./

PodType: Parameter | Parameter Newlines;
Parameter: DecoratedParameterType ParameterName;

DecoratedParameterType: ParameterType | Qualified ParameterType | ParameterType PassByReference | Qualified ParameterType PassByReference;

ParameterType: SimpleType | TemplateType;

TemplateType: TemplateTypename TStart ParameterTypes TStop;

TemplateTypename: Symbol;
/.
    case $rule_number:
    {
        m_argString += m_symbol;
    }
    break;
./

TStart: tstart;
/.
    case $rule_number:
    {
        m_argString += QLatin1Char('<');
    }
    break;
./

TStop: tstop;
/.
    case $rule_number:
    {
        m_argString += QLatin1Char('>');
    }
    break;
./

Qualified: qualifier;
/.
    case $rule_number:
    {
        m_argString += captured().value(QLatin1String("value")).trimmed() + QLatin1Char(' ');
    }
    break;
./

PassByReference: passbyqual;
/.
    case $rule_number:
    {
        m_argString += QLatin1Char(' ') + captured().value(QLatin1String("value")).trimmed();
    }
    break;
./

ParameterTypes: DecoratedParameterType | ParameterTypes CaptureComma DecoratedParameterType;

CaptureComma: comma;
/.
    case $rule_number:
./
CaptureComma: comma Newlines;
/.
    case $rule_number:
    {
        m_argString += QLatin1Char(',');
    }
    break;
./

SimpleType: Symbol;
/.
    case $rule_number:
    {
        m_argString += m_symbol;
    }
    break;
./

ParameterName: Symbol;
/.
    case $rule_number:
    {
        m_argString += QLatin1Char(' ') + m_symbol;
    }
    break;
./

DecoratedSlot: Slot | Comments Slot | Slot Newlines | Comments Slot Newlines;
DecoratedSignal: Signal | Comments Signal | Signal Newlines | Comments Signal Newlines;
DecoratedProp: Prop | Comments Prop | Prop Newlines | Comments Prop Newlines;
DecoratedModel: Model | Comments Model | Model Newlines | Comments Model Newlines;
DecoratedClass: ChildRep | Comments ChildRep | ChildRep Newlines | Comments ChildRep Newlines;
DecoratedEnumParam: EnumParam | Comments EnumParam | EnumParam Newlines | Comments EnumParam Newlines;
DecoratedClassFlag: ClassFlag | Comments ClassFlag | ClassFlag Newlines | Comments ClassFlag Newlines;

DecoratedPODFlag: PODFlag | Comments PODFlag | PODFlag Newlines | Comments PODFlag Newlines;
DecoratedEnum: Enum | Comments Enum | Enum Newlines | Comments Enum Newlines;

Start: start | Comments start | start Newlines | Comments start Newlines;
Stop: stop | stop Newlines;

Enum: EnumStart Start EnumParams Stop;

EnumStart: enum;
/.
    case $rule_number:
    {
        const QString name = captured().value(QLatin1String("name"));
        const QString type = captured().value(QLatin1String("type"));
        const QString _class = captured().value(QLatin1String("class"));

        // new Class declaration
        m_astEnum = ASTEnum(name);
        if (!_class.isEmpty())
            m_astEnum.isScoped = true;
        if (!type.isEmpty())
            m_astEnum.type = type;
        m_astEnumValue = -1;
    }
    break;
./

EnumParams: DecoratedEnumParam | EnumParams Comma DecoratedEnumParam;

EnumParam: Symbol;
/.
    case $rule_number:
    {
        ASTEnumParam param;
        param.name = m_symbol;
        param.value = ++m_astEnumValue;
        if (m_astEnum.max < param.value)
            m_astEnum.max = param.value;
        m_astEnum.params << param;
    }
    break;
./

EnumParam: Symbol Equals Value;
/.
    case $rule_number:
    {
        ASTEnumParam param;
        param.name = m_symbol;
        param.value = m_astEnumValue;
        if (param.value < 0) {
            m_astEnum.isSigned = true;
            if (m_astEnum.max < -param.value)
                m_astEnum.max = -param.value;
        } else if (m_astEnum.max < param.value)
            m_astEnum.max = param.value;
        m_astEnum.params << param;
    }
    break;
./

Symbol: symbol;
/.
    case $rule_number:
    {
        m_symbol = captured().value(QStringLiteral("symbol")).trimmed();
    }
    break;
./

Value: value;
/.
    case $rule_number:
    {
        QString value = captured().value(QStringLiteral("value")).trimmed();
        if (value.startsWith(QLatin1String("0x"), Qt::CaseInsensitive))
            m_astEnumValue = value.toInt(0,16);
        else
            m_astEnumValue = value.toInt();
    }
    break;
./

ClassFlag: flag;
/.
    case $rule_number:
    {
        const QString name = captured().value(QLatin1String("name"));
        const QString _enum = captured().value(QLatin1String("enum"));
        int enumIndex = 0;
        for (auto &en : m_astClass.enums) {
            if (en.name == _enum) {
                en.flagIndex = m_astClass.flags.count();
                break;
            }
            enumIndex++;
        }
        if (enumIndex == m_astClass.enums.count()) {
            setErrorString(QLatin1String("FLAG: Unknown (class) enum: %1").arg(_enum));
            return false;
        }
        auto flag = ASTFlag(name, _enum);
        flag.scope = m_astClass.name;
        flag.generateSignature(m_ast);
        m_astClass.flags.append(flag);
    }
    break;
./

PODFlag: flag;
/.
    case $rule_number:
    {
        const QString name = captured().value(QLatin1String("name"));
        const QString _enum = captured().value(QLatin1String("enum"));
        int enumIndex = 0;
        for (auto &en : m_astPod.enums) {
            if (en.name == _enum) {
                en.flagIndex = m_astPod.flags.count();
                break;
            }
            enumIndex++;
        }
        if (enumIndex == m_astPod.enums.count()) {
            setErrorString(QLatin1String("FLAG: Unknown (pod) enum: %1").arg(_enum));
            return false;
        }
        auto flag = ASTFlag(name, _enum);
        flag.scope = m_astPod.name;
        flag.generateSignature(m_ast);
        m_astPod.flags.append(flag);
    }
    break;
./

Prop: prop;
/.
    case $rule_number:
    {
        const QString args = captured().value(QLatin1String("args"));
        if (!parseProperty(m_astClass, args))
            return false;
    }
    break;
./

Signal: signal;
/.
    case $rule_number:
    {
        ASTFunction signal;
        signal.name = captured().value(QLatin1String("name")).trimmed();

        const QString argString = captured().value(QLatin1String("args")).trimmed();
        RepParser::TypeParser parseType;
        parseType.parseArguments(argString);
        parseType.appendParams(signal);
        m_astClass.signalsList << signal;
    }
    break;
./

Slot: slot;
/.
    case $rule_number:
    {
        QString returnTypeAndName = captured().value(QLatin1String("type")).trimmed();
        const QString argString = captured().value(QLatin1String("args")).trimmed();

        // compat code with old SLOT declaration: "SLOT(func(...))"
        const bool hasWhitespace = returnTypeAndName.indexOf(u' ') != -1;
        if (!hasWhitespace) {
            qWarning() << "[repc] - Adding 'void' for unspecified return type on" << qPrintable(returnTypeAndName);
            returnTypeAndName.prepend(QLatin1String("void "));
        }

        const int startOfFunctionName = returnTypeAndName.lastIndexOf(u' ') + 1;

        ASTFunction slot;
        slot.returnType = returnTypeAndName.mid(0, startOfFunctionName-1);
        slot.name = returnTypeAndName.mid(startOfFunctionName);

        RepParser::TypeParser parseType;
        parseType.parseArguments(argString);
        parseType.appendParams(slot);
        m_astClass.slotsList << slot;
    }
    break;
./

Model: model;
/.
    case $rule_number:
    {
        const QString name = captured().value(QLatin1String("name")).trimmed();
        const QString argString = captured().value(QLatin1String("args")).trimmed();

        ASTModel model(name, m_astClass.name, m_astClass.properties.size());
        if (!parseRoles(model, argString))
            return false;

        model.generateSignature(m_ast);
        m_astClass.modelMetadata << model;
        m_astClass.properties << ASTProperty(QStringLiteral("QAbstractItemModel"), name, QStringLiteral("nullptr"), ASTProperty::SourceOnlySetter, false, true);
    }
    break;
./

ChildRep: childrep;
/.
case $rule_number:
{
    const QString name = captured().value(QLatin1String("name")).trimmed();
    const QString type = captured().value(QLatin1String("type")).trimmed();

    m_astClass.subClassPropertyIndices << m_astClass.properties.size();
    m_astClass.properties << ASTProperty(type, name, QStringLiteral("nullptr"), ASTProperty::SourceOnlySetter, false, true);
}
break;
./

ClassStart: class Newlines;
/.
    case $rule_number:
./
ClassStart: class;
/.
    case $rule_number:
    {
        const QString attribute = captured().value(QLatin1String("attribute"));
        const QString name = captured().value(QLatin1String("name"));

        // new Class declaration
        m_astClass = ASTClass(name);
        m_astClass.compilerAttribute = attribute;
    }
    break;
./

PodStart: pod2;
/.
    case $rule_number:
./
PodStart: pod2 Newlines;
/.
    case $rule_number:
    {
        // new POD declaration
        m_astPod = POD();
        m_astPod.name = captured().value(QLatin1String("name")).trimmed();
        m_astPod.compilerAttribute = captured().value(QLatin1String("attribute"));
        m_argString.clear();
    }
    break;
./

UseEnum: use_enum;
/.
    case $rule_number:
    {
        const QString name = captured().value(QLatin1String("name"));

        m_ast.enumUses.append(name);
    }
    break;
./

Flag: flag;
/.
    case $rule_number:
    {
        const QString name = captured().value(QLatin1String("name"));
        const QString _enum = captured().value(QLatin1String("enum"));
        int enumIndex = 0;
        for (auto &en : m_ast.enums) {
            if (en.name == _enum) {
                en.flagIndex = m_ast.flags.count();
                break;
            }
            if (en.name == _enum)
                break;
            enumIndex++;
        }
        if (enumIndex == m_ast.enums.count()) {
            setErrorString(QLatin1String("FLAG: Unknown (global) enum: %1").arg(_enum));
            return false;
        }
        auto flag = ASTFlag(name, _enum);
        flag.generateSignature(m_ast);
        m_ast.flags.append(flag);
    }
    break;
./

--Error conditions/messages
ClassType: ClassStart;
/.
    case $rule_number:
    {
        setErrorString(QStringLiteral("class: Cannot be nested"));
        return false;
    }
    break;
./
ClassType: Pod;
/.
    case $rule_number:
    {
        setErrorString(QStringLiteral("POD: Can only be used in global scope"));
        return false;
    }
    break;
./
ClassType: UseEnum;
/.
    case $rule_number:
    {
        setErrorString(QStringLiteral("USE_ENUM: Can only be used in global scope"));
        return false;
    }
    break;
./
Type: Signal;
/.
    case $rule_number:
    {
        setErrorString(QStringLiteral("SIGNAL: Can only be used in class scope"));
        return false;
    }
    break;
./
Type: Slot;
/.
    case $rule_number:
    {
        setErrorString(QStringLiteral("SLOT: Can only be used in class scope"));
        return false;
    }
    break;
./
Type: Prop;
/.
    case $rule_number:
    {
        setErrorString(QStringLiteral("PROP: Can only be used in class scope"));
        return false;
    }
    break;
./
Type: Model;
/.
    case $rule_number:
    {
        setErrorString(QStringLiteral("MODEL: Can only be used in class scope"));
        return false;
    }
    break;
./

Type: ChildRep;
/.
    case $rule_number:
    {
        setErrorString(QStringLiteral("CLASS: Can only be used in class scope"));
        return false;
    }
    break;
./

/.
    } // switch
    return true;
}
./
