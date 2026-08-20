const selectorParser = require("postcss-selector-parser");
const valueParser = require("postcss-value-parser");

const selectorProcessor = selectorParser((selectors) => {
  selectors.walk((selector) => {
    selector.spaces = { before: '', after: '' }
    if (selector.raws && selector.raws.spaces) {
      selector.raws.spaces = {}
    }
  })
})

function minifySelector(str) {
  return selectorProcessor.processSync(str)
}

function minifyValue(str) {
  const parsed = valueParser(str.trim())
  parsed.walk((node) => {
    if (node.before) node.before = ""
    if (node.after) node.after = ""
    if (node.type === "space") node.value = " "
  })
  return parsed.toString()
}

module.exports = () => {
  return {
    postcssPlugin: "postcss-minify",

    AtRule: (atrule) => {
      atrule.raws = { before: "", after: "", afterName: " " }
      atrule.params = minifyValue(atrule.params)
    },

    Comment: (comment) => {
      if (comment.text[0] === '!') {
        comment.raws.before = ""
        comment.raws.after = ""
      } else {
        comment.remove()
      }
    },

    Declaration: (decl) => {
      decl.raws = { before: "", between: ":" }
      decl.value = minifyValue(decl.value);
    },

    Rule: (rule) => {
      rule.raws = { before: "", between: "", after: "", semicolon: false }
      rule.selector = minifySelector(rule.selector)
    }
  }
}

module.exports.postcss = true
