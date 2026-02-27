package context

import (
	"celer/pkgs/dirs"
	"fmt"
	"maps"
	"os"
	"path/filepath"
	"regexp"
	"strings"
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
	e.vars = make(map[string]string)

	e.vars["BUILDTREES_DIR"] = dirs.BuildtreesDir
	e.vars["INSTALLED_DIR"] = e.toRelPath(ctx.InstalledDir())
	e.vars["INSTALLED_DEV_DIR"] = e.toRelPath(ctx.InstalledDevDir())
}

// Put stores or updates an expression variable.
func (e *ExprVars) Put(key, value string) {
	e.vars[key] = value
}

// Expand replace express with values.
func (e ExprVars) Expand(content string) string {
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

func (e ExprVars) toRelPath(absPath string) string {
	relativePath, err := filepath.Rel(dirs.WorkspaceDir, absPath)
	if err != nil {
		return filepath.ToSlash(absPath)
	}
	return filepath.ToSlash(filepath.Join("${WORKSPACE_ROOT}", relativePath))
}
