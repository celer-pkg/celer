"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.createFileContext = createFileContext;
function createFileContext(file) {
    return { isProto3Syntax: file.syntax === "proto3" };
}
