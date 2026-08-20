"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.maybeSnakeToCamel = maybeSnakeToCamel;
exports.snakeToCamel = snakeToCamel;
exports.camelToSnake = camelToSnake;
exports.capitalize = capitalize;
exports.uncapitalize = uncapitalize;
exports.camelCaseGrpc = camelCaseGrpc;
const case_anything_1 = require("case-anything");
/** Converts `key` to TS/JS camel-case idiom, unless overridden not to. */
function maybeSnakeToCamel(key, options) {
    if (options.snakeToCamel.includes("keys") && key.includes("_")) {
        return snakeToCamel(key);
    }
    else {
        return key;
    }
}
function snakeToCamel(s) {
    const hasLowerCase = !!s.match(/[a-z]/);
    return s
        .split("_")
        .map((word, i) => {
        // If the word is already mixed case, leave the existing case as-is
        word = hasLowerCase ? word : word.toLowerCase();
        return i === 0 ? word : capitalize(word);
    })
        .join("");
}
function camelToSnake(s) {
    return (s
        // any number or little-char -> big char
        .replace(/[a-z0-9]([A-Z])/g, (m) => m[0] + "_" + m[1])
        // any multiple big char -> next word
        .replace(/[A-Z]([A-Z][a-z])/g, (m) => m[0] + "_" + m.substring(1))
        .toUpperCase());
}
function capitalize(s) {
    return s.substring(0, 1).toUpperCase() + s.substring(1);
}
function uncapitalize(s) {
    return s.substring(0, 1).toLowerCase() + s.substring(1);
}
/* This function uses the exact same semantics found inside the grpc
 * nodejs library. Camel case splitting must be done by word i.e
 * GetAPIValue must become getApiValue (notice the API becomes Api).
 * This needs to be followed otherwise it will not succeed in the grpc nodejs module.
 */
function camelCaseGrpc(s) {
    return (0, case_anything_1.camelCase)(s);
}
