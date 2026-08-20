"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const index_1 = require("./index");
// When loaded from a CDN, most of them will use the standalone version of prettier because it has
// `"browser": "./standalone.js"` (and also "unpkg") in it's package.json. These tests verify basic functionality in
// that case. This is also important for Deno support, since using a CDN is the default method to import dependencies.
jest.mock("prettier", () => jest.requireActual("prettier/standalone"));
describe("standalone", () => {
    it("works with prettier/standalone", async () => {
        const b = (0, index_1.code) `1 +    1`;
        expect(b.toString()).toMatchInlineSnapshot(`
      "1 + 1;
      "
    `);
    });
    it("supports jsx", async () => {
        const b = (0, index_1.code) `
      const a = <div
       class="test">    Test</div>;
    `;
        expect(b.toString()).toMatchInlineSnapshot(`
      "const a = <div class="test">Test</div>;
      "
    `);
    });
    it("test", async () => {
        let b = (0, index_1.code) `${(0, index_1.imp)("Simple@./ui")}`;
        expect(b.toString({ path: "ui/simple.ts" })).toContain('"../ui"');
        b = (0, index_1.code) `${(0, index_1.imp)("Simpleton@./another/path/to/ui")}`;
        expect(b.toString({ path: "another/path/to/ui/simple.ts" })).toContain('"../ui"');
    });
});
