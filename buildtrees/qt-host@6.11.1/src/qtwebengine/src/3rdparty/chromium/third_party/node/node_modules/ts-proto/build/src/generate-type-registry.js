"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.generateTypeRegistry = generateTypeRegistry;
const ts_poet_1 = require("ts-poet");
const options_1 = require("./options");
function generateTypeRegistry(ctx) {
    const chunks = [];
    chunks.push(generateMessageType(ctx));
    if ((0, options_1.addTypeToMessages)(ctx.options)) {
        chunks.push((0, ts_poet_1.code) `
    export type UnknownMessage = {$type: string};
  `);
    }
    else {
        chunks.push((0, ts_poet_1.code) `
    export type UnknownMessage = unknown;
  `);
    }
    chunks.push((0, ts_poet_1.code) `
    export const messageTypeRegistry = new Map<string, MessageType>();
  `);
    chunks.push((0, ts_poet_1.code) ` ${ctx.utils.Builtin.ifUsed} ${ctx.utils.DeepPartial.ifUsed}`);
    return (0, ts_poet_1.joinCode)(chunks, { on: "\n\n" });
}
function generateMessageType(ctx) {
    const chunks = [];
    chunks.push((0, ts_poet_1.code) `export interface MessageType<Message extends UnknownMessage = UnknownMessage> {`);
    if ((0, options_1.addTypeToMessages)(ctx.options)) {
        chunks.push((0, ts_poet_1.code) `$type: Message['$type'];`);
    }
    else {
        chunks.push((0, ts_poet_1.code) `$type: string;`);
    }
    if (ctx.options.outputEncodeMethods) {
        const BinaryReader = (0, ts_poet_1.imp)("BinaryReader@@bufbuild/protobuf/wire");
        const BinaryWriter = (0, ts_poet_1.imp)("BinaryWriter@@bufbuild/protobuf/wire");
        chunks.push((0, ts_poet_1.code) `encode(message: Message, writer?: ${BinaryWriter}): ${BinaryWriter};`);
        chunks.push((0, ts_poet_1.code) `decode(input: ${BinaryReader} | Uint8Array, length?: number): Message;`);
    }
    if (ctx.options.outputJsonMethods) {
        chunks.push((0, ts_poet_1.code) `fromJSON(object: any): Message;`);
        chunks.push((0, ts_poet_1.code) `toJSON(message: Message): unknown;`);
    }
    if (ctx.options.outputPartialMethods) {
        chunks.push((0, ts_poet_1.code) `fromPartial(object: ${ctx.utils.DeepPartial}<Message>): Message;`);
    }
    chunks.push((0, ts_poet_1.code) `}`);
    return (0, ts_poet_1.joinCode)(chunks, { on: "\n" });
}
