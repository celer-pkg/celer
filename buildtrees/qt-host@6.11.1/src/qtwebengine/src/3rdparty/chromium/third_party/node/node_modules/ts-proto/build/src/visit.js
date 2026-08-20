"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.visit = visit;
exports.visitServices = visitServices;
const utils_1 = require("./utils");
const sourceInfo_1 = require("./sourceInfo");
const case_1 = require("./case");
function visit(proto, sourceInfo, messageFn, options, enumFn = () => { }, tsPrefix = "", protoPrefix = "") {
    const isRootFile = "syntax" in proto;
    const childEnumType = isRootFile ? sourceInfo_1.Fields.file.enum_type : sourceInfo_1.Fields.message.enum_type;
    proto.enumType.forEach((enumDesc, index) => {
        // I.e. Foo_Bar.Zaz_Inner
        const protoFullName = protoPrefix + enumDesc.name;
        // I.e. FooBar_ZazInner
        const tsFullName = tsPrefix + (0, case_1.maybeSnakeToCamel)(enumDesc.name, options);
        const tsFullNameWithAffixes = messageName((0, utils_1.wrapTypeName)(options, tsFullName));
        const nestedSourceInfo = sourceInfo.open(childEnumType, index);
        enumFn(tsFullNameWithAffixes, enumDesc, nestedSourceInfo, protoFullName);
    });
    const messages = "messageType" in proto ? proto.messageType : proto.nestedType;
    const childType = isRootFile ? sourceInfo_1.Fields.file.message_type : sourceInfo_1.Fields.message.nested_type;
    messages.forEach((message, index) => {
        // I.e. Foo_Bar.Zaz_Inner
        const protoFullName = protoPrefix + message.name;
        // I.e. FooBar_ZazInner
        const tsFullName = tsPrefix + (0, case_1.maybeSnakeToCamel)(message.name, options);
        const tsFullNameWithAffixes = messageName((0, utils_1.wrapTypeName)(options, tsFullName));
        const nestedSourceInfo = sourceInfo.open(childType, index);
        messageFn(tsFullNameWithAffixes, message, nestedSourceInfo, protoFullName);
        const delim = options.useSnakeTypeName ? "_" : "";
        visit(message, nestedSourceInfo, messageFn, options, enumFn, tsFullName + delim, protoFullName + ".");
    });
}
const builtInNames = ["Date", "Function"];
/** Potentially suffixes `Message` to names to avoid conflicts, i.e. with `Date`. */
function messageName(name) {
    return builtInNames.includes(name) ? `${name}Message` : name;
}
function visitServices(proto, sourceInfo, serviceFn) {
    proto.service.forEach((serviceDesc, index) => {
        const nestedSourceInfo = sourceInfo.open(sourceInfo_1.Fields.file.service, index);
        serviceFn(serviceDesc, nestedSourceInfo);
    });
}
