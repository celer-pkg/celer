// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

/*!
\internal
\page lspgenerate.html

\title Generate Language Server Protocol

The language server protocol is largely generated from the
specification
  src/languageserver/3rdparty/specification.md
by the src/languageserver/generate.ts script.
Use
  npm install
  tsc --downlevelIteration --strictNullChecks generate.ts && node generate.js
to run it. Then git clang-format to reformat its output.
It generates:

src/languageserver/qlanguageserverspectypes_p.h:
  Types defined in the protocol are defined as C++ POD objects in the
  QLspSpecification namespace, that implement the walk template for
  serialization/deserialization, enmuerations are also defined, along
  with helpers to ensure their conversion to json is what defined in
  the specification (some enumeration should be stored as numbers,
  other as lowercase string, other as string equal to the enumeration
  constant).

src/languageserver/qlanguageserverspec_p.h
  Includes qlanguageserverspectypes_p.h, and defines:
  * the Capabilites one has to check/define, and aliases for the types
    defining them in the namespace ClientCapabilitiesInfo for the
    client ones, and in ServerCapabilitiesInfo for the server side.
  * in Requests the method and alias for the paramenters of the
    requests
  * in Notifications the same for notifications
  * in Responses, aliases for the types of the responses or partial
    responses
  * finally it defines RequestParams and NotificationsParams as
    variants able to keep any request or notification parameters

src/languageserver/qlanguageservergen_p.h
  Defines the ProtocolGen class that defines the typed LSP protocol:
  * requestXX or notifyXX methods to preform requests/send
    notifications
  * registerXXRequestHandler methods to handle requests
  * registerXXNotificationHandler methods to handle notifications

src/languageserver/qlanguageservergen_p_p.h
  Declaration of QLanguageServeGenPrivate.

src/languageserver/qlanguageservergen.cpp
  Implementation of QLanguageServeGen and QLanguageServeGenPrivate.

src/languageserver/qlspnotifysignals_p.h An object that emits signals
  for notifications, this way one can easily have multiple handlers
  for the same notification, by simply connecting the signal multiple
  times.

The script also generates the following files that are not checked in:
- protocol.json: the structured information on Capabilites, methods,
  types of requests and notifications
- protocolRaw.json: the raw version of protocol.json that is extracted
  from the markdown specification, useful for debugging (extraction
  from the text is done with regexps, and it doesn't folloe any
  formally checked grammar, but uses structured text)
- specification.ts: the typescript definition of the various
  types (but without information on how they are used in the protocol),
  used to generate qlanguageserverspectypes_p.h

The goal is to keep the generated part simple, that is the reason when
possible setup/tweaks are done in other non generated files:
src/languageserver/qlanguageserverprespectypes_p.h:
  setup for qlanguageserverspectypes_p.h and qlanguageserverspec_p.h
src/languageserver/qlanguageserverbase_p.h and qlanguageserverbase.cpp:
  setup for qlanguageservergen* (base class)
src/languageserver/qlanguageserverprotocol_p.h, qlanguageserverprotocol_p_p.h and
    qlanguageserverprotocol.cpp:
  Define QLanguageServerProtocol the main "user facing" class built on
  the top of qlanguageservergen*
*/
import * as ts from "typescript";

var globalBaseClass = "";

function stringLiteral(text: string)
{
    return "QLatin1String(\"" + text + "\")";
}

var builtinTypes = {
    "string" : "QByteArray",
    "number" : "int",
    "boolean" : "bool",
    "any" : "QJsonValue",
    "unknown" : "QJsonValue",
    "Array" : "QList",
    "null" : "std::nullptr_t",
    "object" : "QJsonObject",
    "[number, number]" : "std::pair<int,int>",
    // For enumerations in typescript single numbers can be types,
    // namely a type containing exactly that number.
    // That allows to also get the exact bit mask, and generally have more control.
    // We just consider them ints.
    "0" : "int",
    "1" : "int",
    "2" : "int",
    "3" : "int",
    "4" : "int"
};

var specialStructs = {
    "Message" : null, // non templatized top level
    "RequestMessage" : null, // non templatized top level
    "ResponseMessage" : null, // non templatized top level
    "NotificationMessage" : null, // non templatized top level
    "TextDocumentContentChangeEvent" : null, // anonymous objects
    "SelectionRange" : null // recursive reference
};

const patchedStructMembers = new Map<string, Map<string, string>>([
    // documentChanges has a weird type, so simplify it
    [
        "WorkspaceEdit",
        new Map<string, string>([ [
            "documentChanges",
            `using DocumentChange = std::variant<TextDocumentEdit, CreateFile, RenameFile, DeleteFile>;
    std::optional<QList<DocumentChange>> %1 = {};`
        ] ]),
    ],
    [
        // hardcode the kind to begin
        "WorkDoneProgressBegin",
        new Map<string, string>([ [ "kind", "QByteArray %1 = \"begin\";" ] ]),
    ],
    [
        // hardcode the kind to report
        "WorkDoneProgressReport",
        new Map<string, string>([ [ "kind", "QByteArray %1 = \"report\";" ] ]),
    ],
    [
        // hardcode the kind to end
        "WorkDoneProgressEnd",
        new Map<string, string>([ [ "kind", "QByteArray %1 = \"end\";" ] ]),
    ],
    [
        // The only type in the LSP specification that has a template member. Ignore the "generic"
        // case for now and use a variant of all possible progress values found in the LSP
        // specification.
        "ProgressParams",
        new Map<string, string>([ [
            "value",
            "std::variant<WorkDoneProgressBegin, WorkDoneProgressReport, WorkDoneProgressEnd> %1 = {};"
        ] ]),
    ],
]);

var specialEnums = { "ErrorCodes" : null, "InitializeError" : "InitializeErrorCode" }

var postStruct = {
    "DocumentFilter" : "using DocumentSelector = QList<DocumentFilter>;\n\n",
    "Range" : `class Q_LANGUAGESERVER_EXPORT TextDocumentContentChangeEvent
{
public:
    std::optional<Range> range = {};
    std::optional<int> rangeLength = {};
    QByteArray text = {};

    template <typename W> void walk(W &w) {
        field(w, "range", range);
        field(w, "rangeLength", rangeLength);
        field(w, "text", text);
    }
};

class Q_LANGUAGESERVER_EXPORT SelectionRange
{
public:
    SelectionRange() = default;
    SelectionRange(const Range &r):
        range(r)
    {}
    SelectionRange(const SelectionRange &o):
        range(o.range)
    {
        if (o.parent)
            parent = std::make_unique<SelectionRange>(*o.parent);
    }
    SelectionRange &operator=(const SelectionRange &o) {
        range = o.range;
        if (o.parent)
            parent = std::make_unique<SelectionRange>(*o.parent);
        return *this;
    }
    SelectionRange(SelectionRange &&) noexcept = default;
    SelectionRange& operator=(SelectionRange &&) noexcept = default;

    Range range = {};
    std::unique_ptr<SelectionRange> parent;

    template <typename W> void walk(W &w) {
        field(w, "range", range);
        field(w, "parent", parent);
    }
};

class Q_LANGUAGESERVER_EXPORT RangePlaceHolder
{
public:
    Range range = {};
    QByteArray placeholder = {};

    template <typename W> void walk(W &w) {
        field(w, "range", range);
        field(w, "placeholder", placeholder);
    }
};

class Q_LANGUAGESERVER_EXPORT DefaultBehaviorStruct
{
public:
    bool defaultBehavior = {};

    template <typename W> void walk(W &w) {
        field(w, "defaultBehavior", defaultBehavior);
    }
};

`
};

let enums: Enum[] = [];
let enumNames = {};
let structs: Struct[] = [];

interface Member {
    name: string, type: string|Struct, isOptional: boolean
}

interface Value {
    name: string, value: string
}

interface Struct {
    name: string, extends: string, members: Member[], hasExtraMembers: boolean
}

interface Enum {
    name: string, members: Value[], type: string
}

function upperCase(type: string): string
{
    return type.substr(0, 1).toUpperCase() + type.substr(1);
}

function baseType(type: string)
{
    if (type.indexOf("\"") != -1 || type.indexOf(" | ") != -1) {
        return "any";
    } else if (type.indexOf("[]") != -1) {
        return "Array";
    } else {
        var angle = type.indexOf("<");
        return (angle != -1) ? type.substr(0, angle) : type;
    }
}

function templateParam(type: string)
{
    if (type.indexOf("\"") != -1 || type.indexOf(" | ") != -1) {
        return "";
    } else if (type.indexOf("[]") != -1) {
        return "<" + effectiveType(type.substr(0, type.length - 2)) + ">";
    } else {
        var angle = type.indexOf("<");
        return (angle != -1)
                ? "<" + effectiveType(type.substring(angle + 1, type.indexOf(">"))) + ">"
                : "";
    }
}

function uniq(a)
{
    var seen = {};
    return a.filter(function(
            item) { return seen.hasOwnProperty(item) ? false : (seen[item] = true); });
}

function effectiveType(type: string)
{
    // with strict mode all optional types get a | undefined appended to them, get rid of it
    // as we handle them as std::optional not as variant that can be undefined.
    let undefinedRemoveRe: RegExp = / \| undefined\b/g;
    while (true) {
        let newT = type.replace(undefinedRemoveRe, "");
        if (newT == type)
            break;
        type = newT;
    }
    // arrays -> QList
    let arrayRegexp: RegExp = /(?:\((.+)\)|([A-Za-z0-9]+|"[^"]+"))\[\]/;
    while (true) {
        var newT = type.replace(arrayRegexp, function(m) {
            let match = m.match(arrayRegexp);
            if (match && match[1])
                return "QList<" + effectiveType(match[1]) + ">";
            else if (match && match[2])
                return "QList<" + effectiveType(match[2]) + ">";
            else {
                console.log("error with array type:" + type);
                return "QJsonValue";
            }
        });
        if (newT == type)
            break;
        type = newT
    }
    if (type.startsWith('"') && type.endsWith('"')) {
        return "QByteArray";
    } else if (type.indexOf("\"") != -1) {
        console.log("type:" + type);
        return "QJsonValue";
    } else if (type[0] == '{' && type[type.length - 1] == '}') {
        return "QJsonObject";
    } else if (type.indexOf(" | ") != -1) {
        return "std::variant<" + uniq(type.split(" | ").map(effectiveType)).join(", ") + ">";
    } else if (builtinTypes[type]) {
        return builtinTypes[type];
    }
    return type;
}

function isNullableVariant(typeStr: String)
{
    // variants containing either a or b are represented as a | b
    // we look at the variants containing null as possible option
    let nestedParentheses = 0;
    let currentTypeStart = 0;
    for (let i = 0; i < typeStr.length; ++i) {
        let c = typeStr[i];
        if (c == "(" || c == "[")
            nestedParentheses += 1;
        else if (c == ")" || c == "]")
            nestedParentheses -= 1;
        else if (c == "|" && nestedParentheses == 0) {
            if (typeStr.slice(currentTypeStart, i).trim() == "null")
                return true;
            currentTypeStart = i + 1;
        }
    }
    return typeStr.slice(currentTypeStart, typeStr.length).trim() == "null";
}

function generateEnum(e: Enum)
{
    let output: string = "enum class " + e.name + "\n{\n";
    output += e.members
                      .map(function(member) {
                          let value = e.type === "number" ? (" = " + (+member.value)) : "";
                          return "    " + member.name + value;
                      })
                      .join(",\n");
    return output + "\n};\nQ_ENUM_NS(" + e.name + ")\n\n";
}

function generateStringAccessors(e: Enum)
{
    let output: string = "";
    let nameValIdentical: boolean = true;
    let nameValSimilar: boolean = true;
    e.members.forEach(function(m) {
        if (m.name !== m.value) {
            nameValIdentical = false;
            if (m.name.toUpperCase() !== m.value.toUpperCase())
                nameValSimilar = false;
        }
    });
    if (!nameValIdentical) {
        output = "template<>\n";
        output += "inline QString enumToString<QLspSpecification::" + e.name
                + ">(QLspSpecification::" + e.name + " value)\n";
        output += "{\n";
        output += "    switch (value) {\n"
        output += e.members
                          .map(function(member) {
                              return "    case QLspSpecification::" + e.name + "::" + member.name
                                      + ": return " + stringLiteral(member.value) + ";\n";
                          })
                          .join("");
        output += "    default: return QString::number(int(value));\n";
        output += "    }\n"
        output += "}\n\n";
    }
    if (!nameValSimilar) {
        output += "template<>\n";
        output += "inline QLspSpecification::" + e.name
                + " enumFromString<QLspSpecification::" + e.name + ">(const QString &string)\n";
        output += "{\n";
        output += e.members
                          .map(function(member) {
                              return "    if (string.compare(" + stringLiteral(member.value)
                                      + ", Qt::CaseInsensitive) == 0)\n" +
                                      "        return QLspSpecification::" + e.name
                                      + "::" + member.name + ";\n";
                          })
                          .join("    else ");
        output += "    return QLspSpecification::" + e.name + "{};\n";
        output += "}\n\n";
    }
    return output;
}

function generateNumberAccessors(e: Enum)
{
    let output: string = "";
    output += "template<>\n";
    output += "inline QString enumToString<QLspSpecification::" + e.name
            + ">(QLspSpecification::" + e.name + " value)\n";
    output += "{\n";
    output += "    return enumToIntString<QLspSpecification::" + e.name + ">(value);";
    output += "}\n\n";

    return output;
}

function setterParam(type: string, name: string)
{
    if (type == "bool" || type == "int")
        return "(" + type + " " + name + ")";
    else
        return "(const " + type + " &" + name + ")";
}

function generateClass(struct: Struct, indent: string)
{
    let output: string = indent + "class Q_LANGUAGESERVER_EXPORT " + struct.name;
    if (struct.extends.length != 0)
        output += " : public " + struct.extends;
    output += "\n";

    var innerIndent = indent + "    ";
    output += indent + "{\n" + indent + "public:\n";

    output += struct.members
                      .map(function(member: Member) {
                          if (patchedStructMembers.has(struct.name)
                              && patchedStructMembers.get(struct.name)?.has(member.name)) {
                              const rType =
                                      patchedStructMembers.get(struct.name)?.get(member.name)!;
                              return innerIndent + rType.replace(/%1/, member.name) + "\n";
                          }

                          var members = "";
                          var type = "";
                          let defaultValue = "{}";
                          if ((<Struct>member.type).members) {
                              members += "\n";
                              members += generateClass(<Struct>member.type, innerIndent);
                              type = (<Struct>member.type).name;
                          } else {
                              let t = <string>member.type;
                              type = effectiveType(t);
                              if (isNullableVariant(t))
                                  defaultValue = "nullptr";
                          }
                          let upperCaseName: string = upperCase(member.name);
                          var rType: string = type;
                          if (member.isOptional)
                              rType = "std::optional<" + type + ">";
                          members += innerIndent + rType + " " + member.name + " = " + defaultValue
                                  + ";\n";
                          return members;
                      })
                      .join("");
    if (struct.hasExtraMembers) {
        output += innerIndent + "QJsonObject extraFields;\n"
    }

    output += "\n"
    if (struct.extends.length != 0 || struct.members.length != 0)
        output += innerIndent + "template <typename W> void walk(W &w) {\n"
        else output += innerIndent + "template <typename W> void walk(W &) {\n"
        if (struct.extends.length != 0)
        struct.extends.split(", ").forEach(function(
                parentName) { output += innerIndent + "    " + parentName + "::walk(w);\n"; })
        if (struct.members.length != 0)
        output += struct.members
                          .map(function(member: Member) {
                              return innerIndent
                                      + `    field(w, "${member.name}", ${member.name});\n`;
                          })
                          .join("")
        output += innerIndent + "}\n";

        if (struct.hasExtraMembers) {
            output += innerIndent
                    + "template <typename W> void walkExtra(W &w) { w.handleExtras(extraFields); }\n"
        }
    output += indent + "};\n\n";
    let post = postStruct[struct.name];
    if (post)
        output += post
        return output;
}

interface GeneratedTypes {
    typeDeclarations: string, enumStringConversions: string
}

/** Generate code for all classes in a set of .ts files */
function generate(fileNames: string[], options: ts.CompilerOptions): GeneratedTypes
{
    let typeDeclarations: string = "";
    let enumStringConversions: string = "";

    // Build a program using the set of root file names in fileNames
    let program = ts.createProgram(fileNames, options);

    // Get the checker, we will use it to find more about classes
    let checker = program.getTypeChecker();

    let identifier: ts.Node|null = null;

    // Visit every sourceFile in the program
    for (const sourceFile of program.getSourceFiles()) {
        // Walk the tree to search for classes
        if (fileNames.indexOf(sourceFile.fileName) != -1)
            ts.forEachChild(sourceFile, visit);
    }

    var generated = {};
    var sortedStructs: Struct[] = [];

    var doGenerateDeclarations = function(struct: Struct) {
        if (generated[struct.name] !== undefined)
            return;

        structs.forEach(function(other) {
            if (struct === other || generated[other.name] !== undefined)
                return;

            // special case: ProgressParams has member of type T, which we changed to
            // std::variant<WorkDoneProgress(Begin|Report|End)>. Make sure the types
            // inside the variant exist before writing out ProgressParams.
            if (struct.name == "ProgressParams"
                && other.name.match(/WorkDoneProgress(Begin|Report|End)/g))
                doGenerateDeclarations(other);

            if (struct.extends.split(", ").includes(other.name)) {
                doGenerateDeclarations(other);
                return;
            }

            var contained = false;
            let otherRe = new RegExp('\\b' + other.name + '\\b')
            var checkMember =
                    function(member) {
                if (member.type.members) {
                    member.type.members.forEach(checkMember);
                } else if (otherRe.test(member.type)) {
                    contained = true;
                }
            }

                    struct.members.forEach(checkMember);
            if (contained)
                doGenerateDeclarations(other);
        });

        typeDeclarations += generateClass(struct, "");

        generated[struct.name] = true;
        sortedStructs.push(struct);
    };

    enums.forEach(function(e) { typeDeclarations += generateEnum(e); });
    structs.forEach(doGenerateDeclarations);

    enums.forEach(function(e) {
        if (e.type == "string")
            enumStringConversions += generateStringAccessors(e);
        else
            enumStringConversions += generateNumberAccessors(e);
    });

    return { typeDeclarations : typeDeclarations, enumStringConversions : enumStringConversions };

    /** visit nodes finding exported classes */
    function visit(node: ts.Node)
    {
        if (node.kind == ts.SyntaxKind.InterfaceDeclaration) {
            // This is a top level class, get its symbol
            var iface = serializeInterface((<ts.InterfaceDeclaration>node));
            var special = specialStructs[iface.name];
            if (special === undefined) {
                structs.push(iface);
            } else if (typeof (special) === "string") {
                iface.name = special;
                structs.push(iface);
            }
        } else if (node.kind == ts.SyntaxKind.ModuleDeclaration) {
            // This is a namespace, visit its children
            ts.forEachChild(node, visit);
        } else if (node.kind == ts.SyntaxKind.Identifier) {
            identifier = node;
        } else if (node.kind == ts.SyntaxKind.ModuleBlock) {
            let e = serializeNamespace((<ts.Identifier>identifier), (<ts.ModuleBlock>node));
            identifier = null;
            var special = specialEnums[e.name];
            if (special === undefined && enumNames[e.name] === undefined) {
                enumNames[e.name] = true
                enums.push(e);
            } else if (typeof (special) === "string") {
                enumNames[e.name] = true
                e.name = special;
                enums.push(e);
            }
        } else if (node.kind == ts.SyntaxKind.EnumDeclaration) {
            let e = serializeEnumDecl(<ts.EnumDeclaration>node);
            enumNames[e.name] = true
            enums.push(e);
        }
    }

    function serializeSymbol(symbol: ts.Symbol, name: string, isOptional: boolean): Member
    {
        if (symbol.flags & ts.SymbolFlags.TypeLiteral) {
            let struct: Struct = {
                name: upperCase(name),
                extends: globalBaseClass,
                members: [],
                hasExtraMembers: false
            };

            symbol.members?.forEach(function(member) {
                var signature = (<ts.PropertySignature>member.valueDeclaration);
                struct.members.push(
                        serializeSymbol(member, <string>member.escapedName,
                                        (signature && signature.questionToken) ? true : false));
            });

            return {
                name : name,
                isOptional : isOptional,
                type : (struct.members.length == 1 && <string>struct.members[0].name == "__index")
                        ? "any"
                        : struct
            };
        } else {
            let d = symbol.declarations?.[0];
            let t = checker.getTypeAtLocation(d!);
            return serializeType(t!, name, isOptional);
        }
    }

    function serializeType(type: ts.Type, name: string, isOptional: boolean): Member
    {
        if (type.symbol && (type.symbol.flags & ts.SymbolFlags.TypeLiteral)) {
            return serializeSymbol(type.symbol, name, isOptional);
        } else {
            return { name : name, isOptional : isOptional, type : checker.typeToString(type) };
        }
    }

    function serializeEnumDecl(enumDecl: ts.EnumDeclaration): Enum
    {
        let identifier = enumDecl.name;
        var e: Enum = {
            name : checker.symbolToString(checker.getSymbolAtLocation(identifier)!),
            members : [],
            type : "number"
        };

        enumDecl.members.forEach(function(m: ts.EnumMember) {
            let nKind = m.name.kind;
            if (nKind == ts.SyntaxKind.Identifier) {
                let t = m.initializer!.getText();
                let v: Value = {
                    name : upperCase(checker.symbolToString(
                            checker.getSymbolAtLocation(<ts.Identifier>m.name)!)),
                    value : t.replace(/['"]/g, '')
                };
                if (t.includes("'") || t.includes('"') || +t + "" !== t)
                    e.type = "string";
                e.members.push(v);
            } else if (ts.isLiteralExpression(m.name)) {
                let t = (<ts.LiteralExpression>m.name).text;
                let v: Value = {
                    name : upperCase(t.replace(/['"]/g, '')),
                    value : t.replace(/['"]/g, '')
                };
                e.type = "string";
                e.members.push(v);
            } else {
                console.error("unsupported type " + nKind + " in EnumMember.name for enum "
                              + e.name);
            }
        });
        return e;
    }

    function serializeBaseTypes(name: ts.Identifier): string
    {
        let result = checker.getBaseTypes((<ts.InterfaceType>checker.getTypeAtLocation(name)))
                             .map(function(type) { return checker.typeToString(type) })
                             .join(", ");
        return (result.length == 0) ? globalBaseClass : result;
    }

    function serializeInterface(iface: ts.InterfaceDeclaration): Struct
    {
        let mm: Member[] = []
        let result = {
            name: checker.symbolToString(checker.getSymbolAtLocation(iface.name)!),
            members: mm,
            extends: serializeBaseTypes(iface.name),
            hasExtraMembers: false
        };

        iface.members.forEach(function(typeElement) {
            if (typeElement.name) {
                result.members.push(serializeType(
                        checker.getTypeAtLocation(typeElement.name)!,
                        checker.symbolToString(checker.getSymbolAtLocation(typeElement.name)!),
                        typeElement.questionToken ? true : false)!);
            } else {
                result.hasExtraMembers = true;
            }
        });

        return result;
    }

    function serializeVariableDeclaration(variable: ts.VariableDeclaration): Value
    {
        return {
            name : upperCase(checker.symbolToString(checker.getSymbolAtLocation(variable.name)!)!),
            value : variable.initializer?.getText()?.replace(/['"]/g, '')!
        };
    }

    function serializeNamespace(identifier: ts.Identifier, block: ts.ModuleBlock): Enum
    {
        var e: Enum = {
            name : checker.symbolToString(checker.getSymbolAtLocation(identifier)!),
            members : [],
            type : "number"
        };

        ts.forEachChild(block, function(node: ts.Node) {
            if (node.kind == ts.SyntaxKind.VariableStatement) {
                var statement = (<ts.VariableStatement>node);
                statement.declarationList.declarations.forEach(function(declaration) {
                    let member: Value = serializeVariableDeclaration(declaration);
                    e.members.push(member);
                    if (+member.value + "" !== member.value)
                        e.type = "string";
                });
            } else {
                console.error(`${e.name} ${node.kind}`);
            }
        });

        return e;
    }
}

function textToDict(s: string)
{
    let typeRe = /`([^`]+)`/g;
    let strRe = /'([^']+)'/g
    let types: string[] = [];
    let strings: string[] = [];
    let tStr = s.replace(/` *where.*/, '`');
    while (true) {
        let uriRe = /\[ *(`[^`)]+`) *\]\([^)]+\)/;
        let arrayRe = /`([A-Z][a-zA-Z0-9]+)`\[\]/;
        let newStr = tStr.replace(uriRe, function(m) { return m.match(uriRe)![1]; })
                             .replace(arrayRe,
                                      function(m) { return "`" + m.match(arrayRe)![1] + "[]`"; })
                             .replace(/` *\\*\| *`/, ' | ');
        if (newStr == tStr)
            break;
        tStr = newStr;
    }
    for (const m of tStr.matchAll(typeRe))
        types.push(m[1]);
    for (const m of s.matchAll(strRe))
        strings.push(m[1]);
    return { text : s, strings : strings, types : types };
}

var checkedTypeExtraction = {
    "`TextDocumentSyncKind | TextDocumentSyncOptions`. The below definition of the `TextDocumentSyncOptions` only covers the properties specific to the open, change and close notifications. A complete definition covering all properties can be found [here](#textDocument_didClose):" :
            "TextDocumentSyncKind | TextDocumentSyncOptions",
    "`CompletionItem[]` \\| `CompletionList` \\| `null`. If a `CompletionItem[]` is provided it is interpreted to be complete. So it is the same as `{ isIncomplete: false, items }`" :
            "CompletionItem[] | CompletionList | null",
    "`CompletionItem[]` or `CompletionList` followed by `CompletionItem[]`. If the first provided result item is of type `CompletionList` subsequent partial results of `CompletionItem[]` add to the `items` property of the `CompletionList`." :
            "CompletionList | CompletionItem[]",
    "`DocumentSymbol[]` \\| `SymbolInformation[]`. `DocumentSymbol[]` and `SymbolInformation[]` can not be mixed. That means the first chunk defines the type of all the other chunks." :
            "DocumentSymbol[] | SymbolInformation[]",
    "`Range | { range: Range, placeholder: string } | { defaultBehavior: boolean } | null` describing a [`Range`](#range) of the string to rename and optionally a placeholder text of the string content to be renamed. If `{ defaultBehavior: boolean }` is returned (since 3.16) the rename position is valid and the client should use its default behavior to compute the rename range. If `null` is returned then it is deemed that a 'textDocument/rename' request is not valid at the given position." :
            "Range | RangePlaceHolder | DefaultBehaviorStruct | null"
};

interface StructuredProtocol {
    structuredSequence: string[]
    structured: any
}

function extractProto(lines): StructuredProtocol
{
    var res = {};
    var resSequence: string[] = [];
    function insert(path: string[], value)
    {
        if (resSequence.length == 0 || resSequence[resSequence.length - 1] != path[0])
            resSequence.push(path[0]);
        let el = res;
        for (const p of path.slice(0, path.length - 1)) {
            if (!el[p])
                el[p] = {};
            el = el[p];
        }
        let last = path[path.length - 1];
        if (el[last])
            console.log(`WARNING: overwriting ${JSON.stringify(el[last])} with ${
                    JSON.stringify(value)} at ${path}`)
            el[last] = value
    }
    let sectionRe: RegExp = /^#+ .* name="([^"]*)"/;
    let groupRe: RegExp =
            /^(?: *|<[^<>]+>)*_([A-Za-z_0-9 ]+)(?:_ *:| *: *_)(?: *|<[^<>]+>)*(\w+.*)?$/;
    var path: string[] = [];
    var basePath: string[] = [];
    let i = 0;
    let ii = 0;
    while (i < lines.length) {
        let line = lines[i];
        i += 1;
        let mList = line.match(/^\* +(?:([^'`:]+):)? *(.*)$/);
        if (mList) {
            if (mList[1]) {
                if (mList[2])
                    insert(path.concat([ mList[1].trim() ]), textToDict(mList[2]));
                else if (!/ *\* *error\.[a-z]*/.test(line))
                    console.log(`WARNING: missing value for ${path}: ${line}`);
            } else if (mList[2]) {
                insert(path.concat([ `${ii}` ]), textToDict(mList[2]));
                ii += 1;
            }
            continue
        }
        let m1 = line.match(sectionRe);
        if (m1) {
            basePath = [ m1[1] ];
            path = basePath;
            continue
        }
        let m2 = line.match(groupRe);
        if (m2) {
            path = basePath.concat([ m2[1] ]);
            ii = 0;
            if (m2[2]) {
                insert(path, textToDict(m2[2]));
            }
            continue
        }
        let m3 = line.match(/^\*\* *(\w.*)\*\* *$/);
        if (m3) {
            basePath = [ basePath[0], m3[1].trim() ];
            path = basePath
            continue
        }
    }
    return { structured : res, structuredSequence : resSequence };
}

function namify(str: string): string
{
    return str.split(/[$./ _]+/).map(upperCase).join("");
}

function stringFromDict(dict): string
{
    if (!dict)
        return "";
    if (dict["strings"] && dict["strings"].length > 0)
        return dict["strings"][0];
    else if (dict["types"] && dict["types"].length > 0)
        return dict["types"][0];
    else
        return "";
}

function typeFromDict(dict): string
{
    if (!dict)
        return "";
    if (dict["types"] && dict["types"].length > 0) {
        if (dict['types'].length != 1) {
            let t = checkedTypeExtraction[dict['text']];
            if (t)
                return t;
            console.log(`Suspicious type extraction \"${dict['text']}\": "${dict['types'][0]}"`);
        }
        return dict["types"][0];
    } else if (dict["strings"] && dict["strings"].length > 0)
        return dict["strings"][0];
    else if (dict["text"].match(/ *(any *\[ *\]) *\.? */))
        return "any[]";
    else if (dict["text"].match(/ *(none|void|null) *\.? */))
        return "null";
    console.log(`cannot extract type from ${JSON.stringify(dict)}`);
    return "";
}

interface ServerCapability {
    propertyPath: string, propertyType: string
}

function getServerCapability(path, dict): ServerCapability
{
    let pPath: string = "";
    if (dict["property path (optional)"])
        pPath = stringFromDict(dict["property path (optional)"]);
    else if (dict["property name (optional)"])
        pPath = stringFromDict(dict["property name (optional)"]);
    else
        console.log(`no property name or path for ServerCapability at ${path}`);
    let res: ServerCapability = {
        propertyPath : pPath,
        propertyType : typeFromDict(dict["property type"])
    };
    if (!res.propertyPath)
        console.log(`ServerCapability missing 'property path (optional)' in ${
                JSON.stringify(dict)} at ${path}`);
    if (!res.propertyType)
        console.log(
                `ServerCapability missing 'property type' in ${JSON.stringify(dict)} at ${path}`);
    return res;
}

interface ClientCapability {
    propertyPath: string, propertyType: string
}

function getClientCapability(path, dict): ClientCapability
{
    let pPath: string = "";
    if (dict["property path (optional)"])
        pPath = stringFromDict(dict["property path (optional)"]);
    else if (dict["property name (optional)"])
        pPath = stringFromDict(dict["property name (optional)"]);
    else
        console.log(`no property name or path for ServerCapability at ${path}`);
    let res: ClientCapability = {
        propertyPath : pPath,
        propertyType : typeFromDict(dict["property type"])
    };
    if (!res.propertyPath)
        console.log(`ClientCapability missing 'property path/name (optional)' in ${
                JSON.stringify(dict)} at ${path}`);
    if (!res.propertyType)
        console.log(
                `ClientCapability missing 'property type' in ${JSON.stringify(dict)} at ${path}`);
    return res;
}

interface Notification {
    method: string, params: string
}

function getNotification(path, dict): Notification
{
    let res: Notification = {
        method : stringFromDict(dict["method"]),
        params : typeFromDict(dict["params"])
    };
    if (!res.method)
        console.log(`Notification missing 'method' in ${JSON.stringify(dict)} at ${path}`);
    if (!res.params)
        console.log(`Notification missing 'params' in ${JSON.stringify(dict)} at ${path}`);
    return res;
}

interface Response {
    result: string;
    partialResult?: string;
    error?: string;
}

function getResponse(path, dict): Response
{
    let res: Response = { result : typeFromDict(dict["result"]) };
    if (!res.result)
        console.log(`Response missing 'result' in ${JSON.stringify(dict)} at ${path}`);
    if (dict["error"])
        res.error = dict["error"]["text"];
    if (dict["partial result"])
        res.partialResult = typeFromDict(dict["partial result"]);
    return res;
}

interface Request {
    method: string, params: string, response?: Response
}

function getRequest(path, dict): Request
{
    let res: Request = {
        method : stringFromDict(dict["method"]),
        params : typeFromDict(dict["params"])
    };
    if (!res.method)
        console.log(`Request missing 'method' in ${JSON.stringify(dict)} at ${path}`);
    if (!res.params)
        console.log(`Request missing 'params' in ${JSON.stringify(dict)} at ${path}`);
    return res;
}

function parseStructuredProtocol(structuredProto, structuredSequence)
{
    function handleGroups(path, dict)
    {
        let res = {};
        if (dict["Server Capability"]) {
            let cap = getServerCapability(path, dict["Server Capability"]);
            res["ServerCapability"] = cap;
        }
        if (dict["Client Capability"]) {
            let cap = getClientCapability(path, dict["Client Capability"]);
            res["ClientCapability"] = cap;
        }
        if (dict["Request"]) {
            let req: Request = getRequest(path, dict["Request"]);
            if (dict["Response"]) {
                let res: Response = getResponse(path, dict["Response"]);
                req.response = res;
            } else {
                console.log(`Request without Response at ${path}`);
            }
            res["Request"] = req;
        } else if (dict["Notification"]) {
            let notif = getRequest(path, dict["Notification"]);
            res["Notification"] = notif;
        } else {
            if (dict["Response"]) {
                console.log(`Response without Request in ${path}`);
            }
            Object.entries(dict).forEach(([ key2, value2 ]) => {
                if (key2 != "Request" && key2 != "Response" && key2 != "Notification"
                    && key2 != "Client Capability" && key2 != "Server Capability") {
                    let newPath = path + "." + key2;
                    let toIgnoreRe: RegExp = /^(?:traceValue\b|version_|snippet_|regExp\b)/;
                    if (!(<any>value2)?.["text"]) {
                        res[key2] = handleGroups(newPath, value2)
                    } else if (!toIgnoreRe.test(path)) {
                        console.log(`Ignoring at ${path}:${JSON.stringify(value2)}`);
                    }
                }
            });
        }
        return res;
    }

    let extractedInfo = {};
    for (const key1 of structuredSequence) {
        let value1 = structuredProto[key1];
        extractedInfo[key1] = handleGroups(key1, value1);
    }
    return extractedInfo;
}

interface GeneratedProtocol {
    notifications: string[], requests: string[], responses: string[]
    clientCapabilities: string[], serverCapabilities: string[], registrations: string[],
            registerDeclarations: string[], signalDeclarations: string[],
            registerImplementations: string[], registerVars: string[], requestParams: string[],
            notificationParams: string[], sendDeclarations: string[], sendImplementations: string[],
            requestMethodMap: string[], notificationMethodMap: string[]
}

function generateProtocol(extractedInfo, structuredSequence): GeneratedProtocol
{
    let notifications: string[] = [];
    let requests: string[] = [];
    let responses: string[] = [];
    let clientCapabilities: string[] = [];
    let serverCapabilities: string[] = [];
    let registrations: string[] = [];
    let registerDeclarations: string[] = [];
    let signalDeclarations: string[] = [];
    let registerImplementations: string[] = [];
    let registerVars: string[] = [];
    let implementations: string[] = [];
    let requestParams: string[] = [];
    let requestParamsKnown = {};
    let notificationParams: string[] = [];
    let notificationParamsKnown = {};
    let sendDeclarations: string[] = [];
    let sendImplementations: string[] = [];
    let requestMethodMap: string[] = [];
    let notificationMethodMap: string[] = [];

    function handleGroups(path, dict)
    {
        if (dict["ServerCapability"]) {
            let cap = dict["ServerCapability"];
            serverCapabilities.push(
                    `constexpr auto ${namify(cap.propertyPath)} = "${cap.propertyPath}";`);
            serverCapabilities.push(
                    `using ${namify(cap.propertyPath)}Type = ${effectiveType(cap.propertyType)};`);
            registerDeclarations.push("");
            registerDeclarations.push(`// ServerCapability::${namify(cap.propertyPath)}`)
            sendDeclarations.push("");
            sendDeclarations.push(`// ServerCapability::${namify(cap.propertyPath)}`)
        }
        if (dict["ClientCapability"]) {
            let cap = dict["ClientCapability"];
            clientCapabilities.push(
                    `constexpr auto ${namify(cap.propertyPath)} = "${cap.propertyPath}";`);
            clientCapabilities.push(
                    `using ${namify(cap.propertyPath)}Type = ${effectiveType(cap.propertyType)};`);
            if (!dict["ServerCapability"]) {
                registerDeclarations.push("");
                sendDeclarations.push("");
            }
            registerDeclarations.push(`// ClientCapability::${namify(cap.propertyPath)}`);
            sendDeclarations.push(`// ClientCapability::${namify(cap.propertyPath)}`)
        }
        if (dict["Request"]) {
            let req: Request = dict["Request"];
            let rName = req.params;
            if (rName.endsWith("Params"))
                rName = rName.slice(0, rName.length - "Params".length);
            else
                rName = namify(path.split(".").pop());
            let pType = effectiveType(req.params);
            if (!requestParamsKnown[pType]) {
                requestParamsKnown[pType] = true;
                requestParams.push(pType);
            }
            requests.push(`constexpr auto ${rName}Method = "${req.method}";`);
            let paramsType = effectiveType(req.params);
            requests.push(`using ${rName}ParamsType = ${paramsType};`);
            if (req.response) {
                let resultType = effectiveType(req.response.result);
                let responseType: string = "";
                responses.push(`using ${rName}ResultType = ${resultType};`);
                if (req.response.partialResult) {
                    let partialResultType = effectiveType(req.response.partialResult);
                    responses.push(`using ${rName}PartialResultType = ${partialResultType};`);
                    responseType = `LSPPartialResponse<${resultType}, ${partialResultType}>`;
                    responses.push(`using ${rName}ResponseType = LSPPartialResponse<${
                            rName}ResultType,${rName}PartialResultType>;`);
                    // skip client implementation for now
                } else {
                    responseType = `LSPResponse<${resultType}>`;
                    responses.push(`using ${rName}ResponseType = LSPResponse<${rName}ResultType>;`);
                }
                let responseHandlerType: string =
                        ((resultType == "std::nullptr_t")
                                 ? "std::function<void()>"
                                 : `std::function<void(const ${resultType} &)>`);
                sendDeclarations.push(`void request${rName}(const ${paramsType}&, ${
                        responseHandlerType} responseHandler, ResponseErrorHandler errorHandler = &ProtocolBase::defaultResponseErrorHandler);`);
                sendImplementations.push(`void ProtocolGen::request${rName}(const ${
                        paramsType} &params, ${
                        responseHandlerType} responseHandler, ResponseErrorHandler errorHandler)
{
    typedRpc()->sendRequest(QByteArray(Requests::${
                        rName}Method), [responseHandler = std::move(responseHandler), errorHandler = std::move(errorHandler)](const QJsonRpcProtocol::Response &response) {
        if (response.errorCode.isDouble())
            errorHandler(ResponseError{response.errorCode.toInt(), response.errorMessage.toUtf8(), response.data});
        else
            decodeAndCall<${resultType}>(response.data, responseHandler, errorHandler);
    }, params);
}`);
                registerDeclarations.push(`void register${
                        rName}RequestHandler(const std::function<void(const QByteArray &, const ${
                        paramsType} &, ${responseType} &&)> &handler);`);
                registerImplementations.push(`
void ProtocolGen::register${
                        rName}RequestHandler(const std::function<void(const QByteArray &, const ${
                        paramsType} &, ${responseType} &&)> &handler)
{
    typedRpc()->registerRequestHandler<QLspSpecification::Requests::${
                        rName}ParamsType, QLspSpecification::Responses::${rName}ResponseType>(
        QByteArray(QLspSpecification::Requests::${rName}Method), handler);
}`);
                requestMethodMap.push(`{QByteArray("${req.method}"), QByteArray("${rName}")}`);
            }
        } else if (dict["Notification"]) {
            let notif = dict["Notification"];
            let nName = notif.params;
            if (nName.endsWith("Params"))
                nName = nName.slice(0, nName.length - "Params".length);
            else
                nName = namify(path.split(".").pop());
            let pType = effectiveType(notif.params);
            notifications.push(`constexpr auto ${nName}Method = "${notif.method}";`);
            notificationMethodMap.push(`{QByteArray("${notif.method}"), QByteArray("${nName}")}`);
            if (specialStructs[notif.params] === undefined) {
                if (!notificationParamsKnown[pType]) {
                    notificationParamsKnown[pType] = true;
                    notificationParams.push(pType);
                }
                let paramsType = effectiveType(notif.params);
                notifications.push(`using ${nName}ParamsType = ${paramsType};`);
                registrations.push(`
protocol->register${nName}NotificationHandler(
    [this, protocol](const QByteArray &method, const QLspSpecification::Notifications::${
                        nName}ParamsType &params) {
        static const QMetaMethod notificationSignal = QMetaMethod::fromSignal(&QLspNotifySignals::received${
                        nName}Notification);
        if (isSignalConnected(notificationSignal))
            emit received${nName}Notification(params);
        else
            protocol->handleUndispatchedNotification(method, params);
    });`);
                signalDeclarations.push(`    void received${
                        nName}Notification(const QLspSpecification::Notifications::${
                        nName}ParamsType &);`)
                registerDeclarations.push(`void register${
                        nName}NotificationHandler(const std::function<void(const QByteArray &, const ${
                        paramsType} &)> &handler);`);
                registerImplementations.push(`
void ProtocolGen::register${
                        nName}NotificationHandler(const std::function<void(const QByteArray &, const ${
                        paramsType} &)> &handler)
{
    typedRpc()->registerNotificationHandler<QLspSpecification::Notifications::${nName}ParamsType>(
        QByteArray(QLspSpecification::Notifications::${nName}Method), handler);
}`);
                sendDeclarations.push(`    void notify${nName}(const ${paramsType}&params);`);
                sendImplementations.push(
                        `void ProtocolGen::notify${nName}(const ${paramsType} &params)
{
    typedRpc()->sendNotification(Notifications::${nName}Method, params);
}`);
            }
        } else {
            Object.entries(dict).forEach(([ key2, value2 ]) => {
                if (key2 != "Request" && key2 != "Response" && key2 != "Notification"
                    && key2 != "ClientCapability" && key2 != "ServerCapability") {
                    let newPath = path + "." + key2;
                    if (!(<any>value2)?.["text"]) {
                        handleGroups(newPath, value2);
                    }
                }
            });
        }
        if (dict["ServerCapability"] || dict["ClientCapability"]) {
            sendDeclarations.push("");
            registerDeclarations.push("");
        }
    }

    for (const key1 of structuredSequence) {
        let value1 = extractedInfo[key1];
        if (value1)
            handleGroups(key1, value1);
    }
    return {
        notifications : notifications,
        requests : requests,
        responses : responses,
        clientCapabilities : clientCapabilities,
        serverCapabilities : serverCapabilities,
        registrations : registrations,
        registerDeclarations : registerDeclarations,
        registerImplementations : registerImplementations,
        registerVars : registerVars,
        signalDeclarations : signalDeclarations,
        requestParams : requestParams,
        notificationParams : notificationParams,
        sendDeclarations : sendDeclarations,
        sendImplementations : sendImplementations,
        requestMethodMap : requestMethodMap,
        notificationMethodMap : notificationMethodMap
    };
}

let contents = ts.sys.readFile("3rdparty/specification.md");
let parts = contents?.split("\n```");
let output = "";
parts?.forEach(function(part) {
    if (part.indexOf("typescript") === 0)
        output += (part.substr("typescript".length));
    else if (part.indexOf("ts") === 0)
        output += (part.substr("ts".length));
});

let protoData = extractProto(contents!.split("\n"));
ts.sys.writeFile("protocolRaw.json", JSON.stringify(protoData, null, 2));
let protoStructs =
        parseStructuredProtocol(protoData["structured"], protoData["structuredSequence"]);
ts.sys.writeFile("protocol.json", JSON.stringify(protoStructs, null, 2));
let proto: GeneratedProtocol = generateProtocol(protoStructs, protoData["structuredSequence"]);

ts.sys.writeFile("specification.ts", output);

let result: GeneratedTypes = generate([ "specification.ts" ],
                                      { target : ts.ScriptTarget.Latest, strictNullChecks : true });

let license = ts.sys.readFile("generate.ts")!.split("\n\n")[0];

ts.sys.writeFile("qlanguageserverspectypes_p.h", `${license}

// this file was generated by the generate.ts script

#ifndef QLANGUAGESERVERSPECTYPES_P_H
#define QLANGUAGESERVERSPECTYPES_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtLanguageServer/qtlanguageserverglobal.h>
#include <QtLanguageServer/private/qlanguageserverprespectypes_p.h>
#include <QtJsonRpc/private/qtypedjson_p.h>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QJsonValue>
#include <QtCore/QJsonObject>
#include <QtCore/QString>

#include <optional>
#include <variant>

QT_BEGIN_NAMESPACE
namespace QLspSpecification {
Q_NAMESPACE_EXPORT(Q_LANGUAGESERVER_EXPORT)

enum class TraceValue
{
    Off,
    Messages,
    Verbose
};
Q_ENUM_NS(TraceValue)

enum class ErrorCodes {
    // Defined by JSON RPC
    ParseError = -32700,
    InvalidRequest = -32600,
    MethodNotFound = -32601,
    InvalidParams = -32602,
    InternalError = -32603,

    jsonrpcReservedErrorRangeStart = -32099,
    /** @deprecated use jsonrpcReservedErrorRangeStart */
    serverErrorStart = jsonrpcReservedErrorRangeStart,

    ServerNotInitialized = -32002,
    UnknownErrorCode = -32001,

    jsonrpcReservedErrorRangeEnd = -32000,
    /** @deprecated use jsonrpcReservedErrorRangeEnd */
    serverErrorEnd = jsonrpcReservedErrorRangeEnd,

    lspReservedErrorRangeStart = -32899,

    ContentModified = -32801,
    RequestCancelled = -32800,

    lspReservedErrorRangeEnd = -32800
};
Q_ENUM_NS(ErrorCodes)

${result.typeDeclarations}
} // namespace QLspSpecification

namespace QTypedJson {

template<>
inline QString enumToString<QLspSpecification::TraceValue>(QLspSpecification::TraceValue value)
{
    switch (value) {
    case QLspSpecification::TraceValue::Off: return QLatin1String("off");
    case QLspSpecification::TraceValue::Messages: return QLatin1String("messages");
    case QLspSpecification::TraceValue::Verbose: return QLatin1String("verbose");
    }
    return QString();
}

template<>
inline QString enumToString<QLspSpecification::ErrorCodes>(QLspSpecification::ErrorCodes value)
{
    return enumToIntString<QLspSpecification::ErrorCodes>(value);
}

${result.enumStringConversions}
} // namespace QTypedJson
QT_END_NAMESPACE
#endif // QLANGUAGESERVERSPECTYPES_P_H
`);

ts.sys.writeFile("qlanguageserverspec_p.h", license + `

// this file was generated by the generate.ts script

#ifndef QLANGUAGESERVERSPEC_P_H
#define QLANGUAGESERVERSPEC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtLanguageServer/qtlanguageserverglobal.h>
#include <QtLanguageServer/private/qlanguageserverspectypes_p.h>

QT_BEGIN_NAMESPACE

namespace QLspSpecification {
namespace ClientCapabilitiesInfo {
${proto.clientCapabilities.join("\n")}
}

namespace ServerCapabilitiesInfo {
${proto.serverCapabilities.join("\n")}
}

namespace Requests {
${proto.requests.join("\n")}
}

namespace Responses {
${proto.responses.join("\n")}
}

namespace Notifications {
${proto.notifications.join("\n")}
}

using RequestParams = std::variant<${proto.requestParams.join(", ")}, QJsonValue>;
using NotificationParams = std::variant<${proto.notificationParams.join(", ")}, QJsonValue>;

} // namespace QLspSpecification

QT_END_NAMESPACE

#endif // QLANGUAGESERVERSPEC_P_H
`);

ts.sys.writeFile("qlanguageservergen_p.h", license + `

// this file was generated by the generate.ts script

#ifndef QLANGUAGESERVERGEN_P_H
#define QLANGUAGESERVERGEN_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtLanguageServer/qtlanguageserverglobal.h>
#include <QtLanguageServer/private/qlanguageserverspec_p.h>
#include <QtLanguageServer/private/qlanguageserverbase_p.h>

#include <memory>
#include <functional>

QT_BEGIN_NAMESPACE

namespace QLspSpecification {

class ProtocolGenPrivate;

class Q_LANGUAGESERVER_EXPORT ProtocolGen: public ProtocolBase
{
protected:
    ProtocolGen(std::unique_ptr<ProtocolGenPrivate> &&p);
public:
    ~ProtocolGen();

// # Send protocol
${proto.sendDeclarations.join("\n")}

// # receive protocol
${proto.registerDeclarations.join("\n")}

private:
    Q_DISABLE_COPY(ProtocolGen)
    Q_DECLARE_PRIVATE(ProtocolGen)
};

} // namespace QLspSpecification

QT_END_NAMESPACE

#endif // QLANGUAGESERVER_P_H
`);

ts.sys.writeFile("qlanguageservergen.cpp", license + `

// this file was generated by the generate.ts script

#include <QtLanguageServer/private/qlanguageservergen_p_p.h>

#include <QtCore/QScopeGuard>

QT_BEGIN_NAMESPACE

namespace QLspSpecification {

QByteArray ProtocolBase::requestMethodToBaseCppName(const QByteArray &method)
{
    static QHash<QByteArray,QByteArray> map({
        ${proto.requestMethodMap.join(",\n        ")}
    });
    return map.value(method);
}

QByteArray ProtocolBase::notificationMethodToBaseCppName(const QByteArray &method)
{
    static QHash<QByteArray,QByteArray> map({
        ${proto.notificationMethodMap.join(",\n        ")}
    });
    return map.value(method);
}

ProtocolGen::ProtocolGen(std::unique_ptr<ProtocolGenPrivate> &&p):
    ProtocolBase(std::move(p))
{
}

ProtocolGen::~ProtocolGen()
{}

${proto.sendImplementations.join("\n\n")}

${proto.registerImplementations.join("\n\n")}

} // namespace QLspSpecification

QT_END_NAMESPACE
`);

ts.sys.writeFile("qlanguageservergen_p_p.h", license + `

// this file was generated by the generate.ts script

#ifndef QLANGUAGESERVERGEN_P_P_H
#define QLANGUAGESERVERGEN_P_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtLanguageServer/qtlanguageserverglobal.h>
#include <QtLanguageServer/private/qlanguageservergen_p.h>
#include <QtLanguageServer/private/qlanguageserverbase_p_p.h>

QT_BEGIN_NAMESPACE

namespace QLspSpecification {

class ProtocolGenPrivate: public ProtocolBasePrivate
{
public:
${proto.registerVars.join("\n")}
};

} // namespace QLspSpecification

QT_END_NAMESPACE

#endif // QLANGUAGESERVERGEN_P_P_H
`);

ts.sys.writeFile("qlspnotifysignals_p.h", license + `

// this file was generated by the generate.ts script

#ifndef QLSPNOTIFYSIGNALS_P_H
#define QLSPNOTIFYSIGNALS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtLanguageServer/private/qlanguageserverprotocol_p.h>

QT_BEGIN_NAMESPACE

class Q_LANGUAGESERVER_EXPORT QLspNotifySignals: public QObject
{
    Q_OBJECT
public:
    QLspNotifySignals(QObject *parent = nullptr) : QObject(parent) { }
    void registerHandlers(QLanguageServerProtocol *protocol);
signals:
${proto.signalDeclarations.join("\n")}
};

QT_END_NAMESPACE

#endif // QLSPNOTIFYSIGNALS_P_H
`);

ts.sys.writeFile("qlspnotifysignals.cpp", license + `

// this file was generated by the generate.ts script

#include <QtLanguageServer/private/qlspnotifysignals_p.h>

QT_BEGIN_NAMESPACE

using namespace QLspSpecification;

void  QLspNotifySignals::registerHandlers(QLanguageServerProtocol *protocol)
{
    ${proto.registrations.join("\n    ")}
}

QT_END_NAMESPACE
`);
