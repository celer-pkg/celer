"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.generateSchema = generateSchema;
const ts_proto_descriptors_1 = require("ts-proto-descriptors");
const ts_poet_1 = require("ts-poet");
const visit_1 = require("./visit");
const utils_1 = require("./utils");
const types_1 = require("./types");
const wire_1 = require("@bufbuild/protobuf/wire");
const options_1 = require("./options");
const fileDescriptorProto = (0, ts_poet_1.imp)("FileDescriptorProto@ts-proto-descriptors");
const extensionCache = {};
function generateSchema(ctx, fileDesc, sourceInfo) {
    const { options } = ctx;
    const chunks = [];
    fileDesc.extension.forEach((extension) => {
        if (!(extension.extendee in extensionCache)) {
            extensionCache[extension.extendee] = {};
        }
        extensionCache[extension.extendee][extension.number] = extension;
    });
    const outputSchemaOptions = ctx.options.outputSchema ? ctx.options.outputSchema : [];
    const outputFileDescriptor = !outputSchemaOptions.includes(options_1.OutputSchemaOption.NO_FILE_DESCRIPTOR);
    const outputAsConst = outputSchemaOptions.includes(options_1.OutputSchemaOption.CONST);
    chunks.push((0, ts_poet_1.code) `
    type ProtoMetaMessageOptions = {
      options?: { [key: string]: any };
      fields?: { [key: string]: { [key: string]: any } };
      oneof?: { [key: string]: { [key: string]: any } };
      nested?: { [key: string]: ProtoMetaMessageOptions };
    };

    export interface ProtoMetadata {
      ${outputFileDescriptor ? (0, ts_poet_1.code) `fileDescriptor: ${fileDescriptorProto};\n` : ""}references: { [key: string]: any };
      dependencies?: ProtoMetadata[];
      options?: {
        options?: { [key: string]: any };
        services?: {
          [key: string]: {
            options?: { [key: string]: any };
            methods?: { [key: string]: { [key: string]: any } };
          }
        };
        messages?: {
          [key: string]: ProtoMetaMessageOptions;
        };
        enums?: {
          [key: string]: {
            options?: { [key: string]: any };
            values?: { [key: string]: { [key: string]: any } };
          };
        };
      };
    }
  `);
    const references = [];
    function addReference(localName, symbol) {
        references.push((0, ts_poet_1.code) `'.${(0, utils_1.maybePrefixPackage)(fileDesc, localName.replace(/_/g, "."))}': ${symbol}`);
    }
    (0, visit_1.visit)(fileDesc, sourceInfo, (fullName) => {
        if (options.outputEncodeMethods) {
            addReference(fullName, fullName);
        }
    }, options, (fullName) => {
        addReference(fullName, fullName);
    });
    (0, visit_1.visitServices)(fileDesc, sourceInfo, (serviceDesc) => {
        if (options.outputClientImpl) {
            addReference(serviceDesc.name, `${serviceDesc.name}ClientImpl`);
        }
    });
    const dependencies = fileDesc.dependency.map((dep) => {
        return (0, ts_poet_1.code) `${(0, utils_1.impFile)(options, `protoMetadata@./${dep.replace(".proto", "")}`)}`;
    });
    // Use toObject so that we get enums as numbers (instead of the default toJSON behavior)
    const descriptor = ts_proto_descriptors_1.FileDescriptorProto.fromPartial(fileDesc);
    // Only keep locations that include comments
    descriptor.sourceCodeInfo = {
        location: descriptor.sourceCodeInfo?.location.filter((loc) => loc["leadingComments"] || loc["trailingComments"]) || [],
    };
    let fileOptions;
    if (fileDesc.options) {
        fileOptions = encodedOptionsToOptions(ctx, ".google.protobuf.FileOptions", fileDesc.options._unknownFields);
        delete fileDesc.options._unknownFields;
    }
    const messagesOptions = [];
    (fileDesc.messageType || []).forEach((message) => {
        const resolvedMessage = resolveMessageOptions(ctx, message);
        if (resolvedMessage) {
            messagesOptions.push(resolvedMessage);
        }
    });
    const servicesOptions = [];
    (fileDesc.service || []).forEach((service) => {
        const methodsOptions = [];
        service.method.forEach((method) => {
            if (method.options) {
                const methodOptions = encodedOptionsToOptions(ctx, ".google.protobuf.MethodOptions", method.options._unknownFields);
                delete method.options._unknownFields;
                if (methodOptions) {
                    methodsOptions.push((0, ts_poet_1.code) `'${method.name}': ${methodOptions}`);
                }
            }
        });
        let serviceOptions;
        if (service.options) {
            serviceOptions = encodedOptionsToOptions(ctx, ".google.protobuf.ServiceOptions", service.options._unknownFields);
            delete service.options._unknownFields;
        }
        if (methodsOptions.length > 0 || serviceOptions) {
            servicesOptions.push((0, ts_poet_1.code) `
        '${service.name}': {
          ${serviceOptions ? (0, ts_poet_1.code) `options: ${serviceOptions},` : ""}
          methods: {${(0, ts_poet_1.joinCode)(methodsOptions, { on: "," })}}
        }
      `);
        }
    });
    const enumsOptions = [];
    (fileDesc.enumType || []).forEach((Enum) => {
        const valuesOptions = [];
        Enum.value.forEach((value) => {
            if (value.options) {
                const valueOptions = encodedOptionsToOptions(ctx, ".google.protobuf.EnumValueOptions", value.options._unknownFields);
                delete value.options._unknownFields;
                if (valueOptions) {
                    valuesOptions.push((0, ts_poet_1.code) `'${value.name}': ${valueOptions}`);
                }
            }
        });
        let enumOptions;
        if (Enum.options) {
            enumOptions = encodedOptionsToOptions(ctx, ".google.protobuf.EnumOptions", Enum.options._unknownFields);
            delete Enum.options._unknownFields;
        }
        if (valuesOptions.length > 0 || enumOptions) {
            enumsOptions.push((0, ts_poet_1.code) `
        '${Enum.name}': {
          ${enumOptions ? (0, ts_poet_1.code) `options: ${enumOptions},` : ""}
          values: {${(0, ts_poet_1.joinCode)(valuesOptions, { on: "," })}}
        }
      `);
        }
    });
    chunks.push((0, ts_poet_1.code) `
    export const ${(0, ts_poet_1.def)("protoMetadata")}${outputAsConst ? "" : ": ProtoMetadata"} = {
      ${outputFileDescriptor ? (0, ts_poet_1.code) `fileDescriptor: ${descriptor},\n` : ""}references: { ${(0, ts_poet_1.joinCode)(references, {
        on: ",",
    })} },
      dependencies: [${(0, ts_poet_1.joinCode)(dependencies, { on: "," })}],
      ${fileOptions || messagesOptions.length > 0 || servicesOptions.length > 0 || enumsOptions.length > 0
        ? (0, ts_poet_1.code) `options: {
          ${fileOptions ? (0, ts_poet_1.code) `options: ${fileOptions},` : ""}
          ${messagesOptions.length > 0 ? (0, ts_poet_1.code) `messages: {${(0, ts_poet_1.joinCode)(messagesOptions, { on: "," })}},` : ""}
          ${servicesOptions.length > 0 ? (0, ts_poet_1.code) `services: {${(0, ts_poet_1.joinCode)(servicesOptions, { on: "," })}},` : ""}
          ${enumsOptions.length > 0 ? (0, ts_poet_1.code) `enums: {${(0, ts_poet_1.joinCode)(enumsOptions, { on: "," })}}` : ""}
        }`
        : ""}
    }${outputAsConst ? " as const satisfies ProtoMetadata" : ""}
  `);
    return chunks;
}
function getExtensionValue(ctx, extension, data) {
    if (extension.type == ts_proto_descriptors_1.FieldDescriptorProto_Type.TYPE_MESSAGE) {
        const typeName = (0, types_1.basicTypeName)(ctx, extension);
        const resultBuffer = Buffer.concat(data.map((d) => {
            // Skip length byte
            const bytes = new wire_1.BinaryReader(d).bytes();
            return Buffer.from(bytes);
        }));
        const result = resultBuffer.toString("base64");
        const encoded = ctx.options.env === options_1.EnvOption.NODE
            ? (0, ts_poet_1.code) `Buffer.from('${result}', 'base64')`
            : (0, ts_poet_1.code) `${ctx.utils.bytesFromBase64}("${result}")`;
        return (0, ts_poet_1.code) `'${extension.name}': ${typeName}.decode(${encoded})`;
    }
    else {
        const reader = new wire_1.BinaryReader(data[0]);
        let value = reader[(0, types_1.toReaderCall)(extension)]();
        if (typeof value === "string") {
            value = (0, ts_poet_1.code) `"${value}"`;
        }
        return (0, ts_poet_1.code) `'${extension.name}': ${value}`;
    }
}
/** Takes the protoc's input of options as proto-encoded messages, and turns them into embedded-able-in-source-code representations. */
function encodedOptionsToOptions(ctx, extendee, encodedOptions) {
    if (!encodedOptions) {
        return undefined;
    }
    const resultOptions = [];
    for (const key in encodedOptions) {
        const value = encodedOptions[key];
        const extension = extensionCache[extendee]?.[parseInt(key, 10) >>> 3];
        if (extension && shouldAddOptionDefinition(ctx, extension)) {
            // todo: we should be able to create an option definition ALWAYS, however,
            // we currently cannot do that because the if the extension is a sub-message
            // (and thus, not just a straightforward value), we don't have an JSON object
            // representation of the option - just it's encoded value. Our approach in
            // getExtensionValue is to decode the encoded value into a message, but this
            // Message.decode(...) method is not always included in the generated code.
            // We should fix this so that we can always create an option definition by
            // somehow preemptively decoding the encoded value and then inserting it into
            // the option definition.
            // please refer to integration/options-types-only to see this in action
            resultOptions.push(getExtensionValue(ctx, extension, value));
        }
    }
    if (resultOptions.length == 0) {
        return undefined;
    }
    return (0, ts_poet_1.code) `{${(0, ts_poet_1.joinCode)(resultOptions, { on: "," })}}`;
}
function shouldAddOptionDefinition(ctx, extension) {
    return (extension.type !== ts_proto_descriptors_1.FieldDescriptorProto_Type.TYPE_MESSAGE ||
        ctx.options.outputEncodeMethods === true ||
        ctx.options.outputEncodeMethods == "decode-only");
}
function resolveMessageOptions(ctx, message) {
    const fieldsOptions = [];
    message.field.forEach((field) => {
        if (field.options) {
            const fieldOptions = encodedOptionsToOptions(ctx, ".google.protobuf.FieldOptions", field.options._unknownFields);
            delete field.options._unknownFields;
            if (fieldOptions) {
                fieldsOptions.push((0, ts_poet_1.code) `'${field.name}': ${fieldOptions}`);
            }
        }
    });
    const oneOfsOptions = [];
    message.oneofDecl.forEach((oneOf) => {
        if (oneOf.options) {
            const oneOfOptions = encodedOptionsToOptions(ctx, ".google.protobuf.OneofOptions", oneOf.options._unknownFields);
            delete oneOf.options._unknownFields;
            if (oneOfOptions) {
                oneOfsOptions.push((0, ts_poet_1.code) `'${oneOf.name}': ${oneOfOptions}`);
            }
        }
    });
    let nestedOptions = [];
    if (message.nestedType && message.nestedType.length > 0) {
        message.nestedType.forEach((nested) => {
            const resolvedMessage = resolveMessageOptions(ctx, nested);
            if (resolvedMessage) {
                nestedOptions.push(resolvedMessage);
            }
        });
    }
    let messageOptions;
    if (message.options) {
        messageOptions = encodedOptionsToOptions(ctx, ".google.protobuf.MessageOptions", message.options._unknownFields);
        delete message.options._unknownFields;
    }
    if (fieldsOptions.length > 0 || oneOfsOptions.length > 0 || nestedOptions.length > 0 || messageOptions) {
        return (0, ts_poet_1.code) `
      '${message.name}': {
        ${messageOptions ? (0, ts_poet_1.code) `options: ${messageOptions},` : ""}
        ${fieldsOptions.length > 0 ? (0, ts_poet_1.code) `fields: {${(0, ts_poet_1.joinCode)(fieldsOptions, { on: "," })}},` : ""}
        ${oneOfsOptions.length > 0 ? (0, ts_poet_1.code) `oneof: {${(0, ts_poet_1.joinCode)(oneOfsOptions, { on: "," })}},` : ""}
        ${nestedOptions.length > 0 ? (0, ts_poet_1.code) `nested: {${(0, ts_poet_1.joinCode)(nestedOptions, { on: "," })}},` : ""}
      }
    `;
    }
}
