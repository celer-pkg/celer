package context

import (
	"fmt"
	"maps"
	"os"
	"regexp"
	"strings"

	"github.com/celer-pkg/celer/pkgs/dirs"
)

type ExprVars struct {
	vars map[string]string
}

func (e ExprVars) Clone() ExprVars {
	cloned := ExprVars{vars: make(map[string]string, len(e.vars))}
	maps.Copy(cloned.vars, e.vars)
	return cloned
}

// Init initialize Variables with values from the context.
func (e *ExprVars) Init(ctx Context) {
	if e.vars == nil {
		e.vars = make(map[string]string)
	}

	e.vars["BUILDTREES_DIR"] = dirs.BuildtreesDir
	e.vars["INSTALLED_DIR"] = "${INSTALLED_DIR}"
	e.vars["INSTALLED_DEV_DIR"] = "${INSTALLED_DEV_DIR}"
}

// Put stores or updates an expression variable.
func (e *ExprVars) Put(key, value string) {
	if e.vars == nil {
		e.vars = make(map[string]string)
	}

	if value != "" {
		e.vars[key] = value
	}
}

// Expand replace express with values.
func (e ExprVars) Expand(content string) string {
	if e.vars == nil {
		e.vars = make(map[string]string)
	}

	for key, value := range e.vars {
		content = strings.ReplaceAll(content, fmt.Sprintf("${%s}", key), value)
		content = strings.ReplaceAll(content, fmt.Sprintf("$%s", key), value)
		content = e.replaceEnvVars(content)
	}

	return content
}

// replaceEnvVars replace env express with env value.
func (e ExprVars) replaceEnvVars(content string) string {
	content = strings.ReplaceAll(content, "~", os.Getenv("HOME"))

	reg := regexp.MustCompile(`\$ENV\{([^}]+)\}`)
	return reg.ReplaceAllStringFunc(content, func(match string) string {
		varName := reg.FindStringSubmatch(match)[1]
		if value, ok := os.LookupEnv(varName); ok {
			return value
		}
		return match
	})
}
